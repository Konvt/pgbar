#ifndef PGBAR__OPTIONWRAPPER
#define PGBAR__OPTIONWRAPPER

#include "../core/Core.hpp"
#include <type_traits>
#include <utility>

namespace pgbar {
  namespace _details {
    namespace wrappers {
      template<typename T>
      struct OptionWrapper {
      protected:
        T data_;

        constexpr OptionWrapper() noexcept( std::is_nothrow_default_constructible<T>::value ) : data_ {} {};
        constexpr OptionWrapper( T&& data ) noexcept( std::is_nothrow_move_constructible<T>::value )
          : data_ { std::move( data ) }
        {}

      public:
        constexpr OptionWrapper( const OptionWrapper& )                         = default;
        constexpr OptionWrapper( OptionWrapper&& )                              = default;
        PGBAR__CXX14_CNSTXPR OptionWrapper& operator=( const OptionWrapper& ) & = default;
        PGBAR__CXX14_CNSTXPR OptionWrapper& operator=( OptionWrapper&& ) &      = default;

        // Intentional non-virtual destructors.
        PGBAR__CXX20_CNSTXPR ~OptionWrapper() = default;
        PGBAR__CXX14_CNSTXPR T& value() & noexcept { return data_; }
        PGBAR__CXX14_CNSTXPR const T& value() const& noexcept { return data_; }
        PGBAR__CXX14_CNSTXPR T&& value() && noexcept { return std::move( data_ ); }

        PGBAR__CXX20_CNSTXPR void swap( T& lhs ) noexcept
        {
          PGBAR__TRUST( this != &lhs );
          using std::swap;
          swap( data_, lhs.data_ );
        }
        friend PGBAR__CXX20_CNSTXPR void swap( T& a, T& b ) noexcept { a.swap( b ); }
      };
    } // namespace wrappers
  } // namespace _details
} // namespace pgbar

#endif
