#ifndef __PGBAR_OPTIONWRAPPER
#define __PGBAR_OPTIONWRAPPER

#include "../config/Core.hpp"
#include <type_traits>
#include <utility>

namespace pgbar {
  namespace __details {
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
        constexpr OptionWrapper( const OptionWrapper& )                          = default;
        constexpr OptionWrapper( OptionWrapper&& )                               = default;
        __PGBAR_CXX14_CNSTXPR OptionWrapper& operator=( const OptionWrapper& ) & = default;
        __PGBAR_CXX14_CNSTXPR OptionWrapper& operator=( OptionWrapper&& ) &      = default;

        // Intentional non-virtual destructors.
        __PGBAR_CXX20_CNSTXPR ~OptionWrapper() = default;
        __PGBAR_CXX14_CNSTXPR T& value() & noexcept { return data_; }
        __PGBAR_CXX14_CNSTXPR const T& value() const& noexcept { return data_; }
        __PGBAR_CXX14_CNSTXPR T&& value() && noexcept { return std::move( data_ ); }

        __PGBAR_CXX20_CNSTXPR void swap( T& lhs ) noexcept
        {
          __PGBAR_TRUST( this != &lhs );
          using std::swap;
          swap( data_, lhs.data_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( T& a, T& b ) noexcept { a.swap( b ); }
      };
    } // namespace wrappers
  } // namespace __details
} // namespace pgbar

#endif
