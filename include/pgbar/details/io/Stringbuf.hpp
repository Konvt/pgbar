#ifndef PGBAR__STRINGBUF
#define PGBAR__STRINGBUF

#include "../charcodes/EncodedView.hpp"
#include "../traits/Backport.hpp"
#include <vector>

namespace pgbar {
  namespace _details {
    namespace io {
      // A simple string buffer, unrelated to the `std::stringbuf` in the STL.
      class Stringbuf {
      protected:
        std::vector<types::Char> buffer_;

      public:
        PGBAR__CXX20_CNSTXPR Stringbuf() = default;

        PGBAR__CXX20_CNSTXPR Stringbuf( const Stringbuf& )              = default;
        PGBAR__CXX20_CNSTXPR Stringbuf( Stringbuf&& ) noexcept          = default;
        PGBAR__CXX20_CNSTXPR Stringbuf& operator=( const Stringbuf& ) & = default;
        PGBAR__CXX20_CNSTXPR Stringbuf& operator=( Stringbuf&& ) &      = default;
        // Intentional non-virtual destructors.
        PGBAR__CXX20_CNSTXPR ~Stringbuf()                               = default;

        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR bool empty() const noexcept
        {
          return buffer_.empty();
        }
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR void clear() & noexcept { buffer_.clear(); }

        // Releases the buffer space completely
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR void release() noexcept
        {
          clear();
          buffer_.shrink_to_fit();
        }

        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& reserve( types::Size capacity ) &
        {
          buffer_.reserve( capacity );
          return *this;
        }

        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& append( types::Char info, types::Size num = 1 ) &
        {
          buffer_.insert( buffer_.end(), num, info );
          return *this;
        }
        template<types::Size N>
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& append( const char ( &info )[N],
                                                                   types::Size num = 1 ) &
        {
          while ( num-- )
            buffer_.insert( buffer_.end(), info, info + N );
          return *this;
        }
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& append( types::ROStr info, types::Size num = 1 ) &
        {
          while ( num-- )
            buffer_.insert( buffer_.end(), info.cbegin(), info.cend() );
          return *this;
        }
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& append( const charcodes::U8Raw& info,
                                                                   types::Size num = 1 ) &
        {
          return append( info.str(), num );
        }
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& append( const types::Char* head,
                                                                   const types::Char* tail ) &
        {
          if ( head != nullptr && tail != nullptr )
            buffer_.insert( buffer_.end(), head, tail );
          return *this;
        }
        PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& append( const charcodes::EncodedView& info,
                                                                   types::Size num = 1 ) &
        {
          if ( info ) {
            while ( num-- )
              return append( info.begin(), info.end() );
          }
          return *this;
        }

        template<typename T>
        friend PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR typename std::enable_if<
          traits::AnyOf<std::is_same<typename std::decay<T>::type, types::Char>,
                        std::is_same<typename std::decay<T>::type, types::String>,
                        std::is_same<typename std::decay<T>::type, charcodes::U8Raw>,
                        std::is_same<typename std::decay<T>::type, charcodes::EncodedView>>::value,
          Stringbuf&>::type
          operator<<( Stringbuf& stream, T&& info )
        {
          return stream.append( std::forward<T>( info ) );
        }
        friend PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& operator<<( Stringbuf& stream,
                                                                              types::ROStr info )
        {
          return stream.append( info );
        }
        template<types::Size N>
        friend PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR Stringbuf& operator<<( Stringbuf& stream,
                                                                              const char ( &info )[N] )
        {
          return stream.append( info );
        }

        PGBAR__CXX20_CNSTXPR void swap( Stringbuf& lhs ) noexcept
        {
          PGBAR__TRUST( this != &lhs );
          buffer_.swap( lhs.buffer_ );
        }
        friend PGBAR__CXX20_CNSTXPR void swap( Stringbuf& a, Stringbuf& b ) noexcept { a.swap( b ); }

#ifdef __cpp_lib_char8_t
        PGBAR__FORCEINLINE Stringbuf& append( types::LitU8 info, types::Size num = 1 ) &
        {
          while ( num-- )
            buffer_.insert( buffer_.end(), info.data(), info.data() + info.size() );
          return *this;
        }
        friend PGBAR__FORCEINLINE Stringbuf& operator<<( Stringbuf& stream, types::LitU8 info )
        {
          return stream.append( info );
        }
#endif
      };
    } // namespace io
  } // namespace _details
} // namespace pgbar

#endif
