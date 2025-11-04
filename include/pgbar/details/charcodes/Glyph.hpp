#ifndef PGBAR__GLYPH
#define PGBAR__GLYPH

#include "../types/Types.hpp"

namespace pgbar {
  namespace _details {
    namespace charcodes {
      // Represents a displayable character cell on the rendered surface.
      // `offset_` is the byte position of this character in the original encoded buffer,
      // not its visual index on screen.
      struct Glyph {
        // At present, the width of each utf character will not exceed 3.
        using RenderWidth = std::uint8_t;

        // The starting offset (in byte) and the rendered width (in character) of the encoded character.
        types::Size offset_;
        RenderWidth width_;

        constexpr Glyph( types::Size offset, RenderWidth width ) noexcept
          : offset_ { offset }, width_ { width }
        {}
      };
    } // namespace charcodes
  } // namespace _details
} // namespace pgbar

#endif
