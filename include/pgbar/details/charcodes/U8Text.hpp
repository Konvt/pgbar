#ifndef PGBAR__U8TEXT
#define PGBAR__U8TEXT

#include "U8Raw.hpp"
#include <numeric>
#include <vector>

namespace pgbar {
  namespace _details {
    namespace charcodes {
      // A UTF-8 string that supports splitting strings by character width.
      class U8Text : public U8Raw {
        using Self = U8Text;

      protected:
        std::vector<Glyph> chars_;

      public:
        struct TextView {
          const types::Char *head_, *tail_;
          types::Size width_;

          constexpr TextView() noexcept : head_ { nullptr }, tail_ { nullptr }, width_ { 0 } {}
          constexpr TextView( const types::Char* head, const types::Char* tail, types::Size width ) noexcept
            : head_ { head }, tail_ { tail }, width_ { width }
          {}

          TextView( std::nullptr_t, std::nullptr_t, types::Size )     = delete;
          TextView( const types::Char*, std::nullptr_t, types::Size ) = delete;
          TextView( std::nullptr_t, const types::Char*, types::Size ) = delete;

          PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr bool empty() const noexcept { return head_ == tail_; }
          constexpr explicit operator bool() const noexcept { return !empty(); }
        };

        PGBAR__NODISCARD static PGBAR__CXX20_CNSTXPR std::vector<Glyph> parse_glyph(
          const types::Char* raw_u8_str,
          types::Size str_length )
        {
          std::vector<Glyph> characters;
          for ( types::Size i = 0; i < str_length; ) {
            const auto startpoint = raw_u8_str + i;
            PGBAR__TRUST( startpoint >= raw_u8_str );
            auto resolved = U8Raw::next_codepoint( startpoint, str_length - i );
            characters.emplace_back( i, U8Raw::glyph_width( resolved.first ) );
            i += resolved.second;
          }
          return characters;
        }

        PGBAR__CXX20_CNSTXPR U8Text() = default;
        PGBAR__CXX20_CNSTXPR explicit U8Text( types::String u8_bytes )
        {
          chars_ = parse_glyph( u8_bytes.data(), u8_bytes.size() );
          width_ = std::accumulate( chars_.cbegin(),
                                    chars_.cend(),
                                    types::Size {},
                                    []( types::Size acc, const Glyph& ch ) noexcept {
                                      return acc + static_cast<types::Size>( ch.width_ );
                                    } );
          bytes_ = std::move( u8_bytes );
        }
        PGBAR__CXX20_CNSTXPR U8Text( const Self& )            = default;
        PGBAR__CXX20_CNSTXPR U8Text( Self&& )                 = default;
        PGBAR__CXX20_CNSTXPR Self& operator=( const Self& ) & = default;
        PGBAR__CXX20_CNSTXPR Self& operator=( Self&& ) &      = default;
        PGBAR__CXX20_CNSTXPR ~U8Text()                        = default;

        PGBAR__CXX20_CNSTXPR Self& operator=( types::ROStr u8_bytes ) &
        {
          auto new_chars = parse_glyph( u8_bytes.data(), u8_bytes.size() );
          auto new_bytes = types::String( u8_bytes );
          chars_.swap( new_chars );
          width_ = std::accumulate( chars_.cbegin(),
                                    chars_.cend(),
                                    types::Size {},
                                    []( types::Size acc, const Glyph& ch ) noexcept {
                                      return acc + static_cast<types::Size>( ch.width_ );
                                    } );
          bytes_.swap( new_bytes );
          return *this;
        }
        PGBAR__CXX20_CNSTXPR Self& operator=( types::String u8_bytes ) &
        {
          auto new_chars = parse_glyph( u8_bytes.data(), u8_bytes.size() );
          chars_.swap( new_chars );
          width_ = std::accumulate( chars_.cbegin(),
                                    chars_.cend(),
                                    types::Size {},
                                    []( types::Size acc, const Glyph& ch ) noexcept {
                                      return acc + static_cast<types::Size>( ch.width_ );
                                    } );
          bytes_.swap( u8_bytes );
          return *this;
        }

        PGBAR__CXX20_CNSTXPR void clear()
          noexcept( noexcept( std::declval<U8Raw&>().clear() ) && noexcept( chars_.clear() ) )
        {
          U8Raw::clear();
          chars_.clear();
        }
        PGBAR__CXX20_CNSTXPR void shrink_to_fit()
          noexcept( noexcept( std::declval<U8Raw&>().shrink_to_fit() ) && noexcept( chars_.shrink_to_fit() ) )
        {
          U8Raw::shrink_to_fit();
          chars_.shrink_to_fit();
        }

        /**
         * @brief Split a string into two parts based on the given width, with UTF-8 characters as the unit.
         * @param width The given width.
         * @return The split result and the width of each part.
         */
        PGBAR__NODISCARD PGBAR__FORCEINLINE std::pair<TextView, TextView> split_by(
          types::Size width ) const noexcept
        {
          if ( bytes_.empty() )
            PGBAR__UNLIKELY return {};

          // split_pos is the starting point of the right part
          types::Size split_pos  = 0;
          types::Size left_width = 0;
          while ( split_pos < chars_.size() && left_width + chars_[split_pos].width_ <= width )
            left_width += chars_[split_pos++].width_;

          const auto split_loc =
            bytes_.data() + ( split_pos < chars_.size() ? chars_[split_pos].offset_ : bytes_.size() );
          return {
            { bytes_.data(), split_loc,                     left_width          },
            { split_loc,     bytes_.data() + bytes_.size(), width_ - left_width }
          };
        }

        PGBAR__CXX20_CNSTXPR void swap( Self& lhs ) noexcept
        {
          U8Raw::swap( lhs );
          chars_.swap( lhs.chars_ );
        }
        PGBAR__CXX20_CNSTXPR friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

#if PGBAR__CXX20
        PGBAR__CXX20_CNSTXPR explicit U8Text( types::LitU8 u8_sv ) : U8Text()
        {
          auto new_bytes = types::String( u8_sv.size(), '\0' );
          std::copy( u8_sv.cbegin(), u8_sv.cend(), new_bytes.begin() );
          chars_ = parse_glyph( new_bytes.data(), new_bytes.size() );
          width_ = std::accumulate( chars_.cbegin(),
                                    chars_.cend(),
                                    types::Size {},
                                    []( types::Size acc, const Glyph& ch ) noexcept {
                                      return acc + static_cast<types::Size>( ch.width_ );
                                    } );
          bytes_ = std::move( new_bytes );
        }
#endif
      };
    } // namespace charcodes
  } // namespace _details
} // namespace pgbar

#endif
