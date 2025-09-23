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

      template<typename Element, types::Size N>
      struct Fill {
      private:
        template<bool Cond, typename List>
        struct _Select;
        template<typename List>
        struct _Select<true, List> : TpPrepend<List, Element> {};
        template<typename List>
        struct _Select<false, List> {
          using type = List;
        };
        using Half_t = typename Fill<Element, N / 2>::type;

      public:
        using type = typename _Select<N % 2 == 0, Combine_t<Half_t, Half_t>>::type;
      };
      template<typename Element>
      struct Fill<Element, 0> {
        using type = TypeList<>;
      };
      template<typename Element>
      struct Fill<Element, 1> {
        using type = TypeList<Element>;
      };
      template<typename Element, types::Size N>
      using Fill_t = typename Fill<Element, N>::type;

      template<typename... Ts, typename T>
      struct TpPrepend<TypeList<Ts...>, T> {
        using type = TypeList<T, Ts...>;
      };

      template<typename... Ts>
      struct TpStartsWith<TypeList<>, Ts...> : std::true_type {};
      template<typename Head, typename... Tail>
      struct TpStartsWith<TypeList<Head, Tail...>> : std::false_type {};
      template<typename Head, typename... Tail, typename T, typename... Ts>
      struct TpStartsWith<TypeList<Head, Tail...>, T, Ts...>
        : AllOf<std::is_same<Head, T>, TpStartsWith<TypeList<Tail...>, Ts...>> {};

      template<typename... First, typename... Second>
      struct Combine<TypeList<First...>, TypeList<Second...>> {
        using type = TypeList<First..., Second...>;
      };

      template<typename... Elements, types::Size Nth>
      struct DropAt<TypeList<Elements...>, Nth> {
      private:
        template<typename Front, typename Back>
        struct Helper;
        template<types::Size... L, types::Size... R>
        struct Helper<IndexSeq<L...>, IndexSeq<R...>> {
          using type = TypeList<TypeAt_t<L, Elements...>..., TypeAt_t<( R + Nth + 1 ), Elements...>...>;
        };

      public:
        static_assert( Nth < sizeof...( Elements ), "pgbar::__details::traits::DropAt: Nth overflow" );

        using type = typename Helper<MakeIndexSeq<Nth>, MakeIndexSeq<sizeof...( Elements ) - Nth - 1>>::type;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
