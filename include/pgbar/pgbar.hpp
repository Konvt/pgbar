// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2025 Konvt
#pragma once

#ifndef __KONVT_PGBAR
# define __KONVT_PGBAR

# include <algorithm>
# include <array>
# include <atomic>
# include <bitset>
# include <chrono>
# include <cmath>
# include <condition_variable>
# include <cstdint>
# include <exception>
# include <initializer_list>
# include <iterator>
# include <limits>
# include <memory>
# include <mutex>
# include <string>
# include <thread>
# include <tuple>
# include <type_traits>
# include <utility>
# include <vector>

# if defined( _MSC_VER ) && defined( _MSVC_LANG ) // for msvc
#  define __PGBAR_CC_STD _MSVC_LANG
# else
#  define __PGBAR_CC_STD __cplusplus
# endif

# if defined( _MSC_VER )
#  define __PGBAR_INLINE_FN __forceinline
# elif defined( __GNUC__ ) || defined( __clang__ )
#  define __PGBAR_INLINE_FN __attribute__( ( always_inline ) ) inline
# else
#  define __PGBAR_INLINE_FN inline
# endif

# if defined( _WIN32 ) || defined( _WIN64 )
#  ifndef NOMINMAX
#   define NOMINMAX
#  endif
#  include <windows.h>
#  define __PGBAR_WIN     1
#  define __PGBAR_UNIX    0
#  define __PGBAR_UNKNOWN 0
# elif defined( __unix__ )
#  include <unistd.h>
#  define __PGBAR_WIN     0
#  define __PGBAR_UNIX    1
#  define __PGBAR_UNKNOWN 0
# else
#  include <iostream>
#  define __PGBAR_WIN     0
#  define __PGBAR_UNIX    0
#  define __PGBAR_UNKNOWN 1
# endif

# if __PGBAR_CC_STD >= 202302L
#  include <functional>
#  define __PGBAR_CXX23          1
#  define __PGBAR_CXX23_CNSTXPR  constexpr
#  define __PGBAR_UNREACHABLE    std::unreachable()
#  define __PGBAR_ASSUME( expr ) [[assume( expr )]]
# else
#  define __PGBAR_CXX23 0
#  define __PGBAR_CXX23_CNSTXPR
#  if defined( _MSC_VER )
#   define __PGBAR_UNREACHABLE    __assume( 0 )
#   define __PGBAR_ASSUME( expr ) __assume( expr )
#  elif defined( __clang__ )
#   define __PGBAR_UNREACHABLE    __builtin_unreachable()
#   define __PGBAR_ASSUME( expr ) __builtin_assume( expr )
#  elif defined( __GNUC__ )
#   define __PGBAR_UNREACHABLE __builtin_unreachable()
#   define __PGBAR_ASSUME( expr ) \
     do {                         \
       if ( expr ) {              \
       } else {                   \
         __builtin_unreachable(); \
       }                          \
     } while ( false )
#  else
#   define __PGBAR_UNREACHABLE __PGBAR_ASSERT( false )
#   define __PGBAR_ASSUME( expr )
#  endif
# endif
# if __PGBAR_CC_STD >= 202002L
#  include <concepts>
#  define __PGBAR_CXX20         1
#  define __PGBAR_NOUNIQUEADDR  [[no_unique_address]]
#  define __PGBAR_UNLIKELY      [[unlikely]]
#  define __PGBAR_CXX20_CNSTXPR constexpr
#  define __PGBAR_CNSTEVAL      consteval
# else
#  define __PGBAR_CXX20 0
#  define __PGBAR_NOUNIQUEADDR
#  define __PGBAR_UNLIKELY
#  define __PGBAR_CXX20_CNSTXPR
#  define __PGBAR_CNSTEVAL constexpr
# endif
# if __PGBAR_CC_STD >= 201703L
#  include <charconv>
#  include <string_view>
#  define __PGBAR_CXX17           1
#  define __PGBAR_CXX17_CNSTXPR   constexpr
#  define __PGBAR_CXX17_INLINE    inline
#  define __PGBAR_FALLTHROUGH     [[fallthrough]]
#  define __PGBAR_NODISCARD       [[nodiscard]]
#  define __PGBAR_LAUNDER( expr ) std::launder( expr )
# else
#  define __PGBAR_CXX17 0
#  define __PGBAR_CXX17_CNSTXPR
#  define __PGBAR_CXX17_INLINE
#  define __PGBAR_FALLTHROUGH
#  define __PGBAR_LAUNDER( expr ) expr
#  if defined( _MSC_VER )
#   define __PGBAR_NODISCARD _Check_return_
#  elif defined( __clang__ ) || defined( __GNUC__ )
#   define __PGBAR_NODISCARD __attribute__( ( warn_unused_result ) )
#  else
#   define __PGBAR_NODISCARD
#  endif
# endif
# if __PGBAR_CC_STD >= 201402L
#  include <shared_mutex>
#  define __PGBAR_CXX14         1
#  define __PGBAR_CXX14_CNSTXPR constexpr
# else
#  define __PGBAR_CXX14 0
#  define __PGBAR_CXX14_CNSTXPR
# endif
# if __PGBAR_CC_STD >= 201103L
#  define __PGBAR_CXX11 1
# else
#  error "The library 'pgbar' requires C++11"
# endif

# ifdef PGBAR_DEBUG
#  include <cassert>
#  define __PGBAR_ASSERT( expr ) assert( expr )
#  undef __PGBAR_INLINE_FN
#  define __PGBAR_INLINE_FN
# else
#  define __PGBAR_ASSERT( expr )
# endif

// For assertion conditions without side effects.
# define __PGBAR_PURE_ASSUME( expr ) \
   do {                              \
     __PGBAR_ASSERT( expr );         \
     __PGBAR_ASSUME( expr );         \
   } while ( false )

# define __PGBAR_DEFAULT 0xC105EA11 // C1O5E -> ClOSE, A11 -> All
# define __PGBAR_BLACK   0x000000
# define __PGBAR_RED     0xFF0000
# define __PGBAR_GREEN   0x00FF00
# define __PGBAR_YELLOW  0xFFFF00
# define __PGBAR_BLUE    0x0000FF
# define __PGBAR_MAGENTA 0x800080
# define __PGBAR_CYAN    0x00FFFF
# define __PGBAR_WHITE   0xFFFFFF

namespace pgbar {
  namespace exception {
    /**
     * The base exception class.
     *
     * It should only takes the literal strings, otherwise it isn't well-defined.
     */
    class Error : public std::exception {
    protected:
      const char* message_;

    public:
      template<std::size_t N>
      Error( const char ( &mes )[N] ) noexcept : message_ { mes }
      {}
      virtual ~Error() = default;
      virtual const char* what() const noexcept { return message_; }
    };

    // Exception for invalid function arguments.
    class InvalidArgument : public Error {
    public:
      using Error::Error;
      virtual ~InvalidArgument() = default;
    };

    // Exception for error state of object.
    class InvalidState : public Error {
    public:
      using Error::Error;
      virtual ~InvalidState() = default;
    };

    // Exception for local system error.
    class SystemError : public Error {
    public:
      using Error::Error;
      virtual ~SystemError() = default;
    };
  } // namespace exception

  namespace __details {
    namespace types {
      using Size   = std::size_t;
      using String = std::string;
      using Char   = char;
# if __PGBAR_CXX17
      using ROStr  = std::string_view;
      using LitStr = ROStr; // literal strings
# else
      using ROStr  = typename std::add_lvalue_reference<typename std::add_const<String>::type>::type;
      using LitStr = typename std::add_pointer<typename std::add_const<Char>::type>::type;
# endif
      // a constant string type
      using ConstStr   = typename std::add_const<typename std::decay<ROStr>::type>::type;
      using HexRGB     = std::uint32_t;
      using UCodePoint = char32_t; // Unicode code point
      using Float      = double;
      using TimeUnit   = std::chrono::nanoseconds;
      using Byte       = std::uint8_t;
    } // namespace types

    namespace constants {
      __PGBAR_CXX17_INLINE constexpr types::Char blank                   = ' ';
      __PGBAR_CXX17_INLINE __PGBAR_CXX20_CNSTXPR types::ConstStr nil_str = "";
    }

    namespace traits {
# if __PGBAR_CXX14
      template<types::Size... Ns>
      using IndexSeq = std::integer_sequence<types::Size, Ns...>;

      template<types::Size N>
      using MakeIndexSeq = std::make_index_sequence<N>;
# else
      template<types::Size... Ns>
      struct IndexSeq {};

      // This is an internal implementation and should not be used outside of this preprocessing block.
      template<typename HeadSeq, typename TailSeq>
      struct ConcatSeq;
      template<typename HeadSeq, typename TailSeq>
      using ConcatSeq_t = typename ConcatSeq<HeadSeq, TailSeq>::type;

      template<types::Size... HeadI, types::Size... TailI>
      struct ConcatSeq<IndexSeq<HeadI...>, IndexSeq<TailI...>> {
        using type = IndexSeq<HeadI..., ( sizeof...( HeadI ) + TailI )...>;
      };

      // Internal implementation, it should not be used outside of this preprocessing block.
      template<types::Size N>
      struct MakeIndexSeqHelper {
        using type =
          ConcatSeq_t<typename MakeIndexSeqHelper<N / 2>::type, typename MakeIndexSeqHelper<N - N / 2>::type>;
      };
      template<>
      struct MakeIndexSeqHelper<0> {
        using type = IndexSeq<>;
      };
      template<>
      struct MakeIndexSeqHelper<1> {
        using type = IndexSeq<0>;
      };

      template<types::Size N>
      using MakeIndexSeq = typename MakeIndexSeqHelper<N>::type;
# endif

# if __PGBAR_CXX17
      template<typename... Preds>
      using AllOf = std::conjunction<Preds...>;

      template<typename... Preds>
      using AnyOf = std::disjunction<Preds...>;

      template<typename Pred>
      using Not = std::negation<Pred>;
# else
      template<typename... Preds>
      struct AllOf : std::true_type {};
      template<typename Pred>
      struct AllOf<Pred> : Pred {};
      template<typename Pred, typename... Preds>
      struct AllOf<Pred, Preds...> : std::conditional<bool( Pred::value ), AllOf<Preds...>, Pred>::type {};

      template<typename... Preds>
      struct AnyOf : std::false_type {};
      template<typename Pred>
      struct AnyOf<Pred> : Pred {};
      template<typename Pred, typename... Preds>
      struct AnyOf<Pred, Preds...> : std::conditional<bool( Pred::value ), Pred, AnyOf<Preds...>>::type {};

      template<typename Pred>
      struct Not : std::integral_constant<bool, !bool( Pred::value )> {};
# endif

      template<typename T, template<typename...> class Template, types::Size N>
      struct FillTemplate {
      private:
        template<typename U, template<typename...> class Tmp, std::size_t M, typename... Us>
        struct Helper : Helper<U, Tmp, M - 1, Us..., U> {};
        template<typename U, template<typename...> class Tmp, typename... Us>
        struct Helper<U, Tmp, 0, Us...> {
          using type = Tmp<Us...>;
        };

      public:
        using type = typename Helper<T, Template, N>::type;
      };
      template<typename T, template<typename...> class Template, types::Size N>
      using FillTemplate_t = typename FillTemplate<T, Template, N>::type;

      template<types::Size Pos, typename... Ts>
      struct TypeAt {
        static_assert( static_cast<bool>( Pos ^ Pos ),
                       "pgbar::__details::traits::TypeAt: Position overflow" );
      };
      template<types::Size Pos, typename... Ts>
      using TypeAt_t = typename TypeAt<Pos, Ts...>::type;
      // After C++26, we can use `Ts...[Pos]`.

      template<types::Size Pos, typename T, typename... Ts>
      struct TypeAt<Pos, T, Ts...> : TypeAt<Pos - 1, Ts...> {};
      template<typename T, typename... Ts>
      struct TypeAt<0, T, Ts...> {
        using type = T;
      };

      /**
       * A lightweight tuple type that stores multiple types.
       *
       * `std::tuple` puts some constraints on the input type that are not metaprogramming related,
       * so here is a lightweight tuple type that is used only for template type parameter passing.
       */
      template<typename... Ts>
      struct TypeList;

      // Checks if a TypeList starts with the specified type sequence.
      template<typename TpList, typename... Ts>
      struct StartsWith;
      template<typename... Ts>
      struct StartsWith<TypeList<>, Ts...> : std::true_type {};
      template<typename Head, typename... Tail>
      struct StartsWith<TypeList<Head, Tail...>> : std::false_type {};
      template<typename Head, typename... Tail, typename T, typename... Ts>
      struct StartsWith<TypeList<Head, Tail...>, T, Ts...>
        : std::conditional<std::is_same<Head, T>::value,
                           StartsWith<TypeList<Tail...>, Ts...>,
                           std::false_type>::type {};

      template<typename HeadTpList, typename TailTpList>
      struct Join;
      template<typename HeadTpList, typename TailTpList>
      using Join_t = typename Join<HeadTpList, TailTpList>::type;

      template<typename... Head, typename... Tail>
      struct Join<TypeList<Head...>, TypeList<Tail...>> {
        using type = TypeList<Head..., Tail...>;
      };

      template<typename HeadTpList, typename... TailTpList>
      struct Merge : Join<HeadTpList, typename Merge<TailTpList...>::type> {};
      template<typename HeadTpList, typename... TailTpList>
      using Merge_t = typename Merge<HeadTpList, TailTpList...>::type;

      template<typename... Ts>
      struct Merge<TypeList<Ts...>> {
        using type = TypeList<Ts...>;
      };

      // Check whether type `T` belongs to the list `TpList`.
      template<typename T, typename TpList>
      struct Belong;
      template<typename T>
      struct Belong<T, TypeList<>> : std::false_type {};
      template<typename T, typename Head, typename... Tail>
      struct Belong<T, TypeList<Head, Tail...>>
        : std::conditional<std::is_same<T, Head>::value, std::true_type, Belong<T, TypeList<Tail...>>>::type {
      };

      // Check whether type `T` belongs to any of the type lists.
      template<typename T, typename TpList, typename... TpLists>
      struct BelongAny
        : std::conditional<Belong<T, TpList>::value, std::true_type, BelongAny<T, TpLists...>>::type {};
      template<typename T, typename G>
      struct BelongAny<T, G> : Belong<T, G> {};

      // Check whether all types in `TpList` belong to any of the type lists `Lists`.
      template<typename TpList, typename... Lists>
      struct AllBelongAny;
      /**
       * Vacuous truth case: the empty set belongs any collection.
       * Universal quantification over empty set is always true.
       */
      template<typename... Gs>
      struct AllBelongAny<TypeList<>, Gs...> : std::true_type {};
      template<typename T, typename... Ts>
      struct AllBelongAny<TypeList<T, Ts...>> : std::false_type {};
      template<typename T, typename G, typename... Gs>
      struct AllBelongAny<TypeList<T>, G, Gs...> : BelongAny<T, G, Gs...> {};
      template<typename T, typename... Ts, typename G, typename... Gs>
      struct AllBelongAny<TypeList<T, Ts...>, G, Gs...>
        : std::conditional<BelongAny<T, G, Gs...>::value,
                           AllBelongAny<TypeList<Ts...>, G, Gs...>,
                           std::false_type>::type {};

      // Check whether the list contains duplicate types.
      template<typename TpList>
      struct Repeat;
      template<>
      struct Repeat<TypeList<>> : std::false_type {};
      template<typename Head, typename... Tail>
      struct Repeat<TypeList<Head, Tail...>>
        : std::conditional<Belong<Head, TypeList<Tail...>>::value,
                           std::true_type,
                           Repeat<TypeList<Tail...>>>::type {};

      // A kind of `std::is_same` that applies to template class types.
      template<template<typename...> class T, template<typename...> class U>
      struct Equal : std::false_type {};
      template<template<typename...> class T>
      struct Equal<T, T> : std::true_type {};

      // A lightweight tuple type that stores multiple template class types.
      template<template<typename...> class... Ts>
      struct TemplateList;

      // Insert a new template type into the head of the TemplateList.
      template<typename TmpList, template<typename...> class T>
      struct Prepend;
      // Get the result of the template `Prepend`.
      template<typename TmpList, template<typename...> class T>
      using Prepend_t = typename Prepend<TmpList, T>::type;

      template<template<typename...> class... Ts, template<typename...> class T>
      struct Prepend<TemplateList<Ts...>, T> {
        using type = TemplateList<T, Ts...>;
      };

      // Check whether a TemplateList contains given template `T`.
      template<typename TmpList, template<typename...> class T>
      struct Contain;
      template<template<typename...> class T>
      struct Contain<TemplateList<>, T> : std::false_type {};
      template<template<typename...> class Head,
               template<typename...>
               class... Tail,
               template<typename...>
               class T>
      struct Contain<TemplateList<Head, Tail...>, T>
        : std::conditional<Equal<Head, T>::value, std::true_type, Contain<TemplateList<Tail...>, T>>::type {};

      /**
       * A template class that records the inheritance structure of a template class.
       *
       * To record the inheritance structure of a template class,
       * you need to manually provide a specialized version of the template class type
       * that holds both the VBs and NBs types.
       */
      template<template<typename...> class Node>
      struct InheritFrom {
        using VBs  = TemplateList<>; // virtual base
        using NVBs = TemplateList<>; // non-virtual base
      };

      // Gets the virtual base list of the template class `Node`.
      template<template<typename...> class Node>
      using InheritFrom_vbt = typename InheritFrom<Node>::VBs;
      // Gets the non-virtual base (also called normal base) list of the template class `Node`.
      template<template<typename...> class Node>
      using InheritFrom_nvbt = typename InheritFrom<Node>::NVBs;

      // Pack multiple macro parameters into a single one.
# define __PGBAR_PACK( ... ) __VA_ARGS__
// A helper macro to register the inheritance structure of a template class.
# define __PGBAR_INHERIT_REGISTER( Node, VBList, NVBList ) \
   template<>                                              \
   struct InheritFrom<Node> {                              \
     using VBs  = TemplateList<VBList>;                    \
     using NVBs = TemplateList<NVBList>;                   \
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
       * NVBList: Non-virtual Base List, VBList: Virtual Base List
       */
      template<typename NVBList, typename VBList = TemplateList<>>
      struct TopoSort;

      // NVB: Non-virtual Base, VB: Virtual Base
      template<template<typename...> class NVB,
               template<typename...>
               class... NVBs,
               template<typename...>
               class... VBs>
      struct TopoSort<TemplateList<NVB, NVBs...>, TemplateList<VBs...>> {
      private:
        // VI: Virtual Inherit
        template<bool VI, typename List, typename SortedList, typename VisitedVBs>
        struct Helper;
        /* Return the virtual base class node that was accessed during the recursive process.
         * pt: path type. */
        template<bool VI, typename List, typename SortedList, typename VisitedVBs>
        using Helper_pt = typename Helper<VI, List, SortedList, VisitedVBs>::path;
        /* Return the sorted list.
         * tp: type. */
        template<bool VI, typename List, typename SortedList, typename VisitedVBs>
        using Helper_tp = typename Helper<VI, List, SortedList, VisitedVBs>::type;

        template<bool VI, typename SortedList, typename VisitedVBs>
        struct Helper<VI, TemplateList<>, SortedList, VisitedVBs> {
          using path = VisitedVBs;
          using type = SortedList;
        };
        template<template<typename...> class Head,
                 template<typename...>
                 class... Tail,
                 typename SortedList,
                 typename VisitedVBs>
        struct Helper<true, TemplateList<Head, Tail...>, SortedList, VisitedVBs> {
        private:
          using SortVB_t = Helper_tp<true, InheritFrom_vbt<Head>, SortedList, VisitedVBs>;
          using MarkVB_t = Helper_pt<true, InheritFrom_vbt<Head>, SortedList, VisitedVBs>;

          using SortTail_t = Helper_tp<true, TemplateList<Tail...>, SortVB_t, MarkVB_t>;
          using MarkTail_t = Helper_pt<true, TemplateList<Tail...>, SortVB_t, MarkVB_t>;

          using SortNVB_t = Helper_tp<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;
          using MarkNVB_t = Helper_pt<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;

        public:
          using path = Prepend_t<MarkNVB_t, Head>;
          using type =
            typename std::conditional<AnyOf<Contain<VisitedVBs, Head>, Contain<MarkTail_t, Head>>::value,
                                      Helper_tp<true, TemplateList<Tail...>, SortedList, VisitedVBs>,
                                      Prepend_t<SortNVB_t, Head>>::type;
        };
        template<template<typename...> class Head,
                 template<typename...>
                 class... Tail,
                 typename SortedList,
                 typename VisitedVBs>
        struct Helper<false, TemplateList<Head, Tail...>, SortedList, VisitedVBs> {
        private:
          using SortVB_t = Helper_tp<true, InheritFrom_vbt<Head>, SortedList, VisitedVBs>;
          using MarkVB_t = Helper_pt<true, InheritFrom_vbt<Head>, SortedList, VisitedVBs>;

          using SortTail_t = Helper_tp<false, TemplateList<Tail...>, SortVB_t, MarkVB_t>;
          using MarkTail_t = Helper_pt<false, TemplateList<Tail...>, SortVB_t, MarkVB_t>;

          using SortNVB_t = Helper_tp<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;
          using MarkNVB_t = Helper_pt<false, InheritFrom_nvbt<Head>, SortTail_t, MarkTail_t>;

        public:
          using path = MarkNVB_t;
          using type = Prepend_t<SortNVB_t, Head>;
        };

      public:
        using type = Helper_tp<false,
                               TemplateList<NVB, NVBs...>,
                               Helper_tp<true, TemplateList<VBs...>, TemplateList<>, TemplateList<>>,
                               Helper_pt<true, TemplateList<VBs...>, TemplateList<>, TemplateList<>>>;
      };
      // Get a list of topological sorting results for the input template classes.
      template<typename NVBList, typename VBList = TemplateList<>>
      using TopoSort_t = typename TopoSort<NVBList, VBList>::type;

      /**
       * Linearization of Inheritance.
       *
       * Topologically sort the input types,
       * then iterate through the resulting sorted list and fill in their template types.
       *
       * It relies on the template `InheritFrom` and `TopoSort` classes to work.
       */
      template<typename NVBList, typename VBList = TemplateList<>>
      struct LI;

      template<template<typename...> class NVB,
               template<typename...>
               class... NVBs,
               template<typename...>
               class... VBs>
      struct LI<TemplateList<NVB, NVBs...>, TemplateList<VBs...>> {
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
                 template<typename...>
                 class... Tail,
                 typename RBC,
                 typename... Args>
        struct Helper<TemplateList<Head, Tail...>, RBC, Args...> {
          using type = Head<Helper_t<TemplateList<Tail...>, RBC, Args...>, Args...>;
        };

      public:
        // RBC: Root Base Class
        template<typename RBC, typename... Args>
        using type = Helper_t<TopoSort_t<TemplateList<NVB, NVBs...>, TemplateList<VBs...>>, RBC, Args...>;
      };

      template<template<typename...> class NVB, template<typename...> class... NVBs>
      struct LI_t {
        template<typename RBC, typename... Args>
        using type = typename LI<TemplateList<NVB, NVBs...>>::template type<RBC, Args...>;
      };

      /**
       * Check whether the type `T` has a type `iterator` or `const_iterator`, and return it if affirmative.
       * Otherwise, return the type `T` itself.
       */
      template<typename T, typename = void>
      struct IteratorTrait {
        using type = typename std::conditional<std::is_array<typename std::remove_reference<T>::type>::value,
                                               typename std::add_pointer<typename std::remove_extent<
                                                 typename std::remove_reference<T>::type>::type>::type,
                                               typename std::remove_reference<T>::type>::type;
      };
      // Get the result type of `IteratorTrait`.
      template<typename T>
      using IteratorTrait_t = typename IteratorTrait<T>::type;

      template<typename T>
      struct IteratorTrait<T,
                           typename std::enable_if<AllOf<
                             Not<std::is_const<typename std::remove_reference<T>::type>>,
                             std::is_same<typename std::remove_reference<T>::type::iterator,
                                          typename std::remove_reference<T>::type::iterator>>::value>::type> {
        using type = typename std::remove_reference<T>::type::iterator;
      };
      template<typename T>
      struct IteratorTrait<
        T,
        typename std::enable_if<
          AllOf<std::is_const<typename std::remove_reference<T>::type>,
                std::is_same<typename std::remove_reference<T>::type::const_iterator,
                             typename std::remove_reference<T>::type::const_iterator>>::value>::type> {
        using type = typename std::remove_reference<T>::type::const_iterator;
      };

      template<typename... Signature>
      struct RemoveNoexcept;
      template<typename... Signature>
      using RemoveNoexcept_t = typename RemoveNoexcept<Signature...>::type;

# define __PGBAR_REMOVE_NOEXCEPT( Qualifier, Noexcept )       \
   template<typename Ret, typename... Args>                   \
   struct RemoveNoexcept<Ret( Args... ) Qualifier Noexcept> { \
     using type = Ret( Args... ) Qualifier;                   \
   }
      __PGBAR_REMOVE_NOEXCEPT(, );
      __PGBAR_REMOVE_NOEXCEPT( &, );
      __PGBAR_REMOVE_NOEXCEPT( &&, );
      __PGBAR_REMOVE_NOEXCEPT( const&, );
      __PGBAR_REMOVE_NOEXCEPT( const&&, );
# if __PGBAR_CXX17
      __PGBAR_REMOVE_NOEXCEPT(, noexcept );
      __PGBAR_REMOVE_NOEXCEPT( &, noexcept );
      __PGBAR_REMOVE_NOEXCEPT( &&, noexcept );
      __PGBAR_REMOVE_NOEXCEPT( const, noexcept );
      __PGBAR_REMOVE_NOEXCEPT( const&, noexcept );
      __PGBAR_REMOVE_NOEXCEPT( const&&, noexcept );
# endif
# undef __PGBAR_REMOVE_NOEXCEPT

# if __PGBAR_CXX20
      template<typename M>
      concept MutexLike = requires( M& mtx ) {
        { mtx.lock() } -> std::same_as<void>;
        { mtx.unlock() } -> std::same_as<void>;
      };
      template<typename M>
      struct is_mutex : std::bool_constant<MutexLike<M>> {};
# else
      template<typename, typename = void>
      struct is_mutex : std::false_type {};
      template<typename M>
      struct is_mutex<
        M,
        typename std::enable_if<AllOf<Not<std::is_reference<M>>,
                                      std::is_void<decltype( std::declval<M&>().lock() )>,
                                      std::is_void<decltype( std::declval<M&>().unlock() )>>::value>::type>
        : std::true_type {};
# endif
    } // namespace traits

    namespace wrappers {
      template<typename I>
      class IterSpanBase {
        static_assert( traits::AnyOf<std::is_class<I>, std::is_pointer<I>>::value,
                       "pgbar::__details::wrappers::IterSpanBase: Only available for iterator types" );
        static_assert( !std::is_void<typename std::iterator_traits<I>::difference_type>::value,
                       "pgbar::__details::wrappers::IterSpanBase: The 'difference_type' of the given "
                       "iterators cannot be 'void'" );
        static_assert(
          traits::AnyOf<std::is_copy_constructible<I>, std::is_move_constructible<I>>::value,
          "pgbar::__details::wrappers::IterSpanBase: The given iterators must be copyable or movable " );

        // Measure the length of the iteration range.
        __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR types::Size measure() const noexcept
        {
          const auto length = std::distance( start_, end_ );
          if __PGBAR_CXX17_CNSTXPR ( std::is_pointer<I>::value )
            return length >= 0 ? length : -length;
          else
            return length;
        }

      protected:
        I start_, end_;
        types::Size size_;

      public:
        __PGBAR_CXX17_CNSTXPR IterSpanBase( I startpoint, I endpoint )
          noexcept( std::is_nothrow_move_constructible<I>::value )
          : start_ { std::move( startpoint ) }, end_ { std::move( endpoint ) }, size_ { 0 }
        {
          size_ = measure();
        }
        __PGBAR_CXX17_CNSTXPR IterSpanBase( const IterSpanBase& )              = default;
        __PGBAR_CXX17_CNSTXPR IterSpanBase( IterSpanBase&& )                   = default;
        __PGBAR_CXX17_CNSTXPR IterSpanBase& operator=( const IterSpanBase& ) & = default;
        __PGBAR_CXX17_CNSTXPR IterSpanBase& operator=( IterSpanBase&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR virtual ~IterSpanBase()                          = 0;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR I& start_iter() & noexcept { return start_; }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR const I& start_iter() const& noexcept { return start_; }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR I start_iter() && noexcept { return std::move( start_ ); }

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR I& end_iter() & noexcept { return end_; }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR const I& end_iter() const& noexcept { return end_; }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR I end_iter() && noexcept { return std::move( end_ ); }

        __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR IterSpanBase& start_iter( I startpoint )
          noexcept( std::is_nothrow_move_assignable<I>::value )
        {
          start_ = std::move( startpoint );
          size_  = measure();
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR IterSpanBase& end_iter( I endpoint )
          noexcept( std::is_nothrow_move_constructible<I>::value )
        {
          end_  = std::move( endpoint );
          size_ = measure();
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size step() const noexcept { return 1; }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size size() const noexcept { return size_; }

        __PGBAR_CXX20_CNSTXPR void swap( IterSpanBase<I>& lhs ) noexcept
        {
          __PGBAR_PURE_ASSUME( this != &lhs );
          using std::swap;
          swap( start_, lhs.start_ );
          swap( end_, lhs.end_ );
          swap( size_, lhs.size_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( IterSpanBase<I>& a, IterSpanBase<I>& b ) noexcept
        {
          a.swap( b );
        }
      };
      template<typename I>
      __PGBAR_CXX20_CNSTXPR IterSpanBase<I>::~IterSpanBase() = default;

# if __PGBAR_CXX23
      template<typename... Signature>
      using UniqueFunction = std::move_only_function<Signature...>;
# else
      template<typename... Signature>
      class UniqueFnCore;

      template<typename Ret, typename... Args>
      class UniqueFnCore<Ret( Args... )> final {
        using Self = UniqueFnCore;

        struct AnyFn {
          __PGBAR_CXX20_CNSTXPR virtual ~AnyFn()       = default;
          virtual Ret operator()( Args... args ) const = 0;
          virtual void move_to( void* const dest )     = 0;
        };
        template<typename Fn, typename = void>
        struct FnContainer final : public AnyFn {
        private:
          static_assert( !std::is_reference<Fn>::value,
                         "pgbar::__details::wrappers::UniqueFnCore: Incomplete type" );

          mutable Fn fntor_;

        public:
          __PGBAR_CXX14_CNSTXPR FnContainer( Fn fntor )
            noexcept( std::is_nothrow_move_constructible<Fn>::value )
            : fntor_ { std::move( fntor ) }
          {}

          FnContainer( const FnContainer& )              = delete;
          FnContainer& operator=( const FnContainer& ) & = delete;
          __PGBAR_CXX20_CNSTXPR virtual ~FnContainer()   = default;

          Ret operator()( Args... args ) const
            noexcept( noexcept( std::declval<Fn>()( std::forward<Args>( args )... ) ) ) override
          {
            return fntor_( std::forward<Args>( args )... );
          }
          void move_to( void* const dest ) noexcept override
          {
            __PGBAR_PURE_ASSUME( dest != nullptr );
            // After C++26, placement new is constexpr.
            new ( dest ) FnContainer( std::move( fntor_ ) );
          }
        };

#  if __PGBAR_CXX14
        template<typename Fn> // EBO
        struct FnContainer<Fn, std::enable_if_t<std::is_empty<Fn>::value && !std::is_final<Fn>::value>> final
          : public AnyFn
          , private Fn {
          __PGBAR_CXX14_CNSTXPR FnContainer( Fn fntor )
            noexcept( std::is_nothrow_move_constructible<Fn>::value )
            : Fn( std::move( fntor ) )
          {}

          FnContainer( const FnContainer& )              = delete;
          FnContainer& operator=( const FnContainer& ) & = delete;
          __PGBAR_CXX20_CNSTXPR virtual ~FnContainer()   = default;

          Ret operator()( Args... args ) const
            noexcept( noexcept( std::declval<Fn>()( std::forward<Args>( args )... ) ) ) override
          {
            return ( ( static_cast<Fn&>( const_cast<FnContainer&>( *this ) ) ) )(
              std::forward<Args>( args )... );
          }
          void move_to( void* const dest ) noexcept override
          {
            __PGBAR_PURE_ASSUME( dest != nullptr );
            new ( dest ) FnContainer( std::move( static_cast<Fn&>( *this ) ) );
          }
        };
#  endif

        union {
          typename std::add_pointer<Ret( Args... )>::type fptr_;
          AnyFn* ftor_;
          unsigned char soo_[sizeof( void* ) * 2]; // small object optimization
        } data_;
        static_assert( sizeof data_ == sizeof data_.soo_,
                       "pgbar::__details::wrappers::UniqueFnCore: Unexpected type size mismatch" );
        enum class Tag : types::Byte { None, Fptr, FtorInline, FtorDync } tag_;

      public:
        constexpr UniqueFnCore() noexcept : data_ {}, tag_ { Tag::None } {}
        constexpr UniqueFnCore( std::nullptr_t ) noexcept : UniqueFnCore() {}

        template<typename Fn,
                 typename = typename std::enable_if<traits::AllOf<
                   std::is_function<Fn>,
                   std::is_convertible<decltype( ( *std::declval<Fn>() )( std::declval<Args>()... ) ),
                                       Ret>>::value>::type>
        __PGBAR_CXX14_CNSTXPR UniqueFnCore( Fn* function_ptr ) noexcept : UniqueFnCore()
        {
          data_.fptr_ = function_ptr;
          tag_        = Tag::Fptr;
        }
        template<typename Fn,
                 typename = typename std::enable_if<traits::AllOf<
                   std::is_class<Fn>,
                   std::is_convertible<decltype( ( std::declval<Fn>() )( std::declval<Args>()... ) ),
                                       Ret>>::value>::type>
        __PGBAR_CXX20_CNSTXPR UniqueFnCore( Fn functor ) : UniqueFnCore()
        {
          static_assert( std::is_move_constructible<Fn>::value,
                         "pgbar::__details::wrappers::UniqueFnCore: Invalid type" );

          if __PGBAR_CXX17_CNSTXPR ( sizeof( FnContainer<Fn> ) <= sizeof data_.soo_ ) {
            const auto fn = new ( &data_.soo_ ) FnContainer<Fn>( std::move( functor ) );
            __PGBAR_PURE_ASSUME( static_cast<void*>( fn ) == static_cast<void*>( &data_.soo_ ) );
            tag_ = Tag::FtorInline;
          } else {
            data_.ftor_ = new FnContainer<Fn>( std::move( functor ) );
            tag_        = Tag::FtorDync;
          }
        }

        __PGBAR_CXX20_CNSTXPR UniqueFnCore( Self&& rhs ) noexcept : UniqueFnCore()
        {
          operator=( std::move( rhs ) );
        }
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_PURE_ASSUME( this != &rhs );
          swap( rhs );
          rhs = nullptr; // exclusive semantics
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR Self& operator=( std::nullptr_t ) & noexcept
        {
          switch ( tag_ ) {
          case Tag::FtorInline: __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &data_.soo_ ) )->~AnyFn(); break;
          case Tag::FtorDync:   delete data_.ftor_; break;
          default:              break;
          }
          tag_ = Tag::None;
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR ~UniqueFnCore() noexcept { ( *this ) = nullptr; }

        __PGBAR_CXX20_CNSTXPR Ret operator()( Args... args ) const
        {
          __PGBAR_PURE_ASSUME( tag_ != Tag::None );
          switch ( tag_ ) {
          case Tag::Fptr: return ( *data_.fptr_ )( std::forward<Args>( args )... );
          case Tag::FtorInline:
            return ( *__PGBAR_LAUNDER( reinterpret_cast<const AnyFn*>( &data_.soo_ ) ) )(
              std::forward<Args>( args )... );
          case Tag::FtorDync: return ( *data_.ftor_ )( std::forward<Args>( args )... );
          default:            break;
          }
          __PGBAR_UNREACHABLE;
        }

        void swap( Self& lhs ) noexcept
        {
          switch ( tag_ ) {
          case Tag::FtorInline: {
            switch ( lhs.tag_ ) {
            case Tag::Fptr:     __PGBAR_FALLTHROUGH;
            case Tag::FtorDync: {
              const auto lhs_fn  = lhs.data_;
              const auto self_fn = __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &data_.soo_ ) );
              self_fn->move_to( &lhs.data_ );
              self_fn->~AnyFn();
              data_ = lhs_fn;
            } break;
            case Tag::FtorInline: {
              alignas( decltype( data_.soo_ ) ) unsigned char buffer[sizeof data_.soo_] {};
              auto self_fn = __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &data_.soo_ ) );
              self_fn->move_to( &buffer );
              self_fn->~AnyFn();

              const auto lhs_fn = __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &lhs.data_.soo_ ) );
              lhs_fn->move_to( &data_.soo_ );
              lhs_fn->~AnyFn();

              self_fn = __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &buffer ) );
              self_fn->move_to( &lhs.data_.soo_ );
              self_fn->~AnyFn();
            } break;
            case Tag::None: {
              const auto self_fn = __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &data_.soo_ ) );
              self_fn->move_to( &lhs.data_ );
              self_fn->~AnyFn();
            } break;
            default: __PGBAR_UNREACHABLE;
            }
          } break;

          default: {
            if ( lhs.tag_ == Tag::FtorInline ) {
              const auto self_fn = data_;
              const auto lhs_fn  = __PGBAR_LAUNDER( reinterpret_cast<AnyFn*>( &lhs.data_.soo_ ) );
              lhs_fn->move_to( &data_.soo_ );
              lhs_fn->~AnyFn();
              lhs.data_ = self_fn;
            } else
              std::swap( data_, lhs.data_ );
          } break;
          }
          std::swap( tag_, lhs.tag_ );
        }
        friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN friend constexpr bool operator==( const Self& a,
                                                                              std::nullptr_t ) noexcept
        {
          return !static_cast<bool>( a );
        }
#  if !__PGBAR_CXX20
        __PGBAR_NODISCARD __PGBAR_INLINE_FN friend constexpr bool operator!=( const Self& a,
                                                                              std::nullptr_t ) noexcept
        {
          return static_cast<bool>( a );
        }
#  endif

        explicit operator bool() const noexcept { return tag_ != Tag::None; }
      };

      // A simplified implementation of std::move_only_function
      template<typename... Signature>
      using UniqueFunction = UniqueFnCore<traits::RemoveNoexcept_t<Signature...>>;
# endif
    } // namespace wrappers
  } // namespace __details

  namespace scope {
    /**
     * An undirectional range delimited by an numeric interval [start, end).
     *
     * The `end` can be less than the `start` only if the `step` is negative,
     * otherwise it throws exception `pgbar::exception::InvalidArgument`.
     */
    template<typename N>
    class NumericSpan {
      static_assert( std::is_arithmetic<N>::value,
                     "pgbar::scope::NumericSpan: Only available for arithmetic types" );

      N start_, end_, step_;

    public:
      class iterator final {
        N itr_start_, itr_step_;
        __details::types::Size itr_cnt_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = N;
        using difference_type   = value_type;
        using pointer           = void;
        using reference         = value_type;

        constexpr iterator( N startpoint, N step, __details::types::Size iterated = 0 ) noexcept
          : itr_start_ { startpoint }, itr_step_ { step }, itr_cnt_ { iterated }
        {}
        constexpr iterator() noexcept : iterator( {}, {}, {} ) {}
        constexpr iterator( const iterator& )                          = default;
        __PGBAR_CXX14_CNSTXPR iterator& operator=( const iterator& ) & = default;
        __PGBAR_CXX20_CNSTXPR ~iterator()                              = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() & noexcept
        {
          ++itr_cnt_;
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) & noexcept
        {
          auto before = *this;
          operator++();
          return before;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator+=( value_type increment ) noexcept
        {
          itr_cnt_ += increment > 0 ? static_cast<__details::types::Size>( increment / itr_step_ ) : 0;
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr reference operator*() const noexcept
        {
          return static_cast<reference>( static_cast<__details::types::Size>( itr_start_ )
                                         + itr_cnt_ * static_cast<__details::types::Size>( itr_step_ ) );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==( value_type num ) const noexcept
        {
          return operator*() == num;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=( value_type num ) const noexcept
        {
          return !( operator==( num ) );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ == b.itr_cnt_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( a == b );
        }
      };

      constexpr NumericSpan() noexcept : start_ {}, end_ {}, step_ { 1 } {}
      /**
       * @throw exception::InvalidArgument
       *
       * If the `startpoint` is greater than `endpoint` while `step` is positive,
       * or the `startpoint` is less than `endpoint` while `step` is negative.
       */
      __PGBAR_CXX20_CNSTXPR NumericSpan( N startpoint, N endpoint, N step ) noexcept( false ) : NumericSpan()
      {
        if ( step > 0 && startpoint > endpoint )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else if ( step < 0 && startpoint < endpoint )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        if ( step == 0 )
          __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: 'step' is zero" );

        start_ = startpoint;
        step_  = step;
        end_   = endpoint;
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the `startpoint` is greater than `endpoint`.
       */
      __PGBAR_CXX20_CNSTXPR NumericSpan( N startpoint, N endpoint ) : NumericSpan( startpoint, endpoint, 1 )
      {}
      /**
       * @throw exception::InvalidArgument
       *
       * If the `endpoint` is less than zero.
       */
      __PGBAR_CXX20_CNSTXPR explicit NumericSpan( N endpoint ) : NumericSpan( {}, endpoint, 1 ) {}
      constexpr NumericSpan( const NumericSpan& )                          = default;
      __PGBAR_CXX14_CNSTXPR NumericSpan& operator=( const NumericSpan& ) & = default;
      __PGBAR_CXX20_CNSTXPR virtual ~NumericSpan()                         = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const noexcept
      {
        return iterator( start_, step_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR iterator end() const noexcept
      {
        return iterator( start_, step_, size() );
      }

      /**
       * @throw exception::InvalidArgument
       *
       * If the `start_value` is greater than `end_value` while `step` is positive,
       * or the `start_value` is less than `end_value` while `step` is negative,
       * or the `step` is zero.
       */
      __PGBAR_CXX20_CNSTXPR NumericSpan& step( N step ) noexcept( false )
      {
        if ( step < 0 && start_ < end_ )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else if ( step > 0 && start_ > end_ )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else if ( step == 0 )
          __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: 'step' is zero" );

        step_ = step;
        return *this;
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the `startpoint` is greater than `end_value` while `step` is positive,
       * or the `startpoint` is less than `end_value` while `step` is negative.
       */
      __PGBAR_CXX20_CNSTXPR NumericSpan& start_value( N startpoint ) noexcept( false )
      {
        if ( step_ < 0 && startpoint < end_ )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else if ( step_ > 0 && startpoint > end_ )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is less than 'start' while 'step' is positive" );

        start_ = startpoint;
        return *this;
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the `start_value` is greater than `endpoint` while `step` is positive,
       * or the `start_value` is less than `endpoint` while `step` is negative.
       */
      __PGBAR_CXX20_CNSTXPR NumericSpan& end_value( N endpoint ) noexcept( false )
      {
        if ( step_ < 0 && start_ < endpoint )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else if ( step_ > 0 && start_ > endpoint )
          __PGBAR_UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is less than 'start' while 'step' is positive" );

        end_ = endpoint;
        return *this;
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N start_value() const noexcept { return start_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N end_value() const noexcept { return end_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N step() const noexcept { return step_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR __details::types::Size size() const noexcept
      {
        if __PGBAR_CXX17_CNSTXPR ( std::is_integral<N>::value )
          return ( ( end_ - start_ + step_ ) - 1 ) / step_;
        else
          return static_cast<__details::types::Size>( std::ceil( ( end_ - start_ ) / step_ ) );
      }

      __PGBAR_CXX14_CNSTXPR void swap( NumericSpan<N>& lhs ) noexcept
      {
        __PGBAR_PURE_ASSUME( this != &lhs );
        using std::swap;
        swap( start_, lhs.start_ );
        swap( end_, lhs.end_ );
        swap( step_, lhs.step_ );
      }
      friend __PGBAR_CXX14_CNSTXPR void swap( NumericSpan<N>& a, NumericSpan<N>& b ) noexcept { a.swap( b ); }
    };

    /**
     * An undirectional range delimited by a pair of iterators, including pointer types.
     *
     * When the type of iterator is pointer, it can figure out whether the iterator is reversed,
     * and iterate it normally.
     *
     * Accepted iterator types must satisfy subtractable.
     */
    template<typename I>
    class IterSpan : public __details::wrappers::IterSpanBase<I> {
      static_assert( !std::is_pointer<I>::value,
                     "pgbar::scope::IterSpan<I>: Only available for iterator types" );

    public:
      class iterator final {
        I current_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = typename std::iterator_traits<I>::value_type;
        using difference_type   = void;
        using pointer           = typename std::iterator_traits<I>::pointer;
        using reference         = typename std::iterator_traits<I>::reference;

        constexpr explicit iterator( I startpoint ) noexcept( std::is_nothrow_move_constructible<I>::value )
          : current_ { std::move( startpoint ) }
        {}
        constexpr iterator( const iterator& )                          = default;
        constexpr iterator( iterator&& )                               = default;
        __PGBAR_CXX14_CNSTXPR iterator& operator=( const iterator& ) & = default;
        __PGBAR_CXX14_CNSTXPR iterator& operator=( iterator&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR ~iterator()                              = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() & noexcept(
          noexcept( ++std::declval<I>() ) )
        {
          ++current_;
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) & noexcept(
          std::is_nothrow_copy_constructible<iterator>::value && noexcept( ++std::declval<I>() ) )
        {
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR reference operator*() noexcept
        {
          return *current_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR pointer operator->() noexcept
        {
          return std::addressof( current_ );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==( const I& lhs ) const noexcept
        {
          return current_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=( const I& lhs ) const noexcept
        {
          return !operator==( lhs );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return a.current_ == b.current_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( a == b );
        }
      };

      using __details::wrappers::IterSpanBase<I>::IterSpanBase;
      __PGBAR_CXX17_CNSTXPR IterSpan( const IterSpan& )              = default;
      __PGBAR_CXX17_CNSTXPR IterSpan( IterSpan&& )                   = default;
      __PGBAR_CXX17_CNSTXPR IterSpan& operator=( const IterSpan& ) & = default;
      __PGBAR_CXX17_CNSTXPR IterSpan& operator=( IterSpan&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR virtual ~IterSpan()                      = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const
        noexcept( __details::traits::AllOf<std::is_nothrow_move_constructible<I>,
                                           std::is_nothrow_copy_constructible<I>>::value )
      {
        return iterator( this->start_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator end() const
        noexcept( __details::traits::AllOf<std::is_nothrow_move_constructible<I>,
                                           std::is_nothrow_copy_constructible<I>>::value )
      {
        return iterator( this->end_ );
      }
    };
    template<typename P>
    class IterSpan<P*> : public __details::wrappers::IterSpanBase<P*> {
      static_assert( std::is_pointer<P*>::value,
                     "pgbar::scope::IterSpan<P*>: Only available for pointer types" );

    public:
      class iterator final {
      public:
        P* current_;
        bool reversed_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = P;
        using difference_type   = void;
        using pointer           = P*;
        using reference         = typename std::add_lvalue_reference<value_type>::type;

        __PGBAR_CXX14_CNSTXPR iterator( P* startpoint, P* endpoint ) noexcept
          : current_ { startpoint }, reversed_ { false }
        {
          __PGBAR_PURE_ASSUME( startpoint != nullptr );
          __PGBAR_PURE_ASSUME( endpoint != nullptr );
          reversed_ = endpoint < startpoint;
        }
        __PGBAR_CXX14_CNSTXPR iterator( const iterator& )              = default;
        __PGBAR_CXX14_CNSTXPR iterator& operator=( const iterator& ) & = default;
        __PGBAR_CXX20_CNSTXPR ~iterator()                              = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() & noexcept
        {
          if ( reversed_ )
            --current_;
          else
            ++current_;
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) noexcept
        {
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR reference operator*() noexcept
        {
          return *current_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR pointer operator->() noexcept
        {
          return std::addressof( current_ );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==( const P* lhs ) const noexcept
        {
          return current_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=( const P* lhs ) const noexcept
        {
          return !operator==( lhs );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return a.current_ == b.current_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( a == b );
        }
      };

      /**
       * @throw exception::InvalidArgument
       * If the `startpoint` or the `endpoint` is null pointer.
       */
      __PGBAR_CXX20_CNSTXPR IterSpan( P* startpoint, P* endpoint ) noexcept( false )
        : __details::wrappers::IterSpanBase<P*>( startpoint, endpoint )
      {
        if ( startpoint == nullptr || endpoint == nullptr )
          __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: null pointer cannot generate a range" );
      }
      __PGBAR_CXX20_CNSTXPR IterSpan( const IterSpan& )              = default;
      __PGBAR_CXX20_CNSTXPR IterSpan& operator=( const IterSpan& ) & = default;
      __PGBAR_CXX20_CNSTXPR virtual ~IterSpan()                      = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const noexcept
      {
        return iterator( this->start_, this->end_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator end() const noexcept
      {
        return iterator( this->end_, this->end_ );
      }
    };

    template<typename, typename>
    class ProxySpan;
  } // namespace scope

  // A enum that specifies the type of the output stream.
  enum class Channel : __details::types::Byte { Stdout = 1, Stderr };

  class Indicator {
  protected:
    std::chrono::steady_clock::time_point zero_point_;
    bool final_mesg_;

    enum class State : __details::types::Byte { Begin, StrictRefresh, LenientRefresh, Finish, Stopped };
    std::atomic<State> state_;

  public:
    Indicator() noexcept : state_ { State::Stopped } {}
    Indicator( Indicator&& rhs ) noexcept : Indicator()
    {
      __PGBAR_ASSERT( rhs.is_running() == false );
      ( (void)rhs );
    }
    Indicator& operator=( Indicator&& rhs ) & noexcept
    {
      __PGBAR_PURE_ASSUME( this != &rhs );
      __PGBAR_ASSERT( is_running() == false );
      __PGBAR_ASSERT( rhs.is_running() == false );
      return *this;
    }

    virtual ~Indicator() = default;

    virtual void tick() &                                   = 0;
    virtual void tick( __details::types::Size next_step ) & = 0;
    virtual void tick_to( std::uint8_t percentage ) &       = 0;
    virtual void reset( bool final_mesg = true )            = 0;

    // Wait until the indicator is stopped.
    void wait() const noexcept
    {
      while ( is_running() )
        std::this_thread::yield();
    }
    // Wait for the indicator is stopped or timed out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& time_duration ) const noexcept
    {
      for ( const auto ending = std::chrono::steady_clock::now() + time_duration;
            std::chrono::steady_clock::now() < ending; ) {
        if ( !is_running() )
          return true;
        std::this_thread::yield();
      }
      return false;
    }

    __PGBAR_NODISCARD bool is_running() const noexcept
    {
      return state_.load( std::memory_order_acquire ) != State::Stopped;
    }
  };

  namespace __details {
    namespace traits {
      template<typename T>
      struct is_scope {
      private:
        template<typename N>
        static std::true_type check( const scope::NumericSpan<N>& );
        template<typename I>
        static std::true_type check( const scope::IterSpan<I>& );
        static std::false_type check( ... );

      public:
        static constexpr bool value =
          std::conditional<std::is_reference<T>::value,
                           std::false_type,
                           decltype( check(
                             std::declval<typename std::remove_volatile<T>::type>() ) )>::type::value;
      };
    } // namespace traits

    namespace utils {
      template<typename E>
      __PGBAR_NODISCARD __PGBAR_CNSTEVAL __PGBAR_INLINE_FN
        typename std::enable_if<std::is_enum<E>::value, typename std::underlying_type<E>::type>::type
        as_val( E enum_val ) noexcept
      {
        return static_cast<typename std::underlying_type<E>::type>( enum_val );
      }

      // Perfectly forwards the I-th element of a tuple.
      template<types::Size I, typename Tuple>
      constexpr auto forward_get( Tuple&& tup ) noexcept
        -> decltype( std::forward<
                     typename std::tuple_element<I, typename std::remove_reference<Tuple>::type>::type>(
          std::get<I>( std::forward<Tuple>( tup ) ) ) )
      {
        return std::forward<
          typename std::tuple_element<I, typename std::remove_reference<Tuple>::type>::type>(
          std::get<I>( std::forward<Tuple>( tup ) ) );
      }

      // Internal implementation.
      template<types::Size I, typename T, typename Tuple>
      constexpr T forward_or( Tuple&& tup, const std::true_type& ) noexcept(
        std::is_nothrow_constructible<T, decltype( forward_get<I>( std::forward<Tuple>( tup ) ) )>::value )
      {
        return T( forward_get<I>( std::forward<Tuple>( tup ) ) );
      }
      // Internal implementation.
      template<types::Size, typename T, typename Tuple>
      constexpr T forward_or( Tuple&&, const std::false_type& )
        noexcept( std::is_nothrow_default_constructible<T>::value )
      {
        return T();
      }
      // Conditionally constructs an object of type T using either the I-th element of a tuple or default
      // initialization.
      template<types::Size I, typename T, typename Tuple>
      constexpr typename std::enable_if<std::is_default_constructible<T>::value, T>::type forward_or(
        Tuple&& tup )
      {
        return forward_or<I, T>(
          std::forward<Tuple>( tup ),
          std::integral_constant<bool, ( I < std::tuple_size<typename std::decay<Tuple>::type>::value )>() );
      }

      template<typename Numeric>
      __PGBAR_NODISCARD __PGBAR_CXX23_CNSTXPR __PGBAR_INLINE_FN typename std::enable_if<
        traits::AllOf<std::is_arithmetic<Numeric>, traits::Not<std::is_unsigned<Numeric>>>::value,
        types::Size>::type
        count_digits( Numeric val ) noexcept
      {
        auto abs_val       = static_cast<std::uint64_t>( std::abs( val ) );
        types::Size digits = abs_val == 0;
        for ( ; abs_val > 0; abs_val /= 10 )
          ++digits;
        return digits;
      }
      template<typename Unsigned>
      __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR __PGBAR_INLINE_FN
        typename std::enable_if<std::is_unsigned<Unsigned>::value, types::Size>::type
        count_digits( Unsigned val ) noexcept
      {
        auto abs_val       = static_cast<std::uint64_t>( val );
        types::Size digits = abs_val == 0;
        for ( ; abs_val > 0; abs_val /= 10 )
          ++digits;
        return digits;
      }

      // Format an integer number.
      template<typename Integer>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN
        typename std::enable_if<std::is_integral<Integer>::value, types::String>::type
        format( Integer val ) noexcept( noexcept( std::to_string( val ) ) )
      {
        /* In some well-designed standard libraries,
         * integer std::to_string has specialized implementations for different bit-length types;

         * This includes directly constructing the destination string using the SOO/SSO nature of the string,
         * optimizing the memory management strategy using internally private resize_and_overwrite, etc.

         * Therefore, the functions of the standard library are called directly here,
         * rather than providing a manual implementation like the other functions. */
        return std::to_string( val );
        // Although, unfortunately, std::to_string is not labeled constexpr.
      }

      // Format a finite floating point number.
      template<typename Floating>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN
        typename std::enable_if<std::is_floating_point<Floating>::value, types::String>::type
        format( Floating val, int precision ) noexcept( false )
      {
        /* Unlike the integer version,
         * the std::to_string in the standard library does not provide a precision limit
         * on floating-point numbers;

         * So the implementation here is provided manually. */
        __PGBAR_ASSERT( std::isfinite( val ) );
        __PGBAR_PURE_ASSUME( precision >= 0 );
# if __PGBAR_CXX17
        const auto abs_rounded_val = std::round( std::abs( val ) );
        const auto int_digits      = count_digits( abs_rounded_val );

        types::String formatted;
#  if __PGBAR_CXX23
        formatted.resize_and_overwrite(
          int_digits + precision + 2,
          [val, precision]( types::Char* buf, types::Size n ) noexcept {
            const auto result = std::to_chars( buf, buf + n, val, std::chars_format::fixed, precision );
            __PGBAR_PURE_ASSUME( result.ec == std::errc {} );
            __PGBAR_PURE_ASSUME( result.ptr >= buf );
            return static_cast<types::Size>( result.ptr - buf );
          } );
#  else
        formatted.resize( int_digits + precision + 2 );
        // The extra 2 is left for the decimal point and carry.
        const auto result = std::to_chars( formatted.data(),
                                           formatted.data() + formatted.size(),
                                           val,
                                           std::chars_format::fixed,
                                           precision );
        __PGBAR_PURE_ASSUME( result.ec == std::errc {} );
        __PGBAR_ASSERT( result.ptr >= formatted.data() );
        formatted.resize( result.ptr - formatted.data() );
#  endif
# else
        const auto scale           = std::pow( 10, precision );
        const auto abs_rounded_val = std::round( std::abs( val ) * scale ) / scale;
        __PGBAR_ASSERT( abs_rounded_val <= std::numeric_limits<std::uint64_t>::max() );
        const auto integer = static_cast<std::uint64_t>( abs_rounded_val );
        const auto fraction =
          static_cast<std::uint64_t>( std::round( ( abs_rounded_val - integer ) * scale ) );
        const auto sign = std::signbit( val );

        auto formatted = types::String( sign, '-' );
        formatted.append( format( integer ) ).reserve( count_digits( integer ) + sign );
        if ( precision > 0 ) {
          formatted.push_back( '.' );
          const auto fract_digits = count_digits( fraction );
          __PGBAR_PURE_ASSUME( fract_digits <= static_cast<types::Size>( precision ) );
          formatted.append( precision - fract_digits, '0' ).append( format( fraction ) );
        }
# endif
        return formatted;
      }

      /**
       * Converts RGB color strings to hexidecimal values.
       *
       * Always returns 0 if defined `PGBAR_COLORLESS`.
       *
       * @throw exception::InvalidArgument
       * If the size of RGB color string is not 7 or 4, and doesn't begin with character `#`.
       */
      __PGBAR_CXX20_CNSTXPR types::HexRGB hex2rgb( types::ROStr hex ) noexcept( false )
      {
        if ( ( hex.size() != 7 && hex.size() != 4 ) || hex.front() != '#' )
          throw exception::InvalidArgument( "pgbar: invalid hex color format" );

        for ( types::Size i = 1; i < hex.size(); i++ ) {
          if ( ( hex[i] < '0' || hex[i] > '9' ) && ( hex[i] < 'A' || hex[i] > 'F' )
               && ( hex[i] < 'a' || hex[i] > 'f' ) )
            throw exception::InvalidArgument( "pgbar: invalid hexadecimal letter" );
        }

# ifdef PGBAR_COLORLESS
        return {};
# else
        std::uint32_t ret = 0;
        if ( hex.size() == 4 ) {
          for ( types::Size i = 1; i < hex.size(); ++i ) {
            ret <<= 4;
            if ( hex[i] >= '0' && hex[i] <= '9' )
              ret = ( ( ret | ( hex[i] - '0' ) ) << 4 ) | ( hex[i] - '0' );
            else if ( hex[i] >= 'A' && hex[i] <= 'F' )
              ret = ( ( ret | ( hex[i] - 'A' + 10 ) ) << 4 ) | ( hex[i] - 'A' + 10 );
            else // no need to check whether it's valid or not
              ret = ( ( ret | ( hex[i] - 'a' + 10 ) ) << 4 ) | ( hex[i] - 'a' + 10 );
          }
        } else {
          for ( types::Size i = 1; i < hex.size(); ++i ) {
            ret <<= 4;
            if ( hex[i] >= '0' && hex[i] <= '9' )
              ret |= hex[i] - '0';
            else if ( hex[i] >= 'A' && hex[i] <= 'F' )
              ret |= hex[i] - 'A' + 10;
            else
              ret |= hex[i] - 'a' + 10;
          }
        }
        return ret;
# endif
      }

      enum class TxtLayout { Left, Right, Center }; // text layout
      // Format the `str`.
      template<TxtLayout Style>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String format( types::Size width,
                                                                                      types::Size len_str,
                                                                                      types::ROStr str )
        noexcept( false )
      {
        if ( width == 0 )
          __PGBAR_UNLIKELY return {};
        if ( len_str >= width )
          return types::String( str );
        if __PGBAR_CXX17_CNSTXPR ( Style == TxtLayout::Right ) {
          auto tmp = types::String( width - len_str, constants::blank );
          tmp.append( str );
          return tmp;
        } else if __PGBAR_CXX17_CNSTXPR ( Style == TxtLayout::Left ) {
          auto tmp = types::String( str );
          tmp.append( width - len_str, constants::blank );
          return tmp;
        } else {
          width -= len_str;
          const types::Size l_blank = width / 2;
          return std::move( types::String( l_blank, constants::blank ).append( str ) )
               + types::String( width - l_blank, constants::blank );
        }
      }
      template<TxtLayout Style>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String format( types::Size width,
                                                                                      types::ROStr __str )
        noexcept( false )
      {
        return format<Style>( width, __str.size(), __str );
      }
    } // namespace utils

    namespace console {
      // escape codes
      namespace escodes {
# ifdef PGBAR_COLORLESS
        constexpr types::LitStr reset_font = "";
        constexpr types::LitStr bold_font  = "";
# else
        constexpr types::LitStr reset_font = "\x1B[0m";
        constexpr types::LitStr bold_font  = "\x1B[1m";
# endif
        constexpr types::LitStr store_cursor   = "\x1B[s";
        constexpr types::LitStr restore_cursor = "\x1B[u";
        constexpr types::LitStr clear_suffix   = "\x1B[K";

        // Assembles an ANSI escape code that clears `__n` characters after the cursor.
        __PGBAR_INLINE_FN types::String clear_next( types::Size __n = 1 )
        {
          return "\x1B[" + utils::format( __n ) + 'X';
        }

        /**
         * Convert a hexidecimal RGB color value to an ANSI escape code.
         *
         * Return nothing if defined `PGBAR_COLORLESS`.
         */
        types::String rgb2ansi( types::HexRGB rgb )
# ifdef PGBAR_COLORLESS
          noexcept( std::is_nothrow_default_constructible<types::String>::value )
        {
          return {};
        }
# else
          noexcept( false )
        {
          if ( rgb == __PGBAR_DEFAULT )
            return types::String( escodes::reset_font );

          switch ( rgb & 0x00FFFFFF ) { // discard the high 8 bits
          case __PGBAR_BLACK:   return "\x1B[30m";
          case __PGBAR_RED:     return "\x1B[31m";
          case __PGBAR_GREEN:   return "\x1B[32m";
          case __PGBAR_YELLOW:  return "\x1B[33m";
          case __PGBAR_BLUE:    return "\x1B[34m";
          case __PGBAR_MAGENTA: return "\x1B[35m";
          case __PGBAR_CYAN:    return "\x1B[36m";
          case __PGBAR_WHITE:   return "\x1B[37m";
          default:
            return "\x1B[38;2;" + utils::format( ( rgb >> 16 ) & 0xFF ) + ';'
                 + utils::format( ( rgb >> 8 ) & 0xFF ) + ';' + utils::format( rgb & 0xFF ) + 'm';
          }
        }
# endif
      } // namespace escodes

      /**
       * Determine if the output stream is binded to the tty based on the platform api.
       *
       * Always returns true if defined `PGBAR_INTTY`,
       * or the local platform is neither `Windows` nor `unix-like`.
       */
      __PGBAR_NODISCARD bool intty( Channel outlet ) noexcept
      {
# if defined( PGBAR_INTTY ) || __PGBAR_UNKNOWN
        return true;
# elif __PGBAR_WIN
        HANDLE stream_handle;
        if ( outlet == Channel::Stdout )
          stream_handle = GetStdHandle( STD_OUTPUT_HANDLE );
        else
          stream_handle = GetStdHandle( STD_ERROR_HANDLE );
        if ( stream_handle == INVALID_HANDLE_VALUE )
          __PGBAR_UNLIKELY return false;
        return GetFileType( stream_handle ) == FILE_TYPE_CHAR;

# else
        if ( outlet == Channel::Stdout )
          return isatty( STDOUT_FILENO );
        else
          return isatty( STDERR_FILENO );
# endif
      }
    } // namespace console

    namespace charcodes {
      // A type of wrapper that stores the mapping between Unicode code chart and character width.
      class CodeChart final {
        types::UCodePoint start_, end_;
        types::Size width_;

      public:
        constexpr CodeChart( types::UCodePoint start, types::UCodePoint end, types::Size width ) noexcept
          : start_ { start }, end_ { end }, width_ { width }
        {          // This is an internal component, so we assume the arguments are always valid.
# if __PGBAR_CXX14 // C++11 requires the constexpr ctor should have an empty function body.
          __PGBAR_PURE_ASSUME( start_ <= end_ );
# endif
        }
        constexpr CodeChart( const CodeChart& )                          = default;
        __PGBAR_CXX14_CNSTXPR CodeChart& operator=( const CodeChart& ) & = default;
        __PGBAR_CXX20_CNSTXPR ~CodeChart()                               = default;

        // Check whether the Unicode code point is within this code chart.
        __PGBAR_NODISCARD constexpr bool contains( types::UCodePoint codepoint ) const noexcept
        {
          return start_ <= codepoint && codepoint <= end_;
        }
        // Return the character width of this Unicode code chart.
        __PGBAR_NODISCARD constexpr types::Size width() const noexcept { return width_; }
        // Return the size of this range of Unicode code chart.
        __PGBAR_NODISCARD constexpr types::UCodePoint size() const noexcept { return end_ - start_ + 1; }
        // Return the start Unicode code point of this code chart.
        __PGBAR_NODISCARD constexpr types::UCodePoint head() const noexcept { return start_; }
        // Return the end Unicode code point of this code chart.
        __PGBAR_NODISCARD constexpr types::UCodePoint tail() const noexcept { return end_; }

        __PGBAR_NODISCARD friend constexpr bool operator<( const CodeChart& a, const CodeChart& b ) noexcept
        {
          return a.end_ < b.start_;
        }
        __PGBAR_NODISCARD friend constexpr bool operator>( const CodeChart& a, const CodeChart& b ) noexcept
        {
          return a.start_ > b.end_;
        }
        __PGBAR_NODISCARD friend constexpr bool operator>( const CodeChart& a,
                                                           const types::UCodePoint& b ) noexcept
        {
          return a.start_ > b;
        }
        __PGBAR_NODISCARD friend constexpr bool operator<( const CodeChart& a,
                                                           const types::UCodePoint& b ) noexcept
        {
          return a.end_ < b;
        }
      };

      // A simple UTF-8 string implementation.
      class U8String final {
        using Self = U8String;

        types::Size width_;
        std::string bytes_;

      public:
        __PGBAR_NODISCARD static __PGBAR_CNSTEVAL __PGBAR_INLINE_FN std::array<CodeChart, 47>
          code_charts() noexcept
        {
          // See the Unicode CodeCharts documentation for complete code points.
          // Also can see the `if-else` version in misc/UTF-8-test.cpp
          return {
            { { 0x0, 0x19, 0 },        { 0x20, 0x7E, 1 },        { 0x7F, 0xA0, 0 },
             { 0xA1, 0xAC, 1 },       { 0xAD, 0xAD, 0 },        { 0xAE, 0x2FF, 1 },
             { 0x300, 0x36F, 0 },     { 0x370, 0x1FFF, 1 },     { 0x2000, 0x200F, 0 },
             { 0x2010, 0x2010, 1 },   { 0x2011, 0x2011, 0 },    { 0x2012, 0x2027, 1 },
             { 0x2028, 0x202F, 0 },   { 0x2030, 0x205E, 1 },    { 0x205F, 0x206F, 0 },
             { 0x2070, 0x2E7F, 1 },   { 0x2E80, 0xA4CF, 2 },    { 0xA4D0, 0xA95F, 1 },
             { 0xA960, 0xA97F, 2 },   { 0xA980, 0xABFF, 1 },    { 0xAC00, 0xD7FF, 2 },
             { 0xE000, 0xF8FF, 2 },   { 0xF900, 0xFAFF, 2 },    { 0xFB00, 0xFDCF, 1 },
             { 0xFDD0, 0xFDEF, 0 },   { 0xFDF0, 0xFDFF, 1 },    { 0xFE00, 0xFE0F, 0 },
             { 0xFE10, 0xFE1F, 2 },   { 0xFE20, 0xFE2F, 0 },    { 0xFE30, 0xFE6F, 2 },
             { 0xFE70, 0xFEFE, 1 },   { 0xFEFF, 0xFEFF, 0 },    { 0xFF00, 0xFF60, 2 },
             { 0xFF61, 0xFFDF, 1 },   { 0xFFE0, 0xFFE6, 2 },    { 0xFFE7, 0xFFEF, 1 },
             { 0xFFF0, 0xFFFF, 1 },   { 0x10000, 0x1F8FF, 2 },  { 0x1F900, 0x1FBFF, 3 },
             { 0x1FF80, 0x1FFFF, 0 }, { 0x20000, 0x3FFFD, 2 },  { 0x3FFFE, 0x3FFFF, 0 },
             { 0xE0000, 0xE007F, 0 }, { 0xE0100, 0xE01EF, 0 },  { 0xEFF80, 0xEFFFF, 0 },
             { 0xFFF80, 0xFFFFF, 2 }, { 0x10FF80, 0x10FFFF, 2 } }
          };
        }
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size char_width(
          types::UCodePoint codepoint ) noexcept
        {
          constexpr auto charts = code_charts();
          __PGBAR_ASSERT( std::is_sorted( charts.cbegin(), charts.cend() ) );
          // Compare with the `if-else` version, here we can search for code points with O(logn).
          const auto itr = std::lower_bound( charts.cbegin(), charts.cend(), codepoint );
          if ( itr != charts.cend() && itr->contains( codepoint ) )
            return itr->width();

          return 1; // Default fallback
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the parameter `u8_str` isn't a valid UTF-8 string.
         *
         * @return Returns the render width of the given string.
         */
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size render_width(
          types::ROStr u8_str )
        {
          types::Size width = 0;
          for ( types::Size i = 0; i < u8_str.size(); ) {
            const auto raw_u8_str  = u8_str.data();
            const auto start_point = raw_u8_str + i;
            // After RFC 3629, the maximum length of each standard UTF-8 character is 4 bytes.
            const auto first_byte  = static_cast<types::UCodePoint>( *start_point );
            auto validator         = [start_point, raw_u8_str, &u8_str]( types::Size expected_len ) -> void {
              __PGBAR_PURE_ASSUME( start_point >= raw_u8_str );
              if ( u8_str.size() - ( start_point - raw_u8_str ) < expected_len )
                throw exception::InvalidArgument( "pgbar: incomplete UTF-8 string" );

              for ( types::Size i = 1; i < expected_len; ++i )
                if ( ( start_point[i] & 0xC0 ) != 0x80 )
                  throw exception::InvalidArgument( "pgbar: broken UTF-8 character" );
            };

            types::UCodePoint utf_codepoint = {};
            if ( ( first_byte & 0x80 ) == 0 ) {
              utf_codepoint = first_byte;
              i += 1;
            } else if ( ( ( first_byte & 0xE0 ) == 0xC0 ) ) {
              validator( 2 );
              utf_codepoint =
                ( ( first_byte & 0x1F ) << 6 ) | ( static_cast<types::UCodePoint>( start_point[1] ) & 0x3F );
              i += 2;
            } else if ( ( first_byte & 0xF0 ) == 0xE0 ) {
              validator( 3 );
              utf_codepoint = ( ( first_byte & 0xF ) << 12 )
                            | ( ( static_cast<types::UCodePoint>( start_point[1] ) & 0x3F ) << 6 )
                            | ( static_cast<types::UCodePoint>( start_point[2] ) & 0x3F );
              i += 3;
            } else if ( ( first_byte & 0xF8 ) == 0xF0 ) {
              validator( 4 );
              utf_codepoint = ( ( first_byte & 0x7 ) << 18 )
                            | ( ( static_cast<types::UCodePoint>( start_point[1] ) & 0x3F ) << 12 )
                            | ( ( static_cast<types::UCodePoint>( start_point[2] ) & 0x3F ) << 6 )
                            | ( static_cast<types::UCodePoint>( start_point[3] ) & 0x3F );
              i += 4;
            } else
              throw exception::InvalidArgument( "pgbar: not a standard UTF-8 string" );

            width += char_width( utf_codepoint );
          }
          return width;
        }

        __PGBAR_CXX20_CNSTXPR U8String()
          noexcept( std::is_nothrow_default_constructible<types::String>::value )
          : width_ { 0 }
        {}
        __PGBAR_CXX20_CNSTXPR explicit U8String( types::String u8_bytes ) : U8String()
        {
          width_ = render_width( u8_bytes );
          bytes_ = std::move( u8_bytes );
        }
        __PGBAR_CXX20_CNSTXPR U8String( const Self& )          = default;
        __PGBAR_CXX20_CNSTXPR U8String( Self&& )               = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& ) & = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR ~U8String()                      = default;

        Self& operator=( types::ROStr u8_bytes ) &
        {
          auto new_width = render_width( u8_bytes );
          auto new_bytes = types::String( u8_bytes );
          std::swap( width_, new_width );
          bytes_.swap( new_bytes );
          return *this;
        }
        Self& operator=( types::String u8_bytes ) &
        {
          auto new_width = render_width( u8_bytes );
          std::swap( width_, new_width );
          bytes_.swap( u8_bytes );
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_CXX20_CNSTXPR bool empty() const noexcept { return bytes_.empty(); }
        __PGBAR_CXX20_CNSTXPR types::Size size() const noexcept { return width_; }
        __PGBAR_CXX20_CNSTXPR types::ROStr str() const noexcept { return bytes_; }

        __PGBAR_CXX20_CNSTXPR void clear() noexcept
        {
          bytes_.clear();
          width_ = 0;
        }
        __PGBAR_CXX20_CNSTXPR void shrink_to_fit() noexcept( noexcept( bytes_.shrink_to_fit() ) )
        { // The standard does not seem to specify whether the function is noexcept,
          // so let's make a judgment here.
          // At least I didn't see it on cppreference.
          bytes_.shrink_to_fit();
        }

        __PGBAR_CXX20_CNSTXPR void swap( Self& lhs ) noexcept
        {
          std::swap( width_, lhs.width_ );
          bytes_.swap( lhs.bytes_ );
        }
        __PGBAR_CXX20_CNSTXPR friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        __PGBAR_CXX20_CNSTXPR explicit operator types::String() & { return bytes_; }
        __PGBAR_CXX20_CNSTXPR explicit operator types::String() const& { return bytes_; }
        __PGBAR_CXX20_CNSTXPR explicit operator types::String() && noexcept { return std::move( bytes_ ); }
        __PGBAR_CXX20_CNSTXPR operator types::ROStr() const noexcept { return str(); }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self operator+( types::String&& a,
                                                                                         const Self& b )
        {
          // Ensure strong exception safety.
          const auto new_width = U8String::render_width( a );
          a.append( b.bytes_ );
          auto ret   = U8String();
          ret.bytes_ = std::move( a );
          ret.width_ = new_width + b.width_;
          return ret;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self
          operator+( const types::String& a, const Self& b )
        {
          auto tmp = types::String( a );
          tmp.append( b.bytes_ );
          return U8String( std::move( tmp ) );
        }
        template<std::size_t N>
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self operator+( const char ( &a )[N],
                                                                                         const Self& b )
        {
          auto tmp = types::String( a );
          tmp.append( b.bytes_ );
          return U8String( std::move( tmp ) );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self operator+( const char* a,
                                                                                         const Self& b )
        {
          auto tmp = types::String( a );
          tmp.append( b.bytes_ );
          return U8String( std::move( tmp ) );
        }
# if __PGBAR_CXX20
        static_assert( sizeof( char8_t ) == sizeof( char ),
                       "pgbar::__details::chaset::U8String: Unexpected type size mismatch" );

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN Self operator+( std::u8string_view a, const Self& b )
        {
          auto tmp = Self( a );
          tmp.bytes_.append( b.bytes_ );
          return U8String( std::move( tmp ) );
        }
# endif
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self operator+( const Self& a,
                                                                                         types::ROStr b )
        {
          auto tmp = a.bytes_;
          tmp.append( b );
          return U8String( std::move( tmp ) );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self operator+( Self&& a,
                                                                                         types::ROStr b )
        {
          const auto new_width = U8String::render_width( b );
          a.bytes_.append( b );
          a.width_ += new_width;
          return a;
        }

# if __PGBAR_CXX20
        explicit U8String( std::u8string_view u8_sv ) : U8String()
        {
          bytes_.resize( u8_sv.size() );
          std::copy( u8_sv.cbegin(), u8_sv.cend(), bytes_.begin() );
          width_ = render_width( bytes_ );
        }

        explicit operator std::u8string() const
        {
          std::u8string ret;
          ret.resize( bytes_.size() );
          std::copy( bytes_.cbegin(), bytes_.cend(), ret.begin() );
          return ret;
        }
# endif
      };
    } // namespace charcodes

    namespace utils {
      template<TxtLayout Style>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String format(
        types::Size width,
        const charcodes::U8String& __str ) noexcept( false )
      {
        return format<Style>( width, __str.size(), __str.str() );
      }
    } // namespace utils

    namespace io {
      // A simple string buffer, unrelated to the `std::stringbuf` in the STL.
      class Stringbuf {
        using Self = Stringbuf;

      protected:
        std::vector<types::Char> buffer_;

      public:
        __PGBAR_CXX20_CNSTXPR Stringbuf() = default;

        __PGBAR_CXX20_CNSTXPR Stringbuf( const Self& lhs ) { operator=( lhs ); }
        __PGBAR_CXX20_CNSTXPR Stringbuf( Self&& rhs ) noexcept : Stringbuf()
        {
          operator=( std::move( rhs ) );
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& lhs ) &
        {
          __PGBAR_PURE_ASSUME( this != &lhs );
          buffer_ = lhs.buffer_;
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_PURE_ASSUME( this != &rhs );
          swap( rhs );
          rhs.buffer_.clear();
          rhs.buffer_.shrink_to_fit();
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR virtual ~Stringbuf() = default;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR bool empty() const noexcept { return buffer_.empty(); }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void clear() & noexcept { buffer_.clear(); }

        // Releases the buffer space completely
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void release() noexcept
        {
          clear();
          buffer_.shrink_to_fit();
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& reserve( types::Size capacity ) &
        {
          buffer_.reserve( capacity );
          return *this;
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( types::Char info, types::Size __num = 1 ) &
        {
          buffer_.insert( buffer_.end(), __num, info );
          return *this;
        }
        template<types::Size N>
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const char ( &info )[N],
                                                              types::Size __num = 1 ) &
        {
          for ( types::Size _ = 0; _ < __num; ++_ )
            buffer_.insert( buffer_.end(), info, info + N );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( types::ROStr info, types::Size __num = 1 ) &
        {
          for ( types::Size _ = 0; _ < __num; ++_ )
            buffer_.insert( buffer_.end(), info.cbegin(), info.cend() );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const charcodes::U8String& info,
                                                              types::Size __num = 1 )
        {
          return append( info.str(), __num );
        }

        template<typename T>
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR typename std::enable_if<
          traits::AnyOf<std::is_same<typename std::decay<T>::type, types::Char>,
                        std::is_same<typename std::decay<T>::type, types::String>,
                        std::is_same<typename std::decay<T>::type, charcodes::U8String>>::value,
          Self&>::type
          operator<<( Self& stream, T&& info )
        {
          return stream.append( std::forward<T>( info ) );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( Self& stream, types::ROStr info )
        {
          return stream.append( info );
        }
        template<types::Size N>
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( Self& stream,
                                                                         const char ( &info )[N] )
        {
          return stream.append( info );
        }

        __PGBAR_CXX20_CNSTXPR void swap( Stringbuf& lhs ) noexcept
        {
          __PGBAR_PURE_ASSUME( this != &lhs );
          buffer_.swap( lhs.buffer_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( Stringbuf& a, Stringbuf& b ) noexcept { a.swap( b ); }
      };

      template<Channel Outlet>
      class OStream;
      template<Channel Outlet>
      OStream<Outlet>& flush( OStream<Outlet>& stream )
      {
        return stream.flush();
      }
      template<Channel Outlet>
      __PGBAR_CXX20_CNSTXPR OStream<Outlet>& release( OStream<Outlet>& stream ) noexcept
      {
        stream.release();
        return stream;
      }

      /**
       * A helper output stream that writes the data to `stdout` or `stderr` directly.
       *
       * It holds a proprietary buffer
       * so that don't have to use the common output buffers in the standard library.
       *
       * If the local platform is neither `Windows` nor `unix-like`,
       * the class still uses the method `write` of `std::ostream` in standard library.
       */
      template<Channel Outlet>
      class OStream final : public Stringbuf {
        using Self = OStream;

        __PGBAR_CXX20_CNSTXPR OStream() = default;

      public:
        static Self& itself() noexcept( std::is_nothrow_default_constructible<Stringbuf>::value )
        {
          static OStream instance;
          return instance;
        }

        OStream( const Self& )                   = delete;
        Self& operator=( const Self& ) &         = delete;
        __PGBAR_CXX20_CNSTXPR virtual ~OStream() = default;

        Self& flush() &
        {
# if __PGBAR_WIN
          types::Size total_written = 0;
          do {
            DWORD num_written = 0;
            if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout ) {
              auto h_stdout = GetStdHandle( STD_OUTPUT_HANDLE );
              if ( h_stdout == INVALID_HANDLE_VALUE )
                __PGBAR_UNLIKELY throw exception::SystemError(
                  "pgbar: cannot open the standard output stream" );
              WriteFile( h_stdout,
                         buffer_.data() + total_written,
                         static_cast<DWORD>( buffer_.size() - total_written ),
                         &num_written,
                         nullptr );
            } else {
              auto h_stderr = GetStdHandle( STD_ERROR_HANDLE );
              if ( h_stderr == INVALID_HANDLE_VALUE )
                __PGBAR_UNLIKELY throw exception::SystemError(
                  "pgbar: cannot open the standard error stream" );
              WriteFile( h_stderr,
                         buffer_.data() + total_written,
                         static_cast<DWORD>( buffer_.size() - total_written ),
                         &num_written,
                         nullptr );
            }
            if ( num_written <= 0 )
              break; // ignore it
            total_written += static_cast<types::Size>( num_written );
          } while ( total_written < buffer_.size() );
# elif __PGBAR_UNIX
          types::Size total_written = 0;
          do {
            ssize_t num_written = 0;
            if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
              num_written =
                write( STDOUT_FILENO, buffer_.data() + total_written, buffer_.size() - total_written );
            else
              num_written =
                write( STDERR_FILENO, buffer_.data() + total_written, buffer_.size() - total_written );
            if ( errno == EINTR )
              num_written = num_written < 0 ? 0 : num_written;
            else if ( num_written < 0 )
              break;
            total_written += static_cast<types::Size>( num_written );
          } while ( total_written < buffer_.size() );
# else
          if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
            std::cout.write( buffer_.data(), buffer_.size() ).flush();
          else
            std::cerr.write( buffer_.data(), buffer_.size() ).flush();
# endif
          clear();
          return *this;
        }

        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( OStream& stream,
                                                                         OStream& ( *fnptr )(OStream&))
        {
          __PGBAR_PURE_ASSUME( fnptr != nullptr );
          return fnptr( stream );
        }
      };
    } // namespace io

    namespace concurrent {
      // Some components require a noexcept mutex.
      class Mutex final {
        using Self = Mutex;

        std::atomic_flag lock_stat_ = ATOMIC_FLAG_INIT;

      public:
        Mutex( const Mutex& )              = delete;
        Mutex& operator=( const Mutex& ) & = delete;

        constexpr Mutex()              = default;
        __PGBAR_CXX20_CNSTXPR ~Mutex() = default;

        __PGBAR_INLINE_FN void lock() & noexcept
        {
          while ( lock_stat_.test_and_set( std::memory_order_acq_rel ) )
            std::this_thread::yield();
        }

        __PGBAR_INLINE_FN void unlock() & noexcept { lock_stat_.clear( std::memory_order_release ); }

        __PGBAR_INLINE_FN bool try_lock() & noexcept
        {
          return !lock_stat_.test_and_set( std::memory_order_acq_rel );
        }
      };

      // A simple `Shared Mutex` implementation for any C++ version.
      class SharedMutex final {
        using Self = SharedMutex;

      protected:
        std::atomic<types::Size> num_readers_;
        Mutex writer_mtx_;

      public:
        SharedMutex( const Self& )       = delete;
        Self& operator=( const Self& ) & = delete;

        constexpr SharedMutex() noexcept : num_readers_ { 0 } {}
        __PGBAR_CXX20_CNSTXPR ~SharedMutex() = default;

        void lock() & noexcept
        {
          while ( true ) {
            while ( num_readers_.load( std::memory_order_acquire ) != 0 )
              std::this_thread::yield();

            writer_mtx_.lock();
            if ( num_readers_.load( std::memory_order_acquire ) == 0 )
              break;
            else // unlock it and wait for readers to finish
              writer_mtx_.unlock();
          }
        }
        __PGBAR_NODISCARD bool try_lock() & noexcept
        {
          if ( num_readers_.load( std::memory_order_acquire ) == 0 && writer_mtx_.try_lock() ) {
            if ( num_readers_.load( std::memory_order_acquire ) == 0 )
              return true;
            else
              writer_mtx_.unlock();
          }
          return false;
        }
        __PGBAR_INLINE_FN void unlock() & noexcept { writer_mtx_.unlock(); }

        void lock_shared() & noexcept
        {
          writer_mtx_.lock();

          num_readers_.fetch_add( 1, std::memory_order_release );
          __PGBAR_ASSERT( num_readers_ > 0 ); // overflow checking

          writer_mtx_.unlock();
        }
        __PGBAR_NODISCARD bool try_lock_shared() & noexcept
        {
          if ( writer_mtx_.try_lock() ) {
            num_readers_.fetch_add( 1, std::memory_order_release );
            __PGBAR_ASSERT( num_readers_ > 0 );
            writer_mtx_.unlock();
            return true;
          }
          return false;
        }
        __PGBAR_INLINE_FN void unlock_shared() & noexcept
        {
          __PGBAR_ASSERT( num_readers_ > 0 ); // underflow checking
          num_readers_.fetch_sub( 1, std::memory_order_release );
        }
      };

# if __PGBAR_CXX14
      template<typename Mtx>
      using SharedLock = std::shared_lock<Mtx>;
# else
      template<typename Mtx>
      class SharedLock final {
        using Self = SharedLock;

        Mtx& mtx_;

      public:
        using mutex_type = Mtx;

        SharedLock( mutex_type& m ) noexcept( noexcept( mtx_.lock_shared() ) ) : mtx_ { m }
        {
          mtx_.lock_shared();
        }
        ~SharedLock() noexcept { mtx_.unlock_shared(); }
      };

# endif

      // A nullable container that holds an exception pointer.
      class ExceptionBox final {
        // This is the component requiring a noexcept mutex.
        using Self = ExceptionBox;

        std::exception_ptr exception_;
        mutable SharedMutex rw_mtx_;

      public:
        ExceptionBox()  = default;
        ~ExceptionBox() = default;

        ExceptionBox( ExceptionBox&& rhs ) noexcept : ExceptionBox() { swap( rhs ); }
        ExceptionBox& operator=( ExceptionBox&& rhs ) & noexcept
        {
          __PGBAR_PURE_ASSUME( this != &rhs );
          // Exception pointers should not be discarded due to movement semantics.
          // Thus we only swap them here.
          swap( rhs );
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          SharedLock<SharedMutex> lock { rw_mtx_ };
          return !static_cast<bool>( exception_ );
        }

        __PGBAR_INLINE_FN Self& store( std::exception_ptr e ) & noexcept
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          if ( !exception_ )
            exception_ = e;
          return *this;
        }
        __PGBAR_INLINE_FN std::exception_ptr load() const noexcept
        {
          SharedLock<SharedMutex> lock { rw_mtx_ };
          return exception_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN Self& clear() noexcept
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          exception_ = std::exception_ptr();
          return *this;
        }

        // Rethrow the exception pointed if it is'nt null.
        __PGBAR_INLINE_FN void rethrow() noexcept( false )
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          if ( !exception_ )
            return;
          auto exception_ptr = exception_;
          exception_         = std::exception_ptr();
          if ( exception_ptr )
            std::rethrow_exception( std::move( exception_ptr ) );
        }

        void swap( ExceptionBox& lhs ) noexcept
        {
          __PGBAR_PURE_ASSUME( this != &lhs );
          std::lock_guard<SharedMutex> lock1 { rw_mtx_ };
          std::lock_guard<SharedMutex> lock2 { lhs.rw_mtx_ };
          using std::swap; // ADL custom point
          swap( exception_, lhs.exception_ );
        }
        friend void swap( ExceptionBox& a, ExceptionBox& b ) noexcept { a.swap( b ); }
      };
    } // namespace concurrent

    namespace render {
      // Multi-channel renderer template class managing background task thread lifecycle and state
      // transitions.
      template<Channel Tag>
      class Renderer final {
        using Self = Renderer;

        static types::TimeUnit _working_interval;
        static concurrent::SharedMutex _rw_mtx;

        /* The state transfer process is:
         *                   activate()                   suspend()
         * Dormant(default) -----------> Awake -> Active ----------> Suspend -> Dormant
         *              dctor
         * (any state) ------> Dead
         *              catch an exception while box_ isn't empty
         * (any state) ------------------------------------------> Dead */
        enum class State : types::Byte { Dormant, Awake, Active, Suspend, Halt, Dead };

        wrappers::UniqueFunction<void()> task_;
        std::thread td_;
        concurrent::ExceptionBox box_;
        std::atomic<State> state_;

        mutable std::condition_variable cond_var_;
        mutable std::mutex mtx_;
        mutable concurrent::SharedMutex rw_mtx_;

        // Create a new thread object.
        __PGBAR_INLINE_FN void launch() & noexcept( false )
        {
          __PGBAR_ASSERT( td_.get_id() == std::thread::id() );
          state_.store( State::Dormant, std::memory_order_release );
          try {
            td_ = std::thread( [this]() -> void {
              while ( state_.load( std::memory_order_acquire ) != State::Dead ) {
                try {
                  switch ( state_.load( std::memory_order_acquire ) ) {
                  case State::Dormant: {
                    std::unique_lock<std::mutex> lock { mtx_ };
                    cond_var_.wait( lock, [this]() noexcept -> bool {
                      return state_.load( std::memory_order_acquire ) != State::Dormant;
                    } );
                  } break;

                  case State::Awake: {
                    /* Awake indicates that the current thread is started,
                     * so semantically, a task must be executed here. */
                    task_();
                    // Used to tell other threads that the current thread has woken up.
                    auto expected = State::Awake;
                    state_.compare_exchange_strong( expected,
                                                    State::Active,
                                                    std::memory_order_acq_rel,
                                                    std::memory_order_relaxed );
                  } break;

                  case State::Active: {
                    task_();
                    std::this_thread::sleep_for( working_interval() );
                  } break;

                  case State::Suspend: {
                    task_(); // same as Awake
                    auto expected = State::Suspend;
                    state_.compare_exchange_strong( expected,
                                                    State::Dormant,
                                                    std::memory_order_acq_rel,
                                                    std::memory_order_relaxed );
                  } break;

                  case State::Halt: {
                    auto expected = State::Halt;
                    state_.compare_exchange_strong( expected,
                                                    State::Dormant,
                                                    std::memory_order_acq_rel,
                                                    std::memory_order_relaxed );
                  } break;

                  default: return;
                  }
                } catch ( ... ) {
                  // keep thread valid
                  if ( box_.empty() ) {
                    auto try_update = [this]( State expected ) noexcept {
                      return state_.compare_exchange_strong( expected,
                                                             State::Dormant,
                                                             std::memory_order_acq_rel,
                                                             std::memory_order_relaxed );
                    };
                    try_update( State::Awake ) || try_update( State::Active ) || try_update( State::Suspend );
                    // Avoid deadlock in main thread when the child thread catchs exception.
                    auto exception = std::current_exception();
                    if ( exception )
                      box_.store( exception );
                  } else {
                    state_.store( State::Dead, std::memory_order_release );
                    throw; // Rethrow it, and let the current thread crash.
                  }
                }
              }
            } );
          } catch ( ... ) {
            state_.store( State::Dead, std::memory_order_release );
            throw;
          }
        }

        // Stop the thread object and release it.
        __PGBAR_INLINE_FN void shutdown() & noexcept
        {
          state_.store( State::Dead, std::memory_order_release );
          {
            std::lock_guard<std::mutex> lock { mtx_ };
            cond_var_.notify_all();
          }
          if ( td_.joinable() )
            td_.join();
          td_ = std::thread();
        }

        Renderer() = default;

      public:
        // Get the current working interval for all threads.
        __PGBAR_NODISCARD static types::TimeUnit working_interval()
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { _rw_mtx };
          return _working_interval;
        }
        // Adjust the thread working interval between this loop and the next loop.
        static void working_interval( types::TimeUnit new_rate )
        {
          std::lock_guard<concurrent::SharedMutex> lock { _rw_mtx };
          _working_interval = std::move( new_rate );
        }

        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }

        Renderer( const Self& )        = delete;
        Self& operator=( const Self& ) = delete;
        ~Renderer() noexcept { shutdown(); }

        // Terminates current task and clears task object.
        __PGBAR_INLINE_FN void appoint() noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          halt();
          task_ = nullptr;
        }
        // Attempts to assign new rendering task.
        __PGBAR_INLINE_FN bool try_appoint( wrappers::UniqueFunction<void()> task ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( state_.load( std::memory_order_acquire ) == State::Dead ) {
            shutdown();
            launch();
          } else if ( td_.get_id() == std::thread::id() )
            launch();
          if ( task_ == nullptr ) {
            __PGBAR_ASSERT( state_.load( std::memory_order_acquire ) == State::Dormant
                            || state_.load( std::memory_order_acquire ) == State::Dead );
            task_.swap( task );
          } else
            return false;
          return true;
        }
        // Activates renderer to execute tasks.
        __PGBAR_INLINE_FN void activate() & noexcept( false )
        {
          {
            std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
            if ( !console::intty( Tag ) )
              return;
            if ( state_.load( std::memory_order_acquire ) == State::Dead )
              __PGBAR_UNLIKELY
              {
                shutdown();
                launch();
              }
          }
          if ( !box_.empty() )
            __PGBAR_UNLIKELY box_.rethrow();
          __PGBAR_ASSERT( task_ != nullptr );
          auto expected = State::Dormant;
          if ( state_.compare_exchange_strong( expected,
                                               State::Awake,
                                               std::memory_order_acq_rel,
                                               std::memory_order_relaxed ) ) {
            {
              std::lock_guard<std::mutex> lock { mtx_ };
              cond_var_.notify_one();
            }
            // spin wait, ensure that the thread has moved to the new state
            do {
              // avoid deadlock and throw the exception the thread received
              if ( !box_.empty() )
                __PGBAR_UNLIKELY box_.rethrow();
            } while ( state_.load( std::memory_order_acquire ) == State::Awake
                      && state_.load( std::memory_order_acquire ) != State::Dead );
          } else if ( !box_.empty() )
            __PGBAR_UNLIKELY box_.rethrow();
        }
        // Suspends task execution into dormant state.
        __PGBAR_INLINE_FN void suspend()
        {
          auto try_update = [this]( State expected ) noexcept -> bool {
            return state_.compare_exchange_strong( expected,
                                                   State::Suspend,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed );
          };
          if ( try_update( State::Awake ) || try_update( State::Active ) ) {
            do {
              if ( !box_.empty() )
                __PGBAR_UNLIKELY box_.rethrow();
            } while ( state_.load( std::memory_order_acquire ) == State::Suspend
                      && state_.load( std::memory_order_acquire ) != State::Dead );
          } else if ( !box_.empty() )
            __PGBAR_UNLIKELY box_.rethrow();
        }
        // Halts task execution immediately.
        __PGBAR_INLINE_FN void halt() noexcept
        {
          auto try_update = [this]( State expected ) noexcept -> bool {
            return state_.compare_exchange_strong( expected,
                                                   State::Halt,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed );
          };
          if ( try_update( State::Awake ) || try_update( State::Active ) )
            while ( state_.load( std::memory_order_acquire ) == State::Halt
                    && state_.load( std::memory_order_acquire ) != State::Dead ) {}
        }
        // Terminates renderer and releases all resources.
        __PGBAR_INLINE_FN void drop() noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          shutdown();
          task_ = nullptr;
        }
        // Checks whether the task object is empty.
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ == nullptr;
        }
        // Checks whether the renderer is in Active state.
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ != nullptr && state_.load( std::memory_order_acquire ) != State::Dormant
              && state_.load( std::memory_order_acquire ) != State::Dead;
        }
      };
      template<Channel Tag>
      types::TimeUnit Renderer<Tag>::_working_interval =
        std::chrono::duration_cast<types::TimeUnit>( std::chrono::milliseconds( 40 ) );
      template<Channel Tag>
      concurrent::SharedMutex Renderer<Tag>::_rw_mtx {};
    } // namespace render
  } // namespace __details

  namespace color {
    constexpr __details::types::HexRGB None    = __PGBAR_DEFAULT;
    constexpr __details::types::HexRGB Black   = __PGBAR_BLACK;
    constexpr __details::types::HexRGB Red     = __PGBAR_RED;
    constexpr __details::types::HexRGB Green   = __PGBAR_GREEN;
    constexpr __details::types::HexRGB Yellow  = __PGBAR_YELLOW;
    constexpr __details::types::HexRGB Blue    = __PGBAR_BLUE;
    constexpr __details::types::HexRGB Magenta = __PGBAR_MAGENTA;
    constexpr __details::types::HexRGB Cyan    = __PGBAR_CYAN;
    constexpr __details::types::HexRGB White   = __PGBAR_WHITE;
  } // namespace color

  namespace option {
# define __PGBAR_OPTIONS( StructName, ValueType )                                  \
 private:                                                                          \
   ValueType data_;                                                                \
                                                                                   \
 public:                                                                           \
   __PGBAR_CXX20_CNSTXPR ~StructName() = default;                                  \
   __PGBAR_CXX14_CNSTXPR ValueType& value() noexcept                               \
   {                                                                               \
     return data_;                                                                 \
   }                                                                               \
   __PGBAR_CXX20_CNSTXPR void swap( StructName& lhs ) noexcept                     \
   {                                                                               \
     __PGBAR_PURE_ASSUME( this != &lhs );                                          \
     using std::swap;                                                              \
     swap( data_, lhs.data_ );                                                     \
   }                                                                               \
   friend __PGBAR_CXX20_CNSTXPR void swap( StructName& a, StructName& b ) noexcept \
   {                                                                               \
     a.swap( b );                                                                  \
   }

# define __PGBAR_OPTIONS_HELPER( StructName, ValueType, ParamName )              \
   __PGBAR_OPTIONS( StructName, ValueType )                                      \
   constexpr StructName( ValueType ParamName ) noexcept : data_ { ParamName } {} \
   constexpr StructName( const StructName& )                         = default;  \
   __PGBAR_CXX14_CNSTXPR StructName& operator=( const StructName& )& = default;

    // A wrapper that stores the value of the bit option setting.
    struct Style final {
      __PGBAR_OPTIONS_HELPER( Style, __details::types::Byte, _settings )
    };

    // A wrapper that stores the value of the color effect setting.
    struct Colored final {
      __PGBAR_OPTIONS_HELPER( Colored, bool, _enable )
    };

    // A wrapper that stores the value of the font boldness setting.
    struct Bolded final {
      __PGBAR_OPTIONS_HELPER( Bolded, bool, _enable )
    };

    // A wrapper that stores the number of tasks.
    struct Tasks final {
      __PGBAR_OPTIONS_HELPER( Tasks, __details::types::Size, _num_tasks )
    };

    // A wrapper that stores the length of the bar indicator, in the character unit.
    struct BarLength final {
      __PGBAR_OPTIONS_HELPER( BarLength, __details::types::Size, _num_char )
    };

    /**
     * A wrapper that stores the rate factor for animation frame transitions.
     *
     * Controls the speed of per-frame animation updates:
     *
     * - Positive values accelerate the transition (higher -> faster).
     *
     * - Negative values decelerate the transition (lower -> slower).
     *
     * - Zero freezes the animation completely.
     *
     * The effective range is between -128 (slowest) and 127 (fastest).
     */
    struct Shift final {
      __PGBAR_OPTIONS_HELPER( Shift, std::int8_t, _shift_factor )
    };

    /**
     * A wrapper that stores the base magnitude for unit scaling in formatted output.
     *
     * Defines the threshold at which values are converted to higher-order units
     * (e.g. 1000 -> "1k", 1000000 -> "1M").
     *
     * The effective range is between 1 and 65535.
     *
     * - A zero value implies no scaling (raw numeric display).
     *
     * - Typical usage: 1000 (decimal) or 1024 (binary) scaling.
     */
    struct Magnitude final {
      __PGBAR_OPTIONS_HELPER( Magnitude, std::uint16_t, _magnitude )
    };

# undef __PGBAR_OPTIONS_HELPER
# if __PGBAR_CXX20
#  define __PGBAR_OPTIONS_HELPER( StructName, ParamName )                  \
    __PGBAR_OPTIONS( StructName, __details::charcodes::U8String )          \
    /**                                                                    \
     * @throw exception::InvalidArgument                                   \
     *                                                                     \
     * If the passed parameter is not coding in UTF-8.                     \
     */                                                                    \
    __PGBAR_CXX20_CNSTXPR StructName( __details::types::String ParamName ) \
      : data_ { std::move( ParamName ) }                                   \
    {}                                                                     \
    StructName( std::u8string_view ParamName ) : data_ { ParamName } {}
# else
#  define __PGBAR_OPTIONS_HELPER( StructName, ParamName )                  \
    __PGBAR_OPTIONS( StructName, __details::charcodes::U8String )          \
    __PGBAR_CXX20_CNSTXPR StructName( __details::types::String ParamName ) \
      : data_ { std::move( ParamName ) }                                   \
    {}
# endif

    // A wrapper that stores the characters of the filler in the bar indicator.
    struct Filler final {
      __PGBAR_OPTIONS_HELPER( Filler, _filler )
    };

    // A wrapper that stores the characters of the remains in the bar indicator.
    struct Remains final {
      __PGBAR_OPTIONS_HELPER( Remains, _remains )
    };

    // A wrapper that stores characters located to the left of the bar indicator.
    struct Starting final {
      __PGBAR_OPTIONS_HELPER( Starting, _starting )
    };

    // A wrapper that stores characters located to the right of the bar indicator.
    struct Ending final {
      __PGBAR_OPTIONS_HELPER( Ending, _ending )
    };

    // A wrapper that stores the description text.
    struct Description final {
      __PGBAR_OPTIONS_HELPER( Description, _desc )
    };

    // A wrapper that stores the `true` message text.
    struct TrueMesg final {
      __PGBAR_OPTIONS_HELPER( TrueMesg, _true_mesg )
    };

    // A wrapper that stores the `false` message text.
    struct FalseMesg final {
      __PGBAR_OPTIONS_HELPER( FalseMesg, _false_mesg )
    };

    // A wrapper that stores the separator component used to separate different infomation.
    struct Divider final {
      __PGBAR_OPTIONS_HELPER( Divider, _divider )
    };

    // A wrapper that stores the border component located to the left of the whole indicator.
    struct LeftBorder final {
      __PGBAR_OPTIONS_HELPER( LeftBorder, _l_border )
    };

    // A wrapper that stores the border component located to the right of the whole indicator.
    struct RightBorder final {
      __PGBAR_OPTIONS_HELPER( RightBorder, _r_border )
    };

# undef __PGBAR_OPTIONS_HELPER
# define __PGBAR_OPTIONS_HELPER( StructName, ParamName )                                         \
   __PGBAR_OPTIONS( StructName, __details::types::String )                                       \
   StructName( __details::types::ROStr ParamName )                                               \
     : data_ { __details::console::escodes::rgb2ansi( __details::utils::hex2rgb( ParamName ) ) } \
   {}                                                                                            \
   StructName( __details::types::HexRGB ParamName )                                              \
     : data_ { __details::console::escodes::rgb2ansi( ParamName ) }                              \
   {}                                                                                            \
   StructName( const StructName& )                                   = default;                  \
   StructName( StructName&& )                                        = default;                  \
   __PGBAR_CXX23_CNSTXPR StructName& operator=( const StructName& )& = default;                  \
   __PGBAR_CXX23_CNSTXPR StructName& operator=( StructName&& )&      = default;

    // A wrapper that stores the description text color.
    struct DescColor final {
      __PGBAR_OPTIONS_HELPER( DescColor, _desc_color )
    };

    // A wrapper that stores the `true` message text color.
    struct TrueColor final {
      __PGBAR_OPTIONS_HELPER( TrueColor, _true_color )
    };

    // A wrapper that stores the `false` message text color.
    struct FalseColor final {
      __PGBAR_OPTIONS_HELPER( FalseColor, _false_color )
    };

    // A wrapper that stores the color of component located to the left of the bar indicator.
    struct StartColor final {
      __PGBAR_OPTIONS_HELPER( StartColor, _start_color )
    };

    // A wrapper that stores the color of component located to the right of the bar indicator.
    struct EndColor final {
      __PGBAR_OPTIONS_HELPER( EndColor, _end_color )
    };

    // A wrapper that stores the color of the filler in the bar indicator.
    struct FillerColor final {
      __PGBAR_OPTIONS_HELPER( FillerColor, _filler_color )
    };

    // A wrapper that stores the color of the remains in the bar indicator.
    struct RemainsColor final {
      __PGBAR_OPTIONS_HELPER( RemainsColor, _remains_color )
    };

    // A wrapper that stores the color of the lead in the bar indicator.
    struct LeadColor final {
      __PGBAR_OPTIONS_HELPER( LeadColor, _lead_color )
    };

    // A wrapper that stores the color of the whole infomation indicator.
    struct InfoColor final {
      __PGBAR_OPTIONS_HELPER( InfoColor, _info_color )
    };

# undef __PGBAR_OPTIONS_HELPER

    /**
     * A wrapper that stores ordered units for information rate formatting (e.g. B/s, kB/s).
     *
     * Encapsulates four consecutive scaling units where each unit is scaled by the
     * configured magnitude factor (default 1,000x if no `option::Magnitude` is explicitly set).
     *
     * Unit order MUST be ascending: [base_unit, scaled_unit_1, scaled_unit_2, scaled_unit_3].
     *
     * Example:
     *
     * - magnitude=1000: ["B/s", "kB/s", "MB/s", "GB/s"]
     *
     * - magnitude=1024: ["B/s", "KiB/s", "MiB/s", "GiB/s"]
     *
     * Scaling logic: value >= magnitude -> upgrade to next unit tier.
     *
     * @throw exception::InvalidArgument
     *   Thrown if any input string fails UTF-8 validation or the array size mismatches.
     */
    struct SpeedUnit final {
      __PGBAR_OPTIONS( SpeedUnit, __PGBAR_PACK( std::array<__details::charcodes::U8String, 4> ) )
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       *
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<__details::types::String, 4> _units ) : data_ {}
      {
        std::transform( std::make_move_iterator( _units.begin() ),
                        std::make_move_iterator( _units.end() ),
                        data_.begin(),
                        []( __details::types::String&& ele ) {
                          return __details::charcodes::U8String( std::move( ele ) );
                        } );
      }
# if __PGBAR_CXX20
      /**
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<std::u8string_view, 4> _units ) : data_ {}
      {
        std::transform( _units.cbegin(), _units.cend(), data_.begin(), []( const std::u8string_view& ele ) {
          return __details::charcodes::U8String( ele );
        } );
      }
# endif
      __PGBAR_CXX20_CNSTXPR SpeedUnit( const SpeedUnit& )              = default;
      __PGBAR_CXX20_CNSTXPR SpeedUnit( SpeedUnit&& )                   = default;
      __PGBAR_CXX20_CNSTXPR SpeedUnit& operator=( const SpeedUnit& ) & = default;
      __PGBAR_CXX20_CNSTXPR SpeedUnit& operator=( SpeedUnit&& ) &      = default;
    };

    // A wrapper that stores the `lead` animated element.
    struct Lead final {
      __PGBAR_OPTIONS( Lead, std::vector<__details::charcodes::U8String> )
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( std::vector<__details::types::String> _leads ) : data_ {}
      {
        std::transform( std::make_move_iterator( _leads.begin() ),
                        std::make_move_iterator( _leads.end() ),
                        std::back_inserter( data_ ),
                        []( __details::types::String&& ele ) {
                          return __details::charcodes::U8String( std::move( ele ) );
                        } );
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( __details::types::String _lead )
        : data_ { __details::charcodes::U8String( std::move( _lead ) ) }
      {}
# if __PGBAR_CXX20
      __PGBAR_CXX20_CNSTXPR Lead( std::vector<std::u8string_view> _leads ) : data_ {}
      {
        std::transform(
          _leads.cbegin(),
          _leads.cend(),
          std::back_inserter( data_ ),
          []( const std::u8string_view& ele ) { return __details::charcodes::U8String( ele ); } );
      }
      Lead( const std::u8string_view& _lead ) : data_ { __details::charcodes::U8String( _lead ) } {}
# endif
      __PGBAR_CXX20_CNSTXPR Lead( const Lead& )              = default;
      __PGBAR_CXX20_CNSTXPR Lead( Lead&& )                   = default;
      __PGBAR_CXX20_CNSTXPR Lead& operator=( const Lead& ) & = default;
      __PGBAR_CXX20_CNSTXPR Lead& operator=( Lead&& ) &      = default;
    };

# undef __PGBAR_OPTIONS
  } // namespace option

  namespace __details {
    // The Basic components of the progress bar.
    namespace assets {
# define __PGBAR_NONEMPTY_CLASS( ClassName, Constexpr )           \
   Constexpr ClassName( const ClassName& )             = default; \
   Constexpr ClassName( ClassName&& )                  = default; \
   Constexpr ClassName& operator=( const ClassName& )& = default; \
   Constexpr ClassName& operator=( ClassName&& )&      = default; \
   __PGBAR_CXX20_CNSTXPR virtual ~ClassName()          = 0;

# define __PGBAR_EMPTY_CLASS( ClassName )                                     \
   constexpr ClassName()                                           = default; \
   constexpr ClassName( const ClassName& )                         = default; \
   constexpr ClassName( ClassName&& )                              = default; \
   __PGBAR_CXX14_CNSTXPR ClassName& operator=( const ClassName& )& = default; \
   __PGBAR_CXX14_CNSTXPR ClassName& operator=( ClassName&& )&      = default; \
   __PGBAR_CXX20_CNSTXPR virtual ~ClassName()                      = 0;

      template<typename Derived>
      class Core {
# define __PGBAR_UNPAKING( OptionName, MemberName )                                                  \
   friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unpacker( Core& cfg,                          \
                                                                 option::OptionName&& val ) noexcept \
   {                                                                                                 \
     cfg.fonts_[utils::as_val( Mask::OptionName )] = val.value();                                    \
   }
        __PGBAR_UNPAKING( Colored, colored_ )
        __PGBAR_UNPAKING( Bolded, bolded_ )
# undef __PGBAR_UNPAKING

      protected:
        mutable concurrent::SharedMutex rw_mtx_;
        enum class Mask : types::Size { Colored = 0, Bolded };
        std::bitset<2> fonts_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::ROStr build_color( types::ROStr ansi_color ) const
        {
          return fonts_[utils::as_val( Mask::Colored )] ? ansi_color : constants::nil_str;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_font( io::Stringbuf& buffer,
                                                                           types::ROStr ansi_color ) const
        {
          return buffer << console::escodes::reset_font << build_color( ansi_color )
                        << ( fonts_[utils::as_val( Mask::Bolded )] ? console::escodes::bold_font
                                                                   : constants::nil_str );
        }

      public:
        Core() noexcept : fonts_ { static_cast<types::Byte>( ~0 ) } {}
        constexpr Core( const Core& lhs ) : fonts_ { lhs.fonts_ } {}
        constexpr Core( Core&& rhs ) noexcept : fonts_ { rhs.fonts_ } {}
        __PGBAR_CXX14_CNSTXPR Core& operator=( const Core& lhs ) &
        {
          fonts_ = lhs.fonts_;
          return *this;
        }
        __PGBAR_CXX14_CNSTXPR Core& operator=( Core&& rhs ) & noexcept
        {
          fonts_ = rhs.fonts_;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR virtual ~Core() = 0;

# define __PGBAR_METHOD( OptionName, ParamName )              \
                                                              \
   std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ }; \
   unpacker( *this, option::OptionName( ParamName ) );        \
   return static_cast<Derived&>( *this )

        // Enable or disable the color effect.
        Derived& colored( bool _enable ) & { __PGBAR_METHOD( Colored, _enable ); }
        // Enable or disable the bold effect.
        Derived& bolded( bool _enable ) & { __PGBAR_METHOD( Bolded, _enable ); }

# undef __PGBAR_METHOD
# define __PGBAR_METHOD( Offset )                                    \
   concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ }; \
   return fonts_[utils::as_val( Mask::Offset )]

        // Check whether the color effect is enabled.
        __PGBAR_NODISCARD bool colored() const { __PGBAR_METHOD( Colored ); }
        // Check whether the bold effect is enabled.
        __PGBAR_NODISCARD bool bolded() const { __PGBAR_METHOD( Bolded ); }

# undef __PGBAR_METHOD

        __PGBAR_CXX14_CNSTXPR void swap( Core& lhs ) noexcept { std::swap( fonts_, lhs.fonts_ ); }
      };
      template<typename Derived>
      __PGBAR_CXX20_CNSTXPR Core<Derived>::~Core() = default;

      template<typename Base, typename Derived>
      class TaskQuantity : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( TaskQuantity& cfg, option::Tasks&& val )
        {
          cfg.task_range_.end_value( val.value() );
        }

      protected:
        scope::NumericSpan<types::Size> task_range_;

      public:
        constexpr TaskQuantity() = default;
        __PGBAR_NONEMPTY_CLASS( TaskQuantity, __PGBAR_CXX14_CNSTXPR )

        // Set the number of tasks, passing in zero is no exception.
        Derived& tasks( types::Size param ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Tasks( param ) );
          return static_cast<Derived&>( *this );
        }
        // Get the current number of tasks.
        __PGBAR_NODISCARD types::Size tasks() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return task_range_.end_value();
        }

        __PGBAR_CXX14_CNSTXPR void swap( TaskQuantity& lhs ) noexcept
        {
          task_range_.swap( lhs.task_range_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR TaskQuantity<Base, Derived>::~TaskQuantity() = default;

      template<typename Base, typename Derived>
      class BasicAnimation : public Base {
        friend __PGBAR_INLINE_FN void unpacker( BasicAnimation& cfg, option::LeadColor&& val ) noexcept
        {
          cfg.lead_col_ = std::move( val.value() );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unpacker( BasicAnimation& cfg,
                                                                      option::Shift&& val ) noexcept
        {
          cfg.shift_factor_ = val.value() < 0 ? ( 1.0 / ( -val.value() ) ) : val.value();
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( BasicAnimation& cfg,
                                                                      option::Lead&& val ) noexcept
        {
          if ( std::all_of( val.value().cbegin(),
                            val.value().cend(),
                            []( const charcodes::U8String& ele ) noexcept { return ele.empty(); } ) ) {
            cfg.lead_.clear();
            cfg.size_longest_lead_ = 0;
          } else {
            cfg.lead_              = std::move( val.value() );
            cfg.size_longest_lead_ = std::max_element( cfg.lead_.cbegin(),
                                                       cfg.lead_.cend(),
                                                       []( types::ROStr a, types::ROStr b ) noexcept {
                                                         return a.size() < b.size();
                                                       } )
                                       ->size();
          }
        }

      protected:
        types::Float shift_factor_;
        types::String lead_col_;
        std::vector<charcodes::U8String> lead_;
        types::Size size_longest_lead_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_animation()
          const noexcept
        {
          return size_longest_lead_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR BasicAnimation() = default;
        __PGBAR_NONEMPTY_CLASS( BasicAnimation, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * Set the rate factor of the animation with negative value slowing down the switch per frame
         * and positive value speeding it up.
         *
         * The maximum and minimum of the rate factor is between -128 and 127.
         *
         * If the value is zero, freeze the animation.
         */
        Derived& shift( std::int8_t _shift_factor ) & { __PGBAR_METHOD( Shift, _shift_factor, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& lead( std::vector<types::String> _leads ) & { __PGBAR_METHOD( Lead, _leads, std::move ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& lead( types::String _lead ) & { __PGBAR_METHOD( Lead, _lead, std::move ); }
# if __PGBAR_CXX20
        Derived& lead( std::vector<std::u8string_view> _leads ) &
        {
          __PGBAR_METHOD( Lead, _leads, std::move );
        }
        Derived& lead( std::u8string_view _lead ) & { __PGBAR_METHOD( Lead, _lead, ); }
# endif

        // Set the color of the component `lead`.
        Derived& lead_color( types::HexRGB _lead_color ) & { __PGBAR_METHOD( LeadColor, _lead_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& lead_color( types::ROStr _lead_color ) & { __PGBAR_METHOD( LeadColor, _lead_color, ); }

# undef __PGBAR_METHOD

        __PGBAR_CXX20_CNSTXPR void swap( BasicAnimation& lhs ) noexcept
        {
          using std::swap;
          swap( shift_factor_, lhs.shift_factor_ );
          lead_col_.swap( lhs.lead_col_ );
          lead_.swap( lhs.lead_ );
          swap( size_longest_lead_, lhs.size_longest_lead_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR BasicAnimation<Base, Derived>::~BasicAnimation() = default;

      template<typename Base, typename Derived>
      class BasicIndicator : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                           \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( BasicIndicator& cfg,                \
                                                     option::OptionName&& val ) noexcept \
   {                                                                                     \
     cfg.MemberName = std::move( val.value() );                                          \
   }
        __PGBAR_UNPAKING( Starting, starting_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( Ending, ending_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( BarLength, bar_length_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( StartColor, start_col_, )
        __PGBAR_UNPAKING( EndColor, end_col_, )
        __PGBAR_UNPAKING( FillerColor, filler_col_, )
# undef __PGBAR_UNPAKING

      protected:
        types::Size bar_length_;
        charcodes::U8String starting_, ending_;
        types::String start_col_, end_col_;
        types::String filler_col_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_bar() const noexcept
        {
          return starting_.size() + ending_.size();
        }

      public:
        __PGBAR_CXX20_CNSTXPR BasicIndicator() = default;
        __PGBAR_NONEMPTY_CLASS( BasicIndicator, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& starting( types::String _starting ) & { __PGBAR_METHOD( Starting, _starting, std::move ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& ending( types::String _ending ) & { __PGBAR_METHOD( Ending, _ending, std::move ); }
# if __PGBAR_CXX20
        Derived& starting( std::u8string_view _starting ) & { __PGBAR_METHOD( Starting, _starting, ); }
        Derived& ending( std::u8string_view _ending ) & { __PGBAR_METHOD( Ending, _ending, ); }
# endif

        Derived& start_color( types::HexRGB _start_color ) & { __PGBAR_METHOD( StartColor, _start_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& start_color( types::ROStr _start_color ) & { __PGBAR_METHOD( StartColor, _start_color, ); }
        Derived& end_color( types::HexRGB _end_color ) & { __PGBAR_METHOD( EndColor, _end_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& end_color( types::ROStr _end_color ) & { __PGBAR_METHOD( EndColor, _end_color, ); }
        Derived& filler_color( types::HexRGB _filler_color ) &
        {
          __PGBAR_METHOD( FillerColor, _filler_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& filler_color( types::ROStr _filler_color ) &
        {
          __PGBAR_METHOD( FillerColor, _filler_color, );
        }
        // Set the length of the bar indicator.
        Derived& bar_length( types::Size _length ) & { __PGBAR_METHOD( BarLength, _length, ); }

        __PGBAR_NODISCARD types::Size bar_length() const
        {
          __details::concurrent::SharedLock<__details::concurrent::SharedMutex> lock { this->rw_mtx_ };
          return bar_length_;
        }

# undef __PGBAR_METHOD
        __PGBAR_CXX20_CNSTXPR void swap( BasicIndicator& lhs ) noexcept
        {
          std::swap( bar_length_, lhs.bar_length_ );
          starting_.swap( lhs.starting_ );
          ending_.swap( lhs.ending_ );
          start_col_.swap( lhs.start_col_ );
          end_col_.swap( lhs.end_col_ );
          filler_col_.swap( lhs.filler_col_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR BasicIndicator<Base, Derived>::~BasicIndicator() = default;

      template<typename Base, typename Derived>
      class CharIndicator : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                                               \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( CharIndicator& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                         \
     cfg.MemberName = std::move( val.value() );                                                              \
   }
        __PGBAR_UNPAKING( Remains, remains_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( RemainsColor, remains_col_, )
        __PGBAR_UNPAKING( Filler, filler_, __PGBAR_CXX20_CNSTXPR )
# undef __PGBAR_UNPAKING

      protected:
        types::String remains_col_;
        charcodes::U8String remains_, filler_;

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_char( io::Stringbuf& buffer,
                                                                           types::Float num_percent,
                                                                           types::Size num_frame_cnt ) const
        {
          __PGBAR_PURE_ASSUME( num_percent >= 0.0 );
          __PGBAR_PURE_ASSUME( num_percent <= 1.0 );

          buffer << console::escodes::reset_font << this->build_color( this->start_col_ ) << this->starting_
                 << console::escodes::reset_font << this->build_color( this->filler_col_ );

          const auto len_finished = static_cast<types::Size>( std::round( this->bar_length_ * num_percent ) );
          types::Size len_unfinished = this->bar_length_ - len_finished;
          __PGBAR_PURE_ASSUME( len_finished + len_unfinished == this->bar_length_ );

          // build filler_
          if ( !filler_.empty() && filler_.size() <= len_finished ) {
            const types::Size fill_num  = len_finished / filler_.size(),
                              remaining = len_finished % filler_.size();
            len_unfinished += remaining;
            buffer.append( filler_, fill_num );
          } else
            len_unfinished += len_finished;
          // build lead_
          buffer << console::escodes::reset_font;
          if ( !this->lead_.empty() ) {
            num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
            num_frame_cnt %= this->lead_.size();
            const auto& current_lead = this->lead_[num_frame_cnt];
            if ( current_lead.size() <= len_unfinished ) {
              len_unfinished -= current_lead.size();
              buffer << this->build_color( this->lead_col_ ) << current_lead << console::escodes::reset_font;
            }
          }
          // build remains_
          buffer << this->build_color( remains_col_ );
          if ( !this->remains_.empty() && this->remains_.size() <= len_unfinished )
            buffer.append( remains_, len_unfinished / remains_.size() )
              .append( constants::blank, len_unfinished % remains_.size() );
          else
            buffer.append( constants::blank, len_unfinished );

          return buffer << console::escodes::reset_font << this->build_color( this->end_col_ )
                        << this->ending_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR CharIndicator() = default;
        __PGBAR_NONEMPTY_CLASS( CharIndicator, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        Derived& remains_color( types::HexRGB _remains_color ) &
        {
          __PGBAR_METHOD( RemainsColor, _remains_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& remains_color( types::ROStr _remains_color ) &
        {
          __PGBAR_METHOD( RemainsColor, _remains_color, );
        }

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& remains( types::String _remains ) & { __PGBAR_METHOD( Remains, _remains, std::move ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& filler( types::String _filler ) & { __PGBAR_METHOD( Filler, _filler, std::move ); }
# if __PGBAR_CXX20
        Derived& remains( std::u8string_view _remains ) & { __PGBAR_METHOD( Remains, _remains, ); }
        Derived& filler( std::u8string_view _filler ) & { __PGBAR_METHOD( Filler, _filler, ); }
# endif
# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( CharIndicator& lhs ) noexcept
        {
          remains_col_.swap( lhs.remains_col_ );
          remains_.swap( lhs.remains_ );
          filler_.swap( lhs.filler_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR CharIndicator<Base, Derived>::~CharIndicator() = default;

      template<typename Base, typename Derived>
      class BlockIndicator : public Base {
      protected:
        const std::array<types::LitStr, 8> filler_ = {
          { "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█" }
        };

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_block( io::Stringbuf& buffer,
                                                                            types::Float num_percent ) const
        {
          __PGBAR_PURE_ASSUME( num_percent >= 0.0 );
          __PGBAR_PURE_ASSUME( num_percent <= 1.0 );

          buffer << console::escodes::reset_font << this->build_color( this->start_col_ ) << this->starting_
                 << console::escodes::reset_font << this->build_color( this->filler_col_ );

          const auto len_finished = static_cast<types::Size>( std::trunc( this->bar_length_ * num_percent ) );
          const types::Float float_part = ( this->bar_length_ * num_percent ) - len_finished;
          __PGBAR_PURE_ASSUME( float_part >= 0.0 );
          __PGBAR_PURE_ASSUME( float_part <= 1.0 );
          const types::Size incomplete_block = static_cast<types::Size>( float_part * filler_.size() );
          const types::Size len_unfinished   = this->bar_length_ - len_finished - ( incomplete_block != 0 );
          __PGBAR_PURE_ASSUME( len_finished + len_unfinished + ( incomplete_block != 0 )
                               == this->bar_length_ );

          return buffer.append( filler_.back(), len_finished )
            .append( filler_[incomplete_block], incomplete_block != 0 )
            .append( console::escodes::reset_font )
            .append( constants::blank, len_unfinished )
            .append( console::escodes::reset_font )
            .append( this->build_color( this->end_col_ ) )
            .append( this->ending_ );
        }

      public:
        constexpr BlockIndicator() = default;
        constexpr BlockIndicator( const BlockIndicator& lhs )
          noexcept( std::is_nothrow_copy_constructible<Base>::value )
          : Base( lhs )
        {}
        constexpr BlockIndicator( BlockIndicator&& rhs )
          noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) )
        {}
        __PGBAR_CXX14_CNSTXPR BlockIndicator& operator=( const BlockIndicator& lhs ) & noexcept(
          std::is_nothrow_copy_assignable<Base>::value )
        {
          Base::operator=( lhs );
          return *this;
        }
        __PGBAR_CXX14_CNSTXPR BlockIndicator& operator=( BlockIndicator&& rhs ) & noexcept(
          std::is_nothrow_move_assignable<Base>::value )
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR virtual ~BlockIndicator() = 0;
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR BlockIndicator<Base, Derived>::~BlockIndicator() = default;

      template<typename Base, typename Derived>
      class Spinner : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_spinner(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt ) const
        {
          if ( this->lead_.empty() )
            return buffer;
          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
          num_frame_cnt %= this->lead_.size();
          __PGBAR_ASSERT( this->size_longest_lead_ >= this->lead_[num_frame_cnt].size() );

          buffer << console::escodes::reset_font;
          return this->build_font( buffer, this->lead_col_ )
              << utils::format<utils::TxtLayout::Left>( this->size_longest_lead_,
                                                        this->lead_[num_frame_cnt] );
        }

      public:
        __PGBAR_EMPTY_CLASS( Spinner )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Spinner<Base, Derived>::~Spinner() = default;

      template<typename Base, typename Derived>
      class Scanner : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( Scanner& cfg,
                                                                      option::Filler&& val ) noexcept
        {
          cfg.filler_ = std::move( val.value() );
        }

      protected:
        charcodes::U8String filler_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_scanner(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt ) const
        {
          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
          buffer << console::escodes::reset_font << this->build_color( this->start_col_ ) << this->starting_
                 << console::escodes::reset_font << this->build_color( this->filler_col_ );

          if ( !this->lead_.empty() ) {
            const auto& current_lead = this->lead_[num_frame_cnt % this->lead_.size()];
            if ( current_lead.size() <= this->bar_length_ ) {
              const auto left_fill_len = [this, num_frame_cnt, &current_lead]() noexcept {
                const auto real_len    = this->bar_length_ - current_lead.size() + 1;
                const auto total_len   = real_len * 2;
                const auto current_pos = num_frame_cnt % total_len;
                return current_pos > real_len ? total_len - current_pos : current_pos - ( current_pos != 0 );
              }();
              const auto right_fill_len = this->bar_length_ - ( left_fill_len + current_lead.size() );
              __PGBAR_ASSERT( left_fill_len + right_fill_len + current_lead.size() == this->bar_length_ );

              buffer.append( filler_, left_fill_len / filler_.size() )
                .append( constants::blank, left_fill_len % filler_.size() )
                .append( console::escodes::reset_font )
                .append( this->lead_col_ )
                .append( current_lead )
                .append( console::escodes::reset_font )
                .append( this->filler_col_ )
                .append( constants::blank, right_fill_len % filler_.size() )
                .append( filler_, right_fill_len / filler_.size() );
            } else
              buffer.append( constants::blank, this->bar_length_ );
          } else if ( filler_.empty() )
            buffer.append( constants::blank, this->bar_length_ );
          else
            buffer.append( filler_, this->bar_length_ / filler_.size() )
              .append( constants::blank, this->bar_length_ % filler_.size() );

          return buffer << console::escodes::reset_font << this->build_color( this->end_col_ )
                        << this->ending_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR Scanner() = default;
        __PGBAR_NONEMPTY_CLASS( Scanner, __PGBAR_CXX20_CNSTXPR )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& filler( types::String param ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Filler( std::move( param ) ) );
          return static_cast<Derived&>( *this );
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( Scanner& lhs ) noexcept
        {
          filler_.swap( lhs.filler_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Scanner<Base, Derived>::~Scanner() = default;

      template<typename Base, typename Derived>
      class Description : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                                             \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Description& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                       \
     cfg.MemberName = std::move( val.value() );                                                            \
   }
        __PGBAR_UNPAKING( Description, description_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( TrueMesg, true_mesg_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( FalseMesg, false_mesg_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( DescColor, desc_col_, )
        __PGBAR_UNPAKING( TrueColor, true_col_, )
        __PGBAR_UNPAKING( FalseColor, false_col_, )
# undef __PGBAR_UNPAKING

      protected:
        types::String desc_col_;
        types::String true_col_;
        types::String false_col_;

        charcodes::U8String description_;
        charcodes::U8String true_mesg_;
        charcodes::U8String false_mesg_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_description(
          io::Stringbuf& buffer ) const
        {
          if ( description_.empty() )
            return buffer;
          buffer << console::escodes::reset_font;
          return this->build_font( buffer, desc_col_ ) << description_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_description( io::Stringbuf& buffer,
                                                                                  bool final_mesg ) const
        {
          if ( ( final_mesg ? true_mesg_ : false_mesg_ ).empty() )
            return build_description( buffer );
          buffer << console::escodes::reset_font;
          return this->build_font( buffer, final_mesg ? true_col_ : false_col_ )
              << ( final_mesg ? true_mesg_ : false_mesg_ );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_description()
          const noexcept
        {
          return std::max( std::max( true_mesg_.size(), false_mesg_.size() ), description_.size() );
        }

      public:
        __PGBAR_CXX20_CNSTXPR Description() = default;
        __PGBAR_NONEMPTY_CLASS( Description, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& description( types::String _desc ) & { __PGBAR_METHOD( Description, _desc, std::move ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& true_mesg( types::String _true_mesg ) &
        {
          __PGBAR_METHOD( TrueMesg, _true_mesg, std::move );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& false_mesg( types::String _false_mesg ) &
        {
          __PGBAR_METHOD( FalseMesg, _false_mesg, std::move );
        }
# if __PGBAR_CXX20
        Derived& description( std::u8string_view _desc ) & { __PGBAR_METHOD( Description, _desc, ); }
        Derived& true_mesg( std::u8string_view _true_mesg ) & { __PGBAR_METHOD( TrueMesg, _true_mesg, ); }
        Derived& false_mesg( std::u8string_view _false_mesg ) & { __PGBAR_METHOD( FalseMesg, _false_mesg, ); }
# endif

        Derived& desc_color( types::HexRGB _desc_color ) & { __PGBAR_METHOD( DescColor, _desc_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& desc_color( types::ROStr _desc_color ) & { __PGBAR_METHOD( DescColor, _desc_color, ); }
        Derived& true_color( types::HexRGB _true_color ) & { __PGBAR_METHOD( TrueColor, _true_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& true_color( types::ROStr _true_color ) & { __PGBAR_METHOD( TrueColor, _true_color, ); }
        Derived& false_color( types::HexRGB _false_color ) & { __PGBAR_METHOD( FalseColor, _false_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& false_color( types::ROStr _false_color ) & { __PGBAR_METHOD( FalseColor, _false_color, ); }

# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( Description& lhs ) noexcept
        {
          desc_col_.swap( lhs.desc_col_ );
          true_col_.swap( lhs.true_col_ );
          false_col_.swap( lhs.false_col_ );
          description_.swap( lhs.description_ );
          true_mesg_.swap( lhs.true_mesg_ );
          false_mesg_.swap( lhs.false_mesg_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Description<Base, Derived>::~Description() = default;

      template<typename Base, typename Derived>
      class Segment : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Operation, Constexpr )                              \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Segment& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                   \
     cfg.MemberName = Operation( val.value() );                                                        \
   }
        __PGBAR_UNPAKING( Style, visibilities_, , __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( InfoColor, info_col_, std::move, )
        __PGBAR_UNPAKING( Divider, divider_, std::move, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( LeftBorder, l_border_, std::move, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( RightBorder, r_border_, std::move, __PGBAR_CXX20_CNSTXPR )
# undef __PGBAR_UNPAKING

      protected:
        types::String info_col_;
        charcodes::U8String divider_;
        charcodes::U8String l_border_, r_border_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_lborder( io::Stringbuf& buffer ) const
        {
          if ( l_border_.empty() )
            return buffer;
          buffer << console::escodes::reset_font;
          return this->build_font( buffer, info_col_ ) << l_border_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_rborder( io::Stringbuf& buffer ) const
        {
          if ( r_border_.empty() )
            return buffer;
          return buffer << r_border_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_divider( io::Stringbuf& buffer ) const
        {
          if ( divider_.empty() )
            return buffer;
          buffer << console::escodes::reset_font;
          return this->build_font( buffer, info_col_ ) << divider_;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_segment(
          types::Size num_column ) const noexcept
        {
          switch ( num_column ) {
          case 0:  return 0;
          case 1:  return l_border_.size() + r_border_.size();
          default: return ( num_column - 1 ) * divider_.size() + l_border_.size() + r_border_.size();
          }
        }

      public:
        __PGBAR_CXX20_CNSTXPR Segment() = default;
        __PGBAR_NONEMPTY_CLASS( Segment, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& divider( types::String _divider ) & { __PGBAR_METHOD( Divider, _divider, std::move ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& left_border( types::String _l_border ) &
        {
          __PGBAR_METHOD( LeftBorder, _l_border, std::move );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& right_border( types::String _r_border ) &
        {
          __PGBAR_METHOD( RightBorder, _r_border, std::move );
        }
# if __PGBAR_CXX20
        Derived& divider( std::u8string_view _divider ) & { __PGBAR_METHOD( Divider, _divider, ); }
        Derived& left_border( std::u8string_view _l_border ) & { __PGBAR_METHOD( LeftBorder, _l_border, ); }
        Derived& right_border( std::u8string_view _r_border ) & { __PGBAR_METHOD( RightBorder, _r_border, ); }
# endif

        Derived& info_color( types::HexRGB _info_color ) & { __PGBAR_METHOD( InfoColor, _info_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& info_color( types::ROStr _info_color ) & { __PGBAR_METHOD( InfoColor, _info_color, ); }

# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( Segment& lhs ) & noexcept
        {
          info_col_.swap( lhs.info_col_ );
          divider_.swap( lhs.divider_ );
          l_border_.swap( lhs.l_border_ );
          r_border_.swap( lhs.r_border_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Segment<Base, Derived>::~Segment() = default;

      template<typename Base, typename Derived>
      class PercentMeter : public Base {
# define __PGBAR_DEFAULT_PERCENT " --.--%"
        static constexpr types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_PERCENT ) - 1;

      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_percent( types::Float num_percent ) const
        {
          __PGBAR_PURE_ASSUME( num_percent >= 0.0 );
          __PGBAR_PURE_ASSUME( num_percent <= 1.0 );

          if ( num_percent <= 0.0 )
            __PGBAR_UNLIKELY return { __PGBAR_DEFAULT_PERCENT };

          auto str = utils::format( num_percent * 100.0, 2 );
          str.push_back( '%' );

          return utils::format<utils::TxtLayout::Right>( _fixed_length, str );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_percent() const noexcept
        {
          return _fixed_length;
        }

      public:
        __PGBAR_EMPTY_CLASS( PercentMeter )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR PercentMeter<Base, Derived>::~PercentMeter() = default;

      template<typename Base, typename Derived>
      class SpeedMeter : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( SpeedMeter& cfg,
                                                                      option::SpeedUnit&& val ) noexcept
        {
          cfg.units_        = std::move( val.value() );
          cfg.longest_unit_ = std::max( std::max( cfg.units_[0].size(), cfg.units_[1].size() ),
                                        std::max( cfg.units_[2].size(), cfg.units_[3].size() ) );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( SpeedMeter& cfg,
                                                                      option::Magnitude&& val ) noexcept
        {
          cfg.magnitude_ = val.value();
        }

# define __PGBAR_DEFAULT_SPEED "   inf "
        static constexpr types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_SPEED ) - 1;

      protected:
        std::array<charcodes::U8String, 4> units_;
        types::Size longest_unit_;
        std::uint16_t magnitude_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_speed( const types::TimeUnit& time_passed,
                                                                       types::Size num_task_done,
                                                                       types::Size num_all_tasks ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          if ( num_all_tasks == 0 )
            __PGBAR_UNLIKELY return utils::format<utils::TxtLayout::Right>( _fixed_length + longest_unit_,
                                                                            "-- " + units_.front() );

          const auto seconds_passed    = std::chrono::duration<types::Float>( time_passed ).count();
          // zero or negetive is invalid
          const types::Float frequency = seconds_passed <= 0.0 ? ( std::numeric_limits<types::Float>::max )()
                                                               : num_task_done / seconds_passed;
          types::String rate_str;

          /* Since the cube of the maximum value of std::uint16_t does not exceed
           * the representable range of std::uint64_t,
           * we choose to use std::uint16_t to represent the scaling magnitude. */
          const types::Size tier1 = magnitude_ * magnitude_;
          const types::Size tier2 = tier1 * magnitude_;
          // tier0 is magnitude_ itself

          if ( frequency < magnitude_ )
            rate_str = utils::format( frequency, 2 ) + ' ' + units_[0];
          else if ( frequency < tier1 ) // "kilo"
            rate_str = utils::format( frequency / magnitude_, 2 ) + ' ' + units_[1];
          else if ( frequency < tier2 ) // "Mega"
            rate_str = utils::format( frequency / tier1, 2 ) + ' ' + units_[2];
          else { // "Giga" or "infinity"
            const types::Float remains = frequency / tier2;
            if ( remains > magnitude_ )
              __PGBAR_UNLIKELY rate_str = __PGBAR_DEFAULT_SPEED + units_[3];
            else
              rate_str = utils::format( remains, 2 ) + ' ' + units_[3];
          }

          return utils::format<utils::TxtLayout::Right>( _fixed_length + longest_unit_, rate_str );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_speed() const noexcept
        {
          return _fixed_length + longest_unit_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR SpeedMeter() = default;
        __PGBAR_NONEMPTY_CLASS( SpeedMeter, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( ParamName, OptionName )                    \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( std::move( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         *
         * @param _units
         * The given each unit will be treated as 1,000 times greater than the previous one
         * (from left to right).
         */
        Derived& speed_unit( std::array<types::String, 4> _units ) & { __PGBAR_METHOD( _units, SpeedUnit ); }
# if __PGBAR_CXX20
        /**
         * @param _units
         * The given each unit will be treated as 1,000 times greater than the previous one
         * (from left to right).
         */
        Derived& speed_unit( std::array<std::u8string_view, 4> _units ) &
        {
          __PGBAR_METHOD( _units, SpeedUnit );
        }
# endif

        /**
         * @param _magnitude
         * The base magnitude for unit scaling in formatted output.
         *
         * Defines the threshold at which values are converted to higher-order units
         * (e.g. 1000 -> "1k", 1000000 -> "1M").
         */
        Derived& magnitude( std::uint16_t _magnitude ) & { __PGBAR_METHOD( _magnitude, Magnitude ); }

# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( SpeedMeter& lhs ) & noexcept
        {
          units_.swap( lhs.units_ );
          std::swap( longest_unit_, lhs.longest_unit_ );
          Base::swap( lhs );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR SpeedMeter<Base, Derived>::~SpeedMeter() = default;

      template<typename Base, typename Derived>
      class CounterMeter : public Base {
      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_counter( types::Size num_task_done,
                                                                         types::Size num_all_tasks ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          if ( num_all_tasks == 0 )
            return { "-/-" };

          auto str = utils::format<utils::TxtLayout::Right>( utils::count_digits( num_all_tasks ) + 1,
                                                             utils::format( num_task_done ) );
          str.append( 1, '/' ).append( utils::format( num_all_tasks ) );

          return str;
        }

        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR __PGBAR_INLINE_FN types::Size fixed_len_counter()
          const noexcept
        {
          return utils::count_digits( this->task_range_.end_value() ) * 2 + 1;
        }

      public:
        __PGBAR_EMPTY_CLASS( CounterMeter )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR CounterMeter<Base, Derived>::~CounterMeter() = default;

      template<typename Base, typename Derived>
      class Timer : public Base {
# define __PGBAR_DEFAULT_TIMER "--:--:--"
# define __PGBAR_TIMER_SEGMENT " < "

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String time_formatter( types::TimeUnit duration ) const
        {
          const auto time2str = []( std::int64_t num_time ) -> types::String {
            auto ret = utils::format( num_time );
            if ( ret.size() < 2 )
              ret.insert( 0, 1, '0' );
            return ret;
          };
          const auto hours = std::chrono::duration_cast<std::chrono::hours>( duration );
          duration -= hours;
          const auto minutes = std::chrono::duration_cast<std::chrono::minutes>( duration );
          duration -= minutes;
          return ( ( ( hours.count() > 99 ? types::String( 2, '-' ) : time2str( hours.count() ) ) + ':' )
                   + ( time2str( minutes.count() ) + ':' )
                   + time2str( std::chrono::duration_cast<std::chrono::seconds>( duration ).count() ) );
        }

      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_elapsed( types::TimeUnit time_passed ) const
        {
          return time_formatter( std::move( time_passed ) );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_elapsed() const noexcept
        {
          return sizeof( __PGBAR_DEFAULT_TIMER ) - 1;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String build_countdown(
          const types::TimeUnit& time_passed,
          types::Size num_task_done,
          types::Size num_all_tasks ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          if ( num_task_done == 0 || num_all_tasks == 0 )
            return { __PGBAR_DEFAULT_TIMER };

          auto time_per_task = time_passed / num_task_done;
          if ( time_per_task.count() == 0 )
            time_per_task = std::chrono::nanoseconds( 1 );

          const auto remaining_tasks = num_all_tasks - num_task_done;
          // overflow check
          if ( remaining_tasks > std::numeric_limits<std::int64_t>::max() / time_per_task.count() )
            return { __PGBAR_DEFAULT_TIMER };
          else
            return time_formatter( time_per_task * remaining_tasks );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_countdown() const noexcept
        {
          return sizeof( __PGBAR_DEFAULT_TIMER ) - 1;
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_hybird(
          io::Stringbuf& buffer,
          const types::TimeUnit& time_passed,
          types::Size num_task_done,
          types::Size num_all_tasks ) const
        {
          return buffer << build_elapsed( time_passed ) << __PGBAR_TIMER_SEGMENT
                        << build_countdown( time_passed, num_task_done, num_all_tasks );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_hybird() const noexcept
        {
          return fixed_len_elapsed() + fixed_len_countdown() + sizeof( __PGBAR_TIMER_SEGMENT ) - 1;
        }

      public:
        __PGBAR_EMPTY_CLASS( Timer )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Timer<Base, Derived>::~Timer() = default;

      template<typename Base, typename Derived>
      class TaskCounter : public Base {
      protected:
        std::atomic<__details::types::Size> task_cnt_, task_end_;

      public:
        constexpr TaskCounter() noexcept( std::is_nothrow_default_constructible<Base>::value )
          : task_cnt_ { 0 }
        {}
        TaskCounter( const TaskCounter& lhs ) noexcept( std::is_nothrow_copy_constructible<Base>::value )
          : Base( lhs )
        {
          task_cnt_.store( 0, std::memory_order_relaxed );
        }
        TaskCounter( TaskCounter&& rhs ) noexcept : Base( std::move( rhs ) )
        {
          task_cnt_.store( 0, std::memory_order_relaxed );
        }
        TaskCounter& operator=( const TaskCounter& lhs ) & noexcept(
          std::is_nothrow_copy_assignable<Base>::value )
        {
          Base::operator=( lhs );
          return *this;
        }
        TaskCounter& operator=( TaskCounter&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR virtual ~TaskCounter() = 0;

        // Get the progress of the task.
        __PGBAR_NODISCARD types::Size progress() const noexcept
        {
          return task_cnt_.load( std::memory_order_acquire );
        }

        /**
         * Visualize unidirectional traversal of a numeric interval defined by parameters.
         *
         * @return Return a range `[startpoint, endpoint)` that moves unidirectionally.
         */
        template<typename N>
# if __PGBAR_CXX20
          requires std::is_arithmetic_v<N>
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR scope::ProxySpan<scope::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_arithmetic<N>::value,
                                  scope::ProxySpan<scope::NumericSpan<N>, Derived>>::type
# endif
          iterate( N startpoint, N endpoint, N step ) &
        { // default parameter will cause ambiguous overloads
          // so we have to write them all
          return { scope::NumericSpan<typename std::decay<N>::type>( startpoint, endpoint, step ),
                   static_cast<Derived&>( *this ) };
        }
        template<typename N, typename F>
# if __PGBAR_CXX20
          requires std::is_arithmetic_v<N>
        __PGBAR_CXX14_CNSTXPR void
# else
        __PGBAR_CXX14_CNSTXPR typename std::enable_if<std::is_arithmetic<N>::value>::type
# endif
          iterate( N startpoint, N endpoint, N step, F&& unary_fn )
        {
          for ( N e : iterate( startpoint, endpoint, step ) )
            unary_fn( e );
        }

        template<typename N>
# if __PGBAR_CXX20
          requires std::is_floating_point_v<N>
        __PGBAR_NODISCARD scope::ProxySpan<scope::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD typename std::enable_if<std::is_floating_point<N>::value,
                                                  scope::ProxySpan<scope::NumericSpan<N>, Derived>>::type
# endif
          iterate( N endpoint, N step ) &
        {
          return { scope::NumericSpan<typename std::decay<N>::type>( {}, endpoint, step ),
                   static_cast<Derived&>( *this ) };
        }
        template<typename N, typename F>
# if __PGBAR_CXX20
          requires std::is_floating_point_v<N>
        void
# else
        typename std::enable_if<std::is_floating_point<N>::value>::type
# endif
          iterate( N endpoint, N step, F&& unary_fn )
        {
          for ( N e : iterate( endpoint, step ) )
            unary_fn( e );
        }

        // Only available for integer types.
        template<typename N>
# if __PGBAR_CXX20
          requires std::is_integral_v<N>
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR scope::ProxySpan<scope::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  scope::ProxySpan<scope::NumericSpan<N>, Derived>>::type
# endif
          iterate( N startpoint, N endpoint ) &
        {
          return { scope::NumericSpan<typename std::decay<N>::type>( startpoint, endpoint, 1 ),
                   static_cast<Derived&>( *this ) };
        }
        template<typename N, typename F>
# if __PGBAR_CXX20
          requires std::is_integral_v<N>
        __PGBAR_CXX14_CNSTXPR void
# else
        __PGBAR_CXX14_CNSTXPR typename std::enable_if<std::is_integral<N>::value>::type
# endif
          iterate( N startpoint, N endpoint, F&& unary_fn )
        {
          for ( N e : iterate( startpoint, endpoint ) )
            unary_fn( e );
        }

        template<typename N>
# if __PGBAR_CXX20
          requires std::is_integral_v<N>
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR scope::ProxySpan<scope::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  scope::ProxySpan<scope::NumericSpan<N>, Derived>>::type
# endif
          iterate( N endpoint ) &
        {
          return { scope::NumericSpan<typename std::decay<N>::type>( {}, endpoint, 1 ),
                   static_cast<Derived&>( *this ) };
        }
        template<typename N, typename F>
# if __PGBAR_CXX20
          requires std::is_integral_v<N>
        __PGBAR_CXX14_CNSTXPR void
# else
        __PGBAR_CXX14_CNSTXPR typename std::enable_if<std::is_integral<N>::value>::type
# endif
          iterate( N endpoint, F&& unary_fn )
        {
          for ( N e : iterate( endpoint ) )
            unary_fn( e );
        }

        // Visualize unidirectional traversal of a iterator interval defined by parameters.
        template<typename I>
# if __PGBAR_CXX20
          requires std::negation_v<std::is_arithmetic<I>>
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR scope::ProxySpan<scope::IterSpan<I>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<!std::is_arithmetic<I>::value,
                                  scope::ProxySpan<scope::IterSpan<I>, Derived>>::type
# endif
          iterate( I startpoint, I endpoint ) & noexcept(
            traits::AnyOf<std::is_pointer<I>, std::is_nothrow_move_constructible<I>>::value )
        {
          return { scope::IterSpan<I>( std::move( startpoint ), std::move( endpoint ) ),
                   static_cast<Derived&>( *this ) };
        }
        template<typename I, typename F>
# if __PGBAR_CXX20
          requires std::negation_v<std::is_arithmetic<I>>
        __PGBAR_CXX14_CNSTXPR void
# else
        __PGBAR_CXX14_CNSTXPR typename std::enable_if<!std::is_arithmetic<I>::value>::type
# endif
          iterate( I startpoint, I endpoint, F&& unary_fn )
        {
          for ( auto&& e : iterate( std::move( startpoint ), std::move( endpoint ) ) )
            unary_fn( std::forward<decltype( e )>( e ) );
        }

        // Visualize unidirectional traversal of a abstract range interval defined by `container`'s
        // scope.
        template<class R>
# if __PGBAR_CXX20
          requires( ( std::is_class_v<std::decay_t<R>> || std::is_array_v<std::remove_reference_t<R>> )
                    && std::is_lvalue_reference_v<R> )
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR
          scope::ProxySpan<scope::IterSpan<traits::IteratorTrait_t<R>>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR typename std::enable_if<
          traits::AllOf<traits::AnyOf<std::is_class<typename std::decay<R>::type>,
                                      std::is_array<typename std::remove_reference<R>::type>>,
                        std::is_lvalue_reference<R>>::value,
          scope::ProxySpan<scope::IterSpan<traits::IteratorTrait_t<R>>, Derived>>::type
# endif
          iterate( R&& container ) &
        { // forward it to the iterator overload
# if __PGBAR_CXX20
          return iterate( std::ranges::begin( container ), std::ranges::end( container ) );
# else
          using std::begin;
          using std::end; // for ADL
          return iterate( begin( container ), end( container ) );
# endif
        }
        template<class R, typename F>
# if __PGBAR_CXX20
          requires( ( std::is_class_v<std::decay_t<R>> || std::is_array_v<std::remove_reference_t<R>> )
                    && std::is_lvalue_reference_v<R> )
        __PGBAR_CXX17_CNSTXPR void
# else
        __PGBAR_CXX17_CNSTXPR typename std::enable_if<
          traits::AllOf<traits::AnyOf<std::is_class<typename std::decay<R>::type>,
                                      std::is_array<typename std::remove_reference<R>::type>>,
                        std::is_lvalue_reference<R>>::value>::type
# endif
          iterate( R&& container, F&& unary_fn )
        {
          for ( auto&& e : iterate( container ) )
            unary_fn( std::forward<decltype( e )>( e ) );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR TaskCounter<Base, Derived>::~TaskCounter() = default;

      template<typename Base, typename Derived>
      class FrameCounter : public Base {
      protected:
        __details::types::Size idx_frame_;

      public:
        __PGBAR_EMPTY_CLASS( FrameCounter )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR FrameCounter<Base, Derived>::~FrameCounter() = default;
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER( assets::CharIndicator,
                                assets::TaskQuantity,
                                __PGBAR_PACK( assets::BasicAnimation, assets::BasicIndicator ) );
      __PGBAR_INHERIT_REGISTER( assets::BlockIndicator, assets::TaskQuantity, assets::BasicIndicator );

      __PGBAR_INHERIT_REGISTER( assets::Spinner, , assets::BasicAnimation );
      __PGBAR_INHERIT_REGISTER( assets::Scanner,
                                ,
                                __PGBAR_PACK( assets::BasicAnimation, assets::BasicIndicator ) );

      __PGBAR_INHERIT_REGISTER( assets::PercentMeter, assets::TaskQuantity, );
      __PGBAR_INHERIT_REGISTER( assets::SpeedMeter, assets::TaskQuantity, );
      __PGBAR_INHERIT_REGISTER( assets::CounterMeter, assets::TaskQuantity, );

      __PGBAR_INHERIT_REGISTER( assets::Timer, assets::TaskQuantity, );

      template<template<typename...> class Component>
      struct ComponentTraits {
        using type = TypeList<>;
      };
      template<template<typename...> class Component>
      using ComponentTraits_t = typename ComponentTraits<Component>::type;

# define __PGBAR_COMPONENT_REGISTER( Component, ... ) \
   template<>                                         \
   struct ComponentTraits<Component> {                \
     using type = TypeList<__VA_ARGS__>;              \
   }

      __PGBAR_COMPONENT_REGISTER( assets::Core, option::Colored, option::Bolded );
      __PGBAR_COMPONENT_REGISTER( assets::TaskQuantity, option::Tasks );
      __PGBAR_COMPONENT_REGISTER( assets::Description,
                                  option::Description,
                                  option::TrueMesg,
                                  option::FalseMesg,
                                  option::DescColor,
                                  option::TrueColor,
                                  option::FalseColor );
      __PGBAR_COMPONENT_REGISTER( assets::Segment,
                                  option::Divider,
                                  option::LeftBorder,
                                  option::RightBorder,
                                  option::InfoColor );
      __PGBAR_COMPONENT_REGISTER( assets::PercentMeter, );
      __PGBAR_COMPONENT_REGISTER( assets::SpeedMeter, option::SpeedUnit );
      __PGBAR_COMPONENT_REGISTER( assets::BasicAnimation, option::Shift, option::Lead, option::LeadColor );
      __PGBAR_COMPONENT_REGISTER( assets::BasicIndicator,
                                  option::Starting,
                                  option::Ending,
                                  option::StartColor,
                                  option::EndColor,
                                  option::BarLength,
                                  option::FillerColor );

      template<>
      struct ComponentTraits<assets::CharIndicator> {
        using type = Merge_t<ComponentTraits_t<assets::TaskQuantity>,
                             ComponentTraits_t<assets::BasicAnimation>,
                             ComponentTraits_t<assets::BasicIndicator>,
                             TypeList<option::Remains, option::Filler, option::RemainsColor>>;
      };
      template<>
      struct ComponentTraits<assets::BlockIndicator> {
        using type = Merge_t<ComponentTraits_t<assets::TaskQuantity>,
                             ComponentTraits_t<assets::BasicAnimation>,
                             ComponentTraits_t<assets::BasicIndicator>,
                             TypeList<option::Remains, option::Filler, option::RemainsColor>>;
      };
      template<>
      struct ComponentTraits<assets::Spinner> {
        using type =
          Merge_t<ComponentTraits_t<assets::TaskQuantity>, ComponentTraits_t<assets::BasicAnimation>>;
      };
      template<>
      struct ComponentTraits<assets::Scanner> {
        using type = Merge_t<ComponentTraits_t<assets::TaskQuantity>,
                             ComponentTraits_t<assets::BasicAnimation>,
                             ComponentTraits_t<assets::BasicIndicator>,
                             TypeList<option::Filler>>;
      };
    } // namespace traits

    namespace prefabs {
      template<template<typename...> class BarType, typename Derived>
      class BasicConfig
        : public traits::LI_t<BarType,
                              assets::Description,
                              assets::Segment,
                              assets::PercentMeter,
                              assets::SpeedMeter,
                              assets::CounterMeter,
                              assets::Timer>::template type<assets::Core<Derived>, Derived> {
        // BarType must inherit from BasicIndicator or BasicAnimation
        static_assert( traits::AnyOf<traits::Contain<traits::TopoSort_t<traits::TemplateList<BarType>>,
                                                     assets::BasicIndicator>,
                                     traits::Contain<traits::TopoSort_t<traits::TemplateList<BarType>>,
                                                     assets::BasicAnimation>>::value,
                       "pgbar::__details::prefabs::BasicConfig: Invalid progress bar type" );

        using Self       = BasicConfig;
        using Base       = typename traits::LI_t<BarType,
                                                 assets::Description,
                                                 assets::Segment,
                                                 assets::PercentMeter,
                                                 assets::SpeedMeter,
                                                 assets::CounterMeter,
                                                 assets::Timer>::template type<assets::Core<Derived>, Derived>;
        using Constraint = traits::Merge_t<traits::TypeList<option::Style>,
                                           traits::ComponentTraits_t<BarType>,
                                           traits::ComponentTraits_t<assets::Description>,
                                           traits::ComponentTraits_t<assets::Segment>,
                                           traits::ComponentTraits_t<assets::SpeedMeter>,
                                           traits::ComponentTraits_t<assets::Timer>>;

        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( BasicConfig& cfg,
                                                                      option::Style&& val ) noexcept
        {
          cfg.visual_masks_ = val.value();
        }

        template<bool Enable>
        class Modifier final {
          friend Self;
          Self& myself_;

# if __PGBAR_CXX17
          Modifier( Self& myself ) noexcept : myself_ { myself } { myself_.rw_mtx_.lock(); }

        public:
          ~Modifier() noexcept { myself_.rw_mtx_.unlock(); }
# else
          // There was not standard NRVO support before C++17.
          std::atomic<bool> is_owner_;

          Modifier( Modifier&& rhs ) noexcept : myself_ { rhs.myself_ }, is_owner_ { true }
          {
            rhs.is_owner_.store( false, std::memory_order_release );
          }
          Modifier( Self& myself ) noexcept : myself_ { myself }, is_owner_ { true }
          {
            myself_.rw_mtx_.lock();
          }

        public:
          ~Modifier() noexcept
          {
            if ( is_owner_.load( std::memory_order_acquire ) )
              myself_.rw_mtx_.unlock();
          }
# endif
          Modifier( const Modifier& )              = delete;
          Modifier& operator=( const Modifier& ) & = delete;

# define __PGBAR_METHOD( MethodName, EnumName )                      \
   Modifier&& MethodName()&& noexcept                                \
   {                                                                 \
     myself_.visual_masks_.set( utils::as_val( EnumName ), Enable ); \
     return static_cast<Modifier&&>( *this );                        \
   }
          __PGBAR_METHOD( percent, Mask::Per )
          __PGBAR_METHOD( animation, Mask::Ani )
          __PGBAR_METHOD( counter, Mask::Cnt )
          __PGBAR_METHOD( speed, Mask::Sped )
          __PGBAR_METHOD( elapsed, Mask::Elpsd )
          __PGBAR_METHOD( countdown, Mask::Cntdwn )
# undef __PGBAR_METHOD
          Modifier&& entire() && noexcept
          {
            if __PGBAR_CXX17_CNSTXPR ( Enable )
              myself_.visual_masks_.set();
            else
              myself_.visual_masks_.reset();
            return static_cast<Modifier&&>( *this );
          }
        };

      protected:
        enum class Mask : types::Size { Per = 0, Ani, Cnt, Sped, Elpsd, Cntdwn };
        std::bitset<6> visual_masks_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size common_render_size() const noexcept
        {
          return ( visual_masks_[utils::as_val( Mask::Per )] ? this->fixed_len_percent() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Cnt )] ? this->fixed_len_counter() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Sped )] ? this->fixed_len_speed() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Elpsd )] && visual_masks_[utils::as_val( Mask::Cntdwn )]
                     ? this->fixed_len_hybird()
                     : ( visual_masks_[utils::as_val( Mask::Elpsd )]
                           ? this->fixed_len_elapsed()
                           : ( this->visual_masks_[utils::as_val( Mask::Cntdwn )]
                                 ? this->fixed_len_countdown()
                                 : 0 ) ) )
               + ( visual_masks_[utils::as_val( Mask::Elpsd )] && visual_masks_[utils::as_val( Mask::Cntdwn )]
                     ? 3
                     : 0 )
               + 1;
        }

      public:
        // Percent Meter
        static constexpr types::Byte Per    = 1 << 0;
        // Animation
        static constexpr types::Byte Ani    = 1 << 1;
        // Task Progress Counter
        static constexpr types::Byte Cnt    = 1 << 2;
        // Speed Meter
        static constexpr types::Byte Sped   = 1 << 3;
        // Elapsed Timer
        static constexpr types::Byte Elpsd  = 1 << 4;
        // Countdown Timer
        static constexpr types::Byte Cntdwn = 1 << 5;
        // Enable all components
        static constexpr types::Byte Entire = ~0;

# if __PGBAR_CXX20
        template<typename... Args>
          requires( !traits::Repeat<traits::TypeList<Args...>>::value
                    && traits::AllBelongAny<traits::TypeList<Args...>, Constraint>::value )
# else
        template<typename... Args,
                 typename = typename std::enable_if<
                   traits::AllOf<traits::Not<traits::Repeat<traits::TypeList<Args...>>>,
                                 traits::AllBelongAny<traits::TypeList<Args...>, Constraint>>::value>::type>
# endif
        BasicConfig( Args... args )
        {
          static_cast<Derived*>( this )->template initialize<Args...>();
          (void)std::initializer_list<char> { ( unpacker( *this, std::move( args ) ), '\0' )... };
        }

        BasicConfig( const Self& lhs ) noexcept( traits::AllOf<std::is_nothrow_default_constructible<Base>,
                                                               std::is_nothrow_copy_assignable<Base>>::value )
          : Base()
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { lhs.rw_mtx_ };
          visual_masks_ = lhs.visual_masks_;
          Base::operator=( lhs );
        }
        BasicConfig( Self&& rhs ) noexcept : Base()
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rhs.rw_mtx_ };
          using std::swap;
          swap( visual_masks_, rhs.visual_masks_ );
          Base::operator=( std::move( rhs ) );
        }
        Self& operator=( const Self& lhs ) & noexcept( std::is_nothrow_copy_assignable<Base>::value )
        {
          __PGBAR_PURE_ASSUME( this != &lhs );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_ };
          concurrent::SharedLock<concurrent::SharedMutex> lock2 { lhs.rw_mtx_ };
          visual_masks_ = lhs.visual_masks_;
          Base::operator=( lhs );
          return *this;
        }
        Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != &rhs );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_ };
          std::lock_guard<concurrent::SharedMutex> lock2 { rhs.rw_mtx_ };
          using std::swap;
          swap( visual_masks_, rhs.visual_masks_ );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        virtual ~BasicConfig() = default;

        Derived& style( types::Byte val ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Style( val ) );
          return static_cast<Derived&>( *this );
        }

        template<typename Arg, typename... Args>
# if __PGBAR_CXX20
          requires( !traits::Repeat<traits::TypeList<Arg, Args...>>::value
                    && traits::AllBelongAny<traits::TypeList<Arg, Args...>, Constraint>::value )
        Derived&
# else
        typename std::enable_if<
          traits::AllOf<traits::Not<traits::Repeat<traits::TypeList<Arg, Args...>>>,
                        traits::AllBelongAny<traits::TypeList<Arg, Args...>, Constraint>>::value,
          Derived&>::type
# endif
          set( Arg arg, Args... args ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, std::move( arg ) );
          (void)std::initializer_list<char> { ( unpacker( *this, std::move( args ) ), '\0' )... };
          return static_cast<Derived&>( *this );
        }

        __PGBAR_NODISCARD types::Size fixed_size() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return static_cast<const Derived*>( this )->fixed_render_size();
        }

        __PGBAR_NODISCARD Modifier<true> enable() & noexcept { return Modifier<true>( *this ); }
        __PGBAR_NODISCARD Modifier<false> disable() & noexcept { return Modifier<false>( *this ); }

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR void swap( BasicConfig& lhs ) noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_ };
          std::lock_guard<concurrent::SharedMutex> lock2 { lhs.rw_mtx_ };
          using std::swap;
          swap( visual_masks_, lhs.visual_masks_ );
          Base::swap( lhs );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR void swap( BasicConfig& a, BasicConfig& b ) noexcept
        {
          a.swap( b );
        }
      };
    } // namespace prefabs
  } // namespace __details

  namespace config {
    using TimeUnit = __details::types::TimeUnit;
    // Get the current output interval.
    template<Channel Outlet>
    __PGBAR_NODISCARD TimeUnit refresh_interval()
    {
      return __details::render::Renderer<Outlet>::working_interval();
    }
    // Set the new output interval.
    template<Channel Outlet>
    void refresh_interval( TimeUnit new_rate )
    {
      __details::render::Renderer<Outlet>::working_interval( std::move( new_rate ) );
    }
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool intty( Channel channel ) noexcept
    {
      return __details::console::intty( channel );
    }

    class CharBar : public __details::prefabs::BasicConfig<__details::assets::CharIndicator, CharBar> {
      using Base = __details::prefabs::BasicConfig<__details::assets::CharIndicator, CharBar>;
      friend Base;

      template<typename... Options>
      void initialize()
      {
        // The types in the tuple are never repeated.
        using ParamList = __details::traits::TypeList<Options...>;
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Shift, ParamList>::value )
          unpacker( *this, option::Shift( -2 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Lead, ParamList>::value )
          unpacker( *this, option::Lead( ">" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Starting, ParamList>::value )
          unpacker( *this, option::Starting( "[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Ending, ParamList>::value )
          unpacker( *this, option::Ending( "]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::BarLength, ParamList>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Filler, ParamList>::value )
          unpacker( *this, option::Filler( "=" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Remains, ParamList>::value )
          unpacker( *this, option::Remains( " " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Divider, ParamList>::value )
          unpacker( *this, option::Divider( " | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::InfoColor, ParamList>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::SpeedUnit, ParamList>::value )
          unpacker(
            *this,
            option::SpeedUnit( std::array<__details::types::String, 4>( { "Hz", "kHz", "MHz", "GHz" } ) ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Magnitude, ParamList>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Style, ParamList>::value )
          unpacker( *this, option::Style( Base::Entire ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size() + this->fixed_len_description()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 )
             + this->fixed_len_segment(
               this->visual_masks_.count()
               - ( this->visual_masks_[__details::utils::as_val( Base::Mask::Cntdwn )]
                   && this->visual_masks_[__details::utils::as_val( Base::Mask::Elpsd )] )
               + ( !this->true_mesg_.empty() || !this->false_mesg_.empty() || !this->description_.empty() ) );
      }

    public:
      using Base::Base;
      CharBar( const CharBar& )              = default;
      CharBar( CharBar&& )                   = default;
      CharBar& operator=( const CharBar& ) & = default;
      CharBar& operator=( CharBar&& ) &      = default;
      virtual ~CharBar()                     = default;
    };

    class BlckBar : public __details::prefabs::BasicConfig<__details::assets::BlockIndicator, BlckBar> {
      using Base = __details::prefabs::BasicConfig<__details::assets::BlockIndicator, BlckBar>;
      friend Base;

      template<typename... Options>
      void initialize()
      {
        using ParamList = __details::traits::TypeList<Options...>;
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::BarLength, ParamList>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Divider, ParamList>::value )
          unpacker( *this, option::Divider( " | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::InfoColor, ParamList>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::SpeedUnit, ParamList>::value )
          unpacker(
            *this,
            option::SpeedUnit( std::array<__details::types::String, 4>( { "Hz", "kHz", "MHz", "GHz" } ) ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Magnitude, ParamList>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Style, ParamList>::value )
          unpacker( *this, option::Style( Base::Entire ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size() + this->fixed_len_description()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 )
             + this->fixed_len_segment(
               this->visual_masks_.count()
               - ( this->visual_masks_[__details::utils::as_val( Base::Mask::Cntdwn )]
                   && this->visual_masks_[__details::utils::as_val( Base::Mask::Elpsd )] )
               + ( !this->true_mesg_.empty() || !this->false_mesg_.empty() || !this->description_.empty() ) );
      }

    public:
      using Base::Base;
      BlckBar( const BlckBar& )              = default;
      BlckBar( BlckBar&& )                   = default;
      BlckBar& operator=( const BlckBar& ) & = default;
      BlckBar& operator=( BlckBar&& ) &      = default;
      virtual ~BlckBar()                     = default;
    };

    class SpinBar : public __details::prefabs::BasicConfig<__details::assets::Spinner, SpinBar> {
      using Base = __details::prefabs::BasicConfig<__details::assets::Spinner, SpinBar>;
      friend Base;

      template<typename... Options>
      void initialize()
      {
        using ParamList = __details::traits::TypeList<Options...>;
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Shift, ParamList>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Lead, ParamList>::value )
          unpacker( *this, option::Lead( { "/", "-", "\\", "|" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Divider, ParamList>::value )
          unpacker( *this, option::Divider( " | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::InfoColor, ParamList>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::SpeedUnit, ParamList>::value )
          unpacker(
            *this,
            option::SpeedUnit( std::array<__details::types::String, 4>( { "Hz", "kHz", "MHz", "GHz" } ) ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Magnitude, ParamList>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Style, ParamList>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )]
                   ? this->fixed_len_animation() + this->fixed_len_description()
                       + ( !this->true_mesg_.empty() || !this->false_mesg_.empty()
                           || !this->description_.empty() )
                   : 0 )
             + this->fixed_len_segment(
               this->visual_masks_.count()
               - ( this->visual_masks_[__details::utils::as_val( Base::Mask::Cntdwn )]
                   && this->visual_masks_[__details::utils::as_val( Base::Mask::Elpsd )] ) );
      }

    public:
      using Base::Base;
      SpinBar( const SpinBar& )              = default;
      SpinBar( SpinBar&& )                   = default;
      SpinBar& operator=( const SpinBar& ) & = default;
      SpinBar& operator=( SpinBar&& ) &      = default;
      virtual ~SpinBar()                     = default;
    };

    class ScanBar : public __details::prefabs::BasicConfig<__details::assets::Scanner, ScanBar> {
      using Base = __details::prefabs::BasicConfig<__details::assets::Scanner, ScanBar>;
      friend Base;

      template<typename... Options>
      void initialize()
      {
        using ParamList = __details::traits::TypeList<Options...>;
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Shift, ParamList>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Starting, ParamList>::value )
          unpacker( *this, option::Starting( "[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Ending, ParamList>::value )
          unpacker( *this, option::Ending( "]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::BarLength, ParamList>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Filler, ParamList>::value )
          unpacker( *this, option::Filler( "-" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Lead, ParamList>::value )
          unpacker( *this, option::Lead( "<==>" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Divider, ParamList>::value )
          unpacker( *this, option::Divider( " | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::InfoColor, ParamList>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::SpeedUnit, ParamList>::value )
          unpacker(
            *this,
            option::SpeedUnit( std::array<__details::types::String, 4>( { "Hz", "kHz", "MHz", "GHz" } ) ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Magnitude, ParamList>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Belong<option::Style, ParamList>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size() + this->fixed_len_description()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 )
             + this->fixed_len_segment(
               this->visual_masks_.count()
               - ( this->visual_masks_[__details::utils::as_val( Base::Mask::Cntdwn )]
                   && this->visual_masks_[__details::utils::as_val( Base::Mask::Elpsd )] )
               + ( !this->true_mesg_.empty() || !this->false_mesg_.empty() || !this->description_.empty() ) );
      }

    public:
      using Base::Base;
      ScanBar( const ScanBar& )              = default;
      ScanBar( ScanBar&& )                   = default;
      ScanBar& operator=( const ScanBar& ) & = default;
      ScanBar& operator=( ScanBar&& ) &      = default;
      virtual ~ScanBar()                     = default;
    };
  } // namespace config

  namespace __details {
    namespace traits {
      template<typename C>
      struct is_config
        : AnyOf<std::is_same<typename std::remove_cv<C>::type, config::CharBar>,
                std::is_same<typename std::remove_cv<C>::type, config::BlckBar>,
                std::is_same<typename std::remove_cv<C>::type, config::SpinBar>,
                std::is_same<typename std::remove_cv<C>::type, config::ScanBar>> {};

      template<typename C>
      struct ConfigTraits;
      template<typename C>
      using ConfigTraits_t = typename ConfigTraits<C>::type;

# define __PGBAR_TRAIT_REGISTER( Config, ... )                                            \
   template<>                                                                             \
   struct ConfigTraits<Config> {                                                          \
     static_assert( is_config<Config>::value,                                             \
                    "pgbar::__details::traits::ConfigTraits: Unexpected type mismatch" ); \
     using type = TemplateList<__VA_ARGS__>;                                              \
   }

      __PGBAR_TRAIT_REGISTER( config::CharBar, assets::TaskCounter, assets::FrameCounter );
      __PGBAR_TRAIT_REGISTER( config::BlckBar, assets::TaskCounter );
      __PGBAR_TRAIT_REGISTER( config::SpinBar, assets::TaskCounter, assets::FrameCounter );
      __PGBAR_TRAIT_REGISTER( config::ScanBar, assets::TaskCounter, assets::FrameCounter );
    } // namespace traits

    namespace render {
      template<typename Config>
      struct CommonBuilder : public Config {
        using Config::Config;
        CommonBuilder( const CommonBuilder& lhs )
          noexcept( std::is_nothrow_copy_constructible<Config>::value )
          : Config( static_cast<Config&>( lhs ) )
        {}
        CommonBuilder( CommonBuilder&& rhs ) noexcept : Config( static_cast<Config&&>( rhs ) ) {}
        CommonBuilder( const Config& config ) noexcept( std::is_nothrow_copy_constructible<Config>::value )
          : Config( config )
        {}
        CommonBuilder( Config&& config ) noexcept : Config( std::move( config ) ) {}
        virtual ~CommonBuilder() = default;

# define __PGBAR_METHOD( ParamType, Operation, Noexcept )  \
   CommonBuilder& operator=( ParamType config ) & Noexcept \
   {                                                       \
     Config::operator=( Operation( config ) );             \
     return *this;                                         \
   }
        __PGBAR_METHOD( const CommonBuilder&, , noexcept( std::is_nothrow_copy_assignable<Config>::value ) )
        __PGBAR_METHOD( CommonBuilder&&, std::move, noexcept )
        __PGBAR_METHOD( const Config&, , noexcept( std::is_nothrow_copy_assignable<Config>::value ) )
        __PGBAR_METHOD( Config&&, std::move, noexcept )
# undef __PGBAR_METHOD

      protected:
        /**
         * Builds and only builds the components belows:
         * `CounterMeter`, `SpeedMeter`, `ElapsedTimer` and `CountdownTimer`
         */
        __PGBAR_INLINE_FN io::Stringbuf& common_build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          using Self = Config;
          if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )]
               || this->visual_masks_[utils::as_val( Self::Mask::Sped )]
               || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
               || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] ) {
            this->build_font( buffer, this->info_col_ );
            if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )] ) {
              buffer << this->build_counter( num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                this->build_divider( buffer );
            }
            const auto time_passed = std::chrono::steady_clock::now() - zero_point;
            if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )] ) {
              buffer << this->build_speed( time_passed, num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                this->build_divider( buffer );
            }
            if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                 && this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
              this->build_hybird( buffer, time_passed, num_task_done, num_all_tasks );
            else if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )] )
              buffer << this->build_elapsed( time_passed );
            else if ( this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
              buffer << this->build_countdown( time_passed, num_task_done, num_all_tasks );
          }
          return buffer;
        }
      };

      template<typename Config, typename Impl>
      struct IndirectBuilder : public CommonBuilder<Config> {
      private:
        using Self = Config;
        using Base = CommonBuilder<Config>;

      protected:
        template<typename... Args>
        __PGBAR_INLINE_FN io::Stringbuf& indirect_build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          types::Float num_percent,
          const std::chrono::steady_clock::time_point& zero_point,
          Args&&... args ) const
        {
          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer );
          if ( !this->description_.empty() && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            static_cast<const Impl*>( this )->build_animation( buffer, std::forward<Args>( args )... );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escodes::reset_font;
        }
        template<typename... Args>
        __PGBAR_INLINE_FN io::Stringbuf& indirect_build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          types::Float num_percent,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point,
          Args&&... args ) const
        {
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer, final_mesg );
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            static_cast<const Impl*>( this )->build_animation( buffer, std::forward<Args>( args )... );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escodes::reset_font;
        }

      public:
        using Base::Base;
      };

      template<typename Config>
      struct Builder;
      template<>
      struct Builder<config::CharBar> final
        : public IndirectBuilder<config::CharBar, Builder<config::CharBar>> {
      private:
        using Base = IndirectBuilder<config::CharBar, Builder<config::CharBar>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Float num_percent,
                                                          types::Size num_frame_cnt ) const
        {
          return this->build_char( buffer, num_percent, num_frame_cnt );
        }

      public:
        using Base::Base;
        Builder( config::CharBar&& rhs ) noexcept : Base( std::move( rhs ) ) {}

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this->indirect_build( buffer,
                                       num_task_done,
                                       num_all_tasks,
                                       num_percent,
                                       zero_point,
                                       num_percent,
                                       num_frame_cnt );
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this->indirect_build( buffer,
                                       num_task_done,
                                       num_all_tasks,
                                       num_percent,
                                       final_mesg,
                                       zero_point,
                                       num_percent,
                                       num_frame_cnt );
        }
      };

      template<>
      struct Builder<config::BlckBar> final
        : public IndirectBuilder<config::BlckBar, Builder<config::BlckBar>> {
      private:
        using Base = IndirectBuilder<config::BlckBar, Builder<config::BlckBar>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Float num_percent ) const
        {
          return this->build_block( buffer, num_percent );
        }

      public:
        using Base::Base;
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_percent );
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this->indirect_build( buffer,
                                       num_task_done,
                                       num_all_tasks,
                                       num_percent,
                                       final_mesg,
                                       zero_point,
                                       num_percent );
        }
      };

      template<>
      struct Builder<config::ScanBar> final
        : public IndirectBuilder<config::ScanBar, Builder<config::ScanBar>> {
      private:
        using Base = IndirectBuilder<config::ScanBar, Builder<config::ScanBar>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Size num_frame_cnt ) const
        {
          return this->build_scanner( buffer, num_frame_cnt );
        }

      public:
        using Base::Base;
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_frame_cnt );
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this->indirect_build( buffer,
                                       num_task_done,
                                       num_all_tasks,
                                       num_percent,
                                       final_mesg,
                                       zero_point,
                                       num_frame_cnt );
        }
      };

      template<>
      struct Builder<config::SpinBar> final : public CommonBuilder<config::SpinBar> {
      private:
        using Self = config::SpinBar;
        using CommonBuilder<Self>::CommonBuilder;

      public:
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( this->visual_masks_.any() )
            this->build_lborder( buffer );

          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            this->build_spinner( buffer, num_frame_cnt );
            if ( !this->description_.empty() ) {
              buffer << constants::blank;
              this->build_description( buffer );
            }
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escodes::reset_font;
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_PURE_ASSUME( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( this->visual_masks_.any() )
            this->build_lborder( buffer );

          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            if ( ( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty() ) {
              this->build_spinner( buffer, num_frame_cnt );
              if ( !this->description_.empty() )
                buffer << constants::blank;
            }
            this->build_description( buffer, final_mesg );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escodes::reset_font;
        }
      };

      template<typename Config, typename Enable = void>
      struct RenderAction;
    } // namespace render

    namespace prefabs {
# if __PGBAR_CXX20
      template<typename Config, traits::MutexLike MutexMode, Channel Outlet>
# else
      template<typename Config, typename MutexMode, Channel Outlet>
# endif
      class BasicBar
        : public traits::LI<
            traits::ConfigTraits_t<Config>>::template type<Indicator, BasicBar<Config, MutexMode, Outlet>> {
        static_assert( traits::Contain<traits::ConfigTraits_t<Config>, assets::TaskCounter>::value,
                       "pgbar::__details::prefabs::BasicBar: Invalid config type" );
# if !__PGBAR_CXX20
        static_assert( traits::is_mutex<MutexMode>::value,
                       "pgbar::__details::prefabs::BasicBar: Invalid mutex type" );
# endif
        using Self = BasicBar;
        using Base = typename traits::LI<traits::ConfigTraits_t<Config>>::template type<Indicator, Self>;

        template<typename, typename>
        friend struct render::RenderAction;

        render::Builder<Config> config_;
        __PGBAR_NOUNIQUEADDR mutable MutexMode mtx_;

      protected:
        virtual void do_terminate( bool forced = false ) noexcept
        {
          auto& executor = render::Renderer<Outlet>::itself();
          if ( !executor.empty() ) {
            if ( !forced )
              executor.suspend();
            executor.appoint();
          }
        }
        virtual void do_setup() & noexcept( false )
        {
          auto& executor = render::Renderer<Outlet>::itself();
          if ( !executor.try_appoint( [this]() {
                 auto& ostream = io::OStream<Outlet>::itself();
                 switch ( this->state_.load( std::memory_order_acquire ) ) {
                 case Self::State::Begin: {
                   ostream << console::escodes::store_cursor;
                   render::RenderAction<Config>::boot( *this );
                   ostream << io::flush;
                 }
                   __PGBAR_FALLTHROUGH;
                 case Self::State::StrictRefresh:  __PGBAR_FALLTHROUGH;
                 case Self::State::LenientRefresh: {
                   ostream << console::escodes::restore_cursor;
                   render::RenderAction<Config>::process( *this );
                   ostream << io::flush;
                 } break;
                 case Self::State::Finish: {
                   ostream << console::escodes::restore_cursor;
                   render::RenderAction<Config>::finish( *this );
                   ostream << '\n';
                   ostream << io::flush << io::release;
                 } break;
                 default: return;
                 }
               } ) )
            __PGBAR_UNLIKELY throw exception::InvalidState(
              "pgbar: another progress bar instance is already running" );

          executor.activate();
        }

        __PGBAR_INLINE_FN void do_reset( bool forced = false ) noexcept
        {
          if ( this->is_running() ) {
            if ( forced )
              __PGBAR_UNLIKELY
            this->state_.store( Indicator::State::Stopped, std::memory_order_release );
            else
            {
              auto try_update = [this]( Indicator::State expected ) noexcept {
                return this->state_.compare_exchange_strong( expected,
                                                             Indicator::State::Finish,
                                                             std::memory_order_acq_rel,
                                                             std::memory_order_relaxed );
              };
              try_update( Indicator::State::Begin ) || try_update( Indicator::State::StrictRefresh )
                || try_update( Indicator::State::LenientRefresh );
            }
            do_terminate( forced );
          } else
            this->state_.store( Indicator::State::Stopped, std::memory_order_release );
        }
        __PGBAR_INLINE_FN void do_tick( types::Size next_step ) & noexcept( false )
        {
          switch ( this->state_.load( std::memory_order_acquire ) ) {
          case Indicator::State::Stopped: {
            this->task_end_.store( config_.tasks(), std::memory_order_release );
            if __PGBAR_CXX17_CNSTXPR ( std::is_same<Config, config::CharBar>::value
                                       || std::is_same<Config, config::BlckBar>::value )
              if ( this->task_end_.load( std::memory_order_acquire ) == 0 )
                __PGBAR_UNLIKELY throw exception::InvalidState( "pgbar: the number of tasks is zero" );

            do_setup();
            this->task_cnt_.store( 0, std::memory_order_release );
            this->zero_point_ = std::chrono::steady_clock::now();
            this->state_.store( Indicator::State::Begin, std::memory_order_release );
          }
            __PGBAR_FALLTHROUGH;
          case Indicator::State::Begin: {
            if __PGBAR_CXX17_CNSTXPR ( !std::is_same<Config, config::CharBar>::value
                                       && !std::is_same<Config, config::BlckBar>::value )
              if ( this->task_end_.load( std::memory_order_acquire ) == 0 )
                return;
          }
            __PGBAR_FALLTHROUGH;
          case Indicator::State::StrictRefresh: {
            this->task_cnt_.fetch_add( next_step, std::memory_order_release );

            if ( this->task_cnt_.load( std::memory_order_acquire )
                 >= this->task_end_.load( std::memory_order_acquire ) )
              __PGBAR_UNLIKELY
              {
                this->final_mesg_ = true;
                do_reset();
              }
          } break;

          default: return;
          }
        }

      public:
        using ConfigType                = Config;
        using MutexType                 = MutexMode;
        static constexpr Channel stream = Outlet;

        BasicBar( Config config = Config() )
          noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
          : config_ { std::move( config ) }
        {}
# if __PGBAR_CXX20
        template<typename... Args>
          requires std::is_constructible_v<Config, Args...>
# else
        template<typename... Args,
                 typename = typename std::enable_if<std::is_constructible<Config, Args...>::value>::type>
# endif
        BasicBar( Args&&... args ) : BasicBar( Config( std::forward<Args>( args )... ) )
        {}
        // There is nothing to move in the base classes.
        BasicBar( Self&& rhs ) noexcept : config_ { std::move( rhs.config_ ) } {}
        Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_PURE_ASSUME( this != &rhs );
          Base::operator=( std::move( rhs ) );
          config_ = std::move( rhs.config_ );
          return *this;
        }
        virtual ~BasicBar() noexcept { do_reset( true ); }

        void tick() & override final
        {
          std::lock_guard<MutexMode> lock { this->mtx_ };
          do_tick( 1 );
        }
        void tick( types::Size next_step ) & override final
        {
          std::lock_guard<MutexMode> lock { this->mtx_ };
          switch ( this->state_.load( std::memory_order_acquire ) ) {
          case Base::State::Stopped: do_tick( 0 ); __PGBAR_FALLTHROUGH;
          default:                   break;
          }
          const auto task_cnt = this->task_cnt_.load( std::memory_order_acquire );
          const auto task_end = this->task_end_.load( std::memory_order_acquire );
          do_tick( next_step + task_cnt > task_end ? task_end - task_cnt : next_step );
        }
        /**
         * Set the iteration step of the progress bar to a specified percentage.
         * Ignore the call if the iteration count exceeds the given percentage.
         * If `percentage` is bigger than 100, it will be set to 100.
         *
         * @param percentage Value range: [0, 100].
         */
        void tick_to( std::uint8_t percentage ) & override final
        {
          std::lock_guard<MutexMode> lock { this->mtx_ };
          const auto task_cnt = this->task_cnt_.load( std::memory_order_acquire );
          const auto task_end = this->task_end_.load( std::memory_order_acquire );
          if ( percentage <= 100 ) {
            const auto target_progress = static_cast<types::Size>( task_end * percentage * 0.01 );
            __PGBAR_ASSERT( target_progress <= task_end );
            if ( target_progress > task_cnt )
              do_tick( target_progress - task_cnt );
          } else
            do_tick( task_end - task_cnt );
        }

        void reset( bool final_mesg = true ) override final
        {
          std::lock_guard<MutexMode> lock { this->mtx_ };
          this->final_mesg_ = final_mesg;
          do_reset();
        }

        Config& config() & noexcept { return config_; }
        const Config& config() const& noexcept { return config_; }
        Config config() && noexcept { return config_; }

        __PGBAR_CXX20_CNSTXPR void swap( BasicBar& lhs ) noexcept
        {
          __PGBAR_PURE_ASSUME( this != &lhs );
          __PGBAR_ASSERT( this->is_running() == false );
          __PGBAR_ASSERT( lhs.is_running() == false );
          Base::swap( lhs );
          config_.swap( lhs.config_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( BasicBar& a, BasicBar& b ) noexcept { a.swap( b ); }
      };
    } // namespace prefabs

    namespace traits {
      template<typename B>
      struct is_bar {
      private:
        template<typename T>
        struct Helper : std::false_type {};
        template<typename C, typename M, Channel O>
        struct Helper<prefabs::BasicBar<C, M, O>> : std::true_type {};

      public:
        static constexpr bool value = Helper<typename std::remove_cv<B>::type>::value;
      };
    } // namespace traits
  } // namespace __details

  using Threadsafe = __details::concurrent::Mutex;
  // A empty class that satisfies the "Basic lockable" requirement.
  class Threadunsafe final {
  public:
    constexpr Threadunsafe()              = default;
    __PGBAR_CXX20_CNSTXPR ~Threadunsafe() = default;
    __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void lock() noexcept {}
    __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unlock() noexcept {}
  };

  /**
   * The simplest progress bar, which is what you think it is.
   *
   * It's structure is shown below:
   * {LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, Channel Outlet = Channel::Stderr>
  using ProgressBar = __details::prefabs::BasicBar<config::CharBar, MutexMode, Outlet>;
  /**
   * A progress bar with a smoother bar, requires an Unicode-supported terminal.
   *
   * It's structure is shown below:
   * {LeftBorder}{Description}{Percent}{Starting}{BlockBar}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, Channel Outlet = Channel::Stderr>
  using BlockProgressBar = __details::prefabs::BasicBar<config::BlckBar, MutexMode, Outlet>;
  /**
   * A progress bar without bar indicator, replaced by a fixed animation component.
   *
   * It's structure is shown below:
   * {LeftBorder}{Lead}{Description}{Percent}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, Channel Outlet = Channel::Stderr>
  using SpinnerBar = __details::prefabs::BasicBar<config::SpinBar, MutexMode, Outlet>;
  /**
   * The indeterminate progress bar.
   *
   * It's structure is shown below:
   * {LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, Channel Outlet = Channel::Stderr>
  using ScannerBar = __details::prefabs::BasicBar<config::ScanBar, MutexMode, Outlet>;

  namespace __details {
    namespace render {
      template<typename Config>
      struct RenderAction<
        Config,
        typename std::enable_if<traits::AnyOf<std::is_same<Config, config::CharBar>,
                                              std::is_same<Config, config::SpinBar>,
                                              std::is_same<Config, config::ScanBar>>::value>::type> {
        template<typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void boot( prefabs::BasicBar<Config, Mutex, Outlet>& bar )
        {
          using Self = prefabs::BasicBar<Config, Mutex, Outlet>;
          __PGBAR_ASSERT( io::OStream<Outlet>::itself().empty() == false );
          __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
          bar.idx_frame_ = 0;
          bar.config_.build( io::OStream<Outlet>::itself(),
                             bar.idx_frame_,
                             bar.task_cnt_.load( std::memory_order_acquire ),
                             bar.task_end_.load( std::memory_order_acquire ),
                             bar.zero_point_ );
          auto expected = Self::State::Begin;
          if __PGBAR_CXX17_CNSTXPR ( std::is_same<Config, config::CharBar>::value )
            bar.state_.compare_exchange_strong( expected,
                                                Self::State::StrictRefresh,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed );
          else
            bar.state_.compare_exchange_strong( expected,
                                                bar.task_end_.load( std::memory_order_acquire ) == 0
                                                  ? Self::State::LenientRefresh
                                                  : Self::State::StrictRefresh,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed );
        }
        template<typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void process( prefabs::BasicBar<Config, Mutex, Outlet>& bar )
        {
          __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
          auto& buffer = io::OStream<Outlet>::itself();
          __PGBAR_ASSERT( buffer.empty() == false );
          buffer << console::escodes::clear_suffix;

          bar.config_.build( buffer,
                             bar.idx_frame_,
                             bar.task_cnt_.load( std::memory_order_acquire ),
                             bar.task_end_.load( std::memory_order_acquire ),
                             bar.zero_point_ );
          ++bar.idx_frame_;
        }
        template<typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void finish( prefabs::BasicBar<Config, Mutex, Outlet>& bar )
        {
          using Self = prefabs::BasicBar<Config, Mutex, Outlet>;
          __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
          auto& buffer = io::OStream<Outlet>::itself();
          __PGBAR_ASSERT( buffer.empty() == false );
          buffer << console::escodes::clear_suffix;

          bar.config_.build( buffer,
                             bar.idx_frame_,
                             bar.task_cnt_.load( std::memory_order_acquire ),
                             bar.task_end_.load( std::memory_order_acquire ),
                             bar.final_mesg_,
                             bar.zero_point_ );
          bar.state_.store( Self::State::Stopped, std::memory_order_release );
        }
      };

      template<>
      struct RenderAction<config::BlckBar, void> {
        template<typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void boot( prefabs::BasicBar<config::BlckBar, Mutex, Outlet>& bar )
        {
          using Self = prefabs::BasicBar<config::BlckBar, Mutex, Outlet>;
          __PGBAR_ASSERT( io::OStream<Outlet>::itself().empty() == false );
          bar.config_.build( io::OStream<Outlet>::itself(),
                             bar.task_cnt_.load( std::memory_order_acquire ),
                             bar.task_end_.load( std::memory_order_acquire ),
                             bar.zero_point_ );

          auto expected = Self::State::Begin;
          bar.state_.compare_exchange_strong( expected,
                                              Self::State::StrictRefresh,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed );
        }
        template<typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void process( prefabs::BasicBar<config::BlckBar, Mutex, Outlet>& bar )
        {
          __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
          auto& buffer = io::OStream<Outlet>::itself();
          __PGBAR_ASSERT( buffer.empty() == false );
          buffer << console::escodes::clear_suffix;

          bar.config_.build( buffer,
                             bar.task_cnt_.load( std::memory_order_acquire ),
                             bar.task_end_.load( std::memory_order_acquire ),
                             bar.zero_point_ );
        }
        template<typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void finish( prefabs::BasicBar<config::BlckBar, Mutex, Outlet>& bar )
        {
          using Self = prefabs::BasicBar<config::BlckBar, Mutex, Outlet>;
          __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
          auto& buffer = io::OStream<Outlet>::itself();
          __PGBAR_ASSERT( buffer.empty() == false );
          buffer << console::escodes::clear_suffix;

          bar.config_.build( buffer,
                             bar.task_cnt_.load( std::memory_order_acquire ),
                             bar.task_end_.load( std::memory_order_acquire ),
                             bar.final_mesg_,
                             bar.zero_point_ );
          bar.state_.store( Self::State::Stopped, std::memory_order_release );
        }
      };

      template<>
      struct RenderAction<void, void> {
        template<typename Config, typename Mutex, Channel Outlet>
        static __PGBAR_INLINE_FN void automate( prefabs::BasicBar<Config, Mutex, Outlet>& bar )
        {
          using Self = prefabs::BasicBar<Config, Mutex, Outlet>;
          switch ( bar.state_.load( std::memory_order_acquire ) ) {
          case Self::State::Begin: {
            RenderAction<Config>::boot( bar );
          } break;
          case Self::State::StrictRefresh:  __PGBAR_FALLTHROUGH;
          case Self::State::LenientRefresh: {
            RenderAction<Config>::process( bar );
          } break;
          case Self::State::Finish: {
            RenderAction<Config>::finish( bar );
          } break;
          default: return;
          }
        }
      };
    } // namespace render

    namespace traits {
      template<typename B, typename = void>
      struct is_iterable_bar : std::false_type {};
      template<typename B>
      struct is_iterable_bar<
        B,
        typename std::enable_if<
          AllOf<is_bar<B>,
                std::is_void<decltype( std::declval<B&>().config().tasks( std::declval<types::Size>() ),
                                       void() )>>::value>::type> : std::true_type {};
    } // namespace traits

    namespace assets {
      template<types::Size, typename Base>
      struct TupleSlot : public Base {
        using Base::Base;
        TupleSlot( TupleSlot&& )              = default;
        TupleSlot& operator=( TupleSlot&& ) & = default;
        TupleSlot( Base&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        TupleSlot& operator=( Base&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( rhs ) );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
      };

      template<typename Seq, typename... Bars>
      class TupleBar;
      template<types::Size... Tags,
               typename Mutex,
               Channel Outlet,
               typename... Configs,
               template<typename, typename, Channel>
               class... Bars>
      class TupleBar<traits::IndexSeq<Tags...>, Bars<Configs, Mutex, Outlet>...> final
        : public TupleSlot<Tags, prefabs::BasicBar<Configs, Mutex, Outlet>>... {
        static_assert( sizeof...( Tags ) == sizeof...( Bars ),
                       "pgbar::__details::assets::TupleBar: Unexpected type mismatch" );
        static_assert( sizeof...( Bars ) > 0,
                       "pgbar::__details::assets::TupleBar: The number of progress bars cannot be zero" );

        std::atomic<types::Size> alive_cnt_;
        // cb: CombinedBar
        enum class CBState : types::Byte { Stopped, Awake, Refresh };
        std::atomic<CBState> cb_state_;
        mutable concurrent::Mutex cb_mtx_;

        std::bitset<sizeof...( Bars )> output_tag_;

        template<types::Size Pos>
        __PGBAR_INLINE_FN typename std::enable_if<( Pos >= sizeof...( Bars ) )>::type do_render() &
        {}
        template<types::Size Pos = 0>
        inline typename std::enable_if<( Pos < sizeof...( Bars ) )>::type do_render() &
        {
          using std::get;
          __PGBAR_ASSERT( active() );
          if ( get<Pos>( *this ).is_running() ) {
            output_tag_.set( Pos );
            render::RenderAction<void>::automate( get<Pos>( *this ) );
          }
          if ( output_tag_[Pos] )
            io::OStream<Outlet>::itself() << '\n';
          return do_render<Pos + 1>(); // tail recursive
        }

        void do_terminate( bool ) noexcept override final
        { // This virtual function is invoked only via the vtable,
          // hence the default arguments from the base class declaration are always used.
          // Any default arguments provided in the derived class are ignored.
          auto& executor = render::Renderer<Outlet>::itself();
          if ( !executor.empty() && active() && alive_cnt_.fetch_sub( 1, std::memory_order_acq_rel ) == 1 ) {
            std::lock_guard<concurrent::Mutex> lock { cb_mtx_ };
            if ( alive_cnt_.load( std::memory_order_acquire ) == 0 ) {
              // double check
              executor.suspend();
              executor.appoint();
              io::OStream<Outlet>::itself() << io::release;
              cb_state_.store( CBState::Stopped, std::memory_order_release );
            }
          }
        }
        void do_setup() & override final
        {
          std::lock_guard<concurrent::Mutex> lock { cb_mtx_ };
          if ( cb_state_.load( std::memory_order_acquire ) == CBState::Stopped ) {
            auto& executor = render::Renderer<Outlet>::itself();
            if ( !executor.try_appoint( [this]() {
                   auto& ostream = io::OStream<Outlet>::itself();
                   switch ( cb_state_.load( std::memory_order_acquire ) ) {
                   case CBState::Awake: {
                     output_tag_.reset();
                     ostream << console::escodes::store_cursor;
                     do_render();
                     ostream << io::flush;

                     auto expected = CBState::Awake;
                     cb_state_.compare_exchange_strong( expected,
                                                        CBState::Refresh,
                                                        std::memory_order_acq_rel,
                                                        std::memory_order_relaxed );
                   } break;
                   case CBState::Refresh: {
                     ostream << console::escodes::restore_cursor;
                     do_render();
                     ostream << io::flush;
                   } break;
                   default: break;
                   }
                 } ) )
              throw exception::InvalidState( "pgbar: another progress bar instance is already running" );

            executor.activate();
            cb_state_.store( CBState::Awake, std::memory_order_release );
          }
          alive_cnt_.fetch_add( 1, std::memory_order_release );
        }

        template<typename Tuple, types::Size... Is>
        TupleBar( Tuple&& tup, const traits::IndexSeq<Is...>& )
          noexcept( std::tuple_size<typename std::decay<Tuple>::type>::value == sizeof...( Bars ) )
          : ElementAt_t<Is>( utils::forward_or<Is, ElementAt_t<Is>>( std::forward<Tuple>( tup ) ) )...
          , alive_cnt_ { 0 }
          , cb_state_ { CBState::Stopped }
        {}

      public:
        template<types::Size Pos>
        using ElementAt_t =
          traits::TypeAt_t<Pos, TupleSlot<Tags, prefabs::BasicBar<Configs, Mutex, Outlet>>...>;

        // SFINAE is used here to prevent infinite recursive matching of errors.
        template<typename... Cfgs,
                 typename = typename std::enable_if<traits::AllOf<traits::is_config<Cfgs>...>::value>::type>
        TupleBar( Cfgs&&... cfgs ) noexcept( sizeof...( Cfgs ) == sizeof...( Bars ) )
          : TupleBar( std::forward_as_tuple( std::forward<Cfgs>( cfgs )... ),
                      traits::MakeIndexSeq<sizeof...( Configs )>() )
        {}
        TupleBar( TupleBar&& rhs ) noexcept
          : TupleSlot<Tags, prefabs::BasicBar<Configs, Mutex, Outlet>>( std::move( rhs ) )...
          , alive_cnt_ { 0 }
          , cb_state_ { CBState::Stopped }
        {}
        TupleBar& operator=( TupleBar&& ) & = default;
        ~TupleBar()                         = default;

        void halt() noexcept
        {
          std::lock_guard<__details::concurrent::Mutex> lock { cb_mtx_ };
          auto try_update = [this]( CBState expected ) noexcept -> bool {
            return cb_state_.compare_exchange_strong( expected,
                                                      CBState::Stopped,
                                                      std::memory_order_acq_rel,
                                                      std::memory_order_relaxed );
          };
          auto& executor = __details::render::Renderer<Outlet>::itself();
          if ( ( try_update( CBState::Awake ) || try_update( CBState::Refresh ) ) && !executor.empty() )
            executor.appoint();
          alive_cnt_.store( 0, std::memory_order_release );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
        {
          return cb_state_.load( std::memory_order_acquire ) != CBState::Stopped;
        }

        void swap( TupleBar& rhs ) noexcept
        {
          __PGBAR_PURE_ASSUME( this != &rhs );
          __PGBAR_ASSERT( active() == false );
          __PGBAR_ASSERT( rhs.active() == false );
          (void)std::initializer_list<char> { (
            static_cast<TupleSlot<Tags, prefabs::BasicBar<Configs, Mutex, Outlet>>&>( *this ).swap(
              static_cast<TupleSlot<Tags, prefabs::BasicBar<Configs, Mutex, Outlet>>&>( rhs ) ),
            '\0' )... };
        }

        template<types::Size Pos>
        friend ElementAt_t<Pos>& get( TupleBar& tup ) noexcept
        {
          return static_cast<ElementAt_t<Pos>&>( tup );
        }
        template<types::Size Pos>
        friend const ElementAt_t<Pos>& get( const TupleBar& tup ) noexcept
        {
          return static_cast<const ElementAt_t<Pos>&>( tup );
        }
        template<types::Size Pos>
        friend ElementAt_t<Pos>&& get( TupleBar&& tup ) noexcept
        {
          return static_cast<ElementAt_t<Pos>&&>( tup );
        }
      };
    } // namespace assets
  } // namespace __details

  template<typename Bar, typename... Bars>
  class MultiBar;
  template<typename Mutex,
           Channel Outlet,
           typename Config,
           typename... Configs,
           template<typename, typename, Channel>
           class Bar,
           template<typename, typename, Channel>
           class... Bars>
  class MultiBar<Bar<Config, Mutex, Outlet>, Bars<Configs, Mutex, Outlet>...> final {
    static_assert(
      __details::traits::AllOf<
        __details::traits::AllOf<
          std::is_same<Bar<Config, Mutex, Outlet>, __details::prefabs::BasicBar<Config, Mutex, Outlet>>,
          std::is_same<Bars<Configs, Mutex, Outlet>,
                       __details::prefabs::BasicBar<Configs, Mutex, Outlet>>...>,
        __details::traits::AllOf<__details::traits::is_config<Config>,
                                 __details::traits::is_config<Configs>...>>::value,
      "pgbar::MultiBar: Invalid type" );
    using Self    = MultiBar;
    using Package = __details::assets::TupleBar<__details::traits::MakeIndexSeq<sizeof...( Bars ) + 1>,
                                                Bar<Config, Mutex, Outlet>,
                                                Bars<Configs, Mutex, Outlet>...>;

    template<__details::types::Size Pos>
    using ConfigAt_t = __details::traits::TypeAt_t<Pos, Config, Configs...>;
    template<__details::types::Size Pos>
    using BarAt_t = typename Package::template ElementAt_t<Pos>;

    Package tuple_;

  public:
    MultiBar( Config cfg, Configs... cfgs ) noexcept : tuple_ { std::move( cfg ), std::move( cfgs )... } {}

# if __PGBAR_CXX20
    template<typename... Cfgs>
      requires(
        sizeof...( Cfgs ) <= sizeof...( Configs )
        && __details::traits::StartsWith<__details::traits::TypeList<Cfgs...>, Config, Configs...>::value )
# else
    template<typename... Cfgs,
             typename = typename std::enable_if<__details::traits::AllOf<
               std::integral_constant<bool, ( sizeof...( Cfgs ) <= sizeof...( Configs ) + 1 )>,
               __details::traits::StartsWith<__details::traits::TypeList<Cfgs...>, Config, Configs...>>::
                                                  value>::type>
# endif
    MultiBar( Cfgs... args ) noexcept( sizeof...( Cfgs ) == sizeof...( Bars ) + 1 )
      : tuple_ { std::move( args )... }
    {}

# if __PGBAR_CXX20
    template<typename... BarObjs>
      requires( sizeof...( BarObjs ) <= sizeof...( Bars ) + 1
                && ( __details::traits::is_bar<std::decay_t<BarObjs>>::value && ... ) )
# else
    template<typename... BarObjs,
             typename = typename std::enable_if<__details::traits::AllOf<
               std::integral_constant<bool, ( sizeof...( BarObjs ) <= sizeof...( Bars ) + 1 )>,
               __details::traits::AllOf<__details::traits::is_bar<typename std::decay<BarObjs>::type>...>>::
                                                  value>::type>
# endif
    MultiBar( BarObjs&&... args ) noexcept(
      __details::traits::AllOf<
        std::integral_constant<bool, sizeof...( BarObjs ) == sizeof...( Bars ) + 1>,
        __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<BarObjs>...>>>::value )
      : MultiBar( std::forward<BarObjs>( args ).config()... )
    {}

    MultiBar( Self&& rhs ) noexcept : tuple_ { std::move( rhs.tuple_ ) }
    {
      __PGBAR_ASSERT( rhs.is_running() == false );
    }
    Self& operator=( Self&& rhs ) & noexcept
    {
      __PGBAR_PURE_ASSUME( this != &rhs );
      __PGBAR_ASSERT( is_running() == false );
      __PGBAR_ASSERT( rhs.is_running() == false );
      tuple_ = std::move( rhs );
      return *this;
    }
    ~MultiBar() noexcept { reset(); }

    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool is_running() const noexcept { return tuple_.active(); }
    __PGBAR_INLINE_FN void reset() noexcept { tuple_.halt(); }
    void wait() const noexcept
    {
      while ( is_running() )
        std::this_thread::yield();
    }
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& time_duration ) const noexcept
    {
      for ( const auto ending = std::chrono::steady_clock::now() + time_duration;
            std::chrono::steady_clock::now() < ending; ) {
        if ( !is_running() )
          return true;
        std::this_thread::yield();
      }
      return false;
    }

    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick() &
    {
      using std::get;
      get<Pos>( tuple_ ).tick();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick( __details::types::Size next_step ) &
    {
      using std::get;
      get<Pos>( tuple_ ).tick( next_step );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick_to( std::uint8_t percentage ) &
    {
      using std::get;
      get<Pos>( tuple_ ).tick_to( percentage );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void reset( bool final_mesg = true )
    {
      using std::get;
      get<Pos>( tuple_ ).reset( final_mesg );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void wait() const noexcept
    {
      using std::get;
      get<Pos>( tuple_ ).wait();
    }
    template<__details::types::Size Pos, class Rep, class Period>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool wait_for(
      const std::chrono::duration<Rep, Period>& time_duration ) const noexcept
    {
      using std::get;
      return get<Pos>( tuple_ ).wait_for( time_duration );
    }
    template<__details::types::Size Pos>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool is_running() const noexcept
    {
      using std::get;
      return get<Pos>( tuple_ )->is_running();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN ConfigAt_t<Pos>& config() &
    {
      using std::get;
      return get<Pos>( tuple_ ).config();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN const ConfigAt_t<Pos>& config() const&
    {
      using std::get;
      return get<Pos>( tuple_ ).config();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN ConfigAt_t<Pos> config() &&
    {
      using std::get;
      return get<Pos>( tuple_ ).config();
    }

# if __PGBAR_CXX20
    template<__details::types::Size Pos, typename... Args>
      requires requires( BarAt_t<Pos>& bar, Args&&... args ) {
        { bar.iterate( std::forward<Args>( args )... ) };
      }
# else
    template<
      __details::types::Size Pos,
      typename... Args,
      typename = typename std::enable_if<std::is_void<
        decltype( std::declval<BarAt_t<Pos>&>().iterate( std::declval<Args>()... ), void() )>::value>::type>
# endif
    __PGBAR_INLINE_FN auto iterate( Args&&... args ) & noexcept(
      noexcept( std::declval<BarAt_t<Pos>&>().iterate( std::forward<Args>( args )... ) ) )
      -> decltype( std::declval<BarAt_t<Pos>&>().iterate( std::forward<Args>( args )... ) )
    {
      using std::get;
      return get<Pos>( tuple_ ).iterate( std::forward<Args>( args )... );
    }

    void swap( Self& rhs ) noexcept { tuple_.swap( rhs.tuple_ ); }
    friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }
  };

# if __PGBAR_CXX17
  // CTAD, only generates the default version,
  // which means the MutexMode is `Threadunsafe` and the Outlet is `Channel::Stderr`.
#  if __PGBAR_CXX20
  template<typename Config, typename... Configs>
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
#  else
  template<
    typename Config,
    typename... Configs,
    typename = std::enable_if_t<__details::traits::AllOf<__details::traits::is_config<Config>,
                                                         __details::traits::is_config<Configs>...>::value>>
#  endif
  MultiBar( Config,
            Configs... ) -> MultiBar<__details::prefabs::BasicBar<Config, Threadunsafe, Channel::Stderr>,
                                     __details::prefabs::BasicBar<Configs, Threadunsafe, Channel::Stderr>...>;
# endif

  // Creates a MultiBar using existing bar instances.
  template<typename Mutex = Threadunsafe, Channel Outlet = Channel::Stderr, typename Bar, typename... Bars>
# if __PGBAR_CXX20
    requires( __details::traits::is_bar<std::decay_t<Bar>>::value
              && ( __details::traits::is_bar<std::decay_t<Bars>>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    MultiBar<__details::prefabs::BasicBar<typename Bar::ConfigType, Mutex, Outlet>,
             __details::prefabs::BasicBar<typename Bars::ConfigType, Mutex, Outlet>...>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_bar<typename std::decay<Bar>::type>,
                             __details::traits::is_bar<typename std::decay<Bars>::type>...>::value,
    MultiBar<__details::prefabs::BasicBar<typename Bar::ConfigType, Mutex, Outlet>,
             __details::prefabs::BasicBar<typename Bars::ConfigType, Mutex, Outlet>...>>::type
# endif
    make_multi( Bar&& bar, Bars&&... bars ) noexcept(
      __details::traits::Not<
        __details::traits::AnyOf<std::is_lvalue_reference<Bar>, std::is_lvalue_reference<Bars>...>>::value )
  {
    return { std::forward<Bar>( bar ), std::forward<Bars>( bars )... };
  }
  // Creates a MultiBar using configuration objects.
  template<typename Mutex = Threadunsafe,
           Channel Outlet = Channel::Stderr,
           typename Config,
           typename... Configs>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
  MultiBar<__details::prefabs::BasicBar<Config, Mutex, Outlet>,
           __details::prefabs::BasicBar<Configs, Mutex, Outlet>...>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    typename std::enable_if<__details::traits::AllOf<__details::traits::is_config<Config>,
                                                     __details::traits::is_config<Configs>...>::value,
                            MultiBar<__details::prefabs::BasicBar<Config, Mutex, Outlet>,
                                     __details::prefabs::BasicBar<Configs, Mutex, Outlet>...>>::type
# endif
    make_multi( Config cfg, Configs... cfgs ) noexcept
  {
    return { std::move( cfg ), std::move( cfgs )... };
  }

  namespace __details {
    namespace assets {
      template<types::Size Cnt, typename M, Channel O, typename C, types::Size... Is>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN traits::FillTemplate_t<prefabs::BasicBar<C, M, O>, MultiBar, Cnt>
        make_multi_helper( C& cfg, const traits::IndexSeq<Is...>& ) noexcept( Cnt == 1 )
      {
        std::array<C, Cnt - 1> cfgs { { ( (void)( Is ), cfg )... } };
        return { std::move( cfg ), std::move( cfgs[Is] )... };
      }
    }
  } // namespace __details

  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single bar object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<__details::types::Size Cnt,
           typename Mutex = Threadunsafe,
           Channel Outlet = Channel::Stderr,
           typename Bar>
# if __PGBAR_CXX20
    requires( Cnt > 0 && __details::traits::is_bar<std::decay_t<Bar>>::value )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::traits::FillTemplate_t<
    __details::prefabs::BasicBar<typename std::decay_t<Bar>::ConfigType, Mutex, Outlet>,
    MultiBar,
    Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             __details::traits::is_bar<typename std::decay<Bar>::type>>::value,
    __details::traits::FillTemplate_t<
      __details::prefabs::BasicBar<typename std::decay<Bar>::type::ConfigType, Mutex, Outlet>,
      MultiBar,
      Cnt>>::type
# endif
    make_multi( Bar&& bar ) noexcept( Cnt == 1 && !std::is_lvalue_reference<Bar>::value )
  {
    auto cfg = std::forward<Bar>( bar ).config();
    return __details::assets::make_multi_helper<Cnt, Mutex, Outlet>(
      cfg,
      __details::traits::MakeIndexSeq<Cnt - 1>() );
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single configuration object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<__details::types::Size Cnt,
           typename Mutex = Threadunsafe,
           Channel Outlet = Channel::Stderr,
           typename Config>
# if __PGBAR_CXX20
    requires( Cnt > 0 && __details::traits::is_config<Config>::value )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::FillTemplate_t<__details::prefabs::BasicBar<Config, Mutex, Outlet>, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             __details::traits::is_config<Config>>::value,
    __details::traits::FillTemplate_t<__details::prefabs::BasicBar<Config, Mutex, Outlet>, MultiBar, Cnt>>::
    type
# endif
    make_multi( Config cfg ) noexcept( Cnt == 1 )
  {
    return __details::assets::make_multi_helper<Cnt, Mutex, Outlet>(
      cfg,
      __details::traits::MakeIndexSeq<Cnt - 1>() );
  }

  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using mutiple single bar objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided arguments;
   * **any remaining instances with no corresponding arguments will be default-initialized.**
   */
  template<__details::types::Size Cnt, typename Bar, typename... Bars>
# if __PGBAR_CXX20
    requires(
      Cnt > 0 && sizeof...( Bars ) <= Cnt && __details::traits::is_bar<Bar>::value
      && ( __details::traits::is_bar<std::decay_t<Bars>>::value && ... )
      && ( std::is_same_v<typename std::remove_cv_t<Bar>::ConfigType, typename std::decay_t<Bars>::ConfigType>
           && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::traits::FillTemplate_t<Bar, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             std::integral_constant<bool, ( sizeof...( Bars ) <= Cnt )>,
                             __details::traits::is_bar<Bar>,
                             __details::traits::is_bar<typename std::decay<Bars>::type>...,
                             std::is_same<typename std::remove_cv<Bar>::type::ConfigType,
                                          typename std::decay<Bars>::type::ConfigType>...>::value,
    __details::traits::FillTemplate_t<Bar, MultiBar, Cnt>>::type
# endif
    make_multi( Bars&&... bars ) noexcept(
      sizeof...( Bars ) == Cnt
      && __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Bars>...>>::value )
  {
    return { std::forward<Bars>( bars ).config()... };
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using mutiple configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided configurations;
   * **any remaining instances with no corresponding configurations will be default-initialized.**
   */
  template<__details::types::Size Cnt,
           typename Config,
           typename Mutex = Threadunsafe,
           Channel Outlet = Channel::Stderr,
           typename... Configs>
# if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && ( std::is_same_v<Config, Configs> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::FillTemplate_t<__details::prefabs::BasicBar<Config, Mutex, Outlet>, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             std::integral_constant<bool, ( sizeof...( Configs ) <= Cnt )>,
                             std::is_same<Config, Configs>...>::value,
    __details::traits::FillTemplate_t<__details::prefabs::BasicBar<Config, Mutex, Outlet>, MultiBar, Cnt>>::
    type
# endif
    make_multi( Configs... configs ) noexcept( sizeof...( Configs ) == Cnt )
  {
    return { std::move( configs )... };
  }

  namespace scope {
    /**
     * A range that contains a bar object and an unidirectional abstract range,
     * which transforms the iterations in the abstract into a visual display of the object.
     */
    template<typename R, typename B>
    class ProxySpan {
      static_assert( __details::traits::is_scope<R>::value,
                     "pgbar::scope::ProxySpan: Only available for certain range types" );
      static_assert( __details::traits::is_iterable_bar<B>::value,
                     "pgbar::scope::ProxySpan: Must have a method to configure the iteration "
                     "count for the object's configuration type" );

      B* itr_bar_;
      R itr_range_;

    public:
      class iterator final {
        typename R::iterator itr_;
        B* itr_bar_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = typename std::iterator_traits<typename R::iterator>::value_type;
        using difference_type   = void;
        using pointer           = typename std::iterator_traits<typename R::iterator>::pointer;
        using reference         = typename std::iterator_traits<typename R::iterator>::reference;

        __PGBAR_CXX17_CNSTXPR iterator( typename R::iterator itr, B& itr_bar )
          noexcept( std::is_nothrow_move_constructible<typename R::iterator>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { std::addressof( itr_bar ) }
        {}
        __PGBAR_CXX17_CNSTXPR iterator( iterator&& rhs )
          noexcept( std::is_nothrow_move_constructible<typename R::iterator>::value )
          : itr_ { std::move( rhs.itr_ ) }, itr_bar_ { rhs.itr_bar_ }
        {
          rhs.itr_bar_ = nullptr;
        }
        __PGBAR_CXX17_CNSTXPR iterator& operator=( iterator&& rhs ) & noexcept(
          std::is_nothrow_move_assignable<typename R::iterator>::value )
        {
          __PGBAR_PURE_ASSUME( this != &rhs );
          itr_         = std::move( rhs.itr_ );
          itr_bar_     = rhs.itr_bar_;
          rhs.itr_bar_ = nullptr;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR ~iterator() = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() &
        {
          __PGBAR_PURE_ASSUME( itr_bar_ != nullptr );
          ++itr_;
          itr_bar_->tick();
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) &
        {
          __PGBAR_PURE_ASSUME( itr_bar_ != nullptr );
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR reference operator*() noexcept
        {
          return *itr_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR pointer operator->() noexcept
        {
          return std::addressof( itr_ );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==(
          const typename R::iterator& lhs ) const noexcept
        {
          return itr_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=(
          const typename R::iterator& lhs ) const noexcept
        {
          return itr_ != lhs;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return a.itr_ == b.itr_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( a == b );
        }

        operator bool() const noexcept { return itr_bar_ != nullptr; }
      };

      __PGBAR_CXX17_CNSTXPR ProxySpan( R itr_range, B& itr_bar )
        noexcept( std::is_nothrow_move_constructible<R>::value )
        : itr_bar_ { std::addressof( itr_bar ) }, itr_range_ { std::move( itr_range ) }
      {}
      __PGBAR_CXX17_CNSTXPR ProxySpan( ProxySpan&& rhs )
        noexcept( std::is_nothrow_move_constructible<R>::value )
        : ProxySpan( std::move( rhs.itr_range_ ), *rhs.itr_bar_ )
      {
        __PGBAR_ASSERT( rhs.empty() == false );
        rhs.itr_bar_ = nullptr;
      }
      __PGBAR_CXX17_CNSTXPR ProxySpan& operator=( ProxySpan&& rhs ) & noexcept(
        std::is_nothrow_move_assignable<R>::value )
      {
        __PGBAR_PURE_ASSUME( this != &rhs );
        swap( rhs );
        rhs.itr_bar_ = nullptr;
        return *this;
      }
      __PGBAR_CXX20_CNSTXPR virtual ~ProxySpan() = default;

      /**
       * This function CHANGES the state of the pgbar object it holds.
       */
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator begin() &
      {
        itr_bar_->config().tasks( itr_range_.size() );
        return iterator( itr_range_.begin(), *itr_bar_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator end() const
      {
        return iterator( itr_range_.end(), *itr_bar_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR bool empty() const noexcept
      {
        return itr_bar_ == nullptr;
      }

      __PGBAR_CXX14_CNSTXPR void swap( ProxySpan<R, B>& lhs ) noexcept
      {
        __PGBAR_PURE_ASSUME( this != &lhs );
        std::swap( itr_bar_, lhs.itr_bar_ );
        itr_range_.swap( lhs.itr_range_ );
      }
      friend __PGBAR_CXX14_CNSTXPR void swap( ProxySpan<R, B>& a, ProxySpan<R, B>& b ) noexcept
      {
        a.swap( b );
      }
    };
  } // namespace scope
} // namespace pgbar

# undef __PGBAR_LAUNDER

# undef __PGBAR_COMPONENT_REGISTER
# undef __PGBAR_TRAIT_REGISTER

# undef __PGBAR_EMPTY_CLASS
# undef __PGBAR_NONEMPTY_CLASS

# undef __PGBAR_TIMER_SEGMENT
# undef __PGBAR_DEFAULT_TIMER
# undef __PGBAR_DEFAULT_SPEED
# undef __PGBAR_DEFAULT_PERCENT
# undef __PGBAR_PACK
# undef __PGBAR_INHERIT_REGISTER

# undef __PGBAR_CC_STD
# undef __PGBAR_WIN
# undef __PGBAR_UNIX
# undef __PGBAR_UNKNOWN
# undef __PGBAR_CXX23
# undef __PGBAR_ASSUME
# undef __PGBAR_CXX23_CNSTXPR
# undef __PGBAR_INLINE_FN
# undef __PGBAR_NODISCARD
# undef __PGBAR_CXX20
# undef __PGBAR_CNSTEVAL
# undef __PGBAR_CXX20_CNSTXPR
# undef __PGBAR_NOUNIQUEADDR
# undef __PGBAR_CXX17
# undef __PGBAR_CXX17_CNSTXPR
# undef __PGBAR_CXX17_INLINE
# undef __PGBAR_FALLTHROUGH
# undef __PGBAR_UNLIKELY
# undef __PGBAR_CXX14
# undef __PGBAR_CXX14_CNSTXPR
# undef __PGBAR_CXX11

# undef __PGBAR_BLACK
# undef __PGBAR_RED
# undef __PGBAR_GREEN
# undef __PGBAR_YELLOW
# undef __PGBAR_BLUE
# undef __PGBAR_MAGENTA
# undef __PGBAR_CYAN
# undef __PGBAR_WHITE
# undef __PGBAR_DEFAULT

# undef __PGBAR_ASSERT

#endif
