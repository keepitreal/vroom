#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#pragma clang diagnostic pop

namespace vroom {

// Returns the platform system font manager, or nullptr if unavailable.
sk_sp<SkFontMgr> makePlatformFontMgr();

}  // namespace vroom
