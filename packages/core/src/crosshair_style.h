// Crosshair style resolution, time-badge text and simple-mode geometry.
//
// Skia-free and header-only so the unit tests can cover it; see
// tests/test_crosshair_style.cpp. Drawing lives in crosshair.cpp.

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ctime>

#include "vroom/vroom_chart.h"

namespace vroom::crosshair_style {

inline constexpr float kDefaultSimpleHeight = 1.f;
inline constexpr float kDefaultDimOpacity = 0.35f;

// The style with every sentinel replaced by its mode's default.
inline VroomCrosshairStyle resolve(const VroomCrosshairStyle& in) {
    VroomCrosshairStyle s = in;
    if (s.mode < VROOM_CROSSHAIR_ADVANCED || s.mode > VROOM_CROSSHAIR_NONE) {
        s.mode = VROOM_CROSSHAIR_ADVANCED;
    }
    const bool simple = s.mode == VROOM_CROSSHAIR_SIMPLE;
    if (s.line_style < 0 || s.line_style > VROOM_CROSSHAIR_LINE_SOLID) {
        s.line_style = simple ? VROOM_CROSSHAIR_LINE_SOLID : VROOM_CROSSHAIR_LINE_DASHED;
    }
    if (!(s.line_width_px > 0.f)) s.line_width_px = 1.f;
    if (s.time_format < 0 || s.time_format > VROOM_CROSSHAIR_TIME_DATE) {
        s.time_format = simple ? VROOM_CROSSHAIR_TIME_TIME : VROOM_CROSSHAIR_TIME_DATETIME;
    }
    s.simple_height = s.simple_height < 0.f ? kDefaultSimpleHeight
                                            : std::min(s.simple_height, 1.f);
    s.dim_opacity = s.dim_opacity < 0.f ? kDefaultDimOpacity
                                        : std::min(s.dim_opacity, 1.f);
    return s;
}

// Badge text for a broken-down local time. `datetime` on a 24h clock is the
// crosshair's original `MM/DD HH:MM`.
inline void format_tm(const std::tm& t, int32_t format, bool hour12, char* buf,
                      size_t len) {
    static const char* const kMonths[12] = {"Jan", "Feb", "Mar", "Apr",
                                            "May", "Jun", "Jul", "Aug",
                                            "Sep", "Oct", "Nov", "Dec"};
    if (format == VROOM_CROSSHAIR_TIME_DATE) {
        std::snprintf(buf, len, "%s %d", kMonths[std::clamp(t.tm_mon, 0, 11)],
                      t.tm_mday);
        return;
    }
    char clock[16];
    if (hour12) {
        const int h = t.tm_hour % 12 == 0 ? 12 : t.tm_hour % 12;
        std::snprintf(clock, sizeof(clock), "%d:%02d %s", h, t.tm_min,
                      t.tm_hour < 12 ? "AM" : "PM");
    } else {
        std::snprintf(clock, sizeof(clock), "%02d:%02d", t.tm_hour, t.tm_min);
    }
    if (format == VROOM_CROSSHAIR_TIME_TIME) {
        std::snprintf(buf, len, "%s", clock);
    } else {
        std::snprintf(buf, len, "%02d/%02d %s", t.tm_mon + 1, t.tm_mday, clock);
    }
}

inline void format_time(int64_t time_ms, int32_t format, bool hour12, char* buf,
                        size_t len) {
    const time_t time_s = static_cast<time_t>(time_ms / 1000);
    std::tm t{};
    localtime_r(&time_s, &t);
    format_tm(t, format, hour12, buf, len);
}

struct SimpleGeometry {
    float badge_top;  // the time badge's top edge
    float line_top;   // where the vertical line starts (the badge's bottom)
};

// Simple mode: the badge's top sits at `height_frac` of `chart_h` (1 = top,
// 0 = bottom), clamped so the badge stays on screen and above `line_bottom`.
inline SimpleGeometry simple_geometry(float chart_h, float line_bottom,
                                      float badge_h, float height_frac) {
    const float frac = std::clamp(height_frac, 0.f, 1.f);
    const float max_top = std::max(0.f, std::min(chart_h, line_bottom) - badge_h);
    const float top = std::clamp((1.f - frac) * chart_h, 0.f, max_top);
    return SimpleGeometry{top, std::min(top + badge_h, std::max(line_bottom, 0.f))};
}

}  // namespace vroom::crosshair_style
