#ifndef __PGBAR_TYPESET
#define __PGBAR_TYPESET

#include "Algorithm.hpp"
#include "TypeList.hpp"

namespace pgbar {
  namespace __details {
    namespace traits {
      /**
       * A TypeList without duplicates;
       * the presence of duplicate elements will result in a hard compile error.
       */
      template<typename... Ts>
      struct TypeSet : TypeList<Ts>... {};

      template<typename... Ts, typename T>
      struct TpContain<TypeSet<Ts...>, T> : std::is_base_of<TypeList<T>, TypeSet<Ts...>> {};

      template<typename... Ts, typename T>
      struct TpExtend<TypeSet<Ts...>, T> {
      private:
        template<bool Cond, typename NewOne>
        struct _Select;
        template<typename NewOne>
        struct _Select<true, NewOne> {
          using type = TypeSet<Ts...>;
        };
        template<typename NewOne>
        struct _Select<false, NewOne> {
          using type = TypeSet<Ts..., NewOne>;
        };

      public:
        using type = typename _Select<TpContain<TypeSet<Ts...>, T>::value, T>::type;
      };

      template<typename FirstSet>
      struct Union<FirstSet, TypeSet<>> {
        using type = FirstSet;
      };
      template<typename FirstSet, typename T, typename... Ts>
      struct Union<FirstSet, TypeSet<T, Ts...>> : Union<TpExtend_t<FirstSet, T>, TypeSet<Ts...>> {};

      template<typename FirstSet>
      struct Merge<FirstSet> {
        using type = FirstSet;
      };
      template<typename FirstSet, typename SecondSet, typename... RestSets>
      struct Merge<FirstSet, SecondSet, RestSets...> : Merge<Union_t<FirstSet, SecondSet>, RestSets...> {};

      template<typename... Elements>
      struct Distinct<TypeList<Elements...>> {
      private:
        template<typename Visited, typename List>
        struct Helper;
        template<typename Visited>
        struct Helper<Visited, TypeList<>> : std::true_type {};
        template<typename Visited, typename U, typename... Us>
        struct Helper<Visited, TypeList<U, Us...>>
          : AllOf<Not<TpContain<Visited, U>>, Helper<TpExtend_t<Visited, U>, TypeList<Us...>>> {};

      public:
        static constexpr bool value = Helper<TypeSet<>, TypeList<Elements...>>::value;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
