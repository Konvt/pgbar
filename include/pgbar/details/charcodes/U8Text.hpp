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
        // The starting offset (in byte) and the rendered width (in character) of each utf-8 character.
        std::vector<std::pair<types::Size, CodeChart::RenderWidth>> chars_;

      public:
        /// @return The starting offset and the rendered width of each character.
        PGBAR__NODISCARD static PGBAR__CXX20_CNSTXPR
          std::vector<std::pair<types::Size, CodeChart::RenderWidth>>
          parse_char( types::ROStr u8_str )
        {
          std::vector<std::pair<types::Size, CodeChart::RenderWidth>> characters;
          const auto raw_u8_str = u8_str.data();
          for ( types::Size i = 0; i < u8_str.size(); ) {
            const auto startpoint = raw_u8_str + i;
            PGBAR__TRUST( startpoint >= raw_u8_str );
            auto resolved = U8Raw::next_char( startpoint, ( u8_str.data() + u8_str.size() ) - startpoint );
            characters.emplace_back( i, U8Raw::char_width( resolved.first ) );
            i += resolved.second;
          }
          return characters;
        }

        PGBAR__CXX20_CNSTXPR U8Text() noexcept( std::is_nothrow_default_constructible<U8Raw>::value )
          : U8Raw()
        {}
        PGBAR__CXX20_CNSTXPR explicit U8Text( types::String u8_bytes ) : U8Text()
        {
          chars_ = parse_char( u8_bytes );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
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
          auto new_chars = parse_char( u8_bytes );
          auto new_bytes = types::String( u8_bytes );
          chars_.swap( new_chars );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
            } );
          bytes_.swap( new_bytes );
          return *this;
        }
        PGBAR__CXX20_CNSTXPR Self& operator=( types::String u8_bytes ) &
        {
          auto new_chars = parse_char( u8_bytes );
          std::swap( chars_, new_chars );
          chars_.swap( new_chars );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
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
         * @return The split result and the width of each part, where the first element is the result not
         * exceeding the width.
         */
        PGBAR__INLINE_FN std::pair<std::array<const types::Char*, 3>, std::pair<types::Size, types::Size>>
          split_by( types::Size width ) const noexcept
        {
          if ( bytes_.empty() )
            PGBAR__UNLIKELY return {
              { nullptr, nullptr, nullptr },
              std::make_pair( 0, 0 )
            };

          // split_pos is the starting point of the right part
          types::Size split_pos  = 0;
          types::Size left_width = 0;
          while ( left_width + chars_[split_pos].second <= width && split_pos < chars_.size() )
            left_width += chars_[split_pos++].second;

          return {
            { bytes_.data(),
             bytes_.data() + ( split_pos < chars_.size() ? chars_[split_pos].first : bytes_.size() ),
             bytes_.data() + bytes_.size() },
            std::make_pair( left_width, width_ - left_width )
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
          chars_ = parse_char( new_bytes );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
            } );
          bytes_ = std::move( new_bytes );
        }
#endif
      };
    } // namespace charcodes
  } // namespace _details
} // namespace pgbar

#endif
