#ifndef __PGBAR_U8RAW
#define __PGBAR_U8RAW

#include "../../exception/Error.hpp"
#include "CodeChart.hpp"
#include <algorithm>
#include <array>
#include <utility>

namespace pgbar {
  namespace __details {
    namespace charcodes {
      // A simple UTF-8 string implementation, but it does not provide specific utf-8 codec operations.
      class U8Raw {
        using Self = U8Raw;

      protected:
        types::Size width_;
        types::String bytes_;

        /// @return The utf codepoint and the number of byte of the utf-8 character.
        static __PGBAR_CXX20_CNSTXPR std::pair<types::UCodePoint, types::Size> next_char(
          const types::Char* raw_u8_str,
          types::Size length )
        {
          // After RFC 3629, the maximum length of each standard UTF-8 character is 4 bytes.
          const auto first_byte = static_cast<types::UCodePoint>( static_cast<std::uint8_t>( *raw_u8_str ) );
          auto validator        = [raw_u8_str, length]( types::Size expected_len ) {
            if ( expected_len > length )
              __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: incomplete UTF-8 string" );

            for ( types::Size i = 1; i < expected_len; ++i ) {
              if ( ( raw_u8_str[i] & 0xC0 ) != 0x80 )
                __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: broken UTF-8 character" );
            }
            return expected_len;
          };

          if ( ( first_byte & 0x80 ) == 0 )
            return std::make_pair( first_byte, 1 );
          else if ( ( ( first_byte & 0xE0 ) == 0xC0 ) )
            return std::make_pair( ( ( first_byte & 0x1F ) << 6 )
                                     | ( static_cast<types::UCodePoint>( raw_u8_str[1] ) & 0x3F ),
                                   validator( 2 ) );
          else if ( ( first_byte & 0xF0 ) == 0xE0 )
            return std::make_pair( ( ( first_byte & 0xF ) << 12 )
                                     | ( ( static_cast<types::UCodePoint>( raw_u8_str[1] ) & 0x3F ) << 6 )
                                     | ( static_cast<types::UCodePoint>( raw_u8_str[2] ) & 0x3F ),
                                   validator( 3 ) );
          else if ( ( first_byte & 0xF8 ) == 0xF0 )
            return std::make_pair( ( ( first_byte & 0x7 ) << 18 )
                                     | ( ( static_cast<types::UCodePoint>( raw_u8_str[1] ) & 0x3F ) << 12 )
                                     | ( ( static_cast<types::UCodePoint>( raw_u8_str[2] ) & 0x3F ) << 6 )
                                     | ( static_cast<types::UCodePoint>( raw_u8_str[3] ) & 0x3F ),
                                   validator( 4 ) );
          else
            __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: not a standard UTF-8 string" );
        }

      public:
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN __PGBAR_CNSTEVAL std::array<CodeChart, 47>
          code_charts() noexcept
        {
          // See the Unicode CodeCharts documentation for complete code points.
          // Also can see the `if-else` version in misc/UTF-8-test.cpp
          return {
            { { 0x0, 0x19, 0 },        { 0x20, 0x7E, 1 },        { 0x7F, 0xA0, 0 },
             { 0xA1, 0xAC, 1 },       { 0xAD, 0xAD, 0 },        { 0xAE, 0x2FF, 1 },
             { 0x300, 0x36F, 0 },     { 0x370, 0x1FFF, 1 },     { 0x2000, 0x200F, 0 },
             { 0x2010, 0x2010, 1 },   { 0x2011, 0x2011, 0 },    { 0x2012, 0x2027, 1 },
             { 0x2028, 0x202F, 0 },   { 0x2030, 0x205E, 1 },    { 0x205F, 0x206F, 0 },
             { 0x2070, 0x2E7F, 1 },   { 0x2E80, 0xA4CF, 2 },    { 0xA4D0, 0xA95F, 1 },
             { 0xA960, 0xA97F, 2 },   { 0xA980, 0xABFF, 1 },    { 0xAC00, 0xD7FF, 2 },
             { 0xE000, 0xF8FF, 2 },   { 0xF900, 0xFAFF, 2 },    { 0xFB00, 0xFDCF, 1 },
             { 0xFDD0, 0xFDEF, 0 },   { 0xFDF0, 0xFDFF, 1 },    { 0xFE00, 0xFE0F, 0 },
             { 0xFE10, 0xFE1F, 2 },   { 0xFE20, 0xFE2F, 0 },    { 0xFE30, 0xFE6F, 2 },
             { 0xFE70, 0xFEFE, 1 },   { 0xFEFF, 0xFEFF, 0 },    { 0xFF00, 0xFF60, 2 },
             { 0xFF61, 0xFFDF, 1 },   { 0xFFE0, 0xFFE6, 2 },    { 0xFFE7, 0xFFEF, 1 },
             { 0xFFF0, 0xFFFF, 1 },   { 0x10000, 0x1F8FF, 2 },  { 0x1F900, 0x1FBFF, 3 },
             { 0x1FF80, 0x1FFFF, 0 }, { 0x20000, 0x3FFFD, 2 },  { 0x3FFFE, 0x3FFFF, 0 },
             { 0xE0000, 0xE007F, 0 }, { 0xE0100, 0xE01EF, 0 },  { 0xEFF80, 0xEFFFF, 0 },
             { 0xFFF80, 0xFFFFF, 2 }, { 0x10FF80, 0x10FFFF, 2 } }
          };
        }
        __PGBAR_NODISCARD static __PGBAR_CXX20_CNSTXPR CodeChart::RenderWidth char_width(
          types::UCodePoint codepoint ) noexcept
        {
          constexpr const auto charts = code_charts();
          __PGBAR_ASSERT( std::is_sorted( charts.cbegin(), charts.cend() ) );
          // Compare with the `if-else` version, here we can search for code points with O(logn).
          const auto itr = std::lower_bound( charts.cbegin(), charts.cend(), codepoint );
          if ( itr != charts.cend() && itr->contains( codepoint ) )
            return itr->width();

          return 1; // Default fallback
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the parameter `u8_str` isn't a valid UTF-8 string.
         *
         * @return Returns the render width of the given string.
         */
        __PGBAR_NODISCARD static __PGBAR_CXX20_CNSTXPR types::Size render_width( types::ROStr u8_str )
        {
          types::Size width     = 0;
          const auto raw_u8_str = u8_str.data();
          for ( types::Size i = 0; i < u8_str.size(); ) {
            const auto startpoint = raw_u8_str + i;
            __PGBAR_TRUST( startpoint >= raw_u8_str );
            auto parsed = next_char( startpoint, std::distance( startpoint, u8_str.data() + u8_str.size() ) );
            width += static_cast<types::Size>( char_width( parsed.first ) );
            i += parsed.second;
          }
          return width;
        }

        __PGBAR_CXX20_CNSTXPR U8Raw() noexcept( std::is_nothrow_default_constructible<types::String>::value )
          : width_ { 0 }
        {}
        __PGBAR_CXX20_CNSTXPR explicit U8Raw( types::String u8_bytes ) : U8Raw()
        {
          width_ = render_width( u8_bytes );
          bytes_ = std::move( u8_bytes );
        }
        __PGBAR_CXX20_CNSTXPR U8Raw( const Self& )             = default;
        __PGBAR_CXX20_CNSTXPR U8Raw( Self&& )                  = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& ) & = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR ~U8Raw()                         = default;

        __PGBAR_CXX20_CNSTXPR Self& operator=( types::ROStr u8_bytes ) &
        {
          const auto new_width = render_width( u8_bytes );
          auto new_bytes       = types::String( u8_bytes );
          bytes_.swap( new_bytes );
          width_ = new_width;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR Self& operator=( types::String u8_bytes ) &
        {
          width_ = render_width( u8_bytes );
          bytes_.swap( u8_bytes );
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR bool empty() const noexcept { return bytes_.empty(); }
        __PGBAR_CXX20_CNSTXPR types::Size size() const noexcept { return bytes_.size(); }
        __PGBAR_CXX20_CNSTXPR types::Size width() const noexcept { return width_; }
        __PGBAR_CXX20_CNSTXPR types::ROStr str() & noexcept { return bytes_; }
        __PGBAR_CXX20_CNSTXPR types::ROStr str() const& noexcept { return bytes_; }
        __PGBAR_CXX20_CNSTXPR types::String&& str() && noexcept { return std::move( bytes_ ); }

        __PGBAR_CXX20_CNSTXPR void clear() noexcept( noexcept( bytes_.clear() ) )
        {
          bytes_.clear();
          width_ = 0;
        }
        __PGBAR_CXX20_CNSTXPR void shrink_to_fit() noexcept( noexcept( bytes_.shrink_to_fit() ) )
        { // The standard does not seem to specify whether the function is noexcept,
          // so let's make a judgment here.
          // At least I didn't see it on cppreference.
          bytes_.shrink_to_fit();
        }

        __PGBAR_CXX20_CNSTXPR void swap( Self& lhs ) noexcept
        {
          std::swap( width_, lhs.width_ );
          bytes_.swap( lhs.bytes_ );
        }
        __PGBAR_CXX20_CNSTXPR friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        __PGBAR_CXX20_CNSTXPR explicit operator types::String() & { return bytes_; }
        __PGBAR_CXX20_CNSTXPR explicit operator types::String() const& { return bytes_; }
        __PGBAR_CXX20_CNSTXPR explicit operator types::String&&() && noexcept { return std::move( bytes_ ); }
        __PGBAR_CXX20_CNSTXPR operator types::ROStr() const noexcept { return str(); }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          Self&& a,
          const Self& b )
        {
          return std::move( a.bytes_ ) + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const Self& a,
          const Self& b )
        {
          return a.bytes_ + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          types::String&& a,
          const Self& b )
        {
          return std::move( a ) + b.bytes_;
        }
        template<types::Size N>
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const char ( &a )[N],
          const Self& b )
        {
          return a + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const char* a,
          const Self& b )
        {
          return a + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          types::ROStr a,
          const Self& b )
        {
          return types::String( a ) + b;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          Self&& a,
          types::ROStr b )
        {
          a.bytes_.append( b );
          return std::move( a.bytes_ );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const Self& a,
          types::ROStr b )
        {
          return a.bytes_ + types::String( b );
        }

#if __PGBAR_CXX20
        static_assert( sizeof( char8_t ) == sizeof( char ),
                       "pgbar::__details::chaset::U8Raw: Unexpected type size mismatch" );

        __PGBAR_CXX20_CNSTXPR explicit U8Raw( types::LitU8 u8_sv ) : U8Raw()
        {
          auto new_bytes = types::String( u8_sv.size(), '\0' );
          std::copy( u8_sv.cbegin(), u8_sv.cend(), new_bytes.begin() );
          width_ = render_width( new_bytes );
          bytes_ = std::move( new_bytes );
        }

        __PGBAR_CXX20_CNSTXPR explicit operator std::u8string() const
        {
          std::u8string ret;
          ret.resize( bytes_.size() );
          std::copy( bytes_.cbegin(), bytes_.cend(), ret.begin() );
          return ret;
        }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          types::LitU8 a,
          const Self& b )
        {
          types::String tmp;
          tmp.reserve( a.size() );
          std::copy( a.cbegin(), a.cend(), std::back_inserter( tmp ) );
          return std::move( tmp ) + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN types::String operator+( Self&& a, types::LitU8 b )
        {
          a.bytes_.reserve( a.bytes_.size() + b.size() );
          std::copy( b.cbegin(), b.cend(), std::back_inserter( a.bytes_ ) );
          return std::move( a.bytes_ );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN types::String operator+( const Self& a, types::LitU8 b )
        {
          auto tmp = a.bytes_;
          tmp.reserve( a.bytes_.size() + b.size() );
          std::copy( b.cbegin(), b.cend(), std::back_inserter( tmp ) );
          return tmp;
        }
#endif
      };
    } // namespace charcodes
  } // namespace __details
} // namespace pgbar

#endif
