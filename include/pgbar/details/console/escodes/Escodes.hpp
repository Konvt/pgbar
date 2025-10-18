#ifndef PGBAR__COLOR
#define PGBAR__COLOR

#include "../../../color/Color.hpp"
#include "../../io/Stringbuf.hpp"
#if PGBAR__CXX17
# include <charconv>
#endif

namespace pgbar {
  namespace _details {
    namespace console {
      namespace escodes {
#ifdef PGBAR_NOCOLOR
        PGBAR__CXX17_INLINE constexpr types::LitU8 fontreset = u8"";
        PGBAR__CXX17_INLINE constexpr types::LitU8 fontbold  = u8"";
#else
        PGBAR__CXX17_INLINE constexpr types::LitU8 fontreset = u8"\x1B[0m";
        PGBAR__CXX17_INLINE constexpr types::LitU8 fontbold  = u8"\x1B[1m";
#endif
        PGBAR__CXX17_INLINE constexpr types::LitU8 savecursor  = u8"\x1B[s";
        PGBAR__CXX17_INLINE constexpr types::LitU8 resetcursor = u8"\x1B[u";
        PGBAR__CXX17_INLINE constexpr types::LitU8 linewipe    = u8"\x1B[K";
        PGBAR__CXX17_INLINE constexpr types::LitU8 prevline    = u8"\x1b[A";
        PGBAR__CXX17_INLINE constexpr types::Char nextline     = '\n';
        PGBAR__CXX17_INLINE constexpr types::Char linestart    = '\r';

        class RGBColor {
          using Self = RGBColor;

          std::array<types::Char, 17> sgr_; // Select Graphic Rendition
          std::uint8_t length_;

          static PGBAR__CXX23_CNSTXPR types::Char* to_char( types::Char* first,
                                                            types::Char* last,
                                                            std::uint8_t value ) noexcept
          {
#if PGBAR__CXX17
            auto result = std::to_chars( first, last, value );
            PGBAR__TRUST( result.ec == std::errc() );
            return result.ptr;
#else
            types::Size offset = 1;
            if ( value >= 100 ) {
              PGBAR__TRUST( last - first >= 3 );
              first[0] = '0' + value / 100;
              first[1] = '0' + ( value / 10 % 10 );
              first[2] = '0' + ( value % 10 );
              offset += 2;
            } else if ( value >= 10 ) {
              PGBAR__TRUST( last - first >= 2 );
              first[0] = '0' + value / 10;
              first[1] = '0' + ( value % 10 );
              offset += 1;
            } else
              first[0] = '0' + value;
            PGBAR__TRUST( last - first >= 1 );
            return first + offset;
#endif
          }

          PGBAR__CXX23_CNSTXPR void from_hex( types::HexRGB hex_val ) & noexcept
          {
#ifndef PGBAR_NOCOLOR
            length_ = 1;
            if ( hex_val == PGBAR__DEFAULT ) {
              sgr_[0] = '0';
              return;
            }

            length_ = 2;
            sgr_[0] = '3';
            switch ( hex_val & 0x00FFFFFF ) { // discard the high 8 bits
            case PGBAR__BLACK:   sgr_[1] = '0'; break;
            case PGBAR__RED:     sgr_[1] = '1'; break;
            case PGBAR__GREEN:   sgr_[1] = '2'; break;
            case PGBAR__YELLOW:  sgr_[1] = '3'; break;
            case PGBAR__BLUE:    sgr_[1] = '4'; break;
            case PGBAR__MAGENTA: sgr_[1] = '5'; break;
            case PGBAR__CYAN:    sgr_[1] = '6'; break;
            case PGBAR__WHITE:   sgr_[1] = '7'; break;
            default:             {
              sgr_[1] = '8', sgr_[2] = ';', sgr_[3] = '2', sgr_[4] = ';';
              auto tail = to_char( sgr_.data() + 5, sgr_.data() + sgr_.size(), ( hex_val >> 16 ) & 0xFF );
              *tail     = ';';
              tail      = to_char( tail + 1, sgr_.data() + sgr_.size(), ( hex_val >> 8 ) & 0xFF );
              *tail     = ';';
              tail      = to_char( tail + 1, sgr_.data() + sgr_.size(), hex_val & 0xFF );
              length_   = static_cast<std::uint8_t>( tail - sgr_.data() );
            } break;
            }
#endif
          }
          PGBAR__CXX23_CNSTXPR void from_str( types::ROStr hex_str ) &
          {
            if ( ( hex_str.size() != 7 && hex_str.size() != 4 ) || hex_str.front() != '#' )
              throw exception::InvalidArgument( "pgbar: invalid hex color format" );

            for ( types::Size i = 1; i < hex_str.size(); i++ ) {
              if ( ( hex_str[i] < '0' || hex_str[i] > '9' ) && ( hex_str[i] < 'A' || hex_str[i] > 'F' )
                   && ( hex_str[i] < 'a' || hex_str[i] > 'f' ) )
                throw exception::InvalidArgument( "pgbar: invalid hexadecimal letter" );
            }

#ifndef PGBAR_NOCOLOR
            std::uint32_t hex_val = 0;
            if ( hex_str.size() == 4 ) {
              for ( types::Size i = 1; i < hex_str.size(); ++i ) {
                hex_val <<= 4;
                if ( hex_str[i] >= '0' && hex_str[i] <= '9' )
                  hex_val = ( ( hex_val | ( hex_str[i] - '0' ) ) << 4 ) | ( hex_str[i] - '0' );
                else if ( hex_str[i] >= 'A' && hex_str[i] <= 'F' )
                  hex_val = ( ( hex_val | ( hex_str[i] - 'A' + 10 ) ) << 4 ) | ( hex_str[i] - 'A' + 10 );
                else // no need to check whether it's valid or not
                  hex_val = ( ( hex_val | ( hex_str[i] - 'a' + 10 ) ) << 4 ) | ( hex_str[i] - 'a' + 10 );
              }
            } else {
              for ( types::Size i = 1; i < hex_str.size(); ++i ) {
                hex_val <<= 4;
                if ( hex_str[i] >= '0' && hex_str[i] <= '9' )
                  hex_val |= hex_str[i] - '0';
                else if ( hex_str[i] >= 'A' && hex_str[i] <= 'F' )
                  hex_val |= hex_str[i] - 'A' + 10;
                else
                  hex_val |= hex_str[i] - 'a' + 10;
              }
            }
            from_hex( hex_val );
#endif
          }

        public:
          PGBAR__CXX23_CNSTXPR RGBColor() noexcept { clear(); }

          PGBAR__CXX23_CNSTXPR RGBColor( types::HexRGB hex_val ) noexcept : RGBColor()
          {
            from_hex( hex_val );
          }
          PGBAR__CXX23_CNSTXPR RGBColor( types::ROStr hex_str ) : RGBColor() { from_str( hex_str ); }

          PGBAR__CXX23_CNSTXPR RGBColor( const Self& lhs ) noexcept = default;
          PGBAR__CXX23_CNSTXPR Self& operator=( const Self& lhs ) & = default;

          PGBAR__CXX23_CNSTXPR Self& operator=( types::HexRGB hex_val ) & noexcept
          {
            from_hex( hex_val );
            return *this;
          }
          PGBAR__CXX23_CNSTXPR Self& operator=( types::ROStr hex_str ) &
          {
            from_str( hex_str );
            return *this;
          }

          PGBAR__INLINE_FN PGBAR__CXX23_CNSTXPR void clear() noexcept
          {
            std::fill( sgr_.begin(), sgr_.end(), '\0' );
            length_ = 0;
          }

          PGBAR__CXX23_CNSTXPR void swap( RGBColor& lhs ) noexcept
          {
            std::swap( sgr_, lhs.sgr_ );
            std::swap( length_, lhs.length_ );
          }
          PGBAR__CXX23_CNSTXPR friend void swap( RGBColor& a, RGBColor& b ) noexcept { a.swap( b ); }

          PGBAR__INLINE_FN friend io::Stringbuf& operator<<( io::Stringbuf& buf, const Self& col )
          {
#ifndef PGBAR_NOCOLOR
            buf.append( '\x1B' )
              .append( '[' )
              .append( col.sgr_.data(), col.sgr_.data() + col.length_ )
              .append( 'm' );
#endif
            return buf;
          }
        };
      } // namespace escodes
    } // namespace console
  } // namespace _details
} // namespace pgbar

#endif
