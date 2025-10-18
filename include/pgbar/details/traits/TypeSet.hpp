#ifndef PGBAR__TYPESET
#define PGBAR__TYPESET

#include "Algorithm.hpp"
#include "TypeList.hpp"

namespace pgbar {
  namespace _details {
    namespace traits {
      /**
       * A TypeList without duplicates;
       * the presence of duplicate elements will result in a hard compile error.
       */
      template<typename... Ts>
      struct TypeSet : TypeList<Ts>... {};

      template<typename... Es, typename T>
      struct TpContain<TypeSet<Es...>, T> : std::is_base_of<TypeList<T>, TypeSet<Es...>> {};

      template<typename... Es, typename T>
      struct TpAppend<TypeSet<Es...>, T> {
      private:
        template<bool Cond, typename NewOne>
        struct _Select;
        template<typename NewOne>
        struct _Select<true, NewOne> {
          using type = TypeSet<Es...>;
        };
        template<typename NewOne>
        struct _Select<false, NewOne> {
          using type = TypeSet<Es..., NewOne>;
        };

      public:
        using type = typename _Select<TpContain<TypeSet<Es...>, T>::value, T>::type;
      };

      template<typename... Es, template<typename...> class Collection>
      struct Combine<TypeSet<Es...>, Collection<>> {
        using type = TypeSet<Es...>;
      };
      template<typename... Es, template<typename...> class Collection, typename T, typename... Ts>
      struct Combine<TypeSet<Es...>, Collection<T, Ts...>>
        : Combine<TpAppend_t<TypeSet<Es...>, T>, Collection<Ts...>> {};

      template<typename... Elements>
      struct Distinct<TypeList<Elements...>> {
      private:
        template<typename Visited, typename List>
        struct Helper;
        template<typename Visited>
        struct Helper<Visited, TypeList<>> : std::true_type {};
        template<typename Visited, typename U, typename... Us>
        struct Helper<Visited, TypeList<U, Us...>>
          : AllOf<Not<TpContain<Visited, U>>, Helper<TpAppend_t<Visited, U>, TypeList<Us...>>> {};

      public:
        static constexpr bool value = Helper<TypeSet<>, TypeList<Elements...>>::value;
      };
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
