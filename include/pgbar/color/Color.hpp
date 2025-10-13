#ifndef PGBAR_COLOR
#define PGBAR_COLOR

#include "../details/types/Types.hpp"

#define __PGBAR_DEFAULT 0xC105EA11 // C1O5E -> ClOSE, A11 -> All
#define __PGBAR_BLACK   0x000000
#define __PGBAR_RED     0xFF0000
#define __PGBAR_GREEN   0x00FF00
#define __PGBAR_YELLOW  0xFFFF00
#define __PGBAR_BLUE    0x0000FF
#define __PGBAR_MAGENTA 0x800080
#define __PGBAR_CYAN    0x00FFFF
#define __PGBAR_WHITE   0xFFFFFF

namespace pgbar {
  namespace color {
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB None    = __PGBAR_DEFAULT;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Black   = __PGBAR_BLACK;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Red     = __PGBAR_RED;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Green   = __PGBAR_GREEN;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Yellow  = __PGBAR_YELLOW;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Blue    = __PGBAR_BLUE;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Magenta = __PGBAR_MAGENTA;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB Cyan    = __PGBAR_CYAN;
    __PGBAR_CXX17_INLINE constexpr __details::types::HexRGB White   = __PGBAR_WHITE;
  } // namespace color
} // namespace pgbar

#endif
