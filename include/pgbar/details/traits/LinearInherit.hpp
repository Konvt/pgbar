#ifndef __PGBAR_LINEARINHERIT // Lineared Inheritance
#define __PGBAR_LINEARINHERIT

#include "Backport.hpp"
#include "TemplateSet.hpp"

namespace pgbar {
  namespace __details {
    namespace traits {
      /**
       * A template class that records the inheritance structure of a template class.
       *
       * To record the inheritance structure of a template class,
       * you need to manually provide a specialized version of the template class type
       * that holds both the VBs and NBs types.
       */
      template<template<typename...> class Node>
      struct InheritFrom {
        using VBs  = TemplateSet<>; // virtual base
        using NVBs = TemplateSet<>; // non-virtual base
      };
      // Gets the virtual base list of the template class `Node`.
      template<template<typename...> class Node>
      using InheritFrom_vbt = typename InheritFrom<Node>::VBs;
      // Gets the non-virtual base (also called normal base) list of the template class `Node`.
      template<template<typename...> class Node>
      using InheritFrom_nvbt = typename InheritFrom<Node>::NVBs;

// A helper macro to register the inheritance structure of a template class.
#define __PGBAR_INHERIT_REGISTER( Node, VBList, NVBList ) \
  template<>                                              \
  struct InheritFrom<Node> {                              \
    using VBs  = TemplateSet<VBList>;                     \
    using NVBs = TemplateSet<NVBList>;                    \
  }

      /**
       * By introducing base class templates,
       * derived classes can inherit from multiple base classes arbitrarily.
       * Using the dependency relationships between these classes,
       * the topological sorting below linearizes complex inheritance structures
       * into a single inheritance chain.
       *
       * This approach retains the benefits of multiple inheritance while avoiding its drawbacks.
       *
       * The only trade-off is a slight increase in compilation time
       * when resolving highly complex inheritance dependencies.
       *
       * NVBList: Non-virtual Base List, VBList: Virtual Base List.
       */
      template<typename NVBSet, typename VBSet = TemplateSet<>>
      struct TopoSort;
      // Get a list of topological sorting results for the input template classes.
      template<typename NVBSet, typename VBSet = TemplateSet<>>
      using TopoSort_t = typename TopoSort<NVBSet, VBSet>::type;

      // NVB: Non-virtual Base, VB: Virtual Base
      template<template<typename...> class NVB,
               template<typename...> class... NVBs,
               template<typename...> class... VBs>
      struct TopoSort<TemplateSet<NVB, NVBs...>, TemplateSet<VBs...>> {
      private:
        // VI: Virtual Inherit.
        template<bool VI,
                 typename /* TemplateSet */ SetNode,
                 typename /* TemplateList */ SortedList,
                 typename /* TemplateSet */ SetVisitedVB>
        struct Helper;
        // Return the virtual base class node that was accessed during the recursive process.
        template<bool VI, typename SetNode, typename SortedList, typename SetVisitedVB>
        using Helper_ps = typename Helper<VI, SetNode, SortedList, SetVisitedVB>::pathset;
        // Return the sorted list.
        template<bool VI, typename SetNode, typename SortedList, typename SetVisitedVB>
        using Helper_tp = typename Helper<VI, SetNode, SortedList, SetVisitedVB>::type;

        template<bool VI, typename SortedList, typename SetVisitedVB>
        struct Helper<VI, TemplateSet<>, SortedList, SetVisitedVB> {
          using /* TemplateSet */ pathset = SetVisitedVB;
          using type                      = SortedList;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename SortedList,
                 typename SetVisitedVB>
        struct Helper<true, TemplateSet<Head, Tail...>, SortedList, SetVisitedVB> {
        private:
          using SortVB_t = Helper_tp<true, InheritFrom_vbt<Head>, SortedList, SetVisitedVB>;
          using MarkVB_t = Helper_ps<true, InheritFrom_vbt<Head>, SortedList, SetVisitedVB>;

          using SortTail_t = Helper_tp<true, TemplateSet<Tail...>, SortVB_t, MarkVB_t>;
          using MarkTail_t = Helper_ps<true, TemplateSet<Tail...>, SortVB_t, MarkVB_t>;

          using SortNVB_t = Helper_tp<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;
          using MarkNVB_t = Helper_ps<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;

          template<bool Cond, typename RestNodes, template<typename...> class Target>
          struct _Select;
          template<typename RestNodes, template<typename...> class Target>
          struct _Select<true, RestNodes, Target> {
            using type = Helper_tp<true, RestNodes, SortedList, SetVisitedVB>;
          };
          template<typename RestNodes, template<typename...> class Target>
          struct _Select<false, RestNodes, Target> {
            using type = TmpPrepend_t<SortNVB_t, Target>;
          };

        public:
          using pathset = TmpAppend_t<MarkNVB_t, Head>;
          using type =
            typename _Select<AnyOf<TmpContain<SetVisitedVB, Head>, TmpContain<MarkTail_t, Head>>::value,
                             TemplateSet<Tail...>,
                             Head>::type;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename SortedList,
                 typename SetVisitedVB>
        struct Helper<false, TemplateSet<Head, Tail...>, SortedList, SetVisitedVB> {
        private:
          using SortVB_t = Helper_tp<true, InheritFrom_vbt<Head>, SortedList, SetVisitedVB>;
          using MarkVB_t = Helper_ps<true, InheritFrom_vbt<Head>, SortedList, SetVisitedVB>;

          using SortTail_t = Helper_tp<false, TemplateSet<Tail...>, SortVB_t, MarkVB_t>;
          using MarkTail_t = Helper_ps<false, TemplateSet<Tail...>, SortVB_t, MarkVB_t>;

          using SortNVB_t = Helper_tp<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;
          using MarkNVB_t = Helper_ps<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;

        public:
          using pathset = MarkNVB_t;
          using type    = TmpPrepend_t<SortNVB_t, Head>;
        };

      public:
        using type = Helper_tp<false,
                               TemplateSet<NVB, NVBs...>,
                               Helper_tp<true, TemplateSet<VBs...>, TemplateList<>, TemplateSet<>>,
                               Helper_ps<true, TemplateSet<VBs...>, TemplateList<>, TemplateSet<>>>;
      };

      /**
       * Linearization of Inheritance.
       *
       * Topologically sort the input types,
       * then iterate through the resulting sorted list and fill in their template types.
       *
       * It relies on the template `InheritFrom` and `TopoSort` classes to work.
       */
      template<typename NVBSet, typename VBSet = TemplateSet<>>
      struct LI;

      template<template<typename...> class NVB,
               template<typename...> class... NVBs,
               template<typename...> class... VBs>
      struct LI<TemplateSet<NVB, NVBs...>, TemplateSet<VBs...>> {
      private:
        template<typename TopoOrder, typename RBC, typename... Args>
        struct Helper;
        template<typename TopoOrder, typename RBC, typename... Args>
        using Helper_t = typename Helper<TopoOrder, RBC, Args...>::type;

        template<typename RBC, typename... Args>
        struct Helper<TemplateList<>, RBC, Args...> {
          using type = RBC;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename RBC,
                 typename... Args>
        struct Helper<TemplateList<Head, Tail...>, RBC, Args...> {
          using type = Head<Helper_t<TemplateList<Tail...>, RBC, Args...>, Args...>;
        };

      public:
        // RBC: Root Base Class.
        template<typename RBC, typename... Args>
        using type = Helper_t<TopoSort_t<TemplateSet<NVB, NVBs...>, TemplateSet<VBs...>>, RBC, Args...>;
      };

      template<template<typename...> class NVB, template<typename...> class... NVBs>
      struct LI_t {
        template<typename RBC, typename... Args>
        using type = typename LI<TemplateSet<NVB, NVBs...>>::template type<RBC, Args...>;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
