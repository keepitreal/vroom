# Changesets

This folder is managed by [Changesets](https://github.com/changesets/changesets).

To record a change for the next release, run:

```sh
pnpm changeset
```

Pick the affected packages (`react-native-vroom-chart`, `@vroomchart/core-wasm`,
`@vroomchart/react`) and a semver bump, and write a short summary. Commit the
generated markdown file. On merge to `main`, the release workflow opens/updates a
"Version Packages" PR; merging that PR publishes the bumped packages to npm.

`@vroomchart/types` is bundled into the published packages (not published), so it's
ignored here.
