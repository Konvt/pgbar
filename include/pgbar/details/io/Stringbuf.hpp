#ifndef __PGBAR_STRINGBUF
#define __PGBAR_STRINGBUF

#include "../charcodes/U8Raw.hpp"
#include "../traits/Backport.hpp"
#include <vector>

namespace pgbar {
  namespace __details {
    namespace io {
      // A simple string buffer, unrelated to the `std::stringbuf` in the STL.
      class Stringbuf {
        using Self = Stringbuf;

      protected:
        std::vector<types::Char> buffer_;

      public:
        __PGBAR_CXX20_CNSTXPR Stringbuf() = default;

        __PGBAR_CXX20_CNSTXPR Stringbuf( const Self& lhs )         = default;
        __PGBAR_CXX20_CNSTXPR Stringbuf( Self&& rhs ) noexcept     = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& lhs ) & = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& rhs ) &      = default;
        // Intentional non-virtual destructors.
        __PGBAR_CXX20_CNSTXPR ~Stringbuf()                         = default;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR bool empty() const noexcept { return buffer_.empty(); }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void clear() & noexcept { buffer_.clear(); }

        // Releases the buffer space completely
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void release() noexcept
        {
          clear();
          buffer_.shrink_to_fit();
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& reserve( types::Size capacity ) &
        {
          buffer_.reserve( capacity );
          return *this;
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( types::Char info, types::Size __num = 1 ) &
        {
          buffer_.insert( buffer_.end(), __num, info );
          return *this;
        }
        template<types::Size N>
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const char ( &info )[N],
                                                              types::Size __num = 1 ) &
        {
          while ( __num-- )
            buffer_.insert( buffer_.end(), info, info + N );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( types::ROStr info, types::Size __num = 1 ) &
        {
          while ( __num-- )
            buffer_.insert( buffer_.end(), info.cbegin(), info.cend() );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const charcodes::U8Raw& info,
                                                              types::Size __num = 1 ) &
        {
          return append( info.str(), __num );
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const types::Char* head,
                                                              const types::Char* tail ) &
        {
          if ( head != nullptr && tail != nullptr )
            buffer_.insert( buffer_.end(), head, tail );
          return *this;
        }

        template<typename T>
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR typename std::enable_if<
          traits::AnyOf<std::is_same<typename std::decay<T>::type, types::Char>,
                        std::is_same<typename std::decay<T>::type, types::String>,
                        std::is_same<typename std::decay<T>::type, charcodes::U8Raw>>::value,
          Self&>::type
          operator<<( Self& stream, T&& info )
        {
          return stream.append( std::forward<T>( info ) );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( Self& stream, types::ROStr info )
        {
          return stream.append( info );
        }
        template<types::Size N>
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( Self& stream,
                                                                         const char ( &info )[N] )
        {
          return stream.append( info );
        }

        __PGBAR_CXX20_CNSTXPR void swap( Stringbuf& lhs ) noexcept
        {
          __PGBAR_TRUST( this != &lhs );
          buffer_.swap( lhs.buffer_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( Stringbuf& a, Stringbuf& b ) noexcept { a.swap( b ); }

#if __PGBAR_CXX20
        __PGBAR_INLINE_FN Self& append( types::LitU8 info, types::Size __num = 1 ) &
        {
          while ( __num-- )
            buffer_.insert( buffer_.end(), info.data(), info.data() + info.size() );
          return *this;
        }
        friend __PGBAR_INLINE_FN Self& operator<<( Self& stream, types::LitU8 info )
        {
          return stream.append( info );
        }
#endif
      };
    } // namespace io
  } // namespace __details
} // namespace pgbar

#endif
