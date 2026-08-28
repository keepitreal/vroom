#!/usr/bin/env ruby
# frozen_string_literal: true

# Asserts that react-native-vroom-chart.podspec emits no absolute filesystem
# paths.
#
# CocoaPods serializes a path-based pod's evaluated spec to
# `Pods/Local Podspecs/<name>.podspec.json` and hashes that file into the
# consumer's Podfile.lock SPEC CHECKSUMS. Any absolute path in the spec encodes
# the checkout root of whoever ran `pod install`, so every other machine
# computes a different checksum and `pod install --deployment` refuses to build.
# The invariant: nothing machine-specific reaches the emitted spec.
#
# Runs without CocoaPods by stubbing the Pod::Spec DSL.

require "json"
require "pathname"

PODSPEC = Pathname.new(__dir__).join("..", "react-native-vroom-chart.podspec").cleanpath
POD_DIR = PODSPEC.dirname

module Pod
  # Collects `s.<attr> = value` assignments and ignores the rest of the DSL.
  class Spec
    attr_reader :attrs

    def self.new
      spec = allocate
      spec.instance_variable_set(:@attrs, {})
      yield spec
      spec
    end

    def method_missing(name, *args)
      key = name.to_s
      @attrs[key.chomp("=")] = args.first if key.end_with?("=")
      nil
    end

    def respond_to_missing?(_name, _include_private = false)
      true
    end
  end
end

# The podspec resolves Skia with a bare `node --print require.resolve(...)`,
# which is relative to the process's working directory. Pin it to the pod
# directory so the check is deterministic.
spec = Dir.chdir(POD_DIR) { eval(PODSPEC.read, TOPLEVEL_BINDING, PODSPEC.to_s) } # rubocop:disable Security/Eval

# xcconfig values are space-separated, optionally-quoted path lists; source_files
# and friends are plain globs. Tokenizing on whitespace covers both, and a
# leading "/" is what makes a token machine-specific. URLs (https://...) and
# relative globs (ios/**/*.h) start with something else.
def absolute_tokens(value, path = "")
  case value
  when String
    value.split(/\s+/)
         .map { |token| token.delete('"') }
         .select { |token| token.start_with?("/") }
         .map { |token| "#{path}: #{token}" }
  when Hash
    value.flat_map { |k, v| absolute_tokens(v, path.empty? ? k.to_s : "#{path}.#{k}") }
  when Array
    value.flat_map { |v| absolute_tokens(v, path) }
  else
    []
  end
end

offenders = absolute_tokens(spec.attrs)

unless offenders.empty?
  warn "Absolute paths found in the generated podspec:"
  offenders.each { |o| warn "  #{o}" }
  warn ""
  warn "These make the consumer's SPEC CHECKSUMS entry machine-specific and break"
  warn "`pod install --deployment`. Emit them relative to $(PODS_TARGET_SRCROOT)."
  exit 1
end

# A relative path that points nowhere is just as broken as an absolute one, so
# resolve the search paths back against the pod directory and confirm they exist.
search_paths = spec.attrs.dig("pod_target_xcconfig", "HEADER_SEARCH_PATHS").to_s
missing = search_paths.split(/\s+/).map { |t| t.delete('"') }.reject(&:empty?).filter_map do |token|
  next unless token.start_with?("$(PODS_TARGET_SRCROOT)")

  resolved = POD_DIR.join(token.sub("$(PODS_TARGET_SRCROOT)/", "")).cleanpath
  token unless resolved.directory?
end

unless missing.empty?
  warn "HEADER_SEARCH_PATHS entries do not resolve to a real directory:"
  missing.each { |m| warn "  #{m}" }
  exit 1
end

puts "podspec is portable: no absolute paths, #{search_paths.scan("$(PODS_TARGET_SRCROOT)").size} search paths resolve"
