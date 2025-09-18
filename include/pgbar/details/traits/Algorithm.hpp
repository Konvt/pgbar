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
      struct TmpExtend;
      template<typename Collection, template<typename...> class Element>
      using TmpExtend_t = typename TmpExtend<Collection, Element>::type;

      template<typename Collection, typename Element>
      struct TpExtend;
      template<typename Collection, typename Element>
      using TpExtend_t = typename TpExtend<Collection, Element>::type;

      // Check whether the elements in the collection are unique.
      template<typename Collection>
      struct Distinct;

      template<typename FirstSet, typename SecondSet>
      struct Union;
      template<typename FirstSet, typename SecondSet>
      using Union_t = typename Union<FirstSet, SecondSet>::type;

      template<typename FirstCollection, typename... TailCollections>
      struct Merge;
      template<typename FirstCollection, typename... TailCollections>
      using Merge_t = typename Merge<FirstCollection, TailCollections...>::type;
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
