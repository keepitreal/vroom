#include "annotations.h"

#include <algorithm>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/effects/SkDashPathEffect.h"
#pragma clang diagnostic pop

#include "annotations_layout.h"
#include "chart.h"
#include "fonts.h"

namespace vroom::annotations {

namespace {

constexpr SkScalar kDotted[2] = {2.f, 2.f};
constexpr SkScalar kDashed[2] = {6.f, 4.f};

sk_sp<SkPathEffect> dash_for(int32_t dash) {
    if (dash == 1) return SkDashPathEffect::Make(kDotted, 0.f);
    if (dash == 2) return SkDashPathEffect::Make(kDashed, 0.f);
    return nullptr;
}

float x_of(const VroomChart& chart, const Layout& lay, int64_t window_ms,
           int64_t time_ms) {
    return vroom::x_at_time(lay, chart.visible_start_ms, window_ms, time_ms);
}

}  // namespace

void draw_boxes(SkCanvas* canvas,
                const VroomChart& chart,
                const Layout& lay,
                const PriceBounds& bounds,
                int64_t window_ms,
                float candle_right,
                float candle_area_h) {
    if (!canvas || chart.annotation_boxes.empty() || candle_right <= 0.f ||
        candle_area_h <= 0.f) {
        return;
    }
    SkPaint fill;
    fill.setAntiAlias(true);
    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));
    for (const VroomAnnotationBox& b : chart.annotation_boxes) {
        const float l = x_of(chart, lay, window_ms, b.a.time_ms);
        const float r = x_of(chart, lay, window_ms, b.b.time_ms);
        if (!overlaps_plot(l, r, candle_right)) continue;
        SkRect rect = SkRect::MakeLTRB(l, vroom::price_to_y(lay, bounds, b.a.price), r,
                                       vroom::price_to_y(lay, bounds, b.b.price));
        rect.sort();
        fill.setColor(b.fill);
        canvas->drawRect(rect, fill);
        if (SkColorGetA(b.border) != 0) {
            border.setColor(b.border);
            border.setStrokeWidth(b.border_width > 0.f ? b.border_width : 1.f);
            canvas->drawRect(rect, border);
        }
    }
    canvas->restore();
}

void draw_marks(SkCanvas* canvas,
                const VroomChart& chart,
                const Layout& lay,
                const PriceBounds& bounds,
                int64_t window_ms,
                float candle_right,
                float candle_area_h) {
    if (!canvas || candle_right <= 0.f || candle_area_h <= 0.f) return;
    if (chart.annotation_segments.empty() && chart.annotation_markers.empty() &&
        chart.annotation_labels.empty()) {
        return;
    }
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0.f, 0.f, candle_right, candle_area_h));

    SkPaint stroke;
    stroke.setAntiAlias(true);
    stroke.setStyle(SkPaint::kStroke_Style);
    for (const VroomAnnotationSegment& s : chart.annotation_segments) {
        const float x0 = x_of(chart, lay, window_ms, s.a.time_ms);
        const float x1 = x_of(chart, lay, window_ms, s.b.time_ms);
        if (!overlaps_plot(x0, x1, candle_right)) continue;
        stroke.setColor(s.color);
        stroke.setStrokeWidth(s.width > 0.f ? s.width : 1.f);
        stroke.setPathEffect(dash_for(s.dash));
        canvas->drawLine(x0, vroom::price_to_y(lay, bounds, s.a.price), x1,
                         vroom::price_to_y(lay, bounds, s.b.price), stroke);
    }
    stroke.setPathEffect(nullptr);

    // Markers sit on their bar's slot center.
    const int64_t half_bar = chart.candle_duration_ms / 2;
    SkPaint fill;
    fill.setAntiAlias(true);
    for (const VroomAnnotationMarker& m : chart.annotation_markers) {
        const float size = marker_size(m.size_px);
        const float cx = x_of(chart, lay, window_ms, m.time_ms + half_bar);
        if (!overlaps_plot(cx - size, cx + size, candle_right)) continue;
        const float cy = vroom::price_to_y(lay, bounds, m.price) + m.offset_px;
        const float h = size * 0.5f;
        fill.setColor(m.color);
        switch (m.shape) {
            case VROOM_ANNOTATION_TRIANGLE_UP:
            case VROOM_ANNOTATION_TRIANGLE_DOWN: {
                const auto pts = triangle(m.shape, cx, cy, size);
                SkPathBuilder path;
                path.moveTo(pts[0].x, pts[0].y);
                path.lineTo(pts[1].x, pts[1].y);
                path.lineTo(pts[2].x, pts[2].y);
                path.close();
                canvas->drawPath(path.detach(), fill);
                break;
            }
            case VROOM_ANNOTATION_CROSS:
                stroke.setColor(m.color);
                stroke.setStrokeWidth(std::max(1.5f, size * 0.2f));
                stroke.setStrokeCap(SkPaint::kRound_Cap);
                canvas->drawLine(cx - h, cy - h, cx + h, cy + h, stroke);
                canvas->drawLine(cx - h, cy + h, cx + h, cy - h, stroke);
                stroke.setStrokeCap(SkPaint::kButt_Cap);
                break;
            case VROOM_ANNOTATION_SQUARE: {
                const float q = h * 0.8f;
                canvas->drawRect(SkRect::MakeLTRB(cx - q, cy - q, cx + q, cy + q), fill);
                break;
            }
            default:
                canvas->drawCircle(cx, cy, h, fill);
                break;
        }
    }

    auto tf = vroom::typeface(chart);
    if (tf && !chart.annotation_labels.empty()) {
        SkFont font(tf, chart.theme.floats[VROOM_FLOAT_AXIS_FONT_SIZE_PX]);
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint text;
        text.setAntiAlias(true);
        SkPaint bg;
        bg.setAntiAlias(true);
        for (const VroomChart::StoredAnnotationLabel& l : chart.annotation_labels) {
            if (l.text.empty()) continue;
            SkRect tb;
            const float w = font.measureText(l.text.data(), l.text.size(),
                                             SkTextEncoding::kUTF8, &tb);
            const float left =
                label_left(l.align, x_of(chart, lay, window_ms, l.time_ms), w);
            if (!overlaps_plot(left - kLabelPadX, left + w + kLabelPadX, candle_right)) {
                continue;
            }
            const float cy = vroom::price_to_y(lay, bounds, l.price) + l.offset_px;
            const float baseline = cy - (tb.fTop + tb.fBottom) * 0.5f;
            if (SkColorGetA(l.background) != 0) {
                const float half_h = (tb.fBottom - tb.fTop) * 0.5f + kLabelPadY;
                bg.setColor(l.background);
                canvas->drawRRect(
                    SkRRect::MakeRectXY(SkRect::MakeLTRB(left - kLabelPadX, cy - half_h,
                                                         left + w + kLabelPadX, cy + half_h),
                                        3.f, 3.f),
                    bg);
            }
            text.setColor(l.color);
            canvas->drawString(l.text.c_str(), left, baseline, font, text);
        }
    }
    canvas->restore();
}

}  // namespace vroom::annotations
