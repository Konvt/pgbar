#ifndef PGBAR__ALGORITHM
#define PGBAR__ALGORITHM

#include "../types/Types.hpp"
#include "Backport.hpp"
#if !PGBAR__BUILTIN( __type_pack_element )
# include <tuple>
#endif

namespace pgbar {
  namespace _details {
    namespace traits {
      template<types::Size Nth, typename... Ts>
      struct TypeAt
#if PGBAR__BUILTIN( __type_pack_element )
      {
        using type = __type_pack_element<Nth, Ts...>;
      };
#else
        // When used as a template metaprogramming tool,
        // the std::tuple here does not trigger type instantiation,
        // and thus does not generate type checks related to construction constraints.
        : std::tuple_element<Nth, std::tuple<Ts...>> {
      };
#endif
      template<types::Size Nth, typename... Ts>
      using TypeAt_t = typename TypeAt<Nth, Ts...>::type;
      // After C++26, we can use `Ts...[Pos]`.

      template<typename Collection, typename Element>
      struct TpContain;

      template<typename Collection, typename T>
      struct TpPrepend;
      template<typename Collection, typename T>
      using TpPrepend_t = typename TpPrepend<Collection, T>::type;

      template<typename Collection, typename Element>
      struct TpAppend;
      template<typename Collection, typename Element>
      using TpAppend_t = typename TpAppend<Collection, Element>::type;

      // Checks if a List starts with the specified type sequence.
      template<typename List, typename... Elements>
      struct TpStartsWith;

      template<typename Collection, template<typename...> class Element>
      struct TmpContain;

      template<typename Collection, template<typename...> class Element>
      struct TmpPrepend;
      template<typename Collection, template<typename...> class Element>
      using TmpPrepend_t = typename TmpPrepend<Collection, Element>::type;

      template<typename Collection, template<typename...> class Element>
      struct TmpAppend;
      template<typename Collection, template<typename...> class Element>
      using TmpAppend_t = typename TmpAppend<Collection, Element>::type;

      // Check whether the elements in the collection are unique.
      template<typename Collection>
      struct Distinct;

      template<typename FirstCollection, typename SecondCollection>
      struct Combine;
      template<typename FirstCollection, typename SecondCollection>
      using Combine_t = typename Combine<FirstCollection, SecondCollection>::type;

      template<typename FirstCollection, typename... TailCollections>
      struct Merge {
      private:
        template<typename Front, typename Back>
        struct Helper;
        template<types::Size... L, types::Size... R>
        struct Helper<IndexSeq<L...>, IndexSeq<R...>>
          : Combine<FirstCollection,
                    Combine_t<typename Merge<TypeAt_t<L, TailCollections...>...>::type,
                              typename Merge<TypeAt_t<( sizeof...( TailCollections ) / 2 ) + R,
                                                      TailCollections...>...>::type>> {};

      public:
        using type = typename Helper<
          MakeIndexSeq<( sizeof...( TailCollections ) / 2 )>,
          MakeIndexSeq<( sizeof...( TailCollections ) - ( sizeof...( TailCollections ) / 2 ) )>>::type;
      };
      template<typename FirstCollection, typename... TailCollections>
      using Merge_t = typename Merge<FirstCollection, TailCollections...>::type;

      template<typename Collection>
      struct Merge<Collection> {
        using type = Collection;
      };
      template<typename Head, typename Tail>
      struct Merge<Head, Tail> : Combine<Head, Tail> {};
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
