#ifndef PGBAR__TYPELIST
#define PGBAR__TYPELIST

#include "Algorithm.hpp"
#include "Backport.hpp"
#include <type_traits>

namespace pgbar {
  namespace _details {
    namespace traits {
      /**
       * A lightweight tuple type that stores multiple types.
       *
       * `std::tuple` puts some constraints on the input type that are not metaprogramming related,
       * so here is a lightweight tuple type that is used only for template type parameter passing.
       */
      template<typename... Ts>
      struct TypeList {};

      template<typename... Es, typename Element>
      struct TpAppend<TypeList<Es...>, Element> {
        using type = TypeList<Es..., Element>;
      };

      template<typename... Es, template<typename...> class Collection>
      struct Combine<TypeList<Es...>, Collection<>> {
        using type = TypeList<Es...>;
      };
      template<typename... Es, template<typename...> class Collection, typename T, typename... Ts>
      struct Combine<TypeList<Es...>, Collection<T, Ts...>>
        : Combine<TpAppend_t<TypeList<Es...>, T>, Collection<Ts...>> {};

      template<typename Element, types::Size N>
      struct TpFill {
      private:
        template<bool Cond, typename List>
        struct _Select;
        template<typename List>
        struct _Select<false, List> : TpAppend<List, Element> {};
        template<typename List>
        struct _Select<true, List> {
          using type = List;
        };
        using Half_t = typename TpFill<Element, N / 2>::type;

      public:
        using type = typename _Select<( N % 2 == 0 ), Combine_t<Half_t, Half_t>>::type;
      };
      template<typename Element>
      struct TpFill<Element, 0> {
        using type = TypeList<>;
      };
      template<typename Element>
      struct TpFill<Element, 1> {
        using type = TypeList<Element>;
      };
      template<typename Element, types::Size N>
      using TpFill_t = typename TpFill<Element, N>::type;

      template<typename... Ts>
      struct TpStartsWith<TypeList<>, Ts...> : std::true_type {};
      template<typename Head, typename... Tail>
      struct TpStartsWith<TypeList<Head, Tail...>> : std::false_type {};
      template<typename Head, typename... Tail, typename T, typename... Ts>
      struct TpStartsWith<TypeList<Head, Tail...>, T, Ts...>
        : AllOf<std::is_same<Head, T>, TpStartsWith<TypeList<Tail...>, Ts...>> {};
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
