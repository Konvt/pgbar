#ifndef PGBAR_COLOR
#define PGBAR_COLOR

#include "../details/types/Types.hpp"

#define PGBAR__DEFAULT 0xC105EA11 // C1O5E -> ClOSE, A11 -> All
#define PGBAR__BLACK   0x000000
#define PGBAR__RED     0xFF0000
#define PGBAR__GREEN   0x00FF00
#define PGBAR__YELLOW  0xFFFF00
#define PGBAR__BLUE    0x0000FF
#define PGBAR__MAGENTA 0x800080
#define PGBAR__CYAN    0x00FFFF
#define PGBAR__WHITE   0xFFFFFF

namespace pgbar {
  namespace color {
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB None    = PGBAR__DEFAULT;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Black   = PGBAR__BLACK;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Red     = PGBAR__RED;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Green   = PGBAR__GREEN;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Yellow  = PGBAR__YELLOW;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Blue    = PGBAR__BLUE;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Magenta = PGBAR__MAGENTA;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB Cyan    = PGBAR__CYAN;
    PGBAR__CXX17_INLINE constexpr _details::types::HexRGB White   = PGBAR__WHITE;
  } // namespace color
} // namespace pgbar

#endif
