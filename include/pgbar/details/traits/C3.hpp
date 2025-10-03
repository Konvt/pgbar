#ifndef __PGBAR_C3
#define __PGBAR_C3

#include "Backport.hpp"
#include "TemplateSet.hpp"

namespace pgbar {
  namespace __details {
    namespace traits {
      /**
       * By introducing base class templates,
       * derived classes can inherit from multiple base classes arbitrarily.
       * Using the dependency relationships between these classes,
       * we can use the C3 algorithm to linearize complex inheritance structures
       * into a single inheritance chain.

       * Since no one would write multiple non-virtual inherited duplicate base classes
       * in the case of multiple inheritance
       * (if there are any, please reconsider whether your class structure is reasonable),
       * here we only need to use the C3 algorithm to treat all inheritance relationships
       * as virtual inheritance.
       * This approach retains the benefits of multiple inheritance while avoiding its drawbacks.

       * The only trade-off is a slight increase in compilation time
       * when resolving highly complex inheritance dependencies.
       */
      template<typename /* TemplateSet<...> */ VBSet>
      struct C3;
      template<template<typename...> class VB, template<typename...> class... VBs>
      using C3_t = typename C3<TemplateSet<VB, VBs...>>::type;

      // The structure that records the inheritance order of Node includes itself,
      // just like Python's MRO.
      // The return value of this meta-function will serve as both the return value
      // and the entry parameter of C3.
      template<template<typename...> class Node>
      struct InheritOrder {
        using type = TemplateSet<Node>;
      };
      // Gets the inheritance order of the template class `Node`.
      template<template<typename...> class Node>
      using InheritOrder_t = typename InheritOrder<Node>::type;

// A helper macro to register the inheritance structure of a template class.
#define __PGBAR_INHERIT_REGISTER( Node, ... )           \
  template<>                                            \
  struct InheritOrder<Node> {                           \
    using type = TmpPrepend_t<C3_t<__VA_ARGS__>, Node>; \
  }

      template<template<typename...> class VB, template<typename...> class... VBs>
      struct C3<TemplateSet<VB, VBs...>> {
      private:
        // Check whether Candidate is the top priority within AnotherVBs.
        template<template<typename...> class Candidate, typename /* TemplateSet<...> */ AnotherVBs>
        struct PreferredWithin;

        template<template<typename...> class Candidate>
        struct PreferredWithin<Candidate, TemplateSet<>> : std::true_type {};
        template<template<typename...> class Candidate, template<typename...> class... Rests>
        struct PreferredWithin<Candidate, TemplateSet<Candidate, Rests...>> : std::true_type {};
        template<template<typename...> class Candidate,
                 template<typename...> class Head,
                 template<typename...> class... Rests>
        struct PreferredWithin<Candidate, TemplateSet<Head, Rests...>>
          : Not<TmpContain<TemplateSet<Rests...>, Candidate>> {};

        // Check whether the next preferred candidate is from Inspected.
        template<typename /* TemplateSet<...> */ Inspected,
                 typename... /* TemplateSet<...>, ... */ MergedLists>
        struct FeasibleList;

        template<typename... MergedLists>
        struct FeasibleList<TemplateSet<>, MergedLists...> : std::false_type {
          // MergedLists contain the source list of Candidate.
          static_assert( sizeof...( MergedLists ) > 1,
                         "pgbar::__details::traits::C3: MergedLists is always non-empty" );
        };
        template<template<typename...> class Candidate,
                 template<typename...> class... Rests,
                 typename... MergedLists>
        struct FeasibleList<TemplateSet<Candidate, Rests...>, MergedLists...>
          : AllOf<PreferredWithin<Candidate, MergedLists>...> {};

        // Pick out the index of the candidate from the MergedList.
        template<typename... MergedLists>
        struct PickCandidate {
        private:
          template<types::Size I>
          struct Helper;

          template<bool Cond, types::Size Pos>
          struct _Select;
          template<types::Size Pos>
          struct _Select<true, Pos> : std::integral_constant<types::Size, Pos> {};
          template<types::Size Pos>
          struct _Select<false, Pos> : Helper<Pos + 1> {};

          template<types::Size I>
          struct Helper : _Select<FeasibleList<TypeAt_t<I, MergedLists...>, MergedLists...>::value, I> {};

        public:
          static constexpr types::Size value = Helper<0>::value;
        };

        // Remove the Candidate from the list (if exists).
        template<template<typename...> class Candidate, typename /* TemplateSet<...> */ List>
        struct DropCandidate;
        template<template<typename...> class Candidate, typename List>
        using DropCandidate_t = typename DropCandidate<Candidate, List>::type;

        template<template<typename...> class Candidate, template<typename...> class... Rests>
        struct DropCandidate<Candidate, TemplateSet<Candidate, Rests...>> {
          using type = TemplateSet<Rests...>;
        };
        template<template<typename...> class Candidate, template<typename...> class... Rests>
        struct DropCandidate<Candidate, TemplateSet<Rests...>> {
          using type = TemplateSet<Rests...>;
        };

        template<typename /* TemplateSet<...> */ Sorted, typename... /* TemplateSet<...>... */ MergedLists>
        struct Linearize {
        private:
          template<typename Selected>
          struct Helper;
          template<template<typename...> class Candidate, template<typename...> class... Others>
          struct Helper<TemplateSet<Candidate, Others...>>
            : Linearize<TmpAppend_t<Sorted, Candidate>, DropCandidate_t<Candidate, MergedLists>...> {};

        public:
          using type = typename Helper<TypeAt_t<PickCandidate<MergedLists...>::value, MergedLists...>>::type;
        };
        template<typename Sorted>
        struct Linearize<Sorted> {
          using type = Sorted;
        };
        template<typename Sorted, typename... OtherLists>
        struct Linearize<Sorted, TemplateSet<>, OtherLists...> : Linearize<Sorted, OtherLists...> {};
        template<typename Sorted, typename... OtherLists>
        struct Linearize<Sorted, TemplateSet<>, TemplateSet<>, OtherLists...>
          : Linearize<Sorted, OtherLists...> {};
        template<typename Sorted, typename... OtherLists>
        struct Linearize<Sorted, TemplateSet<>, TemplateSet<>, TemplateSet<>, OtherLists...>
          : Linearize<Sorted, OtherLists...> {};

      public:
        using type = typename Linearize<TemplateSet<>,
                                        InheritOrder_t<VB>,
                                        InheritOrder_t<VBs>...,
                                        TemplateSet<VB, VBs...>>::type;
      };

      /**
       * Linearization of Inheritance.

       * Linearize the input types using the C3 algorithm,
       * then iterate through the resulting sorted list and fill in their template types.

       * It relies on the template `InheritOrder` and `C3` classes to work.
       */
      template<typename /* TemplateSet<...> */ VBSet>
      struct LI {
      private:
        template<typename LinearedOrder, typename RBC, typename... Args>
        struct Helper;
        template<typename LinearedOrder, typename RBC, typename... Args>
        using Helper_t = typename Helper<LinearedOrder, RBC, Args...>::type;

        template<typename RBC, typename... Args>
        struct Helper<TemplateSet<>, RBC, Args...> {
          using type = RBC;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename RBC,
                 typename... Args>
        struct Helper<TemplateSet<Head, Tail...>, RBC, Args...> {
          using type = Head<Helper_t<TemplateSet<Tail...>, RBC, Args...>, Args...>;
        };

      public:
        // RBC: Root Base Class.
        template<typename RBC, typename... Args>
        using type = Helper_t<typename C3<VBSet>::type, RBC, Args...>;
      };

      template<template<typename...> class VB, template<typename...> class... VBs>
      struct LI_t {
        template<typename RBC, typename... Args>
        using type = typename LI<TemplateSet<VB, VBs...>>::template type<RBC, Args...>;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
