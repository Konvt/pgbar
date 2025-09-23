#ifndef __PGBAR_C3
#define __PGBAR_C3

#include "Backport.hpp"
#include "TemplateSet.hpp"
#include "TypeList.hpp"

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
        template<typename /* TemplateSet<...> */ OtherVBs, template<typename...> class Candidate>
        struct PreferredCandidate;

        template<template<typename...> class Candidate>
        struct PreferredCandidate<TemplateSet<>, Candidate> : std::true_type {};
        template<template<typename...> class Head,
                 template<typename...> class... Rests,
                 template<typename...> class Candidate>
        struct PreferredCandidate<TemplateSet<Head, Rests...>, Candidate>
          : Not<TmpContain<TemplateSet<Rests...>, Candidate>> {};

        template<typename /* TemplateSet<...> */ VBSet, template<typename...> class Candidate>
        struct DropCandidate;
        template<typename VBSet, template<typename...> class Candidate>
        using DropCandidate_t = typename DropCandidate<VBSet, Candidate>::type;

        template<template<typename...> class Candidate, template<typename...> class... Rests>
        struct DropCandidate<TemplateSet<Candidate, Rests...>, Candidate> {
          using type = TemplateSet<Rests...>;
        };
        template<template<typename...> class... Rests, template<typename...> class Candidate>
        struct DropCandidate<TemplateSet<Rests...>, Candidate> {
          using type = TemplateSet<Rests...>;
        };

        template<typename /* TemplateSet<...> */ Sorted,
                 typename /* TypeList<TemplateSet<...>, ...> */ MergedList,
                 types::Size I>
        struct Linearize;
        template<typename Sorted, typename MergedList, types::Size I>
        using Linearize_t = typename Linearize<Sorted, MergedList, I>::type;

        template<typename Sorted, types::Size I>
        struct Linearize<Sorted, TypeList<>, I> {
          using type = Sorted;
        };
        template<typename Sorted, typename... Lists, types::Size I>
        struct Linearize<Sorted, TypeList<TemplateSet<>, Lists...>, I>
          : Linearize<Sorted, TypeList<Lists...>, I> {};
        template<typename Sorted, typename... Lists, types::Size I>
        struct Linearize<Sorted, TypeList<Lists...>, I> {
        private:
          template<typename /* TemplateSet<...> */ Selected,
                   typename /* TypeList<TemplateSet<...>, ...> */ Others>
          struct Helper;
          template<typename Selected, typename Others>
          using Helper_t = typename Helper<Selected, Others>::type;

          template<typename... Others>
          struct Helper<TemplateSet<>, TypeList<Others...>> : Linearize<Sorted, TypeList<Others...>, I> {};
          template<template<typename...> class Candidate,
                   template<typename...> class... Rests,
                   typename... Others>
          struct Helper<TemplateSet<Candidate, Rests...>, TypeList<Others...>> {
          private:
            template<bool Cond, typename Result>
            struct _Select;
            template<typename Result>
            struct _Select<true, Result>
              : Linearize<TmpAppend_t<Result, Candidate>, TypeList<DropCandidate_t<Lists, Candidate>...>, 0> {
            };
            template<typename Result>
            struct _Select<false, Result> : Linearize<Result, TypeList<Lists...>, I + 1> {};

          public:
            using type =
              typename _Select<AllOf<PreferredCandidate<Others, Candidate>...>::value, Sorted>::type;
          };

        public:
          using type = Helper_t<TypeAt_t<I, Lists...>, DropAt_t<TypeList<Lists...>, I>>;
        };

      public:
        using type =
          Linearize_t<TemplateSet<>,
                      TypeList<InheritOrder_t<VB>, InheritOrder_t<VBs>..., TemplateSet<VB, VBs...>>,
                      0>;
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
