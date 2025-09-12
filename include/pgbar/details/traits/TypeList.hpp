#ifndef __PGBAR_TYPELIST
#define __PGBAR_TYPELIST

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

      // Checks if a TypeList starts with the specified type sequence.
      template<typename TpList, typename... Ts>
      struct StartsWith;
      template<typename... Ts>
      struct StartsWith<TypeList<>, Ts...> : std::true_type {};
      template<typename Head, typename... Tail>
      struct StartsWith<TypeList<Head, Tail...>> : std::false_type {};
      template<typename Head, typename... Tail, typename T, typename... Ts>
      struct StartsWith<TypeList<Head, Tail...>, T, Ts...> {
      private:
        template<bool Cond, typename RestList>
        struct _Select;
        template<typename RestList>
        struct _Select<true, RestList> : StartsWith<RestList, Ts...> {};
        template<typename RestList>
        struct _Select<false, RestList> : std::false_type {};

      public:
        static constexpr bool value = _Select<std::is_same<Head, T>::value, TypeList<Tail...>>::value;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
