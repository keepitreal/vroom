#include "price_format.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace vroom {

namespace {

// Room for %.*f of any finite double at kPriceMaxDecimals: sign, the 309
// integer digits DBL_MAX can reach, the point, and the decimals.
constexpr size_t kPlainCap = 340;

constexpr char kGroupSeparator = ',';
constexpr int kGroupSize = 3;

}  // namespace

int significant_decimals(double reference) {
    const double r = std::fabs(reference);
    if (!(r > 0.0) || !std::isfinite(r)) return kPriceMinDecimals;
    // floor(log10) is the exponent of the leading digit, so subtracting it from
    // the digit budget lands the last significant digit on the final decimal.
    const int exponent = static_cast<int>(std::floor(std::log10(r)));
    return std::clamp(kPriceSigDigits - 1 - exponent, kPriceMinDecimals,
                      kPriceMaxDecimals);
}

PriceFormat price_format_for(double reference) {
    return PriceFormat{significant_decimals(reference), true};
}

int price_decimals(double interval) {
    if (!(interval > 0.0) || !std::isfinite(interval)) return kPriceMinDecimals;
    const double d = std::ceil(-std::log10(interval) - 1e-12);
    if (d < 0.0) return 0;
    if (d > static_cast<double>(kPriceMaxDecimals)) return kPriceMaxDecimals;
    return static_cast<int>(d);
}

PriceFormat with_tick_guard(const PriceFormat& fmt, double interval) {
    return PriceFormat{std::max(fmt.decimals, price_decimals(interval)),
                       fmt.group};
}

void format_price(char* buf, size_t buf_size, double price,
                  const PriceFormat& fmt) {
    if (!buf || buf_size == 0) return;
    const int decimals = std::clamp(fmt.decimals, 0, kPriceMaxDecimals);

    char plain[kPlainCap];
    if (std::snprintf(plain, sizeof(plain), "%.*f", decimals, price) < 0) {
        buf[0] = '\0';
        return;
    }
    if (!fmt.group) {
        std::snprintf(buf, buf_size, "%s", plain);
        return;
    }

    const char* src = plain;
    size_t out = 0;
    // Leaves room for the terminator on every write, so the result truncates
    // the way snprintf would rather than running off the end.
    const auto put = [&](char c) {
        if (out + 1 < buf_size) buf[out++] = c;
    };

    if (*src == '-' || *src == '+') put(*src++);

    // Digits up to the point, or the whole run when decimals == 0. A non-finite
    // price has none, and falls through to the tail copy as "inf" / "nan".
    const char* int_end = src;
    while (*int_end >= '0' && *int_end <= '9') ++int_end;

    const ptrdiff_t digits = int_end - src;
    for (ptrdiff_t i = 0; i < digits; ++i) {
        if (i > 0 && (digits - i) % kGroupSize == 0) put(kGroupSeparator);
        put(src[i]);
    }
    for (const char* p = int_end; *p != '\0'; ++p) put(*p);

    buf[out] = '\0';
}

}  // namespace vroom
