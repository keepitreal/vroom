#include "VroomFontMgr.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#elif defined(__ANDROID__)
#include "include/ports/SkFontMgr_android.h"
#include "include/ports/SkFontScanner_FreeType.h"
#endif
#pragma clang diagnostic pop

namespace vroom {

sk_sp<SkFontMgr> makePlatformFontMgr() {
#if defined(__APPLE__)
  return SkFontMgr_New_CoreText(nullptr);
#elif defined(__ANDROID__)
  return SkFontMgr_New_Android(nullptr, SkFontScanner_Make_FreeType());
#else
  return nullptr;
#endif
}

}  // namespace vroom
