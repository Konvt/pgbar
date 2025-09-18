#ifndef __PGBAR_TYPELIST
#define __PGBAR_TYPELIST

#include "Algorithm.hpp"
#include "Backport.hpp"
#include <type_traits>


namespace pgbar {
  namespace __details {
    namespace traits {
      /**
       * A lightweight tuple type that stores multiple types.
       *
       * `std::tuple` puts some constraints on the input type that are not metaprogramming related,
       * so here is a lightweight tuple type that is used only for template type parameter passing.
       */
      template<typename... Ts>
      struct TypeList {};

      template<typename... Ts>
      struct TpStartsWith<TypeList<>, Ts...> : std::true_type {};
      template<typename Head, typename... Tail>
      struct TpStartsWith<TypeList<Head, Tail...>> : std::false_type {};
      template<typename Head, typename... Tail, typename T, typename... Ts>
      struct TpStartsWith<TypeList<Head, Tail...>, T, Ts...>
        : AllOf<std::is_same<Head, T>, TpStartsWith<TypeList<Tail...>, Ts...>> {};
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
