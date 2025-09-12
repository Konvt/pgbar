#ifndef __PGBAR_CODECHART
#define __PGBAR_CODECHART

#include "../types/Types.hpp"

namespace pgbar {
  namespace __details {
    namespace charcodes {
      // A type of wrapper that stores the mapping between Unicode code chart and character width.
      class CodeChart final {
      public:
        // At present, the width of each utf character will not exceed 3.
        using RenderWidth = std::uint8_t;

      private:
        types::UCodePoint start_, end_;
        RenderWidth width_;

      public:
        constexpr CodeChart( types::UCodePoint start, types::UCodePoint end, RenderWidth width ) noexcept
          : start_ { start }, end_ { end }, width_ { width }
        {         // This is an internal component, so we assume the arguments are always valid.
#if __PGBAR_CXX14 // C++11 requires the constexpr ctor should have an empty function body.
          __PGBAR_TRUST( start_ <= end_ );
#endif
        }
        constexpr CodeChart( const CodeChart& )                          = default;
        __PGBAR_CXX14_CNSTXPR CodeChart& operator=( const CodeChart& ) & = default;
        __PGBAR_CXX20_CNSTXPR ~CodeChart()                               = default;

        // Check whether the Unicode code point is within this code chart.
        __PGBAR_NODISCARD constexpr bool contains( types::UCodePoint codepoint ) const noexcept
        {
          return start_ <= codepoint && codepoint <= end_;
        }
        // Return the character width of this Unicode code chart.
        __PGBAR_NODISCARD constexpr RenderWidth width() const noexcept { return width_; }
        // Return the size of this range of Unicode code chart.
        __PGBAR_NODISCARD constexpr types::UCodePoint size() const noexcept { return end_ - start_ + 1; }
        // Return the start Unicode code point of this code chart.
        __PGBAR_NODISCARD constexpr types::UCodePoint head() const noexcept { return start_; }
        // Return the end Unicode code point of this code chart.
        __PGBAR_NODISCARD constexpr types::UCodePoint tail() const noexcept { return end_; }

        __PGBAR_NODISCARD friend constexpr bool operator<( const CodeChart& a, const CodeChart& b ) noexcept
        {
          return a.end_ < b.start_;
        }
        __PGBAR_NODISCARD friend constexpr bool operator>( const CodeChart& a, const CodeChart& b ) noexcept
        {
          return a.start_ > b.end_;
        }
        __PGBAR_NODISCARD friend constexpr bool operator>( const CodeChart& a,
                                                           const types::UCodePoint& b ) noexcept
        {
          return a.start_ > b;
        }
        __PGBAR_NODISCARD friend constexpr bool operator<( const CodeChart& a,
                                                           const types::UCodePoint& b ) noexcept
        {
          return a.end_ < b;
        }
      };
    } // namespace charcodes
  } // namespace __details
} // namespace pgbar

#endif
