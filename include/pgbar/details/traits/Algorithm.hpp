#ifndef __PGBAR_ALGORITHM
#define __PGBAR_ALGORITHM

namespace pgbar {
  namespace __details {
    namespace traits {
      template<typename Collection, template<typename...> class Element>
      struct TmpContain;

      template<typename Collection, typename Element>
      struct TpContain;

      // Checks if a List starts with the specified type sequence.
      template<typename List, typename... Elements>
      struct TpStartsWith;

      template<typename List, template<typename...> class Element>
      struct TmpPrepend;
      template<typename List, template<typename...> class Element>
      using TmpPrepend_t = typename TmpPrepend<List, Element>::type;

      template<typename Collection, template<typename...> class Element>
      struct TmpAppend;
      template<typename Collection, template<typename...> class Element>
      using TmpAppend_t = typename TmpAppend<Collection, Element>::type;

      template<typename Collection, typename Element>
      struct TpAppend;
      template<typename Collection, typename Element>
      using TpAppend_t = typename TpAppend<Collection, Element>::type;

      // Check whether the elements in the collection are unique.
      template<typename Collection>
      struct Distinct;

      template<typename FirstCollection, typename SecondCollection>
      struct Combine;
      template<typename FirstCollection, typename SecondCollection>
      using Combine_t = typename Combine<FirstCollection, SecondCollection>::type;

      template<typename FirstCollection, typename... TailCollections>
      struct Merge;
      template<typename FirstCollection, typename... TailCollections>
      using Merge_t = typename Merge<FirstCollection, TailCollections...>::type;

      template<typename FirstCollection>
      struct Merge<FirstCollection> {
        using type = FirstCollection;
      };
      template<typename FirstCollection, typename SecondCollection, typename... RestCollections>
      struct Merge<FirstCollection, SecondCollection, RestCollections...>
        : Merge<Combine_t<FirstCollection, SecondCollection>, RestCollections...> {};

    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
