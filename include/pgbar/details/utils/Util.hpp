#ifndef PGBAR__UTILS_UTIL
#define PGBAR__UTILS_UTIL

#include "../traits/Backport.hpp"
#include "../types/Types.hpp"
#include <cmath>
#include <tuple>
#if PGBAR__CXX17
# include <charconv>
#else
# include <limits>
#endif

namespace pgbar {
  namespace _details {
    namespace utils {
      // Perfectly forward the I-th element of a tuple, constructing one by default if it's out of bound.
      template<types::Size I,
               typename T,
               typename Tuple,
               typename = typename std::enable_if<traits::AllOf<
                 traits::BoolConstant<( I < std::tuple_size<typename std::decay<Tuple>::type>::value )>,
                 std::is_default_constructible<T>>::value>::type>
      PGBAR__INLINE_FN constexpr auto pick_or( Tuple&& tup ) noexcept
        -> decltype( std::get<I>( std::forward<Tuple>( tup ) ) )
      {
        static_assert( std::is_convertible<typename std::tuple_element<I, Tuple>::type, T>::value,
                       "pgbar::_details::traits::pick_or: Incompatible type" );
        return std::get<I>( std::forward<Tuple>( tup ) );
      }
      template<types::Size I, typename T, typename Tuple>
      PGBAR__INLINE_FN constexpr auto pick_or( Tuple&& )
        noexcept( std::is_nothrow_default_constructible<T>::value ) -> typename std::enable_if<
          traits::AllOf<
            traits::BoolConstant<( I >= std::tuple_size<typename std::decay<Tuple>::type>::value )>,
            std::is_default_constructible<T>>::value,
          T>::type
      {
        return T();
      }

      template<typename Numeric>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR
        typename std::enable_if<std::is_unsigned<Numeric>::value, types::Size>::type
        count_digits( Numeric val ) noexcept
      {
        types::Size digits = val == 0;
        for ( ; val > 0; val /= 10 )
          ++digits;
        return digits;
      }
      template<typename Numeric>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR
        typename std::enable_if<std::is_signed<Numeric>::value, types::Size>::type
        count_digits( Numeric val ) noexcept
      {
        return count_digits( static_cast<std::uint64_t>( val < 0 ? -val : val ) );
      }

      // Format an integer number.
      template<typename Integer>
      PGBAR__NODISCARD PGBAR__INLINE_FN
        typename std::enable_if<std::is_integral<Integer>::value, types::String>::type
        format( Integer val ) noexcept( noexcept( std::to_string( val ) ) )
      {
        /* In some well-designed standard libraries,
         * integer std::to_string has specialized implementations for different bit-length types;

         * This includes directly constructing the destination string using the SOO/SSO nature of the string,
         * optimizing the memory management strategy using internally private resize_and_overwrite, etc.

         * Therefore, the functions of the standard library are called directly here,
         * rather than providing a manual implementation like the other functions. */
        return std::to_string( val );
        // Although, unfortunately, std::to_string is not labeled constexpr.
      }

      // Format a finite floating point number.
      template<typename Floating>
      PGBAR__NODISCARD PGBAR__INLINE_FN
        typename std::enable_if<std::is_floating_point<Floating>::value, types::String>::type
        format( Floating val, int precision ) noexcept( false )
      {
        /* Unlike the integer version,
         * the std::to_string in the standard library does not provide a precision limit
         * on floating-point numbers;

         * So the implementation here is provided manually. */
        PGBAR__ASSERT( std::isfinite( val ) );
        PGBAR__TRUST( precision >= 0 );
#if PGBAR__CXX17
        const auto abs_rounded_val = std::round( std::abs( val ) );
        const auto int_digits      = count_digits( abs_rounded_val );

        types::String formatted;
# if PGBAR__CXX23
        formatted.resize_and_overwrite(
          int_digits + precision + 2,
          [val, precision]( types::Char* buf, types::Size n ) noexcept {
            const auto result = std::to_chars( buf, buf + n, val, std::chars_format::fixed, precision );
            PGBAR__TRUST( result.ec == std::errc() );
            PGBAR__TRUST( result.ptr >= buf );
            return static_cast<types::Size>( result.ptr - buf );
          } );
# else
        formatted.resize( int_digits + precision + 2 );
        // The extra 2 is left for the decimal point and carry.
        const auto result = std::to_chars( formatted.data(),
                                           formatted.data() + formatted.size(),
                                           val,
                                           std::chars_format::fixed,
                                           precision );
        PGBAR__TRUST( result.ec == std::errc() );
        PGBAR__ASSERT( result.ptr >= formatted.data() );
        formatted.resize( result.ptr - formatted.data() );
# endif
#else
        const auto scale           = std::pow( 10, precision );
        const auto abs_rounded_val = std::round( std::abs( val ) * scale ) / scale;
        PGBAR__ASSERT( abs_rounded_val <= ( std::numeric_limits<std::uint64_t>::max )() );
        const auto integer = static_cast<std::uint64_t>( abs_rounded_val );
        const auto fraction =
          static_cast<std::uint64_t>( std::round( ( abs_rounded_val - integer ) * scale ) );
        const auto sign = std::signbit( val );

        auto formatted = types::String( sign, '-' );
        formatted.append( format( integer ) ).reserve( count_digits( integer ) + sign );
        if ( precision > 0 ) {
          formatted.push_back( '.' );
          const auto fract_digits = count_digits( fraction );
          PGBAR__TRUST( fract_digits <= static_cast<types::Size>( precision ) );
          formatted.append( precision - fract_digits, '0' ).append( format( fraction ) );
        }
#endif
        return formatted;
      }

      enum class TxtLayout { Left, Right, Center }; // text layout
      // Format the `str`.
      template<TxtLayout Style>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::String format( types::Size width,
                                                                                   types::Size len_str,
                                                                                   types::ROStr str )
        noexcept( false )
      {
        if ( width == 0 )
          PGBAR__UNLIKELY return {};
        if ( len_str >= width )
          return types::String( str );
        if PGBAR__CXX17_CNSTXPR ( Style == TxtLayout::Right ) {
          auto tmp = types::String( width - len_str, ' ' );
          tmp.append( str );
          return tmp;
        } else if PGBAR__CXX17_CNSTXPR ( Style == TxtLayout::Left ) {
          auto tmp = types::String( str );
          tmp.append( width - len_str, ' ' );
          return tmp;
        } else {
          width -= len_str;
          const types::Size l_blank = width / 2;
          return std::move( types::String( l_blank, ' ' ).append( str ) )
               + types::String( width - l_blank, ' ' );
        }
      }
      template<TxtLayout Style>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::String format( types::Size width,
                                                                                   types::ROStr __str )
        noexcept( false )
      {
        return format<Style>( width, __str.size(), __str );
      }
    } // namespace utils
  } // namespace _details
} // namespace pgbar

#endif
