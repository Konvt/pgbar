#ifndef __PGBAR_TRAITS_UTIL
#define __PGBAR_TRAITS_UTIL

#include "Backport.hpp"

namespace pgbar {
  namespace __details {
    namespace traits {
      template<typename T, template<typename...> class Template, types::Size N>
      struct Repeat {
      private:
        template<typename U, template<typename...> class Tmp, types::Size M, typename... Us>
        struct Helper : Helper<U, Tmp, M - 1, Us..., U> {};
        template<typename U, template<typename...> class Tmp, typename... Us>
        struct Helper<U, Tmp, 0, Us...> {
          using type = Tmp<Us...>;
        };

      public:
        using type = typename Helper<T, Template, N>::type;
      };
      template<typename T, template<typename...> class Template, types::Size N>
      using Repeat_t = typename Repeat<T, Template, N>::type;

      template<types::Size Pos, typename... Ts>
      struct TypeAt {
        static_assert( static_cast<bool>( Pos < sizeof...( Ts ) ),
                       "pgbar::__details::traits::TypeAt: Position overflow" );
      };
      template<types::Size Pos, typename... Ts>
      using TypeAt_t = typename TypeAt<Pos, Ts...>::type;
      // After C++26, we can use `Ts...[Pos]`.

      template<types::Size Pos, typename T, typename... Ts>
      struct TypeAt<Pos, T, Ts...> : TypeAt<Pos - 1, Ts...> {};
      template<typename T, typename... Ts>
      struct TypeAt<0, T, Ts...> {
        using type = T;
      };

      // Check whether the type Instance is an instantiated version of Tmp or whether it inherits from Tmp
      // itself.
      template<typename Instance, template<typename...> class Tmp>
      struct InstanceOf {
      private:
        template<typename... Args>
        static constexpr std::true_type check( const Tmp<Args...>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<Instance>>,
                decltype( check( std::declval<typename std::remove_cv<Instance>::type>() ) )>::value;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
