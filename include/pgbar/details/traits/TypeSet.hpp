#ifndef __PGBAR_TYPESET
#define __PGBAR_TYPESET

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

      template<typename TpSet, typename T>
      struct Exist;
      template<typename... Ts, typename T>
      struct Exist<TypeSet<Ts...>, T> : std::is_base_of<TypeList<T>, TypeSet<Ts...>> {};

      template<typename TpSet, typename T>
      struct Expand;
      template<typename TpSet, typename T>
      using Expand_t = typename Expand<TpSet, T>::type;

      template<typename... Ts, typename T>
      struct Expand<TypeSet<Ts...>, T> {
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
        using type = typename _Select<Exist<TypeSet<Ts...>, T>::value, T>::type;
      };

      template<typename HeadTpSet, typename TailTpSet>
      struct Union;
      template<typename HeadTpSet, typename TailTpSet>
      using Union_t = typename Union<HeadTpSet, TailTpSet>::type;

      template<typename HeadTpSet>
      struct Union<HeadTpSet, TypeSet<>> {
        using type = HeadTpSet;
      };
      template<typename HeadTpSet, typename T, typename... Ts>
      struct Union<HeadTpSet, TypeSet<T, Ts...>> : Union<Expand_t<HeadTpSet, T>, TypeSet<Ts...>> {};

      template<typename HeadTpSet, typename... TailTpSet>
      struct Coalesce;
      template<typename HeadTpSet, typename... TailTpSet>
      using Coalesce_t = typename Coalesce<HeadTpSet, TailTpSet...>::type;

      template<typename HeadTpSet>
      struct Coalesce<HeadTpSet> {
        using type = HeadTpSet;
      };
      template<typename HeadTpSet, typename TailTpSet, typename... RestTpSets>
      struct Coalesce<HeadTpSet, TailTpSet, RestTpSets...>
        : Coalesce<Union_t<HeadTpSet, TailTpSet>, RestTpSets...> {};

      // Check whether the list contains duplicate types.
      template<typename TpList>
      struct Duplicated {
      private:
        template<typename VisitedSet, typename List>
        struct Helper;
        template<typename VisitedSet>
        struct Helper<VisitedSet, TypeList<>> : std::false_type {};
        template<typename VisitedSet, typename U, typename... Us>
        struct Helper<VisitedSet, TypeList<U, Us...>> {
        private:
          template<bool Cond, typename RestList>
          struct _Select;
          template<typename RestList>
          struct _Select<true, RestList> : std::true_type {};
          template<typename RestList>
          struct _Select<false, RestList> : Helper<Expand_t<VisitedSet, U>, RestList> {};

        public:
          static constexpr bool value = _Select<Exist<VisitedSet, U>::value, TypeList<Us...>>::value;
        };

      public:
        static constexpr bool value = Helper<TypeSet<>, TpList>::value;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
