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
# include <cstddef>
# include <cstdint>
# include <exception>
# include <initializer_list>
# include <iterator>
# include <limits>
# include <memory>
# include <mutex>
# include <new>
# include <numeric>
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
#   define NOMINMAX 1
#  endif
#  include <windows.h>
#  define __PGBAR_WIN     1
#  define __PGBAR_UNIX    0
#  define __PGBAR_UNKNOWN 0
# elif defined( __unix__ )
#  include <sys/ioctl.h>
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
#   define __PGBAR_UNREACHABLE __PGBAR_TRUST( false )
#   define __PGBAR_ASSUME( expr )
#  endif
# endif
# if __PGBAR_CC_STD >= 202002L
#  include <ranges>
#  define __PGBAR_CXX20         1
#  define __PGBAR_UNLIKELY      [[unlikely]]
#  define __PGBAR_CXX20_CNSTXPR constexpr
#  define __PGBAR_CNSTEVAL      consteval
# else
#  define __PGBAR_CXX20 0
#  define __PGBAR_UNLIKELY
#  define __PGBAR_CXX20_CNSTXPR
#  define __PGBAR_CNSTEVAL constexpr
# endif
# if __PGBAR_CC_STD >= 201703L
#  include <charconv>
#  include <functional>
#  include <string_view>
#  define __PGBAR_CXX17         1
#  define __PGBAR_CXX17_CNSTXPR constexpr
#  define __PGBAR_CXX17_INLINE  inline
#  define __PGBAR_FALLTHROUGH   [[fallthrough]]
#  define __PGBAR_NODISCARD     [[nodiscard]]
# else
#  define __PGBAR_CXX17 0
#  define __PGBAR_CXX17_CNSTXPR
#  define __PGBAR_CXX17_INLINE
#  define __PGBAR_FALLTHROUGH
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
# define __PGBAR_TRUST( expr ) \
   do {                        \
     __PGBAR_ASSERT( expr );   \
     __PGBAR_ASSUME( expr );   \
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
      ~Error() override = default;
      __PGBAR_NODISCARD const char* what() const noexcept override { return message_; }
    };

    // Exception for invalid function arguments.
    class InvalidArgument : public Error {
    public:
      using Error::Error;
      ~InvalidArgument() override = default;
    };

    // Exception for error state of object.
    class InvalidState : public Error {
    public:
      using Error::Error;
      ~InvalidState() override = default;
    };

    // Exception for local system error.
    class SystemError : public Error {
    public:
      using Error::Error;
      ~SystemError() override = default;
    };
  } // namespace exception

  // A enum that specifies the type of the output stream.
  enum class Channel : int {
# if __PGBAR_UNIX
    Stdout = STDOUT_FILENO,
    Stderr = STDERR_FILENO
# else
    Stdout = 1,
    Stderr = 2
# endif
  };
  enum class Policy : std::uint8_t { Async, Sync };
  enum class Region : std::uint8_t { Fixed, Relative };

  namespace config {
    void hide_completed( bool flag ) noexcept;
    __PGBAR_NODISCARD bool hide_completed() noexcept;
    void disable_styling( bool flag ) noexcept;
    __PGBAR_NODISCARD bool disable_styling() noexcept;
  }

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
# if __PGBAR_CXX20
      using LitU8 = std::u8string_view;
# else
      using LitU8 = LitStr;
# endif
      using HexRGB     = std::uint32_t;
      using UCodePoint = char32_t; // Unicode code point
      using Float      = double;
      using TimeUnit   = std::chrono::nanoseconds;
      using Byte       = std::uint8_t;
    } // namespace types

    namespace constants {
      __PGBAR_CXX17_INLINE constexpr types::Char blank = ' ';
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
      struct _ConcatSeq;
      template<typename HeadSeq, typename TailSeq>
      using _ConcatSeq_t = typename _ConcatSeq<HeadSeq, TailSeq>::type;

      template<types::Size... HeadI, types::Size... TailI>
      struct _ConcatSeq<IndexSeq<HeadI...>, IndexSeq<TailI...>> {
        using type = IndexSeq<HeadI..., ( sizeof...( HeadI ) + TailI )...>;
      };

      // Internal implementation, it should not be used outside of this preprocessing block.
      template<types::Size N>
      struct _MakeIndexSeqHelper {
        using type = _ConcatSeq_t<typename _MakeIndexSeqHelper<N / 2>::type,
                                  typename _MakeIndexSeqHelper<N - N / 2>::type>;
      };
      template<>
      struct _MakeIndexSeqHelper<0> {
        using type = IndexSeq<>;
      };
      template<>
      struct _MakeIndexSeqHelper<1> {
        using type = IndexSeq<0>;
      };

      template<types::Size N>
      using MakeIndexSeq = typename _MakeIndexSeqHelper<N>::type;
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
      struct AllOf<Pred, Preds...> {
      private:
        template<bool Cond, typename... Ps>
        struct _Select;
        template<typename... Ps>
        struct _Select<true, Ps...> : AllOf<Ps...> {};
        template<typename... Ps>
        struct _Select<false, Ps...> : std::false_type {};

      public:
        static constexpr bool value = _Select<bool( Pred::value ), Preds...>::value;
      };

      template<typename... Preds>
      struct AnyOf : std::false_type {};
      template<typename Pred>
      struct AnyOf<Pred> : Pred {};
      template<typename Pred, typename... Preds>
      struct AnyOf<Pred, Preds...> {
      private:
        template<bool Cond, typename... Ps>
        struct _Select;
        template<typename... Ps>
        struct _Select<true, Ps...> : std::true_type {};
        template<typename... Ps>
        struct _Select<false, Ps...> : AnyOf<Ps...> {};

      public:
        static constexpr bool value = _Select<bool( Pred::value ), Preds...>::value;
      };

      template<typename Pred>
      struct Not : std::integral_constant<bool, !bool( Pred::value )> {};
# endif

      template<typename T, template<typename...> class Template, types::Size N>
      struct Repeat {
      private:
        template<typename U, template<typename...> class Tmp, types::Size M, typename... Us>
        struct Helper : Helper<U, Tmp, M - 1, Us..., U> {};
        template<typename U, template<typename...> class Tmp, typename... Us>
        struct Helper<U, Tmp, 0, Us...> {
          using type = Tmp<Us...>;
        };

      public:
        using type = typename Helper<T, Template, N>::type;
      };
      template<typename T, template<typename...> class Template, types::Size N>
      using Repeat_t = typename Repeat<T, Template, N>::type;

      template<types::Size Pos, typename... Ts>
      struct TypeAt {
        static_assert( static_cast<bool>( Pos < sizeof...( Ts ) ),
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
      struct TypeList {};

      // Checks if a TypeList starts with the specified type sequence.
      template<typename TpList, typename... Ts>
      struct StartsWith;
      template<typename... Ts>
      struct StartsWith<TypeList<>, Ts...> : std::true_type {};
      template<typename Head, typename... Tail>
      struct StartsWith<TypeList<Head, Tail...>> : std::false_type {};
      template<typename Head, typename... Tail, typename T, typename... Ts>
      struct StartsWith<TypeList<Head, Tail...>, T, Ts...> {
      private:
        template<bool Cond, typename RestList>
        struct _Select;
        template<typename RestList>
        struct _Select<true, RestList> : StartsWith<RestList, Ts...> {};
        template<typename RestList>
        struct _Select<false, RestList> : std::false_type {};

      public:
        static constexpr bool value = _Select<std::is_same<Head, T>::value, TypeList<Tail...>>::value;
      };

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

      // A kind of `std::is_same` that applies to template class types.
      template<template<typename...> class T, template<typename...> class U>
      struct Equal : std::false_type {};
      template<template<typename...> class T>
      struct Equal<T, T> : std::true_type {};

      // A lightweight tuple type that stores multiple template class types.
      template<template<typename...> class... Ts>
      struct TemplateList {};

      // Check whether a TemplateList contains given template `T`.
      template<typename TmpList, template<typename...> class T>
      struct Include;
      template<template<typename...> class T>
      struct Include<TemplateList<>, T> : std::false_type {};
      template<template<typename...> class Head,
               template<typename...> class... Tail,
               template<typename...> class T>
      struct Include<TemplateList<Head, Tail...>, T> {
      private:
        template<bool Cond, typename RestList>
        struct _Select;
        template<typename RestList>
        struct _Select<true, RestList> : std::true_type {};
        template<typename RestList>
        struct _Select<false, RestList> : Include<TemplateList<Tail...>, T> {};

      public:
        static constexpr bool value = _Select<Equal<Head, T>::value, TemplateList<Tail...>>::value;
      };

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

      template<template<typename...> class... Ts>
      struct TemplateSet : TemplateList<Ts>... {};

      template<typename TmpSet, template<typename...> class T>
      struct Contain;
      template<template<typename...> class... Ts, template<typename...> class T>
      struct Contain<TemplateSet<Ts...>, T> : std::is_base_of<TemplateList<T>, TemplateSet<Ts...>> {};

      template<typename TmpSet, template<typename...> class T>
      struct Extend;
      template<typename TmpSet, template<typename...> class T>
      using Extend_t = typename Extend<TmpSet, T>::type;

      template<template<typename...> class... Ts, template<typename...> class T>
      struct Extend<TemplateSet<Ts...>, T> {
      private:
        template<bool Cond, template<typename...> class NewOne>
        struct _Select;
        template<template<typename...> class NewOne>
        struct _Select<true, NewOne> {
          using type = TemplateSet<Ts...>;
        };
        template<template<typename...> class NewOne>
        struct _Select<false, NewOne> {
          using type = TemplateSet<Ts..., NewOne>;
        };

      public:
        using type = typename _Select<Contain<TemplateSet<Ts...>, T>::value, T>::type;
      };

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

      // Pack multiple macro parameters into a single one.
# define __PGBAR_PACK( ... ) __VA_ARGS__
// A helper macro to register the inheritance structure of a template class.
# define __PGBAR_INHERIT_REGISTER( Node, VBList, NVBList ) \
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
            using type = Prepend_t<SortNVB_t, Target>;
          };

        public:
          using pathset = Extend_t<MarkNVB_t, Head>;
          using type = typename _Select<AnyOf<Contain<SetVisitedVB, Head>, Contain<MarkTail_t, Head>>::value,
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
          using type    = Prepend_t<SortNVB_t, Head>;
        };

      public:
        using type = Helper_tp<false,
                               TemplateSet<NVB, NVBs...>,
                               Helper_tp<true, TemplateSet<VBs...>, TemplateList<>, TemplateSet<>>,
                               Helper_ps<true, TemplateSet<VBs...>, TemplateList<>, TemplateSet<>>>;
      };
      // Get a list of topological sorting results for the input template classes.
      template<typename NVBSet, typename VBSet = TemplateSet<>>
      using TopoSort_t = typename TopoSort<NVBSet, VBSet>::type;

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

      /**
       * Check whether the type `T` has a type `iterator` or `const_iterator`, and return it if affirmative.
       * Otherwise, return the type `T` itself.
       */
      template<typename T>
      struct IteratorOf {
# if __PGBAR_CXX20
      private:
        // Provide a default fallback to avoid the problem of the type not existing
        // in the immediate context derivation.
        template<typename U>
        static constexpr U check( ... );
        template<typename U>
        static constexpr std::ranges::iterator_t<U> check( int );

      public:
        using type = decltype( check<T>( 0 ) );
# else
      private:
        template<typename U, typename = void>
        struct has_iterator : std::false_type {};
        template<typename U>
        struct has_iterator<
          U,
          typename std::enable_if<std::is_same<typename U::iterator, typename U::iterator>::value>::type>
          : std::true_type {};

        template<typename U, typename = void>
        struct has_const_iterator : std::false_type {};
        template<typename U>
        struct has_const_iterator<
          U,
          typename std::enable_if<
            std::is_same<typename U::const_iterator, typename U::const_iterator>::value>::type>
          : std::true_type {};

        template<typename U,
                 bool is_array      = std::is_array<T>::value,
                 bool is_const      = std::is_const<T>::value,
                 bool has_itr       = has_iterator<T>::value,
                 bool has_const_itr = has_const_iterator<T>::value>
        struct _Select {
          using type = U;
        };
        template<typename U, bool P1, bool P2, bool P3>
        struct _Select<U, true, P1, P2, P3> {
          using type = typename std::add_pointer<typename std::remove_extent<U>::type>::type;
        };
        template<typename U, bool P2>
        struct _Select<U, false, true, P2, true> {
          using type = typename U::const_iterator;
        };
        template<typename U, bool P3>
        struct _Select<U, false, false, true, P3> {
          using type = typename U::iterator;
        };
        template<typename U>
        struct _Select<U, false, true, true, false> {
          using type = typename U::iterator;
        };

      public:
        using type = typename _Select<T>::type;
# endif
      };
      // Get the result type of `IteratorOf`.
      template<typename T>
      using IteratorOf_t = typename IteratorOf<T>::type;

# if __PGBAR_CXX20
      template<typename T>
      struct is_sized_iterator
        : std::bool_constant<std::movable<T> && std::weakly_incrementable<T> && std::indirectly_readable<T>
                             && std::sized_sentinel_for<T, T>> {};
# else
      template<typename T>
      struct is_sized_iterator {
      private:
        template<typename>
        static constexpr std::false_type check( ... );
        template<typename U>
        static constexpr typename std::enable_if<
          AllOf<std::is_signed<typename std::iterator_traits<U>::difference_type>,
                std::is_same<decltype( ++std::declval<U>() ), U&>,
                std::is_void<decltype( std::declval<U>()++, void() )>,
                std::is_same<decltype( *std::declval<U>() ), typename std::iterator_traits<U>::reference>,
                std::is_void<decltype( std::distance( std::declval<U>(), std::declval<U>() ), void() )>,
                std::is_convertible<decltype( std::declval<U>() != std::declval<U>() ), bool>>::value,
          std::true_type>::type
          check( int );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<T>>, std::is_move_constructible<T>, decltype( check<T>( 0 ) )>::value;
      };
      template<typename P>
      struct is_sized_iterator<P*> : std::true_type {};
# endif

# if __PGBAR_CXX20
      template<typename T>
      struct is_bounded_range : std::bool_constant<std::ranges::sized_range<T>> {};
# else
      template<typename T>
      struct is_bounded_range {
      private:
        template<typename>
        static constexpr std::false_type check( ... );
        template<typename U>
        static constexpr typename std::enable_if<
          AllOf<std::is_same<decltype( std::declval<U>().begin() ), decltype( std::declval<U>().end() )>,
                std::is_convertible<decltype( std::declval<U>().begin() != std::declval<U>().end() ), bool>,
                Not<std::is_void<decltype( std::declval<U>().size() )>>>::value,
          std::true_type>::type
          check( int );

      public:
        static constexpr bool value = AllOf<Not<std::is_reference<T>>, decltype( check<T>( 0 ) )>::value;
      };
      template<typename T, types::Size N>
      struct is_bounded_range<T[N]> : std::true_type {};
# endif

      // Check whether the type Instance is an instantiated version of Tmp or whether it inherits from Tmp
      // itself.
      template<typename Instance, template<typename...> class Tmp>
      struct InstanceOf {
      private:
        template<typename... Args>
        static constexpr std::true_type check( const Tmp<Args...>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<Instance>>,
                decltype( check( std::declval<typename std::remove_cv<Instance>::type>() ) )>::value;
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

      // Perfectly forward the I-th element of a tuple, constructing one by default if it's out of bound.
      template<types::Size I,
               typename T,
               typename Tuple,
               typename = typename std::enable_if<
                 ( I < std::tuple_size<typename std::decay<Tuple>::type>::value )>::type>
      constexpr auto forward_or( Tuple&& tup ) noexcept
        -> decltype( std::get<I>( std::forward<Tuple>( tup ) ) )
      {
        static_assert( std::is_convertible<typename std::tuple_element<I, Tuple>::type, T>::value,
                       "pgbar::__details::traits::forward_or: Incompatible type" );
        return std::get<I>( std::forward<Tuple>( tup ) );
      }
      template<types::Size I, typename T, typename Tuple>
      constexpr
        typename std::enable_if<( I >= std::tuple_size<typename std::decay<Tuple>::type>::value ), T>::type
        forward_or( Tuple&& ) noexcept( std::is_nothrow_default_constructible<T>::value )
      {
        return T();
      }

      // Available only for buffers that use placement new.
      template<typename To, typename From>
      __PGBAR_INLINE_FN constexpr To* launder_as( From* src ) noexcept
      {
# if __PGBAR_CXX17
        return std::launder( reinterpret_cast<To*>( src ) );
# else
        /**
         * Before c++17 there was no way to prevent compilers from over-optimizing different types
         * of pointers with rules like strict aliases,
         * so we should trust reinerpret_cast to give us what we want.
         */
        return reinterpret_cast<To*>( src );
# endif
      }

# if __PGBAR_CXX14
      template<typename T, typename... Args>
      __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR auto make_unique( Args&&... args )
      {
        return std::make_unique<T>( std::forward<Args>( args )... );
      }
# else
      // picking up the standard's mess...
      template<typename T, typename... Args>
      __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR std::unique_ptr<T> make_unique( Args&&... args )
      {
        return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
      }
# endif

# if __PGBAR_CXX17
      template<typename Fn, typename... Args>
      __PGBAR_INLINE_FN constexpr decltype( auto ) invoke( Fn&& fn, Args&&... args )
        noexcept( std::is_nothrow_invocable_v<Fn, Args...> )
      {
        return std::invoke( std::forward<Fn>( fn ), std::forward<Args>( args )... );
      }
# else
      template<typename C, typename MemFn, typename Object, typename... Args>
      __PGBAR_INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( std::forward<Object>( object ).*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                      std::is_same<C, typename std::decay<Object>::type>>>::value,
          decltype( ( std::forward<Object>( object ).*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( std::forward<Object>( object ).*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemFn, typename Object, typename... Args>
      __PGBAR_INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( object.get().*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::InstanceOf<typename std::decay<Object>::type, std::reference_wrapper>>::value,
          decltype( ( object.get().*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( object.get().*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemFn, typename Object, typename... Args>
      __PGBAR_INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( ( *std::forward<Object>( object ) )
                              .*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::Not<traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                                  std::is_same<C, typename std::decay<Object>::type>,
                                                  traits::InstanceOf<typename std::decay<Object>::type,
                                                                     std::reference_wrapper>>>>::value,
          decltype( ( ( *std::forward<Object>( object ) ).*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( ( *std::forward<Object>( object ) ).*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemObj, typename Object>
      __PGBAR_INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                      std::is_same<C, typename std::decay<Object>::type>>>::value,
          decltype( std::forward<Object>( object ).*member )>::type
      {
        return std::forward<Object>( object ).*member;
      }
      template<typename C, typename MemObj, typename Object>
      __PGBAR_INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::InstanceOf<typename std::decay<Object>::type, std::reference_wrapper>>::value,
          decltype( object.get().*member )>::type
      {
        return object.get().*member;
      }
      template<typename C, typename MemObj, typename Object>
      __PGBAR_INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object )
        noexcept( noexcept( ( *std::forward<Object>( object ) ).*member ) ) -> typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::Not<traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                                  std::is_same<C, typename std::decay<Object>::type>,
                                                  traits::InstanceOf<typename std::decay<Object>::type,
                                                                     std::reference_wrapper>>>>::value,
          decltype( ( *std::forward<Object>( object ) ).*member )>::type
      {
        return ( *std::forward<Object>( object ) ).*member;
      }
      template<typename Fn, typename... Args>
      __PGBAR_INLINE_FN constexpr auto invoke( Fn&& fn, Args&&... args )
        noexcept( noexcept( std::forward<Fn>( fn )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::Not<
            traits::AnyOf<std::is_member_function_pointer<typename std::remove_reference<Fn>::type>,
                          std::is_member_object_pointer<typename std::remove_reference<Fn>::type>>>::value,
          decltype( std::forward<Fn>( fn )( std::forward<Args>( args )... ) )>::type
      {
        return std::forward<Fn>( fn )( std::forward<Args>( args )... );
      }
# endif

      template<typename Numeric>
      __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR __PGBAR_INLINE_FN
        typename std::enable_if<std::is_unsigned<Numeric>::value, types::Size>::type
        count_digits( Numeric val ) noexcept
      {
        types::Size digits = val == 0;
        for ( ; val > 0; val /= 10 )
          ++digits;
        return digits;
      }
      template<typename Numeric>
      __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR __PGBAR_INLINE_FN
        typename std::enable_if<std::is_signed<Numeric>::value, types::Size>::type
        count_digits( Numeric val ) noexcept
      {
        return count_digits( static_cast<std::uint64_t>( val < 0 ? -val : val ) );
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
        __PGBAR_TRUST( precision >= 0 );
# if __PGBAR_CXX17
        const auto abs_rounded_val = std::round( std::abs( val ) );
        const auto int_digits      = count_digits( abs_rounded_val );

        types::String formatted;
#  if __PGBAR_CXX23
        formatted.resize_and_overwrite(
          int_digits + precision + 2,
          [val, precision]( types::Char* buf, types::Size n ) noexcept {
            const auto result = std::to_chars( buf, buf + n, val, std::chars_format::fixed, precision );
            __PGBAR_TRUST( result.ec == std::errc {} );
            __PGBAR_TRUST( result.ptr >= buf );
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
        __PGBAR_TRUST( result.ec == std::errc {} );
        __PGBAR_ASSERT( result.ptr >= formatted.data() );
        formatted.resize( result.ptr - formatted.data() );
#  endif
# else
        const auto scale           = std::pow( 10, precision );
        const auto abs_rounded_val = std::round( std::abs( val ) * scale ) / scale;
        __PGBAR_ASSERT( abs_rounded_val <= ( std::numeric_limits<std::uint64_t>::max )() );
        const auto integer = static_cast<std::uint64_t>( abs_rounded_val );
        const auto fraction =
          static_cast<std::uint64_t>( std::round( ( abs_rounded_val - integer ) * scale ) );
        const auto sign = std::signbit( val );

        auto formatted = types::String( sign, '-' );
        formatted.append( format( integer ) ).reserve( count_digits( integer ) + sign );
        if ( precision > 0 ) {
          formatted.push_back( '.' );
          const auto fract_digits = count_digits( fraction );
          __PGBAR_TRUST( fract_digits <= static_cast<types::Size>( precision ) );
          formatted.append( precision - fract_digits, '0' ).append( format( fraction ) );
        }
# endif
        return formatted;
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

    namespace traits {
# if __PGBAR_CXX17
      template<typename R, typename Fn, typename... ArgTypes>
      using Invocable_r = std::is_invocable_r<R, Fn, ArgTypes...>;
# else
      template<typename R, typename Fn, typename... ArgTypes>
      struct Invocable_r {
      private:
        template<typename F>
        static constexpr auto check( F&& fn ) -> typename std::enable_if<
          std::is_convertible<decltype( utils::invoke( std::forward<F>( fn ), std::declval<ArgTypes>()... ) ),
                              R>::value,
          std::true_type>::type;
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check( std::declval<Fn>() ) )::value;
      };
# endif
    }

    namespace wrappers {
      template<typename T>
      struct OptionWrapper {
      protected:
        T data_;

        constexpr OptionWrapper() noexcept( std::is_nothrow_default_constructible<T>::value ) : data_ {} {};
        constexpr OptionWrapper( T&& data ) noexcept( std::is_nothrow_move_constructible<T>::value )
          : data_ { std::move( data ) }
        {}

      public:
        constexpr OptionWrapper( const OptionWrapper& )                          = default;
        constexpr OptionWrapper( OptionWrapper&& )                               = default;
        __PGBAR_CXX14_CNSTXPR OptionWrapper& operator=( const OptionWrapper& ) & = default;
        __PGBAR_CXX14_CNSTXPR OptionWrapper& operator=( OptionWrapper&& ) &      = default;

        // Intentional non-virtual destructors.
        __PGBAR_CXX20_CNSTXPR ~OptionWrapper() = default;
        __PGBAR_CXX14_CNSTXPR T& value() & noexcept { return data_; }
        __PGBAR_CXX14_CNSTXPR const T& value() const& noexcept { return data_; }
        __PGBAR_CXX14_CNSTXPR T&& value() && noexcept { return std::move( data_ ); }

        __PGBAR_CXX20_CNSTXPR void swap( T& lhs ) noexcept
        {
          __PGBAR_TRUST( this != &lhs );
          using std::swap;
          swap( data_, lhs.data_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( T& a, T& b ) noexcept { a.swap( b ); }
      };

# if __PGBAR_CXX23
      template<typename... Signature>
      using UniqueFunction = std::move_only_function<Signature...>;
# else
      template<typename Derived>
      class FnStorageBlock {
      protected:
#  if __PGBAR_CXX17
        using Data = std::byte;
#  else
        using Data = unsigned char;
#  endif
        union alignas( std::max_align_t ) AnyFn {
          Data buf_[sizeof( void* ) * 2];
          Data* dptr_;
        };
        struct VTable final {
          const typename Derived::Invoker invoke;
          // The handling of function types by msvc is very strange:
          // it often triggers internal compiler errors for no apparent reason,
          // so here we have to manually write the function pointer type.
          void ( *const destroy )( AnyFn& ) noexcept;
          void ( *const move )( AnyFn& dst, AnyFn& src ) noexcept;
        };

        template<typename T>
        using Inlinable = traits::AllOf<std::is_nothrow_move_constructible<T>,
                                        std::integral_constant<bool,
                                                               ( sizeof( AnyFn::buf_ ) >= sizeof( T )
                                                                 && alignof( AnyFn ) >= alignof( T ) )>>;

        const VTable* vtable_;
        AnyFn callee_;

        static __PGBAR_CXX14_CNSTXPR void destroy_null( AnyFn& ) noexcept {}
        static __PGBAR_CXX14_CNSTXPR void move_null( AnyFn&, AnyFn& ) noexcept {}

        template<typename T>
        static __PGBAR_CXX14_CNSTXPR void destroy_inline( AnyFn& fn ) noexcept
        {
          const auto ptr = utils::launder_as<T>( &fn.buf_ );
          ptr->~T();
        }
        template<typename T>
        static void move_inline( AnyFn& dst, AnyFn& src ) noexcept
        {
          new ( &dst ) T( std::move( *utils::launder_as<T>( &src.buf_ ) ) );
          destroy_inline<T>( src );
        }

        template<typename T>
        static __PGBAR_CXX20_CNSTXPR void destroy_dynamic( AnyFn& fn ) noexcept
        {
          const auto dptr = utils::launder_as<T>( fn.dptr_ );
          dptr->~T();
#  if __PGBAR_CXX17
          operator delete( fn.dptr_, std::align_val_t( alignof( T ) ) );
#  else
          operator delete( fn.dptr_ );
#  endif
        }
        template<typename T>
        static __PGBAR_CXX14_CNSTXPR void move_dynamic( AnyFn& dst, AnyFn& src ) noexcept
        {
          dst.dptr_ = src.dptr_;
          src.dptr_ = nullptr;
        }

        template<typename F>
        static __PGBAR_INLINE_FN typename std::enable_if<Inlinable<typename std::decay<F>::type>::value>::type
          store_fn( const VTable*( &vtable ), AnyFn& any, F&& fn ) noexcept
        {
          using T             = typename std::decay<F>::type;
          const auto location = new ( &any.buf_ ) T( std::forward<F>( fn ) );
          __PGBAR_TRUST( static_cast<void*>( location ) == static_cast<void*>( &any.buf_ ) );
          (void)location;
          vtable = &table_inline<T>();
        }
        template<typename F>
        static __PGBAR_INLINE_FN
          typename std::enable_if<!Inlinable<typename std::decay<F>::type>::value>::type
          store_fn( const VTable*( &vtable ), AnyFn& any, F&& fn ) noexcept( false )
        {
          using T   = typename std::decay<F>::type;
          auto dptr = std::unique_ptr<void, void ( * )( void* )>(
#  if __PGBAR_CXX17
            operator new( sizeof( T ), std::align_val_t( alignof( T ) ) ),
#  else
            operator new( sizeof( T ) ),
#  endif
            +[]( void* ptr ) { operator delete( ptr ); } );

          const auto location = new ( dptr.get() ) T( std::forward<F>( fn ) );
          __PGBAR_ASSERT( static_cast<void*>( location ) == dptr.get() );
          (void)location;

          any.dptr_ = static_cast<Data*>( dptr.release() );
          vtable    = &table_dynamic<T>();
        }

        template<typename T>
        static __PGBAR_CXX23_CNSTXPR const VTable& table_inline() noexcept
        {
          static const VTable tbl { Derived::template invoke_inline<T>, destroy_inline<T>, move_inline<T> };
          return tbl;
        }
        template<typename T>
        static __PGBAR_CXX23_CNSTXPR const VTable& table_dynamic() noexcept
        {
          static const VTable tbl { Derived::template invoke_dynamic<T>,
                                    destroy_dynamic<T>,
                                    move_dynamic<T> };
          return tbl;
        }
        static __PGBAR_CXX23_CNSTXPR const VTable& table_null() noexcept
        {
          static const VTable tbl { Derived::invoke_null, destroy_null, move_null };
          return tbl;
        }

        __PGBAR_CXX23_CNSTXPR FnStorageBlock() noexcept : vtable_ { &table_null() } {}

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR void reset() noexcept
        {
          __PGBAR_TRUST( vtable_ != nullptr );
          vtable_->destroy( callee_ );
          vtable_ = &table_null();
        }
        template<typename F>
        __PGBAR_INLINE_FN void reset( F&& fn ) noexcept( Inlinable<typename std::decay<F>::type>::value )
        {
          const VTable* vtable = nullptr;
          AnyFn tmp;
          store_fn( vtable, tmp, std::forward<F>( fn ) );
          reset();
          std::swap( vtable_, vtable );
          vtable_->move( callee_, tmp );
        }

      public:
        __PGBAR_CXX23_CNSTXPR FnStorageBlock( FnStorageBlock&& rhs ) noexcept : vtable_ { rhs.vtable_ }
        {
          __PGBAR_TRUST( rhs.vtable_ != nullptr );
          vtable_->move( callee_, rhs.callee_ );
          rhs.vtable_ = &table_null();
        }
        __PGBAR_CXX23_CNSTXPR FnStorageBlock& operator=( FnStorageBlock&& rhs ) & noexcept
        {
          __PGBAR_TRUST( this != &rhs );
          __PGBAR_TRUST( vtable_ != nullptr );
          __PGBAR_TRUST( rhs.vtable_ != nullptr );
          reset();
          std::swap( vtable_, rhs.vtable_ );
          vtable_->move( callee_, rhs.callee_ );
          return *this;
        }
        __PGBAR_CXX23_CNSTXPR ~FnStorageBlock() noexcept { reset(); }

        __PGBAR_CXX23_CNSTXPR void swap( FnStorageBlock& lhs ) noexcept
        {
          __PGBAR_TRUST( vtable_ != nullptr );
          __PGBAR_TRUST( lhs.vtable_ != nullptr );
          AnyFn tmp;
          vtable_->move( tmp, callee_ );
          lhs.vtable_->move( callee_, lhs.callee_ );
          vtable_->move( lhs.callee_, tmp );
          std::swap( vtable_, lhs.vtable_ );
        }
        __PGBAR_CXX23_CNSTXPR friend void swap( FnStorageBlock& a, FnStorageBlock& b ) noexcept
        {
          return a.swap( b );
        }
        friend constexpr bool operator==( const FnStorageBlock& a, std::nullptr_t ) noexcept
        {
          return !static_cast<bool>( a );
        }
        friend constexpr bool operator!=( const FnStorageBlock& a, std::nullptr_t ) noexcept
        {
          return static_cast<bool>( a );
        }
        constexpr explicit operator bool() const noexcept { return vtable_ != &table_null(); }
      };
      // `CrefInfo` can be any types that contains the `cref` info of the functor.
      // e.g. For the function type `void () const&`, the `CrefInfo` can be: `const int&`.
      template<typename Derived, typename CrefInfo, typename R, bool Noexcept, typename... Args>
      class FnInvokeBlock : public FnStorageBlock<Derived> {
        friend class FnStorageBlock<Derived>;

        using typename FnStorageBlock<Derived>::AnyFn;
        template<typename T>
        using Param_t = typename std::conditional<std::is_scalar<T>::value, T, T&&>::type;
        template<typename T>
        using Fn_t = typename std::
          conditional<std::is_const<typename std::remove_reference<CrefInfo>::type>::value, const T, T>::type;
        template<typename As, typename T>
        static __PGBAR_INLINE_FN constexpr typename std::enable_if<
          !std::is_reference<As>::value,
          typename std::conditional<std::is_const<typename std::remove_reference<As>::type>::value,
                                    const T&&,
                                    T&&>::type>::type
          forward_as( T&& param ) noexcept
        {
          return std::forward<T>( param );
        }
        template<typename As, typename T>
        static __PGBAR_INLINE_FN constexpr typename std::enable_if<
          std::is_lvalue_reference<As>::value,
          typename std::conditional<traits::AnyOf<std::is_const<typename std::remove_reference<As>::type>,
                                                  std::is_rvalue_reference<T>>::value,
                                    const typename std::remove_reference<T>::type&,
                                    typename std::remove_reference<T>::type&>::type>::type
          forward_as( T&& param ) noexcept
        {
          return param;
        }
        template<typename As, typename T>
        static __PGBAR_INLINE_FN constexpr typename std::enable_if<
          std::is_rvalue_reference<As>::value,
          typename std::conditional<std::is_const<typename std::remove_reference<As>::type>::value,
                                    const typename std::remove_reference<T>::type&&,
                                    typename std::remove_reference<T>::type>::type&&>::type
          forward_as( T&& param ) noexcept
        {
          return std::move( param );
        }

      protected:
        using Invoker = R ( * )( const AnyFn&, Param_t<Args>... )
#  if __PGBAR_CXX17
          noexcept( Noexcept )
#  endif
          ;

        static __PGBAR_CXX14_CNSTXPR R invoke_null( const AnyFn&, Param_t<Args>... ) noexcept( Noexcept )
        {
          __PGBAR_UNREACHABLE;
          // The standard says this should trigger an undefined behavior.
        }
        template<typename T>
        static __PGBAR_CXX14_CNSTXPR R invoke_inline( const AnyFn& fn, Param_t<Args>... args )
          noexcept( Noexcept )
        {
          const auto ptr = utils::launder_as<Fn_t<T>>( ( &const_cast<AnyFn&>( fn ).buf_ ) );
          return utils::invoke( forward_as<CrefInfo>( *ptr ), std::forward<Args>( args )... );
        }
        template<typename T>
        static __PGBAR_CXX14_CNSTXPR R invoke_dynamic( const AnyFn& fn, Param_t<Args>... args )
          noexcept( Noexcept )
        {
          const auto dptr = utils::launder_as<Fn_t<T>>( fn.dptr_ );
          return utils::invoke( forward_as<CrefInfo>( *dptr ), std::forward<Args>( args )... );
        }

        constexpr FnInvokeBlock()                                           = default;
        constexpr FnInvokeBlock( FnInvokeBlock&& )                          = default;
        __PGBAR_CXX14_CNSTXPR FnInvokeBlock& operator=( FnInvokeBlock&& ) & = default;
      };

      // A simplified implementation of std::move_only_function
      template<typename...>
      class UniqueFunction;
      template<typename R, typename... Args>
      class UniqueFunction<R( Args... )>
        : public FnInvokeBlock<UniqueFunction<R( Args... )>, int, R, false, Args...> {
        using Base = FnStorageBlock<UniqueFunction>;

      public:
        UniqueFunction( const UniqueFunction& )              = delete;
        UniqueFunction& operator=( const UniqueFunction& ) & = delete;

        constexpr UniqueFunction()                                            = default;
        constexpr UniqueFunction( UniqueFunction&& )                          = default;
        __PGBAR_CXX14_CNSTXPR UniqueFunction& operator=( UniqueFunction&& ) & = default;

        constexpr UniqueFunction( std::nullptr_t ) noexcept : UniqueFunction() {}
        template<typename F,
                 typename = typename std::enable_if<
                   traits::AllOf<std::is_constructible<typename std::decay<F>::type, F>,
                                 traits::Invocable_r<R, F, Args...>>::value>::type>
        UniqueFunction( F&& fn ) noexcept( Base::template Inlinable<typename std::decay<F>::type>::value )
        {
          Base::store_fn( this->vtable_, this->callee_, std::forward<F>( fn ) );
        }
        // In C++11, `std::in_place_type` does not exist, and we will not use it either.
        // Therefore, we do not provide an overloaded constructor for this type here.
        template<typename F>
        typename std::enable_if<traits::AllOf<std::is_constructible<typename std::decay<F>::type, F>,
                                              traits::Invocable_r<R, F, Args...>>::value,
                                UniqueFunction&>::type
          operator=( F&& fn ) & noexcept( Base::template Inlinable<typename std::decay<F>::type>::value )
        {
          this->reset( std::forward<F>( fn ) );
          return *this;
        }
        __PGBAR_CXX23_CNSTXPR UniqueFunction& operator=( std::nullptr_t ) noexcept
        {
          this->reset();
          return *this;
        }

        R operator()( Args... args )
        {
          __PGBAR_TRUST( this->vtable_ != nullptr );
          return ( *this->vtable_->invoke )( this->callee_, std::forward<Args>( args )... );
        }
      };
# endif
    } // namespace wrappers

    namespace charcodes {
      // A type of wrapper that stores the mapping between Unicode code chart and character width.
      class CodeChart final {
      public:
        // At present, the width of each utf character will not exceed 3.
        using RenderWidth = std::uint8_t;

      private:
        types::UCodePoint start_, end_;
        RenderWidth width_;

      public:
        constexpr CodeChart( types::UCodePoint start, types::UCodePoint end, RenderWidth width ) noexcept
          : start_ { start }, end_ { end }, width_ { width }
        {          // This is an internal component, so we assume the arguments are always valid.
# if __PGBAR_CXX14 // C++11 requires the constexpr ctor should have an empty function body.
          __PGBAR_TRUST( start_ <= end_ );
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
        __PGBAR_NODISCARD constexpr RenderWidth width() const noexcept { return width_; }
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

      // A simple UTF-8 string implementation, but it does not provide specific utf-8 codec operations.
      class U8Raw {
        using Self = U8Raw;

      protected:
        types::Size width_;
        types::String bytes_;

        /// @return The utf codepoint and the number of byte of the utf-8 character.
        static __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR std::pair<types::UCodePoint, types::Size> next_char(
          const types::Char* raw_u8_str,
          types::Size length )
        {
          // After RFC 3629, the maximum length of each standard UTF-8 character is 4 bytes.
          const auto first_byte = static_cast<types::UCodePoint>( static_cast<types::Byte>( *raw_u8_str ) );
          auto validator        = [raw_u8_str, length]( types::Size expected_len ) {
            if ( expected_len > length )
              __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: incomplete UTF-8 string" );

            for ( types::Size i = 1; i < expected_len; ++i ) {
              if ( ( raw_u8_str[i] & 0xC0 ) != 0x80 )
                __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: broken UTF-8 character" );
            }
            return expected_len;
          };

          if ( ( first_byte & 0x80 ) == 0 )
            return std::make_pair( first_byte, 1 );
          else if ( ( ( first_byte & 0xE0 ) == 0xC0 ) )
            return std::make_pair( ( ( first_byte & 0x1F ) << 6 )
                                     | ( static_cast<types::UCodePoint>( raw_u8_str[1] ) & 0x3F ),
                                   validator( 2 ) );
          else if ( ( first_byte & 0xF0 ) == 0xE0 )
            return std::make_pair( ( ( first_byte & 0xF ) << 12 )
                                     | ( ( static_cast<types::UCodePoint>( raw_u8_str[1] ) & 0x3F ) << 6 )
                                     | ( static_cast<types::UCodePoint>( raw_u8_str[2] ) & 0x3F ),
                                   validator( 3 ) );
          else if ( ( first_byte & 0xF8 ) == 0xF0 )
            return std::make_pair( ( ( first_byte & 0x7 ) << 18 )
                                     | ( ( static_cast<types::UCodePoint>( raw_u8_str[1] ) & 0x3F ) << 12 )
                                     | ( ( static_cast<types::UCodePoint>( raw_u8_str[2] ) & 0x3F ) << 6 )
                                     | ( static_cast<types::UCodePoint>( raw_u8_str[3] ) & 0x3F ),
                                   validator( 4 ) );
          else
            __PGBAR_UNLIKELY throw exception::InvalidArgument( "pgbar: not a standard UTF-8 string" );
        }

      public:
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN __PGBAR_CNSTEVAL std::array<CodeChart, 47>
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
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR CodeChart::RenderWidth char_width(
          types::UCodePoint codepoint ) noexcept
        {
          constexpr const auto charts = code_charts();
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
          types::Size width     = 0;
          const auto raw_u8_str = u8_str.data();
          for ( types::Size i = 0; i < u8_str.size(); ) {
            const auto startpoint = raw_u8_str + i;
            __PGBAR_TRUST( startpoint >= raw_u8_str );
            auto parsed = next_char( startpoint, std::distance( startpoint, u8_str.data() + u8_str.size() ) );
            width += static_cast<types::Size>( char_width( parsed.first ) );
            i += parsed.second;
          }
          return width;
        }

        __PGBAR_CXX20_CNSTXPR U8Raw() noexcept( std::is_nothrow_default_constructible<types::String>::value )
          : width_ { 0 }
        {}
        __PGBAR_CXX20_CNSTXPR explicit U8Raw( types::String u8_bytes ) : U8Raw()
        {
          width_ = render_width( u8_bytes );
          bytes_ = std::move( u8_bytes );
        }
        __PGBAR_CXX20_CNSTXPR U8Raw( const Self& )             = default;
        __PGBAR_CXX20_CNSTXPR U8Raw( Self&& )                  = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& ) & = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR ~U8Raw()                         = default;

        __PGBAR_CXX20_CNSTXPR Self& operator=( types::ROStr u8_bytes ) &
        {
          const auto new_width = render_width( u8_bytes );
          auto new_bytes       = types::String( u8_bytes );
          bytes_.swap( new_bytes );
          width_ = new_width;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR Self& operator=( types::String u8_bytes ) &
        {
          width_ = render_width( u8_bytes );
          bytes_.swap( u8_bytes );
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR bool empty() const noexcept { return bytes_.empty(); }
        __PGBAR_CXX20_CNSTXPR types::Size size() const noexcept { return bytes_.size(); }
        __PGBAR_CXX20_CNSTXPR types::Size width() const noexcept { return width_; }
        __PGBAR_CXX20_CNSTXPR types::ROStr str() & noexcept { return bytes_; }
        __PGBAR_CXX20_CNSTXPR types::ROStr str() const& noexcept { return bytes_; }
        __PGBAR_CXX20_CNSTXPR types::String&& str() && noexcept { return std::move( bytes_ ); }

        __PGBAR_CXX20_CNSTXPR void clear() noexcept( noexcept( bytes_.clear() ) )
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
        __PGBAR_CXX20_CNSTXPR explicit operator types::String&&() && noexcept { return std::move( bytes_ ); }
        __PGBAR_CXX20_CNSTXPR operator types::ROStr() const noexcept { return str(); }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          Self&& a,
          const Self& b )
        {
          return std::move( a.bytes_ ) + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const Self& a,
          const Self& b )
        {
          return a.bytes_ + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          types::String&& a,
          const Self& b )
        {
          return std::move( a ) + b.bytes_;
        }
        template<types::Size N>
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const char ( &a )[N],
          const Self& b )
        {
          return a + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const char* a,
          const Self& b )
        {
          return a + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          types::ROStr a,
          const Self& b )
        {
          return types::String( a ) + b;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          Self&& a,
          types::ROStr b )
        {
          a.bytes_.append( b );
          return std::move( a.bytes_ );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          const Self& a,
          types::ROStr b )
        {
          return a.bytes_ + types::String( b );
        }

# if __PGBAR_CXX20
        static_assert( sizeof( char8_t ) == sizeof( char ),
                       "pgbar::__details::chaset::U8Raw: Unexpected type size mismatch" );

        __PGBAR_CXX20_CNSTXPR explicit U8Raw( types::LitU8 u8_sv ) : U8Raw()
        {
          auto new_bytes = types::String( u8_sv.size(), '\0' );
          std::copy( u8_sv.cbegin(), u8_sv.cend(), new_bytes.begin() );
          width_ = render_width( new_bytes );
          bytes_ = std::move( new_bytes );
        }

        __PGBAR_CXX20_CNSTXPR explicit operator std::u8string() const
        {
          std::u8string ret;
          ret.resize( bytes_.size() );
          std::copy( bytes_.cbegin(), bytes_.cend(), ret.begin() );
          return ret;
        }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String operator+(
          types::LitU8 a,
          const Self& b )
        {
          types::String tmp;
          tmp.reserve( a.size() );
          std::copy( a.cbegin(), a.cend(), std::back_inserter( tmp ) );
          return std::move( tmp ) + b.bytes_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN types::String operator+( Self&& a, types::LitU8 b )
        {
          a.bytes_.reserve( a.bytes_.size() + b.size() );
          std::copy( b.cbegin(), b.cend(), std::back_inserter( a.bytes_ ) );
          return std::move( a.bytes_ );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN types::String operator+( const Self& a, types::LitU8 b )
        {
          auto tmp = a.bytes_;
          tmp.reserve( a.bytes_.size() + b.size() );
          std::copy( b.cbegin(), b.cend(), std::back_inserter( tmp ) );
          return tmp;
        }
# endif
      };

      class U8Text : public U8Raw {
        using Self = U8Text;

      protected:
        // The starting offset (in byte) of each utf-8 character and its width.
        std::vector<std::pair<types::Size, CodeChart::RenderWidth>> chars_;

      public:
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR
          std::vector<std::pair<types::Size, CodeChart::RenderWidth>>
          parse_char( types::ROStr u8_str )
        {
          std::vector<std::pair<types::Size, CodeChart::RenderWidth>> characters;
          const auto raw_u8_str = u8_str.data();
          for ( types::Size i = 0; i < u8_str.size(); ) {
            const auto startpoint = raw_u8_str + i;
            __PGBAR_TRUST( startpoint >= raw_u8_str );
            auto resolved = U8Raw::next_char( startpoint, ( u8_str.data() + u8_str.size() ) - startpoint );
            characters.emplace_back( i, U8Raw::char_width( resolved.first ) );
            i += resolved.second;
          }
          return characters;
        }

        __PGBAR_CXX20_CNSTXPR U8Text() noexcept( std::is_nothrow_default_constructible<U8Raw>::value )
          : U8Raw()
        {}
        __PGBAR_CXX20_CNSTXPR explicit U8Text( types::String u8_bytes ) : U8Text()
        {
          chars_ = parse_char( u8_bytes );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
            } );
          bytes_ = std::move( u8_bytes );
        }
        __PGBAR_CXX20_CNSTXPR U8Text( const Self& )            = default;
        __PGBAR_CXX20_CNSTXPR U8Text( Self&& )                 = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& ) & = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR ~U8Text()                        = default;

        __PGBAR_CXX20_CNSTXPR Self& operator=( types::ROStr u8_bytes ) &
        {
          auto new_chars = parse_char( u8_bytes );
          auto new_bytes = types::String( u8_bytes );
          chars_.swap( new_chars );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
            } );
          bytes_.swap( new_bytes );
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR Self& operator=( types::String u8_bytes ) &
        {
          auto new_chars = parse_char( u8_bytes );
          std::swap( chars_, new_chars );
          chars_.swap( new_chars );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
            } );
          bytes_.swap( u8_bytes );
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR void clear()
          noexcept( noexcept( std::declval<U8Raw&>().clear() ) && noexcept( chars_.clear() ) )
        {
          U8Raw::clear();
          chars_.clear();
        }
        __PGBAR_CXX20_CNSTXPR void shrink_to_fit()
          noexcept( noexcept( std::declval<U8Raw&>().shrink_to_fit() ) && noexcept( chars_.shrink_to_fit() ) )
        {
          U8Raw::shrink_to_fit();
          chars_.shrink_to_fit();
        }

        /**
         * @brief Split a string into two parts based on the given length, with UTF-8 characters as the unit.
         * @param length The given length.
         * @return The split result and the width of each part, where the first element is the result not
         * exceeding the length.
         */
        std::pair<std::array<const types::Char*, 3>, std::pair<types::Size, types::Size>> split_by(
          types::Size length ) const noexcept
        {
          if ( bytes_.empty() )
            __PGBAR_UNLIKELY return {
              { nullptr, nullptr, nullptr },
              std::make_pair( 0, 0 )
            };

          // split_pos is the starting point of the right part
          types::Size split_pos  = 0;
          types::Size left_width = 0;
          while ( left_width + chars_[split_pos].second <= length && split_pos < chars_.size() )
            left_width += chars_[split_pos++].second;

          return {
            { bytes_.data(),
             bytes_.data() + ( split_pos < chars_.size() ? chars_[split_pos].first : bytes_.size() ),
             bytes_.data() + bytes_.size() },
            std::make_pair( left_width, width_ - left_width )
          };
        }

        __PGBAR_CXX20_CNSTXPR void swap( Self& lhs ) noexcept
        {
          U8Raw::swap( lhs );
          chars_.swap( lhs.chars_ );
        }
        __PGBAR_CXX20_CNSTXPR friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self operator+( Self&& a,
                                                                                         const Self& b )
        {
          const auto num_a_char = a.chars_.size();
          a.chars_.resize( a.chars_.size() + b.chars_.size() );
          std::transform( b.chars_.cbegin(),
                          b.chars_.cend(),
                          a.chars_.begin() + num_a_char,
                          [&a]( const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
                            return std::make_pair( ch.first + a.size(),
                                                   static_cast<types::Size>( ch.second ) );
                          } );
          a.bytes_.append( b.bytes_ );
          a.width_ += b.width_;
          return a;
        }

# if __PGBAR_CXX20
        __PGBAR_CXX20_CNSTXPR explicit U8Text( types::LitU8 u8_sv ) : U8Text()
        {
          auto new_bytes = types::String( u8_sv.size(), '\0' );
          std::copy( u8_sv.cbegin(), u8_sv.cend(), new_bytes.begin() );
          chars_ = parse_char( new_bytes );
          width_ = std::accumulate(
            chars_.cbegin(),
            chars_.cend(),
            types::Size {},
            []( types::Size acc, const std::pair<types::Size, CodeChart::RenderWidth>& ch ) noexcept {
              return acc + static_cast<types::Size>( ch.second );
            } );
          bytes_ = std::move( new_bytes );
        }
# endif
      };
    } // namespace charcodes

    namespace concurrent {
      // Wait for pred to be true.
      template<typename F>
      __PGBAR_INLINE_FN void spin_wait( F&& pred, types::Size threshold ) noexcept( noexcept( pred() ) )
      {
        for ( types::Size cnt = 0; !pred(); ) {
          if ( cnt < threshold )
            ++cnt;
          else
            std::this_thread::yield();
        }
      }
      template<typename F>
      __PGBAR_INLINE_FN void spin_wait( F&& pred ) noexcept( noexcept( pred() ) )
      {
        spin_wait( std::forward<F>( pred ), 128 );
      }

      template<typename F, typename Rep, typename Period>
      __PGBAR_INLINE_FN bool spin_wait_for( F&& pred,
                                            types::Size threshold,
                                            const std::chrono::duration<Rep, Period>& timeout )
        noexcept( noexcept( pred() ) )
      {
        const auto start = std::chrono::steady_clock::now();
        for ( types::Size cnt; !pred(); ) {
          const auto elapsed = std::chrono::steady_clock::now() - start;
          if ( elapsed >= timeout )
            return false;
          else if ( cnt < threshold ) {
            ++cnt;
            continue;
          }
          std::this_thread::yield();
        }
        return true;
      }
      template<typename F, typename Rep, typename Period>
      __PGBAR_INLINE_FN bool spin_wait_for( F&& pred, const std::chrono::duration<Rep, Period>& timeout )
        noexcept( noexcept( pred() ) )
      {
        return spin_wait_for( std::forward<F>( pred ), 128, timeout );
      }

# if __PGBAR_CXX17
      using SharedMutex = std::shared_mutex;
# else
      // A simple `Shared Mutex` implementation for any C++ version.
      class SharedMutex final {
        using Self = SharedMutex;

      protected:
        std::atomic<std::uint64_t> num_readers_;
        std::mutex writer_mtx_;
        /**
         * Although the `lock()` and `unlock()` functions of `std::mutex`
         * in the standard library are not marked as `noexcept`,
         * in practice, it is very rare for exceptions to be thrown,
         * and usually indicate a bug in the program itself.
         * Therefore, it is assumed here that `std::mutex` is `noexcept`.

         * For more details, see: https://stackoverflow.com/questions/17551256
         */

      public:
        SharedMutex( const Self& )       = delete;
        Self& operator=( const Self& ) & = delete;

        SharedMutex() noexcept : num_readers_ { 0 } {}
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
# endif
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
        // Internal component, assume that the lock object always holds a mutex when destructing.
        SharedLock( mutex_type& m, std::defer_lock_t ) noexcept : mtx_ { m } {}
        SharedLock( mutex_type& m, std::adopt_lock_t ) noexcept : mtx_ { m } {}
        ~SharedLock() noexcept { mtx_.unlock_shared(); }

        void lock() & noexcept { mtx_.lock_shared(); }
        __PGBAR_NODISCARD bool try_lock() & noexcept { return mtx_.try_lock_shared(); }
        __PGBAR_INLINE_FN void unlock() & noexcept { mtx_.unlock_shared(); }
      };

# endif

      // A nullable container that holds an exception pointer.
      class ExceptionBox final {
        using Self = ExceptionBox;

        std::exception_ptr exception_;
        mutable SharedMutex rw_mtx_;

      public:
        ExceptionBox()  = default;
        ~ExceptionBox() = default;

        ExceptionBox( ExceptionBox&& rhs ) noexcept : ExceptionBox()
        {
          std::lock_guard<SharedMutex> lock { rhs.rw_mtx_ };
          using std::swap;
          swap( exception_, rhs.exception_ );
        }
        ExceptionBox& operator=( ExceptionBox&& rhs ) & noexcept
        {
          __PGBAR_TRUST( this != &rhs );
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

        // Store the exception if it is empty and return true, otherwise return false.
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool try_store( std::exception_ptr e ) & noexcept
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          if ( exception_ )
            return false;
          exception_ = std::move( e );
          return true;
        }
        __PGBAR_INLINE_FN std::exception_ptr load() const noexcept
        {
          SharedLock<SharedMutex> lock { rw_mtx_ };
          return exception_;
        }
        __PGBAR_INLINE_FN Self& clear() noexcept
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          exception_ = std::exception_ptr();
          return *this;
        }

        // Rethrow the exception pointed if it isn't null.
        __PGBAR_INLINE_FN void rethrow() noexcept( false )
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          if ( exception_ ) {
            auto exception_ptr = exception_;
            exception_         = std::exception_ptr();
            if ( exception_ptr )
              std::rethrow_exception( std::move( exception_ptr ) );
          }
        }

        void swap( ExceptionBox& lhs ) noexcept
        {
          __PGBAR_TRUST( this != &lhs );
          std::lock( this->rw_mtx_, lhs.rw_mtx_ );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_, std::adopt_lock };
          std::lock_guard<concurrent::SharedMutex> lock2 { lhs.rw_mtx_, std::adopt_lock };
          using std::swap; // ADL custom point
          swap( exception_, lhs.exception_ );
        }
        friend void swap( ExceptionBox& a, ExceptionBox& b ) noexcept { a.swap( b ); }
      };
    } // namespace concurrent

    namespace console {
      template<Channel Outlet>
      class TermContext {
        std::atomic<bool> cache_;

        TermContext() noexcept { detect(); }

      public:
        TermContext( const TermContext& )              = delete;
        TermContext& operator=( const TermContext& ) & = delete;

        ~TermContext() = default;

        static TermContext& itself() noexcept
        {
          static TermContext self;
          return self;
        }

        // Detect whether the specified output stream is bound to a terminal.
        bool detect() noexcept
        {
          const bool value = []() noexcept {
# if defined( PGBAR_INTTY ) || __PGBAR_UNKNOWN
            return true;
# elif __PGBAR_WIN
            HANDLE hConsole;
            if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
              hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
            else
              hConsole = GetStdHandle( STD_ERROR_HANDLE );
            if ( hConsole == INVALID_HANDLE_VALUE )
              __PGBAR_UNLIKELY return false;
            return GetFileType( hConsole ) == FILE_TYPE_CHAR;
# else
            return isatty( static_cast<int>( Outlet ) );
# endif
          }();
          cache_.store( value, std::memory_order_release );
          return value;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool connected() const noexcept
        {
          return cache_.load( std::memory_order_acquire );
        }

        /**
         * Enable virtual terminal processing on the specified output channel (Windows only).
         * Guaranteed to be thread-safe and performed only once.
         */
        void virtual_term() const noexcept
        {
# if __PGBAR_WIN && !defined( PGBAR_NOCOLOR ) && defined( ENABLE_VIRTUAL_TERMINAL_PROCESSING )
          static std::once_flag flag;
          std::call_once( flag, []() noexcept {
            HANDLE stream_handle =
              GetStdHandle( Outlet == Channel::Stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE );
            if ( stream_handle == INVALID_HANDLE_VALUE )
              __PGBAR_UNLIKELY return;

            DWORD mode {};
            if ( !GetConsoleMode( stream_handle, &mode ) )
              __PGBAR_UNLIKELY return;
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode( stream_handle, mode );
          } );
# endif
        }

        __PGBAR_NODISCARD types::Size width() noexcept
        {
          if ( !detect() )
            return 0;
# if __PGBAR_WIN
          HANDLE hConsole;
          if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
            hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
          else
            hConsole = GetStdHandle( STD_ERROR_HANDLE );
          if ( hConsole != INVALID_HANDLE_VALUE ) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if ( GetConsoleScreenBufferInfo( hConsole, &csbi ) )
              return csbi.srWindow.Right - csbi.srWindow.Left + 1;
          }
# elif __PGBAR_UNIX
          struct winsize ws;
          auto fd = static_cast<int>( Outlet );
          if ( ioctl( fd, TIOCGWINSZ, &ws ) != -1 )
            return ws.ws_col;
# endif
          return 100;
        }
      };
    } // namespace console

    namespace io {
      // A simple string buffer, unrelated to the `std::stringbuf` in the STL.
      class Stringbuf {
        using Self = Stringbuf;

      protected:
        std::vector<types::Char> buffer_;

      public:
        __PGBAR_CXX20_CNSTXPR Stringbuf() = default;

        __PGBAR_CXX20_CNSTXPR Stringbuf( const Self& lhs )                           = default;
        __PGBAR_CXX20_CNSTXPR Stringbuf( Self&& rhs ) noexcept                       = default;
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& lhs ) & = default;
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& rhs ) &      = default;
        // Intentional non-virtual destructors.
        __PGBAR_CXX20_CNSTXPR ~Stringbuf()                                           = default;

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
          while ( __num-- )
            buffer_.insert( buffer_.end(), info, info + N );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( types::ROStr info, types::Size __num = 1 ) &
        {
          while ( __num-- )
            buffer_.insert( buffer_.end(), info.cbegin(), info.cend() );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const charcodes::U8Raw& info,
                                                              types::Size __num = 1 ) &
        {
          return append( info.str(), __num );
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const types::Char* head,
                                                              const types::Char* tail ) &
        {
          if ( head != nullptr && tail != nullptr )
            buffer_.insert( buffer_.end(), head, tail );
          return *this;
        }

        template<typename T>
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR typename std::enable_if<
          traits::AnyOf<std::is_same<typename std::decay<T>::type, types::Char>,
                        std::is_same<typename std::decay<T>::type, types::String>,
                        std::is_same<typename std::decay<T>::type, charcodes::U8Raw>>::value,
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
          __PGBAR_TRUST( this != &lhs );
          buffer_.swap( lhs.buffer_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( Stringbuf& a, Stringbuf& b ) noexcept { a.swap( b ); }

# if __PGBAR_CXX20
        __PGBAR_INLINE_FN Self& append( types::LitU8 info, types::Size __num = 1 ) &
        {
          while ( __num-- )
            buffer_.insert( buffer_.end(), info.data(), info.data() + info.size() );
          return *this;
        }
        friend __PGBAR_INLINE_FN Self& operator<<( Self& stream, types::LitU8 info )
        {
          return stream.append( info );
        }
# endif
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

# if __PGBAR_WIN
        std::vector<WCHAR> wb_buffer_;
        std::vector<types::Char> localized_;
# endif

        __PGBAR_CXX20_CNSTXPR OStream() = default;

      public:
        static Self& itself() noexcept( std::is_nothrow_default_constructible<Stringbuf>::value )
        {
          static OStream instance;
          return instance;
        }

        static void writeout( const std::vector<types::Char>& bytes )
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
                         bytes.data() + total_written,
                         static_cast<DWORD>( bytes.size() - total_written ),
                         &num_written,
                         nullptr );
            } else {
              auto h_stderr = GetStdHandle( STD_ERROR_HANDLE );
              if ( h_stderr == INVALID_HANDLE_VALUE )
                __PGBAR_UNLIKELY throw exception::SystemError(
                  "pgbar: cannot open the standard error stream" );
              WriteFile( h_stderr,
                         bytes.data() + total_written,
                         static_cast<DWORD>( bytes.size() - total_written ),
                         &num_written,
                         nullptr );
            }
            if ( num_written <= 0 )
              break; // ignore it
            total_written += static_cast<types::Size>( num_written );
          } while ( total_written < bytes.size() );
# elif __PGBAR_UNIX
          types::Size total_written = 0;
          do {
            ssize_t num_written = 0;
            if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
              num_written =
                write( STDOUT_FILENO, bytes.data() + total_written, bytes.size() - total_written );
            else
              num_written =
                write( STDERR_FILENO, bytes.data() + total_written, bytes.size() - total_written );
            if ( errno == EINTR )
              num_written = num_written < 0 ? 0 : num_written;
            else if ( num_written < 0 )
              break;
            total_written += static_cast<types::Size>( num_written );
          } while ( total_written < bytes.size() );
# else
          if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
            std::cout.write( bytes.data(), bytes.size() ).flush();
          else
            std::cerr.write( bytes.data(), bytes.size() ).flush();
# endif
        }

        __PGBAR_CXX20_CNSTXPR OStream( const Self& )           = delete;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& ) & = delete;
        // Intentional non-virtual destructors.
        __PGBAR_CXX20_CNSTXPR ~OStream()                       = default;

# if __PGBAR_WIN
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void release() noexcept
        {
          Stringbuf::release();
          wb_buffer_.clear();
          wb_buffer_.shrink_to_fit();
          localized_.clear();
          localized_.shrink_to_fit();
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void clear() & noexcept
        {
          Stringbuf::clear();
          wb_buffer_.clear();
          localized_.clear();
        }
# endif

        __PGBAR_INLINE_FN Self& flush() &
        {
          if ( this->buffer_.empty() )
            return *this;
# if __PGBAR_WIN
          const auto codepage = GetConsoleOutputCP();
          if ( !console::TermContext<Outlet>::itself().connected() || codepage == CP_UTF8 ) {
            writeout( this->buffer_ );
            return *this;
          }

          const auto wlen =
            MultiByteToWideChar( CP_UTF8, 0, buffer_.data(), static_cast<int>( buffer_.size() ), nullptr, 0 );
          __PGBAR_ASSERT( wlen > 0 );
          wb_buffer_.resize( static_cast<types::Size>( wlen ) );
          MultiByteToWideChar( CP_UTF8,
                               0,
                               buffer_.data(),
                               static_cast<int>( buffer_.size() ),
                               wb_buffer_.data(),
                               wlen );

          const auto mblen =
            WideCharToMultiByte( codepage, 0, wb_buffer_.data(), wlen, nullptr, 0, nullptr, nullptr );
          __PGBAR_ASSERT( mblen > 0 );
          localized_.resize( static_cast<types::Size>( mblen ) );
          WideCharToMultiByte( codepage,
                               0,
                               wb_buffer_.data(),
                               wlen,
                               localized_.data(),
                               mblen,
                               nullptr,
                               nullptr );
          writeout( localized_ );
# else
          writeout( this->buffer_ );
# endif
          clear();
          return *this;
        }

        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( OStream& stream,
                                                                         OStream& ( *fnptr )(OStream&))
        {
          __PGBAR_TRUST( fnptr != nullptr );
          return fnptr( stream );
        }
      };
    } // namespace io

    namespace console {
      // escape codes
      namespace escodes {
# ifdef PGBAR_NOCOLOR
        __PGBAR_CXX17_INLINE constexpr types::LitU8 fontreset = u8"";
        __PGBAR_CXX17_INLINE constexpr types::LitU8 fontbold  = u8"";
# else
        __PGBAR_CXX17_INLINE constexpr types::LitU8 fontreset = u8"\x1B[0m";
        __PGBAR_CXX17_INLINE constexpr types::LitU8 fontbold  = u8"\x1B[1m";
# endif
        __PGBAR_CXX17_INLINE constexpr types::LitU8 savecursor  = u8"\x1B[s";
        __PGBAR_CXX17_INLINE constexpr types::LitU8 resetcursor = u8"\x1B[u";
        __PGBAR_CXX17_INLINE constexpr types::LitU8 linewipe    = u8"\x1B[K";
        __PGBAR_CXX17_INLINE constexpr types::LitU8 prevline    = u8"\x1b[A";
        __PGBAR_CXX17_INLINE constexpr types::Char nextline     = '\n';
        __PGBAR_CXX17_INLINE constexpr types::Char linestart    = '\r';

        class RGBColor {
          using Self = RGBColor;

          std::array<types::Char, 17> sgr_; // Select Graphic Rendition
          std::uint8_t length_;

          static __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR types::Char* to_char( types::Char* first,
                                                                               types::Char* last,
                                                                               std::uint8_t value ) noexcept
          {
# if __PGBAR_CXX17
            auto result = std::to_chars( first, last, value );
            __PGBAR_TRUST( result.ec == std::errc() );
            return result.ptr;
# else
            types::Size offset = 1;
            if ( value >= 100 ) {
              __PGBAR_TRUST( last - first >= 3 );
              first[0] = '0' + value / 100;
              first[1] = '0' + ( value / 10 % 10 );
              first[2] = '0' + ( value % 10 );
              offset += 2;
            } else if ( value >= 10 ) {
              __PGBAR_TRUST( last - first >= 2 );
              first[0] = '0' + value / 10;
              first[1] = '0' + ( value % 10 );
              offset += 1;
            } else
              first[0] = '0' + value;
            __PGBAR_TRUST( last - first >= 1 );
            return first + offset;
# endif
          }

          __PGBAR_CXX23_CNSTXPR void from_hex( types::HexRGB hex_val ) & noexcept
          {
# ifndef PGBAR_NOCOLOR
            length_ = 1;
            if ( hex_val == __PGBAR_DEFAULT ) {
              sgr_[0] = '0';
              return;
            }

            length_ = 2;
            sgr_[0] = '3';
            switch ( hex_val & 0x00FFFFFF ) { // discard the high 8 bits
            case __PGBAR_BLACK:   sgr_[1] = '0'; break;
            case __PGBAR_RED:     sgr_[1] = '1'; break;
            case __PGBAR_GREEN:   sgr_[1] = '2'; break;
            case __PGBAR_YELLOW:  sgr_[1] = '3'; break;
            case __PGBAR_BLUE:    sgr_[1] = '4'; break;
            case __PGBAR_MAGENTA: sgr_[1] = '5'; break;
            case __PGBAR_CYAN:    sgr_[1] = '6'; break;
            case __PGBAR_WHITE:   sgr_[1] = '7'; break;
            default:              {
              sgr_[1] = '8', sgr_[2] = ';', sgr_[3] = '2', sgr_[4] = ';';
              auto tail = to_char( sgr_.data() + 5, sgr_.data() + sgr_.size(), ( hex_val >> 16 ) & 0xFF );
              *tail     = ';';
              tail      = to_char( tail + 1, sgr_.data() + sgr_.size(), ( hex_val >> 8 ) & 0xFF );
              *tail     = ';';
              tail      = to_char( tail + 1, sgr_.data() + sgr_.size(), hex_val & 0xFF );
              length_   = static_cast<std::uint8_t>( tail - sgr_.data() );
            } break;
            }
# endif
          }
          __PGBAR_CXX23_CNSTXPR void from_str( types::ROStr hex_str ) &
          {
            if ( ( hex_str.size() != 7 && hex_str.size() != 4 ) || hex_str.front() != '#' )
              throw exception::InvalidArgument( "pgbar: invalid hex color format" );

            for ( types::Size i = 1; i < hex_str.size(); i++ ) {
              if ( ( hex_str[i] < '0' || hex_str[i] > '9' ) && ( hex_str[i] < 'A' || hex_str[i] > 'F' )
                   && ( hex_str[i] < 'a' || hex_str[i] > 'f' ) )
                throw exception::InvalidArgument( "pgbar: invalid hexadecimal letter" );
            }

# ifndef PGBAR_NOCOLOR
            std::uint32_t hex_val = 0;
            if ( hex_str.size() == 4 ) {
              for ( types::Size i = 1; i < hex_str.size(); ++i ) {
                hex_val <<= 4;
                if ( hex_str[i] >= '0' && hex_str[i] <= '9' )
                  hex_val = ( ( hex_val | ( hex_str[i] - '0' ) ) << 4 ) | ( hex_str[i] - '0' );
                else if ( hex_str[i] >= 'A' && hex_str[i] <= 'F' )
                  hex_val = ( ( hex_val | ( hex_str[i] - 'A' + 10 ) ) << 4 ) | ( hex_str[i] - 'A' + 10 );
                else // no need to check whether it's valid or not
                  hex_val = ( ( hex_val | ( hex_str[i] - 'a' + 10 ) ) << 4 ) | ( hex_str[i] - 'a' + 10 );
              }
            } else {
              for ( types::Size i = 1; i < hex_str.size(); ++i ) {
                hex_val <<= 4;
                if ( hex_str[i] >= '0' && hex_str[i] <= '9' )
                  hex_val |= hex_str[i] - '0';
                else if ( hex_str[i] >= 'A' && hex_str[i] <= 'F' )
                  hex_val |= hex_str[i] - 'A' + 10;
                else
                  hex_val |= hex_str[i] - 'a' + 10;
              }
            }
            from_hex( hex_val );
# endif
          }

        public:
          __PGBAR_CXX23_CNSTXPR RGBColor() noexcept { clear(); }

          __PGBAR_CXX23_CNSTXPR RGBColor( types::HexRGB hex_val ) noexcept : RGBColor()
          {
            from_hex( hex_val );
          }
          __PGBAR_CXX23_CNSTXPR RGBColor( types::ROStr hex_str ) : RGBColor() { from_str( hex_str ); }

          __PGBAR_CXX23_CNSTXPR RGBColor( const Self& lhs ) noexcept = default;
          __PGBAR_CXX23_CNSTXPR Self& operator=( const Self& lhs ) & = default;

          __PGBAR_CXX23_CNSTXPR Self& operator=( types::HexRGB hex_val ) & noexcept
          {
            from_hex( hex_val );
            return *this;
          }
          __PGBAR_CXX23_CNSTXPR Self& operator=( types::ROStr hex_str ) &
          {
            from_str( hex_str );
            return *this;
          }

          __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR void clear() noexcept
          {
            std::fill( sgr_.begin(), sgr_.end(), '\0' );
            length_ = 0;
          }

          __PGBAR_CXX23_CNSTXPR void swap( RGBColor& lhs ) noexcept
          {
            std::swap( sgr_, lhs.sgr_ );
            std::swap( length_, lhs.length_ );
          }
          __PGBAR_CXX23_CNSTXPR friend void swap( RGBColor& a, RGBColor& b ) noexcept { a.swap( b ); }

          __PGBAR_INLINE_FN friend io::Stringbuf& operator<<( io::Stringbuf& buf, const Self& col )
          {
# ifndef PGBAR_NOCOLOR
            buf.append( '\x1B' )
              .append( '[' )
              .append( col.sgr_.data(), col.sgr_.data() + col.length_ )
              .append( 'm' );
# endif
            return buf;
          }
        };
      } // namespace escodes
    } // namespace console

    namespace render {
      template<Channel Tag>
      class Runner final {
        using Self = Runner;

        enum class State : std::uint8_t { Dormant, Suspend, Active, Dead };
        std::atomic<State> state_;

        std::thread td_;
        concurrent::ExceptionBox box_;
        mutable std::condition_variable cond_var_;
        mutable std::mutex mtx_;
        mutable concurrent::SharedMutex rw_mtx_;

        wrappers::UniqueFunction<void()> task_;

        void launch() & noexcept( false )
        {
          console::TermContext<Tag>::itself().virtual_term();

          __PGBAR_ASSERT( td_.get_id() == std::thread::id() );
          state_.store( State::Dormant, std::memory_order_release );
          try {
            td_ = std::thread( [this]() {
              try {
                while ( state_.load( std::memory_order_acquire ) != State::Dead ) {
                  switch ( state_.load( std::memory_order_acquire ) ) {
                  case State::Dormant: __PGBAR_FALLTHROUGH;
                  case State::Suspend: {
                    std::unique_lock<std::mutex> lock { mtx_ };
                    auto expected = State::Suspend;
                    state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
                    cond_var_.wait( lock, [this]() noexcept {
                      return state_.load( std::memory_order_acquire ) != State::Dormant;
                    } );
                  } break;
                  case State::Active: {
                    task_();
                  } break;
                  default: return;
                  }
                }
              } catch ( ... ) {
                if ( !box_.try_store( std::current_exception() ) ) {
                  state_.store( State::Dead, std::memory_order_release );
                  throw;
                }
              }
            } );
          } catch ( ... ) {
            state_.store( State::Dead, std::memory_order_release );
            throw;
          }
        }

        // Since the control flow of the child thread has been completely handed over to task_,
        // this function does not guarantee that the child thread can be closed immediately after being
        // called.
        void shutdown() noexcept
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

        Runner() noexcept : state_ { State::Dead } {}

      public:
        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }

        Runner( const Self& )            = delete;
        Self& operator=( const Self& ) & = delete;
        ~Runner() noexcept { shutdown(); }

        void suspend() noexcept
        {
          auto expected = State::Active;
          if ( state_.compare_exchange_strong( expected, State::Suspend, std::memory_order_release ) ) {
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Suspend; } );
            std::lock_guard<std::mutex> lock { mtx_ };
            // Ensure that the background thread is truely suspended.

            // Discard the current cycle's captured exception.
            // Reason: Delaying the exception and throwing it on the next `activate()` call is meaningless,
            // as it refers to a previous cycle's failure and will no longer be relevant in the new cycle.
            box_.clear();
          }
        }
        void activate() & noexcept( false )
        {
          {
            std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
            if ( td_.get_id() == std::thread::id() )
              launch();
            else if ( state_.load( std::memory_order_acquire ) == State::Dead ) {
              shutdown();
              launch();
            }
          }

          // The operations below are all thread safe without locking.
          // Also, only one thread needs to be able to execute concurrently.
          box_.rethrow();
          __PGBAR_ASSERT( state_ != State::Dead );
          __PGBAR_ASSERT( task_ != nullptr );
          auto expected = State::Dormant;
          if ( state_.compare_exchange_strong( expected, State::Active, std::memory_order_release ) ) {
            std::lock_guard<std::mutex> lock { mtx_ };
            cond_var_.notify_one();
          }
        }

        void appoint() noexcept
        {
          suspend();
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          task_ = nullptr;
        }
        __PGBAR_NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr )
            return false;
          // Under normal circumstances, `online() == false` implies that `task_ == nullptr`.
          __PGBAR_ASSERT( online() == false );
          task_.swap( task );
          return true;
        }

        void drop() noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          shutdown();
        }
        void throw_if() noexcept( false ) { box_.rethrow(); }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ == nullptr;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool online() const noexcept
        {
          return state_.load( std::memory_order_acquire ) == State::Active;
        }
      };

      template<Channel Tag, Policy Mode>
      class Renderer;
      template<Channel Tag>
      class Renderer<Tag, Policy::Sync> final {
        using Self = Renderer;

        enum class State : std::uint8_t { Dormant, Finish, Active, Quit };
        std::atomic<State> state_;

        mutable concurrent::SharedMutex rw_mtx_;
        mutable std::mutex mtx_;
        mutable std::condition_variable cond_var_;

        wrappers::UniqueFunction<void()> task_;

        Renderer() noexcept : state_ { State::Quit } {}

      public:
        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }
        Renderer( const Self& )        = delete;
        Self& operator=( const Self& ) = delete;

        ~Renderer() noexcept { appoint(); }

        // `activate` guarantees to perform the render task at least once.
        void activate() & noexcept( false )
        { /**
           * Since the rendering function is a state machine,
           * `activate` must ensure it runs at least once to trigger state transitions.
           * As asynchronous renderers do not provide an interface that guarantees at-least-once execution,
           * `activate` must explicitly invoke the rendering function during scheduling.
           */
          std::lock_guard<concurrent::SharedMutex> lock1 { rw_mtx_ };
          // Internal component does not make validity judgments.
          __PGBAR_ASSERT( task_ != nullptr );
          __PGBAR_ASSERT( state_ == State::Dormant );
          Runner<Tag>::itself().activate();
          task_();
          // If the rendering task throws some exception, this rendering should be abandoned.
        }

        template<typename F>
        void appoint_then( F&& noexpt_fn ) noexcept
        {
          static_assert( noexcept( (void)noexpt_fn() ),
                         "pgbar::__details::render::Renderer::appoint_then: Unsafe functor types" );

          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr ) {
            state_.store( State::Quit, std::memory_order_release );
            {
              std::unique_lock<std::mutex> lock { mtx_ };
              cond_var_.notify_one();
            }
            Runner<Tag>::itself().appoint();
            task_ = nullptr;
          }
          (void)noexpt_fn();
        }
        void appoint() noexcept
        {
          appoint_then( []() noexcept {} );
        }

        __PGBAR_NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr || !Runner<Tag>::itself().try_appoint( [this]() {
                 try {
                   while ( state_.load( std::memory_order_acquire ) != State::Quit ) {
                     switch ( state_.load( std::memory_order_acquire ) ) {
                     case State::Dormant: __PGBAR_FALLTHROUGH;
                     case State::Finish:  {
                       std::unique_lock<std::mutex> lock { mtx_ };
                       auto expected = State::Finish;
                       state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
                       cond_var_.wait( lock, [this]() noexcept {
                         return state_.load( std::memory_order_acquire ) != State::Dormant;
                       } );
                     } break;
                     case State::Active: {
                       {
                         concurrent::SharedLock<concurrent::SharedMutex> lock1 { rw_mtx_ };
                         std::lock_guard<std::mutex> lock2 { mtx_ };
                         task_();
                       }
                       auto expected = State::Active;
                       state_.compare_exchange_strong( expected, State::Finish, std::memory_order_release );
                     } break;
                     default: return;
                     }
                   }
                 } catch ( ... ) {
                   state_.store( State::Quit, std::memory_order_release );
                   throw;
                 }
               } ) )
            return false;
          state_.store( State::Dormant, std::memory_order_release );
          task_.swap( task );
          return true;
        }

        // Assume that the task is not empty and execute it once.
        __PGBAR_INLINE_FN void execute() & noexcept( false )
        { /**
           * Although not relevant here,
           * OStream is called non-atomically for each rendering task,
           * so it needs to be locked for the entire duration of the rendering task execution.

           * By the way, although the implementation tries to lock the internal component
           * when it renders the string,
           * that is a reader lock on the configuration data type of the component that stores the string,
           * and it does not prevent concurrent threads from simultaneously
           * trying to concatenate the string in the same OStream.
           */
          concurrent::SharedLock<concurrent::SharedMutex> lock1 { rw_mtx_ };
          std::lock_guard<std::mutex> lock2 { mtx_ }; // Fixed locking order.
          __PGBAR_ASSERT( task_ != nullptr );
          task_();
          /**
           * In short: The mtx_ here is to prevent multiple progress bars in MultiBar
           * from concurrently attempting to render strings in synchronous rendering mode;
           * each of these progress bars will lock itself before rendering,
           * but they cannot mutually exclusively access the renderer.
           */
        }
        // When the task is not empty, execute at least one task.
        void attempt() & noexcept
        {
          // The lock here is to ensure that only one thread executes the task_ at any given time.
          // And synchronization semantics require that no call request be dropped.
          concurrent::SharedLock<concurrent::SharedMutex> lock1 { rw_mtx_ };
          /**
           * Here, asynchronous threads and forced state checks are used to achieve synchronous semantics;
           * this is because task_ is not exception-free,
           * so if task_ needs to be executed with synchronous semantics in some functions marked as noexcept,
           * the execution operation needs to be dispatched to another thread.
           */
          auto expected = State::Dormant;
          if ( state_.compare_exchange_strong( expected, State::Active, std::memory_order_release ) ) {
            {
              std::lock_guard<std::mutex> lock { mtx_ };
              cond_var_.notify_one();
            }
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Active; } );
            std::lock_guard<std::mutex> lock { mtx_ };
          }
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ == nullptr;
        }
      };

      template<Channel Tag>
      class Renderer<Tag, Policy::Async> final {
        using Self = Renderer;

        static std::atomic<types::TimeUnit> _working_interval;

        /**
         * Unlike synchronous renderers,
         * asynchronous renderers do not require the background thread to be suspended for a long time,
         * so the condition variable is simplified away here.
         */
        enum class State : std::uint8_t { Awake, Active, Attempt, Quit };
        std::atomic<State> state_;
        wrappers::UniqueFunction<void()> task_;
        mutable concurrent::SharedMutex rw_mtx_;

        Renderer() noexcept : state_ { State::Quit } {}

      public:
        // Get the current working interval for all threads.
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN types::TimeUnit working_interval()
        {
          return _working_interval.load( std::memory_order_acquire );
        }
        // Adjust the thread working interval between this loop and the next loop.
        static __PGBAR_INLINE_FN void working_interval( types::TimeUnit new_rate )
        {
          _working_interval.store( new_rate, std::memory_order_release );
        }

        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }

        Renderer( const Self& )          = delete;
        Self& operator=( const Self& ) & = delete;
        ~Renderer() noexcept { appoint(); }

        void activate() & noexcept( false )
        {
          auto expected = State::Quit;
          if ( state_.compare_exchange_strong( expected, State::Awake, std::memory_order_release ) ) {
            __PGBAR_ASSERT( task_ != nullptr );
            auto& renderer = Runner<Tag>::itself();
            try {
              renderer.activate();
            } catch ( ... ) {
              /**
               * In Runner::activate, there are only two sources of exceptions,
               * Runner::launch or the background thread;
               * in either case, the Runner retains the assigned task,
               * so the task status is still valid and should be switched back to Quit.
               */
              state_.store( State::Quit, std::memory_order_release );
              throw;
            }
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Awake; } );
            renderer.throw_if();
            // Recheck whether any exceptions have been received in the background thread.
            // If so, abandon this rendering.
          }
        }

        template<typename F>
        void appoint_then( F&& noexpt_fn ) noexcept
        {
          static_assert( noexcept( (void)noexpt_fn() ),
                         "pgbar::__details::render::Renderer::appoint_then: Unsafe functor types" );

          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr ) {
            state_.store( State::Quit, std::memory_order_release );
            Runner<Tag>::itself().appoint();
            task_ = nullptr;
          }
          (void)noexpt_fn();
        }
        void appoint() noexcept
        {
          appoint_then( []() noexcept {} );
        }
        __PGBAR_NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr || !Runner<Tag>::itself().try_appoint( [this]() {
                 try {
                   while ( state_.load( std::memory_order_acquire ) != State::Quit ) {
                     switch ( state_.load( std::memory_order_acquire ) ) {
                     case State::Awake: {
                       task_();
                       auto expected = State::Awake;
                       state_.compare_exchange_strong( expected, State::Active, std::memory_order_release );
                     }
                       __PGBAR_FALLTHROUGH;
                     case State::Active: {
                       task_();
                       std::this_thread::sleep_for( working_interval() );
                     } break;

                     case State::Attempt: {
                       task_();
                       auto expected = State::Attempt;
                       state_.compare_exchange_strong( expected, State::Active, std::memory_order_release );
                     } break;

                     default: return;
                     }
                   }
                 } catch ( ... ) {
                   state_.store( State::Quit, std::memory_order_release );
                   throw;
                 }
               } ) )
            return false;
          state_.store( State::Quit, std::memory_order_release );
          task_.swap( task );
          return true;
        }

        __PGBAR_INLINE_FN void execute() & noexcept { /* Empty implementation */ }
        __PGBAR_INLINE_FN void attempt() & noexcept
        {
          // Each call should not be discarded.
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          auto try_update = [this]( State expected ) noexcept {
            return state_.compare_exchange_strong( expected, State::Attempt, std::memory_order_release );
          };
          if ( try_update( State::Awake ) || try_update( State::Active ) )
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Attempt; } );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ == nullptr;
        }
      };
      template<Channel Tag>
      std::atomic<types::TimeUnit> Renderer<Tag, Policy::Async>::_working_interval {
        std::chrono::duration_cast<types::TimeUnit>( std::chrono::milliseconds( 40 ) )
      };
    } // namespace render
  } // namespace __details

  class Indicator {
    static std::atomic<bool> _hide_completed;
    static std::atomic<bool> _disable_styling;

    friend void config::hide_completed( bool ) noexcept;
    friend bool config::hide_completed() noexcept;
    friend void config::disable_styling( bool ) noexcept;
    friend bool config::disable_styling() noexcept;

  public:
    Indicator()                                = default;
    Indicator( const Indicator& )              = delete;
    Indicator& operator=( const Indicator& ) & = delete;
    Indicator( Indicator&& )                   = default;
    Indicator& operator=( Indicator&& ) &      = default;
    virtual ~Indicator()                       = default;

    virtual void reset() noexcept                          = 0;
    __PGBAR_NODISCARD virtual bool active() const noexcept = 0;

    // Wait until the indicator is Stop.
    void wait() const noexcept
    {
      __details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for the indicator is Stop or timed out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return __details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }
  };
  __PGBAR_CXX17_INLINE std::atomic<bool> Indicator::_hide_completed { false };
  __PGBAR_CXX17_INLINE std::atomic<bool> Indicator::_disable_styling { true };

  namespace slice {
    /**
     * An bidirectional range delimited by an numeric interval [start, end).
     *
     * The `end` can be less than the `start` only if the `step` is negative,
     * otherwise it throws exception `pgbar::exception::InvalidArgument`.
     */
    template<typename N>
    class NumericSpan
# if __PGBAR_CXX20
      : public std::ranges::view_interface<NumericSpan<N>>
# endif
    {
      static_assert( std::is_arithmetic<N>::value,
                     "pgbar::slice::NumericSpan: Only available for arithmetic types" );

      N start_, end_, step_;

    public:
      class iterator {
        N itr_start_, itr_step_;
        __details::types::Size itr_cnt_;

      public:
# if __PGBAR_CXX20
        using iterator_category = std::random_access_iterator_tag;
# else
        using iterator_category = std::bidirectional_iterator_tag;
# endif
        using value_type      = N;
        using difference_type = std::ptrdiff_t;
        using pointer         = value_type*;
        using reference       = value_type;

        constexpr iterator( N startpoint, N step, __details::types::Size iterated = 0 ) noexcept
          : itr_start_ { startpoint }, itr_step_ { step }, itr_cnt_ { iterated }
        {}
        constexpr iterator() noexcept : iterator( {}, 1, {} ) {}
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
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator--() & noexcept
        {
          --itr_cnt_;
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator--( int ) & noexcept
        {
          auto before = *this;
          operator--();
          return before;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr reference operator*() const noexcept
        {
          return static_cast<reference>( itr_start_
                                         + itr_cnt_ * static_cast<__details::types::Size>( itr_step_ ) );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr reference operator[](
          __details::types::Size inc ) const noexcept
        {
          return *( *this + inc );
        }

        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator+( const iterator& itr,
                                                                           value_type increment ) noexcept
        {
          return { itr.itr_start_,
                   itr.itr_step_,
                   itr.itr_cnt_ + static_cast<__details::types::Size>( increment / itr.itr_step_ ) };
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator+( value_type increment,
                                                                           const iterator& itr ) noexcept
        {
          return itr + increment;
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator-( const iterator& itr,
                                                                           value_type increment ) noexcept
        {
          return { itr.itr_start_,
                   itr.itr_step_,
                   itr.itr_cnt_ - static_cast<__details::types::Size>( increment / itr.itr_step_ ) };
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator-( value_type increment,
                                                                           const iterator& itr ) noexcept
        {
          return itr - increment;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr difference_type operator-(
          const iterator& a,
          const iterator& b ) noexcept
        {
          return static_cast<difference_type>( *a - *b );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator+=( iterator& itr,
                                                                             value_type increment ) noexcept
        {
          itr.itr_cnt_ += static_cast<__details::types::Size>( increment / itr.itr_step_ );
          return itr;
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator-=( iterator& itr,
                                                                             value_type increment ) noexcept
        {
          itr.itr_cnt_ -= static_cast<__details::types::Size>( increment / itr.itr_step_ );
          return itr;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& itr,
                                                                              value_type num ) noexcept
        {
          return *itr == num;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& itr,
                                                                              value_type num ) noexcept
        {
          return !( itr == num );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ == b.itr_cnt_;
        }
# if __PGBAR_CXX20
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr auto operator<=>( const iterator& a,
                                                                               const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ <=> b.itr_cnt_;
        }
# else
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( a == b );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator<( const iterator& a,
                                                                             const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ < b.itr_cnt_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator>( const iterator& a,
                                                                             const iterator& b ) noexcept
        {
          return b < a;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator<=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( b < a );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator>=( const iterator& a,
                                                                              const iterator& b ) noexcept
        {
          return !( a < b );
        }
# endif
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
      __PGBAR_CXX20_CNSTXPR ~NumericSpan()                                 = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const noexcept
      {
        return iterator( start_, step_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR iterator end() const noexcept
      {
        return iterator( start_, step_, size() );
      }

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N front() const noexcept { return start_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N back() const noexcept { return end_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N step() const noexcept { return step_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR __details::types::Size size() const noexcept
      {
        __PGBAR_TRUST( step_ != 0 );
        if __PGBAR_CXX17_CNSTXPR ( std::is_unsigned<N>::value )
          return ( ( end_ - start_ + step_ ) - 1 ) / step_;
        else if __PGBAR_CXX17_CNSTXPR ( std::is_integral<N>::value ) {
          if ( step_ > 0 )
            return ( ( end_ - start_ + step_ - 1 ) / step_ );
          else
            return ( ( start_ - end_ - step_ ) - 1 ) / ( -step_ );
        } else
          return static_cast<__details::types::Size>( std::ceil( ( end_ - start_ ) / step_ ) );
      }

      __PGBAR_NODISCARD constexpr bool empty() const noexcept { return size() == 0; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr typename iterator::reference operator[](
        __details::types::Size inc ) const noexcept
      {
        return *( begin() + inc );
      }
      __PGBAR_CXX14_CNSTXPR void swap( NumericSpan<N>& lhs ) noexcept
      {
        __PGBAR_TRUST( this != &lhs );
        using std::swap;
        swap( start_, lhs.start_ );
        swap( end_, lhs.end_ );
        swap( step_, lhs.step_ );
      }
      friend __PGBAR_CXX14_CNSTXPR void swap( NumericSpan<N>& a, NumericSpan<N>& b ) noexcept { a.swap( b ); }

      constexpr explicit operator bool() const noexcept { return !empty(); }
    };

    /**
     * An undirectional range delimited by a pair of iterators, including pointer types.
     *
     * Accepted iterator types must satisfy subtractable.
     */
    template<typename I>
    class IterSpan
# if __PGBAR_CXX20
      : public std::ranges::view_interface<IterSpan<I>>
# endif
    {
      static_assert( __details::traits::is_sized_iterator<I>::value,
                     "pgbar::slice::IterSpan: Only available for sized iterator types" );
      static_assert(
        std::is_convertible<typename std::iterator_traits<I>::difference_type, __details::types::Size>::value,
        "pgbar::slice::IterSpan: The 'difference_type' must be convertible to Size" );

      I start_, end_;
      __details::types::Size size_;

# if __PGBAR_CXX20
      using Reference_t = std::iter_reference_t<I>;
# else
      using Reference_t = typename std::iterator_traits<I>::reference;
# endif

    public:
      class iterator {
        I current_;

      public:
        using iterator_category = typename std::conditional<
          __details::traits::AnyOf<
            std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>,
            std::is_same<typename std::iterator_traits<I>::iterator_category,
                         std::output_iterator_tag>>::value,
          typename std::iterator_traits<I>::iterator_category,
          std::forward_iterator_tag>::type;
# if __PGBAR_CXX20
        using value_type      = std::iter_value_t<I>;
        using difference_type = std::iter_difference_t<I>;
        using reference       = std::iter_reference_t<I>;
# else
        using value_type      = typename std::iterator_traits<I>::value_type;
        using difference_type = typename std::iterator_traits<I>::difference_type;
        using reference       = typename std::iterator_traits<I>::reference;
# endif
        using pointer = I;

        constexpr iterator() = default;
        constexpr iterator( I startpoint ) noexcept( std::is_nothrow_move_constructible<I>::value )
          : current_ { std::move( startpoint ) }
        {}
        constexpr iterator( const iterator& )                          = default;
        constexpr iterator( iterator&& )                               = default;
        __PGBAR_CXX14_CNSTXPR iterator& operator=( const iterator& ) & = default;
        __PGBAR_CXX14_CNSTXPR iterator& operator=( iterator&& ) &      = default;
        __PGBAR_CXX20_CNSTXPR ~iterator()                              = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() &
        {
          ++current_;
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) &
        {
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR reference operator*() const
        {
          return *current_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR pointer operator->() const noexcept
        {
          return current_;
        }

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& itr,
                                                                              const I& ir )
        {
          return itr.current_ == ir;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const I& ir,
                                                                              const iterator& itr )
        {
          return itr == ir;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& itr,
                                                                              const I& ir )
        {
          return !( itr == ir );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const I& ir,
                                                                              const iterator& itr )
        {
          return !( itr == ir );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b )
        {
          return a.current_ == b.current_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b )
        {
          return !( a == b );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr difference_type operator-( const iterator& a,
                                                                                        const iterator& b )
        {
          return std::distance( a.current_, b.current_ );
        }

        constexpr explicit operator bool() const { return current_ != I(); }
      };

      __PGBAR_CXX17_CNSTXPR IterSpan( I startpoint, I endpoint )
        : start_ { std::move( startpoint ) }, end_ { std::move( endpoint ) }
      {
# if __PGBAR_CXX20
        const auto length = std::ranges::distance( start_, end_ );
# else
        const auto length = std::distance( start_, end_ );
# endif
        if ( length < 0 )
          throw exception::InvalidArgument( "pgbar: negative iterator range" );
        size_ = static_cast<__details::types::Size>( length );
      }
      __PGBAR_CXX17_CNSTXPR IterSpan( const IterSpan& )              = default;
      __PGBAR_CXX17_CNSTXPR IterSpan( IterSpan&& )                   = default;
      __PGBAR_CXX17_CNSTXPR IterSpan& operator=( const IterSpan& ) & = default;
      __PGBAR_CXX17_CNSTXPR IterSpan& operator=( IterSpan&& ) &      = default;
      // Intentional non-virtual destructors.
      __PGBAR_CXX20_CNSTXPR ~IterSpan()                              = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR Reference_t front() const noexcept
      {
        return *start_;
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR Reference_t back() const noexcept
      {
        return *std::next( start_, size_ - 1 );
      }

      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CNSTEVAL __details::types::Size step() const noexcept
      {
        return 1;
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr __details::types::Size size() const noexcept
      {
        return size_;
      }

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool empty() const noexcept { return size_ == 0; }

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const
        noexcept( __details::traits::AllOf<std::is_nothrow_move_constructible<I>,
                                           std::is_nothrow_copy_constructible<I>>::value )
      {
        return { this->start_ };
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator end() const
        noexcept( __details::traits::AllOf<std::is_nothrow_move_constructible<I>,
                                           std::is_nothrow_copy_constructible<I>>::value )
      {
        return { this->end_ };
      }

      __PGBAR_CXX20_CNSTXPR void swap( IterSpan<I>& lhs ) noexcept
      {
        __PGBAR_TRUST( this != &lhs );
        using std::swap;
        swap( start_, lhs.start_ );
        swap( end_, lhs.end_ );
        swap( size_, lhs.size_ );
      }
      friend __PGBAR_CXX20_CNSTXPR void swap( IterSpan<I>& a, IterSpan<I>& b ) noexcept { a.swap( b ); }

      __PGBAR_CXX17_CNSTXPR explicit operator bool() const noexcept { return !empty(); }
    };

    template<typename R>
    class BoundedSpan
# if __PGBAR_CXX20
      : public std::ranges::view_interface<BoundedSpan<R>>
# endif
    {
# if __PGBAR_CXX20
      static_assert( __details::traits::is_bounded_range<R>::value && !std::ranges::view<R>,
                     "pgbar::slice::BoundedSpan: Only available for bounded ranges, excluding view types" );
# else
      static_assert( __details::traits::is_bounded_range<R>::value,
                     "pgbar::slice::BoundedSpan: Only available for bounded ranges" );
# endif

      R* rnge_;

    public:
      using iterator = __details::traits::IteratorOf_t<R>;

    private:
# if __PGBAR_CXX20
      using Reference_t = std::iter_reference_t<iterator>;

      template<typename Rn>
      static __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR __details::types::Size size( Rn& rn )
      {
        return std::ranges::size( rn );
      }
# else
      using Reference_t = typename std::iterator_traits<iterator>::reference;

#  if __PGBAR_CXX17
      template<typename Rn>
      static __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR __details::types::Size size( Rn& rn )
      {
        return std::size( rn );
      }
#  else
      template<typename Rn>
      static __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR __details::types::Size size( Rn& rn )
      {
        return rn.size();
      }
      template<typename Rn, __details::types::Size N>
      static __PGBAR_INLINE_FN __PGBAR_CNSTEVAL __details::types::Size size( Rn ( & )[N] ) noexcept
      {
        return N;
      }
#  endif
# endif
    public:
      __PGBAR_CXX17_CNSTXPR BoundedSpan( R& rnge ) noexcept : rnge_ { std::addressof( rnge ) } {}

      __PGBAR_CXX17_CNSTXPR BoundedSpan( const BoundedSpan& )              = default;
      __PGBAR_CXX17_CNSTXPR BoundedSpan& operator=( const BoundedSpan& ) & = default;
      __PGBAR_CXX20_CNSTXPR ~BoundedSpan()                                 = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator begin() const
      {
# if __PGBAR_CXX20
        return std::ranges::begin( *rnge_ );
# else
        return std::begin( *rnge_ );
# endif
      }
# if __PGBAR_CXX17
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR auto end() const
# else
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator end() const
# endif
      {
# if __PGBAR_CXX20
        return std::ranges::end( *rnge_ );
# else
        return std::end( *rnge_ );
# endif
      }

      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR Reference_t front() const { return *begin(); }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR Reference_t back() const
      {
        return *std::next( begin(), size() - 1 );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CNSTEVAL __details::types::Size step() const noexcept
      {
        return 1;
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR __details::types::Size size() const
      {
        return size( *rnge_ );
      }

      __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR bool empty() const noexcept { return rnge_ == nullptr; }

      __PGBAR_CXX17_CNSTXPR void swap( BoundedSpan<R>& lhs ) noexcept
      {
        __PGBAR_TRUST( this != &lhs );
        std::swap( rnge_, lhs.rnge_ );
      }
      friend __PGBAR_CXX17_CNSTXPR void swap( BoundedSpan<R>& a, BoundedSpan<R>& b ) noexcept { a.swap( b ); }

      __PGBAR_CXX17_CNSTXPR explicit operator bool() const noexcept { return !empty(); }
    };

    template<typename R, typename B>
    class ProxySpan;
  } // namespace slice

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
    // The purpose of generating code with macros here is to annotate each type and method to provide more
    // friendly IDE access.
# define __PGBAR_BASE( ValueType ) \
 public                            \
   __details::wrappers::OptionWrapper<ValueType>
# define __PGBAR_OPTION( StructName, ValueType, ParamName )                    \
   constexpr StructName( ValueType ParamName ) noexcept                        \
     : __details::wrappers::OptionWrapper<ValueType>( std::move( ParamName ) ) \
   {}
# define __PGBAR_NULLABLE_OPTION( StructName, ValueType, ParamName ) \
   constexpr StructName() = default;                                 \
   __PGBAR_OPTION( StructName, ValueType, ParamName )

    // A wrapper that stores the value of the bit option setting.
    struct Style : __PGBAR_BASE( __details::types::Byte ) {
      __PGBAR_NULLABLE_OPTION( Style, __details::types::Byte, _settings )
    };

    // A wrapper that stores the value of the color effect setting.
    struct Colored : __PGBAR_BASE( bool ) {
      __PGBAR_NULLABLE_OPTION( Colored, bool, _enable )
    };

    // A wrapper that stores the value of the font boldness setting.
    struct Bolded : __PGBAR_BASE( bool ) {
      __PGBAR_NULLABLE_OPTION( Bolded, bool, _enable )
    };

    // A wrapper that stores the number of tasks.
    struct Tasks : __PGBAR_BASE( std::uint64_t ) {
      __PGBAR_OPTION( Tasks, std::uint64_t, _num_tasks )
    };

    // A wrapper that stores the flag of direction.
    struct Reversed : __PGBAR_BASE( bool ) {
      __PGBAR_NULLABLE_OPTION( Reversed, bool, _flag )
    };

    // A wrapper that stores the length of the bar indicator, in the character unit.
    struct BarLength : __PGBAR_BASE( __details::types::Size ) {
      __PGBAR_OPTION( BarLength, __details::types::Size, _num_char )
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
    struct Shift : __PGBAR_BASE( std::int8_t ) {
      __PGBAR_OPTION( Shift, std::int8_t, _shift_factor )
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
    struct Magnitude : __PGBAR_BASE( std::uint16_t ) {
      __PGBAR_OPTION( Magnitude, std::uint16_t, _magnitude )
    };

# undef __PGBAR_NULLABLE_OPTION
# undef __PGBAR_OPTION
# if __PGBAR_CXX20
#  define __PGBAR_OPTION( StructName, ParamName )                          \
  private:                                                                 \
    using Data = __details::charcodes::U8Raw;                              \
    using Base = __details::wrappers::OptionWrapper<Data>;                 \
                                                                           \
  public:                                                                  \
    __PGBAR_CXX20_CNSTXPR StructName() = default;                          \
    /**                                                                    \
     * @throw exception::InvalidArgument                                   \
     *                                                                     \
     * If the passed parameter is not coding in UTF-8.                     \
     */                                                                    \
    __PGBAR_CXX20_CNSTXPR StructName( __details::types::String ParamName ) \
      : Base( Data( std::move( ParamName ) ) )                             \
    {}                                                                     \
    StructName( __details::types::LitU8 ParamName ) : Base( Data( std::move( ParamName ) ) ) {}
# else
#  define __PGBAR_OPTION( StructName, ParamName )                          \
    __PGBAR_CXX20_CNSTXPR StructName() = default;                          \
    __PGBAR_CXX20_CNSTXPR StructName( __details::types::String ParamName ) \
      : __details::wrappers::OptionWrapper<__details::charcodes::U8Raw>(   \
          __details::charcodes::U8Raw( std::move( ParamName ) ) )          \
    {}
# endif

    // A wrapper that stores the characters of the filler in the bar indicator.
    struct Filler : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Filler, _filler )
    };

    // A wrapper that stores the characters of the remains in the bar indicator.
    struct Remains : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Remains, _remains )
    };

    // A wrapper that stores characters located to the left of the bar indicator.
    struct Starting : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Starting, _starting )
    };

    // A wrapper that stores characters located to the right of the bar indicator.
    struct Ending : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Ending, _ending )
    };

    // A wrapper that stores the prefix text.
    struct Prefix : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Prefix, _prefix )
    };

    // A wrapper that stores the postfix text.
    struct Postfix : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Postfix, _postfix )
    };

    // A wrapper that stores the separator component used to separate different infomation.
    struct Divider : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( Divider, _divider )
    };

    // A wrapper that stores the border component located to the left of the whole indicator.
    struct LeftBorder : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( LeftBorder, _l_border )
    };

    // A wrapper that stores the border component located to the right of the whole indicator.
    struct RightBorder : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_OPTION( RightBorder, _r_border )
    };

# undef __PGBAR_OPTION
# define __PGBAR_OPTION( StructName, ParamName )                                                        \
 private:                                                                                               \
   using Data = __details::console::escodes::RGBColor;                                                  \
   using Base = __details::wrappers::OptionWrapper<Data>;                                               \
                                                                                                        \
 public:                                                                                                \
   __PGBAR_CXX23_CNSTXPR StructName() = default;                                                        \
   __PGBAR_CXX23_CNSTXPR StructName( __details::types::ROStr ParamName ) : Base( Data( ParamName ) ) {} \
   __PGBAR_CXX23_CNSTXPR StructName( __details::types::HexRGB ParamName ) : Base( Data( ParamName ) ) {}

    // A wrapper that stores the prefix text color.
    struct PrefixColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( PrefixColor, _prfx_color )
    };

    // A wrapper that stores the postfix text color.
    struct PostfixColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( PostfixColor, _pstfx_color )
    };

    // A wrapper that stores the color of component located to the left of the bar indicator.
    struct StartColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( StartColor, _start_color )
    };

    // A wrapper that stores the color of component located to the right of the bar indicator.
    struct EndColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( EndColor, _end_color )
    };

    // A wrapper that stores the color of the filler in the bar indicator.
    struct FillerColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( FillerColor, _filler_color )
    };

    // A wrapper that stores the color of the remains in the bar indicator.
    struct RemainsColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( RemainsColor, _remains_color )
    };

    // A wrapper that stores the color of the lead in the bar indicator.
    struct LeadColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( LeadColor, _lead_color )
    };

    // A wrapper that stores the color of the whole infomation indicator.
    struct InfoColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_OPTION( InfoColor, _info_color )
    };

# undef __PGBAR_OPTION

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
    struct SpeedUnit : __PGBAR_BASE( __PGBAR_PACK( std::array<__details::charcodes::U8Raw, 4> ) ) {
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       *
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<__details::types::String, 4> _units )
      {
        std::transform(
          std::make_move_iterator( _units.begin() ),
          std::make_move_iterator( _units.end() ),
          data_.begin(),
          []( __details::types::String&& ele ) { return __details::charcodes::U8Raw( std::move( ele ) ); } );
      }
# if __PGBAR_CXX20
      /**
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<__details::types::LitU8, 4> _units )
      {
        std::transform(
          _units.cbegin(),
          _units.cend(),
          data_.begin(),
          []( const __details::types::LitU8& ele ) { return __details::charcodes::U8Raw( ele ); } );
      }
# endif
    };

    // A wrapper that stores the `lead` animated element.
    struct Lead : __PGBAR_BASE( std::vector<__details::charcodes::U8Text> ) {
    private:
      using Base = __details::wrappers::OptionWrapper<std::vector<__details::charcodes::U8Text>>;

    public:
      __PGBAR_CXX20_CNSTXPR Lead() = default;
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( std::vector<__details::types::String> _leads )
      {
        std::transform(
          std::make_move_iterator( _leads.begin() ),
          std::make_move_iterator( _leads.end() ),
          std::back_inserter( data_ ),
          []( __details::types::String&& ele ) { return __details::charcodes::U8Text( std::move( ele ) ); } );
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( __details::types::String _lead )
        : Base( { __details::charcodes::U8Text( std::move( _lead ) ) } )
      {}
# if __PGBAR_CXX20
      __PGBAR_CXX20_CNSTXPR Lead( const std::vector<__details::types::LitU8>& _leads )
      {
        std::transform(
          _leads.cbegin(),
          _leads.cend(),
          std::back_inserter( data_ ),
          []( const __details::types::LitU8& ele ) { return __details::charcodes::U8Text( ele ); } );
      }
      Lead( const __details::types::LitU8& _lead ) : Base( { __details::charcodes::U8Text( _lead ) } ) {}
# endif
    };

# undef __PGBAR_OPTION
# undef __PGBAR_BASE
  } // namespace option

  namespace __details {
    // The Basic components of the progress bar.
    namespace assets {
# define __PGBAR_NONEMPTY_COMPONENT( ClassName, Constexpr )       \
   Constexpr ClassName( const ClassName& )             = default; \
   Constexpr ClassName( ClassName&& )                  = default; \
   Constexpr ClassName& operator=( const ClassName& )& = default; \
   Constexpr ClassName& operator=( ClassName&& )&      = default; \
   __PGBAR_CXX20_CNSTXPR ~ClassName()                  = default;

# define __PGBAR_EMPTY_COMPONENT( ClassName )                                 \
   constexpr ClassName()                                           = default; \
   constexpr ClassName( const ClassName& )                         = default; \
   constexpr ClassName( ClassName&& )                              = default; \
   __PGBAR_CXX14_CNSTXPR ClassName& operator=( const ClassName& )& = default; \
   __PGBAR_CXX14_CNSTXPR ClassName& operator=( ClassName&& )&      = default; \
   __PGBAR_CXX20_CNSTXPR ~ClassName()                              = default;

      template<typename Derived>
      class CoreConfig {
# define __PGBAR_UNPAKING( OptionName, MemberName )                                                  \
   friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unpacker( CoreConfig& cfg,                    \
                                                                 option::OptionName&& val ) noexcept \
   {                                                                                                 \
     cfg.fonts_[utils::as_val( Mask::OptionName )] = val.value();                                    \
   }
        __PGBAR_UNPAKING( Colored, colored_ )
        __PGBAR_UNPAKING( Bolded, bolded_ )
# undef __PGBAR_UNPAKING

      protected:
        mutable concurrent::SharedMutex rw_mtx_;
        enum class Mask : std::uint8_t { Colored = 0, Bolded };
        std::bitset<2> fonts_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& try_dye(
          io::Stringbuf& buffer,
          const console::escodes::RGBColor& rgb ) const
        {
          if ( fonts_[utils::as_val( Mask::Colored )] )
            buffer << rgb;
          return buffer;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& try_style(
          io::Stringbuf& buffer,
          const console::escodes::RGBColor& rgb ) const
        {
          try_dye( buffer, rgb );
          if ( fonts_[utils::as_val( Mask::Bolded )] )
            buffer << console::escodes::fontbold;
          return buffer;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& try_reset( io::Stringbuf& buffer ) const
        {
          if ( fonts_.any() )
            buffer << console::escodes::fontreset;
          return buffer;
        }

      public:
        CoreConfig() noexcept : fonts_ { static_cast<types::Byte>( ~0 ) } {}
        constexpr CoreConfig( const CoreConfig& lhs ) : fonts_ { lhs.fonts_ } {}
        constexpr CoreConfig( CoreConfig&& rhs ) noexcept : fonts_ { rhs.fonts_ } {}
        __PGBAR_CXX14_CNSTXPR CoreConfig& operator=( const CoreConfig& lhs ) &
        {
          fonts_ = lhs.fonts_;
          return *this;
        }
        __PGBAR_CXX14_CNSTXPR CoreConfig& operator=( CoreConfig&& rhs ) & noexcept
        {
          fonts_ = rhs.fonts_;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR ~CoreConfig() = default;

# define __PGBAR_METHOD( OptionName, ParamName )              \
                                                              \
   std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ }; \
   unpacker( *this, option::OptionName( ParamName ) );        \
   return static_cast<Derived&>( *this )

        // Enable or disable the color effect.
        Derived& colored( bool _enable ) & noexcept { __PGBAR_METHOD( Colored, _enable ); }
        // Enable or disable the bold effect.
        Derived& bolded( bool _enable ) & noexcept { __PGBAR_METHOD( Bolded, _enable ); }

# undef __PGBAR_METHOD
# define __PGBAR_METHOD( Offset )                                    \
   concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ }; \
   return fonts_[utils::as_val( Mask::Offset )]

        // Check whether the color effect is enabled.
        __PGBAR_NODISCARD bool colored() const noexcept { __PGBAR_METHOD( Colored ); }
        // Check whether the bold effect is enabled.
        __PGBAR_NODISCARD bool bolded() const noexcept { __PGBAR_METHOD( Bolded ); }

# undef __PGBAR_METHOD

        __PGBAR_CXX14_CNSTXPR void swap( CoreConfig& lhs ) noexcept { std::swap( fonts_, lhs.fonts_ ); }
      };

      template<typename Base, typename Derived>
      class Countable : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( Countable& cfg,
                                                                      option::Tasks&& val ) noexcept
        {
          cfg.task_range_ = slice::NumericSpan<std::uint64_t>( val.value() );
        }

      protected:
        slice::NumericSpan<std::uint64_t> task_range_;

      public:
        constexpr Countable() = default;
        __PGBAR_NONEMPTY_COMPONENT( Countable, __PGBAR_CXX14_CNSTXPR )

        // Set the number of tasks, passing in zero is no exception.
        Derived& tasks( std::uint64_t param ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Tasks( param ) );
          return static_cast<Derived&>( *this );
        }
        // Get the current number of tasks.
        __PGBAR_NODISCARD std::uint64_t tasks() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return task_range_.back();
        }

        __PGBAR_CXX14_CNSTXPR void swap( Countable& lhs ) noexcept
        {
          task_range_.swap( lhs.task_range_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Reversible : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( Reversible& cfg,
                                                                      option::Reversed&& val ) noexcept
        {
          cfg.reversed_ = val.value();
        }

      protected:
        bool reversed_;

      public:
        constexpr Reversible() = default;
        __PGBAR_NONEMPTY_COMPONENT( Reversible, __PGBAR_CXX14_CNSTXPR )

        Derived& reverse( bool flag ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Reversed( flag ) );
          return static_cast<Derived&>( *this );
        }

        __PGBAR_NODISCARD bool reverse() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return reversed_;
        }

        __PGBAR_CXX20_CNSTXPR void swap( Reversible& lhs ) noexcept
        {
          std::swap( reversed_, lhs.reversed_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Frames : public Base {
        friend __PGBAR_INLINE_FN void unpacker( Frames& cfg, option::LeadColor&& val ) noexcept
        {
          cfg.lead_col_ = val.value();
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( Frames& cfg,
                                                                      option::Lead&& val ) noexcept
        {
          if ( std::all_of( val.value().cbegin(),
                            val.value().cend(),
                            []( const charcodes::U8Raw& ele ) noexcept { return ele.empty(); } ) ) {
            cfg.lead_.clear();
            cfg.len_longest_lead_ = 0;
          } else {
            cfg.lead_ = std::move( val.value() );
            cfg.len_longest_lead_ =
              std::max_element( cfg.lead_.cbegin(),
                                cfg.lead_.cend(),
                                []( const charcodes::U8Raw& a, const charcodes::U8Raw& b ) noexcept {
                                  return a.width() < b.width();
                                } )
                ->width();
          }
        }

      protected:
        std::vector<charcodes::U8Text> lead_;
        console::escodes::RGBColor lead_col_;
        types::Size len_longest_lead_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_frames()
          const noexcept
        {
          return len_longest_lead_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR Frames() = default;
        __PGBAR_NONEMPTY_COMPONENT( Frames, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

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
        Derived& lead( const std::vector<types::LitU8>& _leads ) & { __PGBAR_METHOD( Lead, _leads, ); }
        Derived& lead( types::LitU8 _lead ) & { __PGBAR_METHOD( Lead, _lead, ); }
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

        __PGBAR_CXX20_CNSTXPR void swap( Frames& lhs ) noexcept
        {
          using std::swap;
          lead_col_.swap( lhs.lead_col_ );
          lead_.swap( lhs.lead_ );
          swap( len_longest_lead_, lhs.len_longest_lead_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Filler : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                                        \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Filler& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                  \
     cfg.MemberName = std::move( val.value() );                                                       \
   }
        __PGBAR_UNPAKING( Filler, filler_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( FillerColor, filler_col_, )
# undef __PGBAR_UNPAKING

      protected:
        charcodes::U8Raw filler_;
        console::escodes::RGBColor filler_col_;

      public:
        __PGBAR_CXX20_CNSTXPR Filler() = default;
        __PGBAR_NONEMPTY_COMPONENT( Filler, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& filler( types::String _filler ) & { __PGBAR_METHOD( Filler, _filler, std::move ); }
# if __PGBAR_CXX20
        Derived& filler( types::LitU8 _filler ) & { __PGBAR_METHOD( Filler, _filler, ); }
# endif
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

# undef __PGBAR_METHOD
        __PGBAR_CXX20_CNSTXPR void swap( Filler& lhs ) noexcept
        {
          filler_.swap( lhs.filler_ );
          filler_col_.swap( lhs.filler_col_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Remains : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                                         \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Remains& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                   \
     cfg.MemberName = std::move( val.value() );                                                        \
   }
        __PGBAR_UNPAKING( Remains, remains_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( RemainsColor, remains_col_, )
# undef __PGBAR_UNPAKING

      protected:
        charcodes::U8Raw remains_;
        console::escodes::RGBColor remains_col_;

      public:
        __PGBAR_CXX20_CNSTXPR Remains() = default;
        __PGBAR_NONEMPTY_COMPONENT( Remains, __PGBAR_CXX20_CNSTXPR )

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
# if __PGBAR_CXX20
        Derived& remains( types::LitU8 _remains ) & { __PGBAR_METHOD( Remains, _remains, ); }
# endif
# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( Remains& lhs ) noexcept
        {
          remains_col_.swap( lhs.remains_col_ );
          remains_.swap( lhs.remains_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class BasicAnimation : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unpacker( BasicAnimation& cfg,
                                                                      option::Shift&& val ) noexcept
        {
          cfg.shift_factor_ = val.value() < 0 ? ( 1.0 / ( -val.value() ) ) : val.value();
        }

      protected:
        types::Float shift_factor_;

      public:
        __PGBAR_CXX20_CNSTXPR BasicAnimation() = default;
        __PGBAR_NONEMPTY_COMPONENT( BasicAnimation, __PGBAR_CXX20_CNSTXPR )

        /**
         * Set the rate factor of the animation with negative value slowing down the switch per frame
         * and positive value speeding it up.
         *
         * The maximum and minimum of the rate factor is between -128 and 127.
         *
         * If the value is zero, freeze the animation.
         */
        Derived& shift( std::int8_t _shift_factor ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Shift( _shift_factor ) );
          return static_cast<Derived&>( *this );
        }

        __PGBAR_CXX20_CNSTXPR void swap( BasicAnimation& lhs ) noexcept
        {
          using std::swap;
          swap( shift_factor_, lhs.shift_factor_ );
          Base::swap( lhs );
        }
      };

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
# undef __PGBAR_UNPAKING

      protected:
        types::Size bar_length_;
        charcodes::U8Raw starting_, ending_;
        console::escodes::RGBColor start_col_, end_col_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_bar() const noexcept
        {
          return starting_.width() + ending_.width();
        }

      public:
        __PGBAR_CXX20_CNSTXPR BasicIndicator() = default;
        __PGBAR_NONEMPTY_COMPONENT( BasicIndicator, __PGBAR_CXX20_CNSTXPR )

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
        Derived& starting( types::LitU8 _starting ) & { __PGBAR_METHOD( Starting, _starting, ); }
        Derived& ending( types::LitU8 _ending ) & { __PGBAR_METHOD( Ending, _ending, ); }
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
        // Set the length of the bar indicator.
        Derived& bar_length( types::Size _length ) & noexcept { __PGBAR_METHOD( BarLength, _length, ); }

        __PGBAR_NODISCARD types::Size bar_length() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
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
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class CharIndic : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_char( io::Stringbuf& buffer,
                                                                           types::Float num_percent,
                                                                           types::Size num_frame_cnt ) const
        {
          __PGBAR_TRUST( num_percent >= 0.0 );
          __PGBAR_TRUST( num_percent <= 1.0 );
          if ( this->bar_length_ == 0 )
            return buffer;

          const auto len_finished = static_cast<types::Size>( std::round( this->bar_length_ * num_percent ) );
          types::Size len_vacancy = this->bar_length_ - len_finished;

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->reversed_ ) {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( constants::blank, len_finished % this->filler_.width() );

            if ( !this->lead_.empty() ) {
              num_frame_cnt =
                static_cast<types::Size>( num_frame_cnt * this->shift_factor_ ) % this->lead_.size();
              const auto& current_lead = this->lead_[num_frame_cnt];
              if ( current_lead.width() <= len_vacancy ) {
                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ ).append( current_lead );
                len_vacancy -= current_lead.width();
              }
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ );
            buffer.append( constants::blank, len_vacancy % this->remains_.width() )
              .append( this->remains_, len_vacancy / this->remains_.width() );
          } else {
            const auto flag = [this, num_frame_cnt, &len_vacancy]() noexcept {
              if ( !this->lead_.empty() ) {
                const auto offset =
                  static_cast<types::Size>( num_frame_cnt * this->shift_factor_ ) % this->lead_.size();
                if ( this->lead_[offset].width() <= len_vacancy ) {
                  len_vacancy -= this->lead_[offset].width();
                  return true;
                }
              }
              return false;
            }();

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ );
            buffer.append( constants::blank, len_vacancy % this->remains_.width() )
              .append( this->remains_, len_vacancy / this->remains_.width() );

            if ( flag ) {
              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( this->lead_[num_frame_cnt] );
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( constants::blank, len_finished % this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( CharIndic )
      };

      template<typename Base, typename Derived>
      class BlockIndic : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_block( io::Stringbuf& buffer,
                                                                            types::Float num_percent ) const
        {
          __PGBAR_TRUST( num_percent >= 0.0 );
          __PGBAR_TRUST( num_percent <= 1.0 );
          if ( this->bar_length_ == 0 )
            return buffer;

          const auto len_finished     = static_cast<types::Size>( this->bar_length_ * num_percent );
          const types::Float fraction = ( this->bar_length_ * num_percent ) - len_finished;
          __PGBAR_TRUST( fraction >= 0.0 );
          __PGBAR_TRUST( fraction <= 1.0 );
          const auto incomplete_block = static_cast<types::Size>( fraction * this->lead_.size() );
          __PGBAR_ASSERT( incomplete_block <= this->lead_.size() );
          types::Size len_vacancy = this->bar_length_ - len_finished;

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->reversed_ ) {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( constants::blank, len_finished % this->filler_.width() );

            if ( this->bar_length_ != len_finished && !this->lead_.empty()
                 && this->lead_[incomplete_block].width() <= len_vacancy ) {
              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( this->lead_[incomplete_block] );
              len_vacancy -= this->lead_[incomplete_block].width();
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ )
              .append( constants::blank, len_vacancy % this->remains_.width() )
              .append( this->remains_, len_vacancy / this->remains_.width() );
          } else {
            const auto flag = this->bar_length_ != len_finished && !this->lead_.empty()
                           && this->lead_[incomplete_block].width() <= len_vacancy;
            if ( flag )
              len_vacancy -= this->lead_[incomplete_block].width();

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ )
              .append( this->remains_, len_vacancy / this->remains_.width() )
              .append( constants::blank, len_vacancy % this->remains_.width() );

            if ( flag ) {
              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( this->lead_[incomplete_block] );
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( constants::blank, len_finished % this->filler_.width() )
              .append( this->filler_, len_finished / this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( BlockIndic )
      };

      template<typename Base, typename Derived>
      class SpinIndic : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_spin( io::Stringbuf& buffer,
                                                                           types::Size num_frame_cnt ) const
        {
          if ( this->lead_.empty() )
            return buffer;
          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
          num_frame_cnt %= this->lead_.size();
          __PGBAR_ASSERT( this->len_longest_lead_ >= this->lead_[num_frame_cnt].width() );

          this->try_reset( buffer );
          return this->try_style( buffer, this->lead_col_ )
              << utils::format<utils::TxtLayout::Left>( this->len_longest_lead_, this->lead_[num_frame_cnt] );
        }

      public:
        __PGBAR_EMPTY_COMPONENT( SpinIndic )
      };

      template<typename Base, typename Derived>
      class SweepIndic : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_sweep( io::Stringbuf& buffer,
                                                                            types::Size num_frame_cnt ) const
        {
          if ( this->bar_length_ == 0 )
            return buffer;

          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->lead_.empty() ) {
            const auto& current_lead = this->lead_[num_frame_cnt % this->lead_.size()];
            if ( current_lead.width() <= this->bar_length_ ) {
              // virtual_point is a value between 1 and this->bar_length
              const auto virtual_point = [this, num_frame_cnt]() noexcept -> types::Size {
                if ( this->bar_length_ == 1 )
                  return 1;
                const auto period = 2 * this->bar_length_ - 2;
                const auto pos    = num_frame_cnt % period;
                return pos < this->bar_length_ ? pos + 1 : 2 * this->bar_length_ - pos - 1;
              }();
              const auto len_left_fill = [this, virtual_point, &current_lead]() noexcept -> types::Size {
                const auto len_half_lead = ( current_lead.width() / 2 ) + current_lead.width() % 2;
                if ( virtual_point <= len_half_lead )
                  return 0;
                const auto len_unreached = this->bar_length_ - virtual_point;
                if ( len_unreached <= len_half_lead - current_lead.width() % 2 )
                  return this->bar_length_ - current_lead.width();

                return virtual_point - len_half_lead;
              }();
              const auto len_right_fill = this->bar_length_ - ( len_left_fill + current_lead.width() );
              __PGBAR_ASSERT( len_left_fill + len_right_fill + current_lead.width() == this->bar_length_ );

              this->try_reset( buffer );
              this->try_dye( buffer, this->filler_col_ )
                .append( this->filler_, len_left_fill / this->filler_.width() )
                .append( constants::blank, len_left_fill % this->filler_.width() );

              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( current_lead );

              this->try_reset( buffer );
              this->try_dye( buffer, this->filler_col_ )
                .append( constants::blank, len_right_fill % this->filler_.width() )
                .append( this->filler_, len_right_fill / this->filler_.width() );
            } else
              buffer.append( constants::blank, this->bar_length_ );
          } else if ( this->filler_.empty() )
            buffer.append( constants::blank, this->bar_length_ );
          else {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, this->bar_length_ / this->filler_.width() )
              .append( constants::blank, this->bar_length_ % this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( SweepIndic )
      };

      template<typename Base, typename Derived>
      class FlowIndic : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_flow( io::Stringbuf& buffer,
                                                                           types::Size num_frame_cnt ) const
        {
          if ( this->bar_length_ == 0 )
            return buffer;

          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;
          this->try_reset( buffer );

          if ( !this->lead_.empty() ) {
            const auto& current_lead = this->lead_[num_frame_cnt % this->lead_.size()];
            if ( current_lead.width() <= this->bar_length_ ) {
              // virtual_point is a value between 0 and this->bar_length - 1
              const auto virtual_point = [this, num_frame_cnt]() noexcept {
                const auto pos = num_frame_cnt % this->bar_length_;
                return !this->reversed_ ? pos : ( this->bar_length_ - 1 - pos ) % this->bar_length_;
              }();
              const auto len_vacancy = this->bar_length_ - virtual_point;

              if ( current_lead.width() <= len_vacancy ) {
                const auto len_right_fill = len_vacancy - current_lead.width();

                this->try_dye( buffer, this->filler_col_ )
                  .append( this->filler_, virtual_point / this->filler_.width() )
                  .append( constants::blank, virtual_point % this->filler_.width() );
                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ ).append( current_lead );

                this->try_reset( buffer );
                this->try_dye( buffer, this->filler_col_ )
                  .append( constants::blank, len_right_fill % this->filler_.width() )
                  .append( this->filler_, len_right_fill / this->filler_.width() );
              } else {
                const auto division      = current_lead.split_by( len_vacancy );
                const auto len_left_fill = virtual_point - division.second.second;

                this->try_dye( buffer, this->lead_col_ ).append( division.first[1], division.first[2] );
                this->try_reset( buffer );
                this->try_dye( buffer, this->filler_col_ )
                  .append( constants::blank, len_left_fill % this->filler_.width() )
                  .append( this->filler_, len_left_fill / this->filler_.width() );

                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ )
                  .append( division.first[0], division.first[1] )
                  .append( constants::blank, len_vacancy - division.second.first );
              }
            } else
              buffer.append( constants::blank, this->bar_length_ );
          } else if ( this->filler_.empty() )
            buffer.append( constants::blank, this->bar_length_ );
          else
            buffer.append( this->filler_, this->bar_length_ / this->filler_.width() )
              .append( constants::blank, this->bar_length_ % this->filler_.width() );

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( FlowIndic )
      };

      template<typename Base, typename Derived>
      class Prefix : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                                        \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Prefix& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                  \
     cfg.MemberName = std::move( val.value() );                                                       \
   }
        __PGBAR_UNPAKING( Prefix, prefix_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( PrefixColor, prfx_col_, )
# undef __PGBAR_UNPAKING

      protected:
        charcodes::U8Raw prefix_;
        console::escodes::RGBColor prfx_col_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_prefix( io::Stringbuf& buffer ) const
        {
          if ( prefix_.empty() )
            return buffer;
          this->try_reset( buffer );
          return this->try_style( buffer, prfx_col_ ) << prefix_;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_prefix()
          const noexcept
        {
          return prefix_.width();
        }

      public:
        __PGBAR_CXX20_CNSTXPR Prefix() = default;
        __PGBAR_NONEMPTY_COMPONENT( Prefix, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& prefix( types::String _prefix ) & { __PGBAR_METHOD( Prefix, _prefix, std::move ); }

# if __PGBAR_CXX20
        Derived& prefix( types::LitU8 _prefix ) & { __PGBAR_METHOD( Prefix, _prefix, ); }
# endif

        Derived& prefix_color( types::HexRGB _prfx_color ) & { __PGBAR_METHOD( PrefixColor, _prfx_color, ); }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& prefix_color( types::ROStr _prfx_color ) & { __PGBAR_METHOD( PrefixColor, _prfx_color, ); }

# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( Prefix& lhs ) noexcept
        {
          prfx_col_.swap( lhs.prfx_col_ );
          prefix_.swap( lhs.prefix_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Postfix : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Constexpr )                                         \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Postfix& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                   \
     cfg.MemberName = std::move( val.value() );                                                        \
   }
        __PGBAR_UNPAKING( Postfix, postfix_, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( PostfixColor, pstfx_col_, )
# undef __PGBAR_UNPAKING

      protected:
        charcodes::U8Raw postfix_;
        console::escodes::RGBColor pstfx_col_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_postfix( io::Stringbuf& buffer ) const
        {
          if ( postfix_.empty() )
            return buffer;
          this->try_reset( buffer );
          return this->try_style( buffer, pstfx_col_ ) << postfix_;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_postfix()
          const noexcept
        {
          return postfix_.width();
        }

      public:
        __PGBAR_CXX20_CNSTXPR Postfix() = default;
        __PGBAR_NONEMPTY_COMPONENT( Postfix, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )         \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
   return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& postfix( types::String _postfix ) & { __PGBAR_METHOD( Postfix, _postfix, std::move ); }
# if __PGBAR_CXX20
        Derived& postfix( types::LitU8 _postfix ) & { __PGBAR_METHOD( Postfix, _postfix, ); }
# endif

        Derived& postfix_color( types::HexRGB _pstfx_color ) &
        {
          __PGBAR_METHOD( PostfixColor, _pstfx_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& postfix_color( types::ROStr _pstfx_color ) &
        {
          __PGBAR_METHOD( PostfixColor, _pstfx_color, );
        }

# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( Postfix& lhs ) noexcept
        {
          pstfx_col_.swap( lhs.pstfx_col_ );
          postfix_.swap( lhs.postfix_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Segment : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Operation, Constexpr )                              \
   friend __PGBAR_INLINE_FN Constexpr void unpacker( Segment& cfg, option::OptionName&& val ) noexcept \
   {                                                                                                   \
     cfg.MemberName = Operation( val.value() );                                                        \
   }
        __PGBAR_UNPAKING( InfoColor, info_col_, std::move, )
        __PGBAR_UNPAKING( Divider, divider_, std::move, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( LeftBorder, l_border_, std::move, __PGBAR_CXX20_CNSTXPR )
        __PGBAR_UNPAKING( RightBorder, r_border_, std::move, __PGBAR_CXX20_CNSTXPR )
# undef __PGBAR_UNPAKING

      protected:
        charcodes::U8Raw divider_;
        charcodes::U8Raw l_border_, r_border_;
        console::escodes::RGBColor info_col_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_segment(
          types::Size num_column ) const noexcept
        {
          switch ( num_column ) {
          case 0:  return 0;
          case 1:  return l_border_.width() + r_border_.width();
          default: return ( num_column - 1 ) * divider_.width() + l_border_.width() + r_border_.width();
          }
        }

      public:
        __PGBAR_CXX20_CNSTXPR Segment() = default;
        __PGBAR_NONEMPTY_COMPONENT( Segment, __PGBAR_CXX20_CNSTXPR )

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
        Derived& divider( types::LitU8 _divider ) & { __PGBAR_METHOD( Divider, _divider, ); }
        Derived& left_border( types::LitU8 _l_border ) & { __PGBAR_METHOD( LeftBorder, _l_border, ); }
        Derived& right_border( types::LitU8 _r_border ) & { __PGBAR_METHOD( RightBorder, _r_border, ); }
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
      class PercentMeter : public Base {
# define __PGBAR_DEFAULT_PERCENT u8" --.--%"
        static constexpr types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_PERCENT ) - 1;

      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_percent( types::Float num_percent ) const
        {
          __PGBAR_TRUST( num_percent >= 0.0 );
          __PGBAR_TRUST( num_percent <= 1.0 );

          if ( num_percent <= 0.0 )
# if __PGBAR_CXX20
            __PGBAR_UNLIKELY
            {
              constexpr types::LitU8 tmp = __PGBAR_DEFAULT_PERCENT;
              auto ret                   = types::String( tmp.size(), '\0' );
              std::copy( tmp.cbegin(), tmp.cend(), ret.begin() );
              return ret;
            }
# else
            __PGBAR_UNLIKELY return { __PGBAR_DEFAULT_PERCENT, _fixed_length };
# endif

          auto str = utils::format( num_percent * 100.0, 2 );
          str.push_back( '%' );

          return utils::format<utils::TxtLayout::Right>( _fixed_length, str );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_percent() const noexcept
        {
          return _fixed_length;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( PercentMeter )
      };
# undef __PGBAR_DEFAULT_PERCENT

      template<typename Base, typename Derived>
      class SpeedMeter : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( SpeedMeter& cfg,
                                                                      option::SpeedUnit&& val ) noexcept
        {
          cfg.units_ = std::move( val.value() );
          cfg.len_longest_unit_ =
            ( std::max )( ( std::max )( cfg.units_[0].width(), cfg.units_[1].width() ),
                          ( std::max )( cfg.units_[2].width(), cfg.units_[3].width() ) );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( SpeedMeter& cfg,
                                                                      option::Magnitude&& val ) noexcept
        {
          cfg.magnitude_ = val.value();
        }

# define __PGBAR_DEFAULT_SPEED u8"   inf "
        static constexpr types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_SPEED ) - 1;

      protected:
        std::array<charcodes::U8Raw, 4> units_;
        types::Size len_longest_unit_;
        std::uint16_t magnitude_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_speed( const types::TimeUnit& time_passed,
                                                                       std::uint64_t num_task_done,
                                                                       std::uint64_t num_all_tasks ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          if ( num_all_tasks == 0 )
            __PGBAR_UNLIKELY return utils::format<utils::TxtLayout::Right>( _fixed_length + len_longest_unit_,
                                                                            u8"-- " + units_.front() );

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

          return utils::format<utils::TxtLayout::Right>( _fixed_length + len_longest_unit_, rate_str );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_speed() const noexcept
        {
          return _fixed_length + len_longest_unit_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR SpeedMeter() = default;
        __PGBAR_NONEMPTY_COMPONENT( SpeedMeter, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName )                    \
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
        Derived& speed_unit( std::array<types::String, 4> _units ) & { __PGBAR_METHOD( SpeedUnit, _units ); }
# if __PGBAR_CXX20
        /**
         * @param _units
         * The given each unit will be treated as 1,000 times greater than the previous one
         * (from left to right).
         */
        Derived& speed_unit( std::array<types::LitU8, 4> _units ) & { __PGBAR_METHOD( SpeedUnit, _units ); }
# endif

        /**
         * @param _magnitude
         * The base magnitude for unit scaling in formatted output.
         *
         * Defines the threshold at which values are converted to higher-order units
         * (e.g. 1000 -> "1k", 1000000 -> "1M").
         */
        Derived& magnitude( std::uint16_t _magnitude ) & noexcept { __PGBAR_METHOD( Magnitude, _magnitude ); }

# undef __PGBAR_METHOD

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void swap( SpeedMeter& lhs ) & noexcept
        {
          units_.swap( lhs.units_ );
          std::swap( len_longest_unit_, lhs.len_longest_unit_ );
          Base::swap( lhs );
        }
      };
# undef __PGBAR_DEFAULT_SPEED

      template<typename Base, typename Derived>
      class CounterMeter : public Base {
      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_counter( std::uint64_t num_task_done,
                                                                         std::uint64_t num_all_tasks ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
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
          return utils::count_digits( this->task_range_.back() ) * 2 + 1;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( CounterMeter )
      };

      template<typename Base, typename Derived>
      class Timer : public Base {
# define __PGBAR_DEFAULT_TIMER u8"--:--:--"
# define __PGBAR_TIMER_SEGMENT u8" < "
        static constexpr types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_TIMER ) - 1;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String time_formatter( types::TimeUnit duration ) const
        {
          auto time2str = []( std::int64_t num_time ) {
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
          return time_formatter( time_passed );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_elapsed() const noexcept
        {
          return _fixed_length;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String build_countdown(
          const types::TimeUnit& time_passed,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          if ( num_task_done == 0 || num_all_tasks == 0 )
# if __PGBAR_CXX20
          {
            constexpr types::LitU8 tmp = __PGBAR_DEFAULT_TIMER;
            auto ret                   = types::String( tmp.size(), '\0' );
            std::copy( tmp.cbegin(), tmp.cend(), ret.begin() );
            return ret;
          }
# else
            return { __PGBAR_DEFAULT_TIMER, _fixed_length };
# endif

          auto time_per_task = time_passed / num_task_done;
          if ( time_per_task.count() == 0 )
            time_per_task = std::chrono::nanoseconds( 1 );

          const auto remaining_tasks = num_all_tasks - num_task_done;
          // overflow check
          if ( remaining_tasks > ( std::numeric_limits<std::int64_t>::max )() / time_per_task.count() )
# if __PGBAR_CXX20
          {
            constexpr types::LitU8 tmp = __PGBAR_DEFAULT_TIMER;
            auto ret                   = types::String( tmp.size(), '\0' );
            std::copy( tmp.cbegin(), tmp.cend(), ret.begin() );
            return ret;
          }
# else
            return { __PGBAR_DEFAULT_TIMER, _fixed_length };
# endif
          else
            return time_formatter( time_per_task * remaining_tasks );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_countdown() const noexcept
        {
          return _fixed_length;
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_hybird(
          io::Stringbuf& buffer,
          const types::TimeUnit& time_passed,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks ) const
        {
          return buffer << build_elapsed( time_passed ) << __PGBAR_TIMER_SEGMENT
                        << build_countdown( time_passed, num_task_done, num_all_tasks );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_hybird() const noexcept
        {
          return fixed_len_elapsed() + fixed_len_countdown() + sizeof( __PGBAR_TIMER_SEGMENT ) - 1;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( Timer )
      };
# undef __PGBAR_DEFAULT_TIMER
# undef __PGBAR_TIMER_SEGMENT
    } // namespace assets

    namespace traits {
      // There is no need to declare the dependency on assets::CoreConfig here.
      // It will be directly passed to the dependency resolution function as the initial base class.
      __PGBAR_INHERIT_REGISTER( assets::BasicAnimation, , assets::Frames );

      __PGBAR_INHERIT_REGISTER( assets::CharIndic,
                                assets::Countable,
                                __PGBAR_PACK( assets::Filler,
                                              assets::Remains,
                                              assets::BasicAnimation,
                                              assets::BasicIndicator,
                                              assets::Reversible ) );
      __PGBAR_INHERIT_REGISTER( assets::BlockIndic,
                                assets::Countable,
                                __PGBAR_PACK( assets::Filler,
                                              assets::Remains,
                                              assets::Frames,
                                              assets::BasicIndicator,
                                              assets::Reversible ) );
      __PGBAR_INHERIT_REGISTER( assets::SpinIndic, , assets::BasicAnimation );
      __PGBAR_INHERIT_REGISTER( assets::SweepIndic,
                                ,
                                __PGBAR_PACK( assets::Filler,
                                              assets::BasicAnimation,
                                              assets::BasicIndicator ) );
      __PGBAR_INHERIT_REGISTER(
        assets::FlowIndic,
        ,
        __PGBAR_PACK( assets::Filler, assets::BasicAnimation, assets::BasicIndicator, assets::Reversible ) );

      __PGBAR_INHERIT_REGISTER( assets::PercentMeter, assets::Countable, );
      __PGBAR_INHERIT_REGISTER( assets::SpeedMeter, assets::Countable, );
      __PGBAR_INHERIT_REGISTER( assets::CounterMeter, assets::Countable, );

      __PGBAR_INHERIT_REGISTER( assets::Timer, assets::Countable, );

      template<template<typename...> class Component>
      struct OptionFor {
        using type = TypeSet<>;
      };
      template<template<typename...> class Component>
      using OptionFor_t = typename OptionFor<Component>::type;

# define __PGBAR_BIND_OPTION( Component, ... ) \
   template<>                                  \
   struct OptionFor<Component> {               \
     using type = TypeSet<__VA_ARGS__>;        \
   }

      __PGBAR_BIND_OPTION( assets::CoreConfig, option::Colored, option::Bolded );
      __PGBAR_BIND_OPTION( assets::Countable, option::Tasks );
      __PGBAR_BIND_OPTION( assets::Reversible, option::Reversed );
      __PGBAR_BIND_OPTION( assets::Frames, option::Lead, option::LeadColor );
      __PGBAR_BIND_OPTION( assets::Filler, option::Filler, option::FillerColor );
      __PGBAR_BIND_OPTION( assets::Remains, option::Remains, option::RemainsColor );
      __PGBAR_BIND_OPTION( assets::BasicAnimation, option::Shift );
      __PGBAR_BIND_OPTION( assets::BasicIndicator,
                           option::Starting,
                           option::Ending,
                           option::StartColor,
                           option::EndColor,
                           option::BarLength );
      __PGBAR_BIND_OPTION( assets::Prefix, option::Prefix, option::PrefixColor );
      __PGBAR_BIND_OPTION( assets::Postfix, option::Postfix, option::PostfixColor );
      __PGBAR_BIND_OPTION( assets::Segment,
                           option::Divider,
                           option::LeftBorder,
                           option::RightBorder,
                           option::InfoColor );
      __PGBAR_BIND_OPTION( assets::PercentMeter, );
      __PGBAR_BIND_OPTION( assets::SpeedMeter, option::SpeedUnit, option::Magnitude );

      template<>
      struct OptionFor<assets::CharIndic>
        : Coalesce<OptionFor_t<assets::Countable>,
                   OptionFor_t<assets::Reversible>,
                   OptionFor_t<assets::Frames>,
                   OptionFor_t<assets::Filler>,
                   OptionFor_t<assets::Remains>,
                   OptionFor_t<assets::BasicAnimation>,
                   OptionFor_t<assets::BasicIndicator>> {};
      template<>
      struct OptionFor<assets::BlockIndic>
        : Coalesce<OptionFor_t<assets::Countable>,
                   OptionFor_t<assets::Reversible>,
                   OptionFor_t<assets::Frames>,
                   OptionFor_t<assets::Filler>,
                   OptionFor_t<assets::Remains>,
                   OptionFor_t<assets::BasicAnimation>,
                   OptionFor_t<assets::BasicIndicator>> {};
      template<>
      struct OptionFor<assets::SpinIndic>
        : Coalesce<OptionFor_t<assets::Countable>, OptionFor_t<assets::BasicAnimation>> {};
      template<>
      struct OptionFor<assets::SweepIndic>
        : Coalesce<OptionFor_t<assets::Countable>,
                   OptionFor_t<assets::Frames>,
                   OptionFor_t<assets::Filler>,
                   OptionFor_t<assets::BasicAnimation>,
                   OptionFor_t<assets::BasicIndicator>> {};
      template<>
      struct OptionFor<assets::FlowIndic>
        : Coalesce<OptionFor_t<assets::Countable>,
                   OptionFor_t<assets::Reversible>,
                   OptionFor_t<assets::Frames>,
                   OptionFor_t<assets::Filler>,
                   OptionFor_t<assets::BasicAnimation>,
                   OptionFor_t<assets::BasicIndicator>> {};
    } // namespace traits

    namespace prefabs {
      template<template<typename...> class BarType, typename Derived>
      class BasicConfig
        : public traits::LI_t<BarType,
                              assets::Prefix,
                              assets::Postfix,
                              assets::Segment,
                              assets::PercentMeter,
                              assets::SpeedMeter,
                              assets::CounterMeter,
                              assets::Timer>::template type<assets::CoreConfig<Derived>, Derived> {
        // In fact, all the dependent components of BasicConfig can be fully injected from the outside.
        // This is not done here just to reduce repetitive code

        // BarType must inherit from BasicIndicator or BasicAnimation
        static_assert(
          traits::AnyOf<
            traits::Include<traits::TopoSort_t<traits::TemplateSet<BarType>>, assets::BasicIndicator>,
            traits::Include<traits::TopoSort_t<traits::TemplateSet<BarType>>, assets::BasicAnimation>>::value,
          "pgbar::__details::prefabs::BasicConfig: Invalid progress bar type" );

        using Self = BasicConfig;
        using Base =
          typename traits::LI_t<BarType,
                                assets::Prefix,
                                assets::Postfix,
                                assets::Segment,
                                assets::PercentMeter,
                                assets::SpeedMeter,
                                assets::CounterMeter,
                                assets::Timer>::template type<assets::CoreConfig<Derived>, Derived>;
        using PermittedSet = traits::Coalesce_t<traits::TypeSet<option::Style>,
                                                traits::OptionFor_t<BarType>,
                                                traits::OptionFor_t<assets::Prefix>,
                                                traits::OptionFor_t<assets::Postfix>,
                                                traits::OptionFor_t<assets::Segment>,
                                                traits::OptionFor_t<assets::SpeedMeter>,
                                                traits::OptionFor_t<assets::Timer>,
                                                traits::OptionFor_t<assets::CoreConfig>>;

        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacker( BasicConfig& cfg,
                                                                      option::Style&& val ) noexcept
        {
          cfg.visual_masks_ = val.value();
        }

        template<bool Enable>
        class Modifier final {
          friend Self;
          Self& self_;
          std::atomic<bool> owner_;

          Modifier( Self& self ) noexcept : self_ { self }, owner_ { true } { self_.rw_mtx_.lock(); }
          Modifier( Self& self, std::adopt_lock_t ) noexcept : self_ { self }, owner_ { true } {}
# if !__PGBAR_CXX17
          // There was not standard NRVO support before C++17.
          Modifier( Modifier&& rhs ) noexcept : self_ { rhs.self_ }, owner_ { true }
          {
            rhs.owner_.store( false, std::memory_order_release );
          }
# endif
        public:
          ~Modifier() noexcept
          {
            if ( owner_.load( std::memory_order_acquire ) )
              self_.rw_mtx_.unlock();
          }
          Modifier( const Modifier& )              = delete;
          Modifier& operator=( const Modifier& ) & = delete;

# define __PGBAR_METHOD( MethodName, EnumName )                      \
   Modifier&& MethodName()&& noexcept                                \
   {                                                                 \
     if ( owner_.load( std::memory_order_acquire ) )                 \
       self_.visual_masks_.set( utils::as_val( EnumName ), Enable ); \
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
            if ( owner_.load( std::memory_order_acquire ) ) {
              if __PGBAR_CXX17_CNSTXPR ( Enable )
                self_.visual_masks_.set();
              else
                self_.visual_masks_.reset();
            }
            return static_cast<Modifier&&>( *this );
          }

          Modifier<!Enable> negate() && noexcept
          {
            auto negate = Modifier<!Enable>( this->self_, std::adopt_lock );
            owner_.store( false, std::memory_order_release );
            return negate;
          }
        };

      protected:
        enum class Mask : std::uint8_t { Per = 0, Ani, Cnt, Sped, Elpsd, Cntdwn };
        std::bitset<6> visual_masks_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size common_render_size() const noexcept
        {
          return this->fixed_len_prefix() + this->fixed_len_postfix()
               + ( visual_masks_[utils::as_val( Mask::Per )] ? this->fixed_len_percent() : 0 )
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
               + this->fixed_len_segment( this->visual_masks_.count()
                                          - ( this->visual_masks_[utils::as_val( Mask::Cntdwn )]
                                              && this->visual_masks_[utils::as_val( Mask::Elpsd )] )
                                          + ( !this->prefix_.empty() || !this->postfix_.empty() ) );
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
          requires( !traits::Duplicated<traits::TypeList<Args...>>::value
                    && ( traits::Exist<PermittedSet, Args>::value && ... ) )
# else
        template<typename... Args,
                 typename = typename std::enable_if<
                   traits::AllOf<traits::Not<traits::Duplicated<traits::TypeList<Args...>>>,
                                 traits::Exist<PermittedSet, Args>...>::value>::type>
# endif
        __PGBAR_CXX23_CNSTXPR BasicConfig( Args... args )
        {
          static_cast<Derived*>( this )->template initialize<traits::TypeSet<Args...>>();
          (void)std::initializer_list<char> { ( unpacker( *this, std::move( args ) ), '\0' )... };
        }

        BasicConfig( const Self& lhs ) noexcept( traits::AllOf<std::is_nothrow_default_constructible<Base>,
                                                               std::is_nothrow_copy_assignable<Base>>::value )
          : Base( lhs )
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { lhs.rw_mtx_ };
          visual_masks_ = lhs.visual_masks_;
        }
        BasicConfig( Self&& rhs ) noexcept : Base( std::move( rhs ) )
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rhs.rw_mtx_ };
          using std::swap;
          swap( visual_masks_, rhs.visual_masks_ );
        }
        Self& operator=( const Self& lhs ) & noexcept( std::is_nothrow_copy_assignable<Base>::value )
        {
          __PGBAR_TRUST( this != &lhs );
          std::unique_lock<concurrent::SharedMutex> lock1 { this->rw_mtx_, std::defer_lock };
          concurrent::SharedLock<concurrent::SharedMutex> lock2 { lhs.rw_mtx_, std::defer_lock };
          std::lock( lock1, lock2 );

          visual_masks_ = lhs.visual_masks_;
          Base::operator=( lhs );
          return *this;
        }
        Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != &rhs );
          std::lock( this->rw_mtx_, rhs.rw_mtx_ );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_, std::adopt_lock };
          std::lock_guard<concurrent::SharedMutex> lock2 { rhs.rw_mtx_, std::adopt_lock };

          using std::swap;
          swap( visual_masks_, rhs.visual_masks_ );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        /**
         * Note: Because there are no exposed public base classes for config type,
         * there should be no scenarios for managing derived objects using base class references.
         * So the destructor here is deliberately set to be non-virtual.
         */
        ~BasicConfig() = default;

        Derived& style( types::Byte val ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Style( val ) );
          return static_cast<Derived&>( *this );
        }

        template<typename Arg, typename... Args>
# if __PGBAR_CXX20
          requires( !traits::Duplicated<traits::TypeList<Arg, Args...>>::value
                    && traits::Exist<PermittedSet, Arg>::value
                    && ( traits::Exist<PermittedSet, Args>::value && ... ) )
        Derived&
# else
        typename std::enable_if<traits::AllOf<traits::Not<traits::Duplicated<traits::TypeList<Arg, Args...>>>,
                                              traits::Exist<PermittedSet, Arg>,
                                              traits::Exist<PermittedSet, Args>...>::value,
                                Derived&>::type
# endif
          set( Arg arg, Args... args ) & noexcept(
            traits::AllOf<
              std::integral_constant<bool,
                                     noexcept( unpacker( std::declval<Derived&>(), std::move( arg ) ) )>,
              std::integral_constant<bool,
                                     noexcept( unpacker( std::declval<Derived&>(),
                                                         std::move( args ) ) )>...>::value )
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, std::move( arg ) );
          (void)std::initializer_list<char> { ( unpacker( *this, std::move( args ) ), '\0' )... };
          return static_cast<Derived&>( *this );
        }

        __PGBAR_NODISCARD types::Size fixed_length() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return static_cast<const Derived*>( this )->fixed_render_size();
        }

        __PGBAR_NODISCARD Modifier<true> enable() & noexcept { return Modifier<true>( *this ); }
        __PGBAR_NODISCARD Modifier<false> disable() & noexcept { return Modifier<false>( *this ); }

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR void swap( BasicConfig& lhs ) noexcept
        {
          std::lock( this->rw_mtx_, lhs.rw_mtx_ );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_, std::adopt_lock };
          std::lock_guard<concurrent::SharedMutex> lock2 { lhs.rw_mtx_, std::adopt_lock };
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

    namespace traits {
      template<typename C>
      struct is_config {
      private:
        template<template<typename...> class B, typename D>
        static constexpr std::true_type check( const prefabs::BasicConfig<B, D>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<C>>,
                decltype( check( std::declval<typename std::remove_cv<C>::type>() ) )>::value;
      };
    } // namespace traits
  } // namespace __details

  namespace config {
    /**
     * Determine if the output stream is binded to the tty based on the platform api.
     *
     * Always returns true if defined `PGBAR_INTTY`,
     * or the local platform is neither `Windows` nor `unix-like`.
     */
    __PGBAR_NODISCARD inline bool intty( Channel channel ) noexcept
    {
      if ( channel == Channel::Stdout )
        return __details::console::TermContext<Channel::Stdout>::itself().detect();
      return __details::console::TermContext<Channel::Stderr>::itself().detect();
    }

    __PGBAR_NODISCARD inline __details::types::Size terminal_width( Channel channel ) noexcept
    {
      if ( channel == Channel::Stdout )
        return __details::console::TermContext<Channel::Stdout>::itself().width();
      return __details::console::TermContext<Channel::Stderr>::itself().width();
    }

    using TimeUnit = __details::types::TimeUnit;
    // Get the current output interval.
    template<Channel Outlet>
    __PGBAR_NODISCARD TimeUnit refresh_interval() noexcept
    {
      return __details::render::Renderer<Outlet, Policy::Async>::working_interval();
    }
    // Set the new output interval.
    template<Channel Outlet>
    void refresh_interval( TimeUnit new_rate ) noexcept
    {
      __details::render::Renderer<Outlet, Policy::Async>::working_interval( new_rate );
    }
    // Set every channels to the same output interval.
    inline void refresh_interval( TimeUnit new_rate ) noexcept
    {
      __details::render::Renderer<Channel::Stderr, Policy::Async>::working_interval( new_rate );
      __details::render::Renderer<Channel::Stdout, Policy::Async>::working_interval( new_rate );
    }

    inline void hide_completed( bool flag ) noexcept
    {
      Indicator::_hide_completed.store( flag, std::memory_order_relaxed );
    }
    __PGBAR_NODISCARD inline bool hide_completed() noexcept
    {
      return Indicator::_hide_completed.load( std::memory_order_relaxed );
    }
    /**
     * Whether to automatically disable the style effect of the configuration object
     * when the output stream is not directed to a terminal.
     */
    inline void disable_styling( bool flag ) noexcept
    {
      Indicator::_disable_styling.store( flag, std::memory_order_relaxed );
    }
    __PGBAR_NODISCARD inline bool disable_styling() noexcept
    {
      return Indicator::_disable_styling.load( std::memory_order_relaxed );
    }

    class Line : public __details::prefabs::BasicConfig<__details::assets::CharIndic, Line> {
      using Base = __details::prefabs::BasicConfig<__details::assets::CharIndic, Line>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        // The types in the tuple are never repeated.
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Line::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -2 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8">" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8"=" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Remains>::value )
          unpacker( *this, option::Remains( u8" " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Entire ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 );
      }

    public:
      using Base::Base;
      Line( const Line& )              = default;
      Line( Line&& )                   = default;
      Line& operator=( const Line& ) & = default;
      Line& operator=( Line&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Line()    = default;
    };

    class Block : public __details::prefabs::BasicConfig<__details::assets::BlockIndic, Block> {
      using Base = __details::prefabs::BasicConfig<__details::assets::BlockIndic, Block>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Block::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this,
                    option::Lead( { u8" ",
                                    u8"\u258F",
                                    u8"\u258E",
                                    u8"\u258D",
                                    u8"\u258C",
                                    u8"\u258B",
                                    u8"\u258A",
                                    u8"\u2589" } ) );
        // In some editing environments,
        // directly writing character literals can lead to very strange encoding conversion errors.
        // Therefore, here we use Unicode code points to directly specify the required characters.
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8"\u2588" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Remains>::value )
          unpacker( *this, option::Remains( u8" " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Entire ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 );
      }

    public:
      using Base::Base;
      Block( const Block& )              = default;
      Block( Block&& )                   = default;
      Block& operator=( const Block& ) & = default;
      Block& operator=( Block&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Block()     = default;
    };

    class Spin : public __details::prefabs::BasicConfig<__details::assets::SpinIndic, Spin> {
      using Base = __details::prefabs::BasicConfig<__details::assets::SpinIndic, Spin>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Spin::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( { u8"/", u8"-", u8"\\", u8"|" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )]
                   ? this->fixed_len_frames() + ( !this->prefix_.empty() )
                   : 0 );
      }

    public:
      using Base::Base;
      Spin( const Spin& )              = default;
      Spin( Spin&& )                   = default;
      Spin& operator=( const Spin& ) & = default;
      Spin& operator=( Spin&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Spin()    = default;
    };

    class Sweep : public __details::prefabs::BasicConfig<__details::assets::SweepIndic, Sweep> {
      using Base = __details::prefabs::BasicConfig<__details::assets::SweepIndic, Sweep>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Sweep::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8"-" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8"<=>" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 );
      }

    public:
      using Base::Base;
      Sweep( const Sweep& )              = default;
      Sweep( Sweep&& )                   = default;
      Sweep& operator=( const Sweep& ) & = default;
      Sweep& operator=( Sweep&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Sweep()     = default;
    };

    class Flow : public __details::prefabs::BasicConfig<__details::assets::FlowIndic, Flow> {
      using Base = __details::prefabs::BasicConfig<__details::assets::FlowIndic, Flow>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Flow::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8" " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8"====" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 );
      }

    public:
      using Base::Base;
      Flow( const Flow& )              = default;
      Flow( Flow&& )                   = default;
      Flow& operator=( const Flow& ) & = default;
      Flow& operator=( Flow&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Flow()    = default;
    };
  } // namespace config

  namespace __details {
    namespace render {
      template<typename Config>
      struct CommonBuilder : public Config {
        using Config::Config;
        __PGBAR_CXX23_CNSTXPR CommonBuilder( const CommonBuilder& lhs ) = default;
        __PGBAR_CXX23_CNSTXPR CommonBuilder( CommonBuilder&& rhs )      = default;
        constexpr CommonBuilder( const Config& config )
          noexcept( std::is_nothrow_copy_constructible<Config>::value )
          : Config( config )
        {}
        constexpr CommonBuilder( Config&& config ) noexcept : Config( std::move( config ) ) {}
        __PGBAR_CXX20_CNSTXPR ~CommonBuilder() = default;

# define __PGBAR_METHOD( ParamType, Operation, Noexcept )                        \
   __PGBAR_CXX14_CNSTXPR CommonBuilder& operator=( ParamType config ) & Noexcept \
   {                                                                             \
     Config::operator=( Operation( config ) );                                   \
     return *this;                                                               \
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
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          using Self = Config;
          if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )]
               || this->visual_masks_[utils::as_val( Self::Mask::Sped )]
               || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
               || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] ) {
            if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )] ) {
              buffer << this->build_counter( num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
            }
            const auto time_passed = std::chrono::steady_clock::now() - zero_point;
            if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )] ) {
              buffer << this->build_speed( time_passed, num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
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
      struct AnimatedBuilder : public CommonBuilder<Config> {
      private:
        using Self = Config;
        using Base = CommonBuilder<Config>;

      protected:
        template<typename... Args>
        __PGBAR_INLINE_FN io::Stringbuf& indirect_build(
          io::Stringbuf& buffer,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          types::Float num_percent,
          const std::chrono::steady_clock::time_point& zero_point,
          Args&&... args ) const
        {
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->l_border_;
          }

          this->build_prefix( buffer );
          this->try_reset( buffer );
          if ( this->visual_masks_.any() )
            this->try_style( buffer, this->info_col_ );
          if ( !this->prefix_.empty() && ( this->visual_masks_.any() || !this->postfix_.empty() ) )
            buffer << constants::blank;
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Per ) ).any() )
              buffer << this->divider_;
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            static_cast<const Impl*>( this )->build_animation( buffer, std::forward<Args>( args )... );
            this->try_reset( buffer );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() ) {
              this->try_style( buffer, this->info_col_ );
              buffer << this->divider_;
            }
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->postfix_.empty() && this->visual_masks_.any() )
            buffer << constants::blank;
          this->build_postfix( buffer );
          this->try_reset( buffer );
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->r_border_;
          }
          return this->try_reset( buffer );
        }

      public:
        using Base::Base;
      };

      template<typename Config>
      struct Builder;
      template<>
      struct Builder<config::Line> final : public AnimatedBuilder<config::Line, Builder<config::Line>> {
      private:
        using Base = AnimatedBuilder<config::Line, Builder<config::Line>>;
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

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
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
      };

      template<>
      struct Builder<config::Block> final : public AnimatedBuilder<config::Block, Builder<config::Block>> {
      private:
        using Base = AnimatedBuilder<config::Block, Builder<config::Block>>;
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
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_percent );
        }
      };

      template<>
      struct Builder<config::Sweep> final : public AnimatedBuilder<config::Sweep, Builder<config::Sweep>> {
      private:
        using Base = AnimatedBuilder<config::Sweep, Builder<config::Sweep>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Size num_frame_cnt ) const
        {
          return this->build_sweep( buffer, num_frame_cnt );
        }

      public:
        using Base::Base;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_frame_cnt );
        }
      };

      template<>
      struct Builder<config::Flow> final : public AnimatedBuilder<config::Flow, Builder<config::Flow>> {
      private:
        using Base = AnimatedBuilder<config::Flow, Builder<config::Flow>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Size num_frame_cnt ) const
        {
          return this->build_flow( buffer, num_frame_cnt );
        }

      public:
        using Base::Base;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_frame_cnt );
        }
      };

      template<>
      struct Builder<config::Spin> final : public CommonBuilder<config::Spin> {
      private:
        using Self = config::Spin;

      public:
        using CommonBuilder<Self>::CommonBuilder;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->l_border_;
          }

          this->build_prefix( buffer );
          this->try_reset( buffer );
          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            if ( !this->prefix_.empty() )
              buffer << constants::blank;
            this->build_spin( buffer, num_frame_cnt );
            this->try_reset( buffer );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) ).any() ) {
              this->try_style( buffer, this->info_col_ );
              buffer << this->divider_;
            }
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() )
              buffer << this->divider_;
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->postfix_.empty() && ( !this->prefix_.empty() || this->visual_masks_.any() ) )
            buffer << this->divider_;
          this->build_postfix( buffer );
          this->try_reset( buffer );
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->r_border_;
          }
          return this->try_reset( buffer );
        }
      };
    } // namespace render

    namespace assets {
      template<typename Base, typename Derived>
      class TaskCounter : public Base {
        // Throws the exception::InvalidState if current object is active.
        __PGBAR_INLINE_FN void throw_if_active() &
        {
          if ( this->active() )
            __PGBAR_UNLIKELY throw exception::InvalidState(
              "pgbar: try to iterate using a running progress bar" );
        }

      protected:
        std::atomic<std::uint64_t> task_cnt_;
        std::uint64_t task_end_;

      public:
        template<typename... Args>
        constexpr TaskCounter( Args&&... args )
          noexcept( std::is_nothrow_constructible<Base, Args...>::value )
          : Base( std::forward<Args>( args )... ), task_cnt_ { 0 }
        {}
        __PGBAR_CXX14_CNSTXPR TaskCounter( TaskCounter&& rhs ) noexcept : Base( std::move( rhs ) )
        {
          task_cnt_.store( 0, std::memory_order_relaxed );
        }
        __PGBAR_CXX14_CNSTXPR TaskCounter& operator=( TaskCounter&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR ~TaskCounter() = default;

        // Get the progress of the task.
        __PGBAR_NODISCARD std::uint64_t progress() const noexcept
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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR slice::ProxySpan<slice::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_arithmetic<N>::value,
                                  slice::ProxySpan<slice::NumericSpan<N>, Derived>>::type
# endif
          iterate( N startpoint, N endpoint, N step ) &
        { // default parameter will cause ambiguous overloads
          // so we have to write them all
          throw_if_active();
          return {
            { startpoint, endpoint, step },
            static_cast<Derived&>( *this )
          };
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
        __PGBAR_NODISCARD slice::ProxySpan<slice::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD typename std::enable_if<std::is_floating_point<N>::value,
                                                  slice::ProxySpan<slice::NumericSpan<N>, Derived>>::type
# endif
          iterate( N endpoint, N step ) &
        {
          throw_if_active();
          return {
            { {}, endpoint, step },
            static_cast<Derived&>( *this )
          };
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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR slice::ProxySpan<slice::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  slice::ProxySpan<slice::NumericSpan<N>, Derived>>::type
# endif
          iterate( N startpoint, N endpoint ) &
        {
          throw_if_active();
          return {
            { startpoint, endpoint, 1 },
            static_cast<Derived&>( *this )
          };
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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR slice::ProxySpan<slice::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  slice::ProxySpan<slice::NumericSpan<N>, Derived>>::type
# endif
          iterate( N endpoint ) &
        {
          throw_if_active();
          return {
            { {}, endpoint, 1 },
            static_cast<Derived&>( *this )
          };
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
          requires traits::is_sized_iterator<I>::value
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR slice::ProxySpan<slice::IterSpan<I>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<traits::is_sized_iterator<I>::value,
                                  slice::ProxySpan<slice::IterSpan<I>, Derived>>::type
# endif
          iterate( I startpoint, I endpoint ) & noexcept(
            traits::AnyOf<std::is_pointer<I>, std::is_nothrow_move_constructible<I>>::value )
        {
          throw_if_active();
          return {
            { std::move( startpoint ), std::move( endpoint ) },
            static_cast<Derived&>( *this )
          };
        }
        template<typename I, typename F>
# if __PGBAR_CXX20
          requires traits::is_sized_iterator<I>::value
        __PGBAR_CXX14_CNSTXPR void
# else
        __PGBAR_CXX14_CNSTXPR typename std::enable_if<traits::is_sized_iterator<I>::value>::type
# endif
          iterate( I startpoint, I endpoint, F&& unary_fn )
        {
          for ( auto&& e : iterate( std::move( startpoint ), std::move( endpoint ) ) )
            unary_fn( std::forward<decltype( e )>( e ) );
        }

        // Visualize unidirectional traversal of a abstract range interval defined by `container`'s
        // slice.
        template<class R>
# if __PGBAR_CXX20
          requires( traits::is_bounded_range<std::remove_reference_t<R>>::value
                    && !std::ranges::view<std::remove_reference_t<R>> )
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR
          slice::ProxySpan<slice::BoundedSpan<std::remove_reference_t<R>>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR typename std::enable_if<
          traits::is_bounded_range<typename std::remove_reference<R>::type>::value,
          slice::ProxySpan<slice::BoundedSpan<typename std::remove_reference<R>::type>, Derived>>::type
# endif
          iterate( R& container ) &
        {
          throw_if_active();
          return { { container }, static_cast<Derived&>( *this ) };
        }
# if __PGBAR_CXX20
        template<class R>
          requires( traits::is_bounded_range<R>::value && std::ranges::view<R> )
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR slice::ProxySpan<R, Derived> iterate( R view ) &
        {
          throw_if_active();
          return { std::move( view ), static_cast<Derived&>( *this ) };
        }
# endif
        template<class R, typename F>
# if __PGBAR_CXX20
          requires traits::is_bounded_range<std::remove_reference_t<R>>::value
        __PGBAR_CXX17_CNSTXPR void
# else
        __PGBAR_CXX17_CNSTXPR typename std::enable_if<
          traits::is_bounded_range<typename std::remove_reference<R>::type>::value>::type
# endif
          iterate( R&& range, F&& unary_fn )
        {
          for ( auto&& e : iterate( std::forward<R>( range ) ) )
            unary_fn( std::forward<decltype( e )>( e ) );
        }
      };

      template<typename Base, typename Derived>
      class FrameCounter : public Base {
      protected:
        types::Size idx_frame_;

      public:
        using Base::Base;
        constexpr FrameCounter() = default;
        constexpr FrameCounter( FrameCounter&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        __PGBAR_CXX14_CNSTXPR FrameCounter& operator=( FrameCounter&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR ~FrameCounter() = default;
      };

      template<typename Base, typename Derived>
      class CoreBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename Soul,
               Channel Outlet,
               Policy Mode,
               Region Area>
      class CoreBar<Base, Derived<Soul, Outlet, Mode, Area>> : public Base {
        static_assert( traits::is_config<Soul>::value,
                       "pgbar::__details::prefabs::CoreBar: Invalid config type" );
        using Self   = CoreBar;
        using Subcls = Derived<Soul, Outlet, Mode, Area>;

        __PGBAR_INLINE_FN void make_frame()
        {
          switch ( static_cast<Subcls*>( this )->categorize() ) {
          case StateCategory::Awake: {
            static_cast<Subcls*>( this )->startframe();
          } break;
          case StateCategory::Refresh: {
            static_cast<Subcls*>( this )->refreshframe();
          } break;
          case StateCategory::Finish: {
            static_cast<Subcls*>( this )->endframe();
          } break;
          default: return;
          }
        }
        __PGBAR_INLINE_FN friend void make_frame( Self& self ) { self.make_frame(); }

      protected:
        enum class StateCategory : std::uint8_t { Stop, Awake, Refresh, Finish };

        render::Builder<Soul> config_;
        mutable std::mutex mtx_;

        std::chrono::steady_clock::time_point zero_point_;

        // An extension point that performs global resource cleanup related to the progress bar semantics
        // themselves.
        virtual void do_halt( bool forced = false ) noexcept
        {
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          __PGBAR_ASSERT( executor.empty() == false );
          if ( !forced )
            executor.attempt();
          executor.appoint_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
        }
        virtual void do_boot() & noexcept( false )
        {
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          if ( !executor.try_appoint( [this]() {
                 // No exceptions are caught here, this should be done by the thread manager.
                 auto& ostream    = io::OStream<Outlet>::itself();
                 const auto istty = console::TermContext<Outlet>::itself().connected();
                 switch ( static_cast<Subcls*>( this )->categorize() ) {
                 case StateCategory::Awake: {
                   if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
                     if ( istty )
                       ostream << console::escodes::savecursor;
                   static_cast<Subcls*>( this )->startframe();
                   ostream << console::escodes::nextline;
                   ostream << io::flush;
                 } break;
                 case StateCategory::Refresh: {
                   if ( istty ) {
                     if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
                       ostream << console::escodes::resetcursor;
                     else
                       ostream << console::escodes::prevline << console::escodes::linestart
                               << console::escodes::linewipe;
                   }
                   static_cast<Subcls*>( this )->refreshframe();
                   ostream << console::escodes::nextline;
                   ostream << io::flush;
                 } break;
                 case StateCategory::Finish: {
                   if ( istty ) {
                     if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
                       ostream << console::escodes::resetcursor;
                     else
                       ostream << console::escodes::prevline << console::escodes::linestart
                               << console::escodes::linewipe;
                   }
                   static_cast<Subcls*>( this )->endframe();
                   if ( istty && config::hide_completed() )
                     ostream << console::escodes::linestart << console::escodes::linewipe;
                   else
                     ostream << console::escodes::nextline;
                   ostream << io::flush;
                 } break;
                 default: return;
                 }
               } ) )
            __PGBAR_UNLIKELY throw exception::InvalidState(
              "pgbar: another progress bar instance is already running" );

          io::OStream<Outlet>::itself() << io::release; // reset the state.
          try {
            executor.activate();
          } catch ( ... ) {
            executor.appoint();
            throw;
          }
        }

      public:
        CoreBar( const Self& )           = delete;
        Self& operator=( const Self& ) & = delete;

        CoreBar( Soul&& config ) noexcept : config_ { std::move( config ) } {}
        CoreBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) ), config_ { std::move( rhs.config_ ) }
        {}
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        {
          __PGBAR_TRUST( this != &rhs );
          config_ = std::move( rhs.config_ );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        ~CoreBar() noexcept
        {
          std::lock_guard<std::mutex> lock { mtx_ };
          static_cast<Subcls*>( this )->do_reset( true );
        }

        __PGBAR_NODISCARD bool active() const noexcept final
        {
          return static_cast<const Subcls*>( this )->categorize() != StateCategory::Stop;
        }
        void reset() noexcept final
        {
          std::lock_guard<std::mutex> lock { mtx_ };
          static_cast<Subcls*>( this )->do_reset();
          __PGBAR_ASSERT( active() == false );
        }

        Soul& config() & noexcept { return config_; }
        const Soul& config() const& noexcept { return config_; }
        Soul&& config() && noexcept { return std::move( config_ ); }

        __PGBAR_CXX20_CNSTXPR void swap( CoreBar& lhs ) noexcept
        {
          __PGBAR_TRUST( this != &lhs );
          __PGBAR_ASSERT( active() == false );
          __PGBAR_ASSERT( active() == false );
          Base::swap( lhs );
          config_.swap( lhs.config_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( CoreBar& a, CoreBar& b ) noexcept { a.swap( b ); }
      };

      template<typename Base, typename Derived>
      class ReactiveBar : public Base {
        using Self = ReactiveBar;
# if __PGBAR_CXX20
#  define __PGBAR_ACTIONABLE                   \
    ( !std::is_null_pointer_v<std::decay_t<F>> \
      && std::is_constructible_v<wrappers::UniqueFunction<void()>, F> )
# else
        template<typename F, typename Ret>
        using Actionable = typename std::enable_if<
          traits::AllOf<traits::Not<std::is_null_pointer<typename std::decay<F>::type>>,
                        std::is_constructible<wrappers::UniqueFunction<void()>, F>>::value,
          Ret>::type;
# endif

      protected:
        wrappers::UniqueFunction<void()> hook_;

      public:
        ReactiveBar( const Self& )       = delete;
        Self& operator=( const Self& ) & = delete;

        using Base::Base;
        constexpr ReactiveBar( Self&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        __PGBAR_CXX14_CNSTXPR Self& operator=( Self&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR ~ReactiveBar() = default;

        template<typename F>
# if __PGBAR_CXX20
          requires __PGBAR_ACTIONABLE
        Derived&
# else
        Actionable<F, Derived&>
# endif
          action( F&& fn ) & noexcept(
            std::is_nothrow_assignable<wrappers::UniqueFunction<void()>, F>::value )
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          hook_ = std::forward<F>( fn );
          return static_cast<Derived&>( *this );
        }
        Derived& action() noexcept
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          hook_.reset();
          return static_cast<Derived&>( *this );
        }

        template<typename F>
# if __PGBAR_CXX20
          requires __PGBAR_ACTIONABLE
        friend Derived&
# else
        friend Actionable<F, Derived&>
# endif
          operator|=( Self& bar, F&& fn )
            noexcept( std::is_nothrow_assignable<wrappers::UniqueFunction<void()>, F>::value )
        {
          return bar.action( std::forward<F>( fn ) );
        }
        template<typename F>
# if __PGBAR_CXX20
          requires __PGBAR_ACTIONABLE
        friend Derived&
# else
        friend Actionable<F, Derived&>
# endif
          operator|( Self& bar, F&& fn )
            noexcept( std::is_nothrow_assignable<wrappers::UniqueFunction<void()>, F>::value )
        {
          return bar.action( std::forward<F>( fn ) );
        }
        template<typename F>
# if __PGBAR_CXX20
          requires __PGBAR_ACTIONABLE
        friend Derived&
# else
        friend Actionable<F, Derived&>
# endif
          operator|( F&& fn, Self& bar )
            noexcept( std::is_nothrow_assignable<wrappers::UniqueFunction<void()>, F>::value )
        {
          return bar.action( std::forward<F>( fn ) );
        }
        template<typename F>
# if __PGBAR_CXX20
          requires __PGBAR_ACTIONABLE
        friend Derived&&
# else
        friend Actionable<F, Derived&&>
# endif
          operator|( Self&& bar, F&& fn )
            noexcept( std::is_nothrow_assignable<wrappers::UniqueFunction<void()>, F>::value )
        {
          return std::move( bar.action( std::forward<F>( fn ) ) );
        }
        template<typename F>
# if __PGBAR_CXX20
          requires __PGBAR_ACTIONABLE
        friend Derived&&
# else
        friend Actionable<F, Derived&&>
# endif
          operator|( F&& fn, Self&& bar )
            noexcept( std::is_nothrow_assignable<wrappers::UniqueFunction<void()>, F>::value )
        {
          return std::move( bar.action( std::forward<F>( fn ) ) );
        }
      };
# undef __PGBAR_ACTIONABLE

      template<typename Base, typename Derived>
      class TickableBar : public Base {
        using Self = TickableBar;

      public:
        using Base::Base;
        constexpr TickableBar( Self&& rhs )                   = default;
        __PGBAR_CXX14_CNSTXPR Self& operator=( Self&& rhs ) & = default;
        __PGBAR_CXX20_CNSTXPR virtual ~TickableBar()          = default;

        void tick() &
        {
          static_cast<Derived*>( this )->do_tick(
            [this]() noexcept { this->task_cnt_.fetch_add( 1, std::memory_order_release ); } );
        }
        void tick( std::uint64_t next_step ) &
        {
          static_cast<Derived*>( this )->do_tick( [&]() noexcept {
            const auto task_cnt = this->task_cnt_.load( std::memory_order_acquire );
            this->task_cnt_.fetch_add( task_cnt + next_step > this->task_end_ ? this->task_end_ - task_cnt
                                                                              : next_step );
          } );
        }
        /**
         * Set the iteration step of the progress bar to a specified percentage.
         * Ignore the call if the iteration count exceeds the given percentage.
         * If `percentage` is bigger than 100, it will be set to 100.
         *
         * @param percentage Value range: [0, 100].
         */
        void tick_to( std::uint8_t percentage ) &
        {
          static_cast<Derived*>( this )->do_tick( [&]() noexcept {
            auto updater = [this]( std::uint64_t target ) noexcept {
              auto current = this->task_cnt_.load( std::memory_order_acquire );
              while ( !this->task_cnt_.compare_exchange_weak( current,
                                                              target,
                                                              std::memory_order_release,
                                                              std::memory_order_acquire )
                      && target <= current ) {}
            };
            if ( percentage <= 100 ) {
              const auto target = static_cast<std::uint64_t>( this->task_end_ * percentage * 0.01 );
              __PGBAR_ASSERT( target <= task_end_ );
              updater( target );
            } else
              updater( this->task_end_ );
          } );
        }
      };

      template<typename Base, typename Derived>
      class PlainBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename Soul,
               Channel Outlet,
               Policy Mode,
               Region A>
      class PlainBar<Base, Derived<Soul, Outlet, Mode, A>> : public Base {
        using Self = PlainBar;
        template<typename, typename>
        friend class CoreBar;
        template<typename, typename>
        friend class TickableBar;

        using State = typename Base::StateCategory;
        std::atomic<State> state_;

      protected:
        __PGBAR_INLINE_FN void startframe() &
        {
          refreshframe();
          auto expected = State::Awake;
          this->state_.compare_exchange_strong( expected, State::Refresh, std::memory_order_release );
        }
        __PGBAR_INLINE_FN void refreshframe() &
        {
          __PGBAR_ASSERT( this->task_cnt_ <= this->task_end_ );
          this->config_.build( io::OStream<Outlet>::itself(),
                               this->task_cnt_.load( std::memory_order_acquire ),
                               this->task_end_,
                               this->zero_point_ );
        }
        __PGBAR_INLINE_FN void endframe() &
        {
          __PGBAR_ASSERT( this->task_cnt_ <= this->task_end_ );
          refreshframe();
          this->state_.store( State::Stop, std::memory_order_release );
        }

        __PGBAR_INLINE_FN typename Base::StateCategory categorize() const noexcept
        {
          return state_.load( std::memory_order_acquire );
        }

        __PGBAR_INLINE_FN void do_reset( bool forced = false ) noexcept
        {
          if ( this->active() ) {
            if ( forced )
              __PGBAR_UNLIKELY state_.store( State::Stop, std::memory_order_release );
            else {
              if ( this->hook_ )
                this->hook_();
              state_.store( State::Finish, std::memory_order_release );
            }
            this->do_halt( forced );
          } else
            state_.store( State::Stop, std::memory_order_release );
        }
        template<typename F>
        __PGBAR_INLINE_FN void do_tick( F&& ticker ) & noexcept( false )
        {
          switch ( state_.load( std::memory_order_acquire ) ) {
          case State::Stop:  __PGBAR_FALLTHROUGH;
          case State::Awake: {
            std::lock_guard<std::mutex> lock { this->mtx_ };
            if ( state_.load( std::memory_order_acquire ) == State::Stop ) {
              this->task_end_ = this->config_.tasks();
              if ( this->task_end_ == 0 )
                __PGBAR_UNLIKELY throw exception::InvalidState( "pgbar: the number of tasks is zero" );

              if ( config::disable_styling() && !config::intty( Outlet ) )
                this->config_.colored( false ).bolded( false );
              this->task_cnt_.store( 0, std::memory_order_release );
              this->zero_point_ = std::chrono::steady_clock::now();
              state_.store( State::Awake, std::memory_order_release );
              try {
                this->do_boot();
              } catch ( ... ) {
                state_.store( State::Stop, std::memory_order_release );
                throw;
              }
            }
          }
            __PGBAR_FALLTHROUGH;
          case State::Refresh: {
            ticker();

            if ( this->task_cnt_.load( std::memory_order_acquire ) >= this->task_end_ )
              __PGBAR_UNLIKELY
              {
                if ( this->mtx_.try_lock() ) {
                  std::lock_guard<std::mutex> lock { this->mtx_, std::adopt_lock };
                  do_reset();
                }
              }
            else if __PGBAR_CXX17_CNSTXPR ( Mode == Policy::Sync )
              render::Renderer<Outlet, Mode>::itself().execute();
          } break;

          default: __PGBAR_UNREACHABLE;
          }
        }

      public:
        PlainBar( const Self& )          = delete;
        Self& operator=( const Self& ) & = delete;

        PlainBar( Soul&& config ) noexcept( std::is_nothrow_constructible<Base, Soul&&>::value )
          : Base( std::move( config ) ), state_ { State::Stop }
        {}
        PlainBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) ), state_ { State::Stop }
        {}
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        ~PlainBar() = default;
      };

      template<typename Base, typename Derived>
      class FrameBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename Soul,
               Channel Outlet,
               Policy Mode,
               Region A>
      class FrameBar<Base, Derived<Soul, Outlet, Mode, A>> : public Base {
        using Self   = FrameBar;
        using Subcls = Derived<Soul, Outlet, Mode, A>;
        template<typename, typename>
        friend class CoreBar;
        template<typename, typename>
        friend class TickableBar;

        enum class State : std::uint8_t { Stop, Awake, ProgressRefresh, ActivityRefresh, Finish };
        std::atomic<State> state_;

      protected:
        __PGBAR_INLINE_FN void startframe() &
        {
          this->idx_frame_ = 0;
          refreshframe();
          auto expected = State::Awake;
          state_.compare_exchange_strong( expected,
                                          this->task_end_ == 0 ? State::ActivityRefresh
                                                               : State::ProgressRefresh,
                                          std::memory_order_release );
        }
        __PGBAR_INLINE_FN void refreshframe() &
        {
          __PGBAR_ASSERT( this->task_cnt_ <= this->task_end_ );
          auto& buffer = io::OStream<Outlet>::itself();
          this->config_.build( buffer,
                               this->idx_frame_,
                               this->task_cnt_.load( std::memory_order_acquire ),
                               this->task_end_,
                               this->zero_point_ );
          ++this->idx_frame_;
        }
        __PGBAR_INLINE_FN void endframe() &
        {
          __PGBAR_ASSERT( this->task_cnt_ <= this->task_end_ );
          this->config_.build( io::OStream<Outlet>::itself(),
                               this->idx_frame_,
                               this->task_cnt_.load( std::memory_order_acquire ),
                               this->task_end_,
                               this->zero_point_ );
          state_.store( State::Stop, std::memory_order_release );
        }

        __PGBAR_INLINE_FN typename Base::StateCategory categorize() const noexcept
        {
          switch ( state_.load( std::memory_order_acquire ) ) {
          case State::Awake:           return Base::StateCategory::Awake;
          case State::ProgressRefresh: __PGBAR_FALLTHROUGH;
          case State::ActivityRefresh: return Base::StateCategory::Refresh;
          case State::Finish:          return Base::StateCategory::Finish;
          default:                     break;
          }
          return Base::StateCategory::Stop;
        }

        template<typename F>
        __PGBAR_INLINE_FN void do_tick( F&& ticker ) & noexcept( false )
        {
          switch ( this->state_.load( std::memory_order_acquire ) ) {
          case State::Stop:  __PGBAR_FALLTHROUGH;
          case State::Awake: {
            std::lock_guard<std::mutex> lock { this->mtx_ };
            if ( this->state_.load( std::memory_order_acquire ) == State::Stop ) {
              this->task_end_ = this->config_.tasks();
              static_cast<Subcls*>( this )->warmup();

              if ( config::disable_styling() && !config::intty( Outlet ) )
                this->config_.colored( false ).bolded( false );
              this->task_cnt_.store( 0, std::memory_order_release );
              this->zero_point_ = std::chrono::steady_clock::now();
              this->state_.store( State::Awake, std::memory_order_release );
              try {
                this->do_boot();
              } catch ( ... ) {
                this->state_.store( State::Stop, std::memory_order_release );
                throw;
              }
            }
            if ( this->state_.load( std::memory_order_acquire ) == State::ActivityRefresh )
              return;
          }
            __PGBAR_FALLTHROUGH;
          case State::ProgressRefresh: {
            ticker();

            if ( this->task_cnt_.load( std::memory_order_acquire ) >= this->task_end_ )
              __PGBAR_UNLIKELY
              {
                if ( this->mtx_.try_lock() ) {
                  std::lock_guard<std::mutex> lock { this->mtx_, std::adopt_lock };
                  do_reset();
                }
              }
            else if __PGBAR_CXX17_CNSTXPR ( Mode == Policy::Sync )
              render::Renderer<Outlet, Mode>::itself().execute();
          } break;

          case State::ActivityRefresh: {
            if __PGBAR_CXX17_CNSTXPR ( Mode == Policy::Sync )
              render::Renderer<Outlet, Mode>::itself().execute();
          } break;

          default: __PGBAR_UNREACHABLE;
          }
        }
        __PGBAR_INLINE_FN void do_reset( bool forced = false ) noexcept
        {
          if ( this->active() ) {
            if ( forced )
              __PGBAR_UNLIKELY state_.store( State::Stop, std::memory_order_release );
            else {
              if ( this->hook_ )
                this->hook_();
              state_.store( State::Finish, std::memory_order_release );
            }
            this->do_halt( forced );
          } else
            state_.store( State::Stop, std::memory_order_release );
        }

      public:
        FrameBar( const Self& )          = delete;
        Self& operator=( const Self& ) & = delete;

        FrameBar( Soul&& config ) noexcept( std::is_nothrow_constructible<Base, Soul&&>::value )
          : Base( std::move( config ) ), state_ { State::Stop }
        {}
        FrameBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) ), state_ { State::Stop }
        {}
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        ~FrameBar() = default;
      };

      template<typename Base, typename Derived>
      class BoundedFrameBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename S,
               Channel O,
               Policy M,
               Region A>
      class BoundedFrameBar<Base, Derived<S, O, M, A>> : public Base {
        friend Base;

        void warmup() &
        {
          if ( this->task_end_ == 0 )
            __PGBAR_UNLIKELY throw exception::InvalidState( "pgbar: the number of tasks is zero" );
        }

      public:
        using Base::Base;
        constexpr BoundedFrameBar( BoundedFrameBar&& )                          = default;
        __PGBAR_CXX14_CNSTXPR BoundedFrameBar& operator=( BoundedFrameBar&& ) & = default;
        __PGBAR_CXX20_CNSTXPR ~BoundedFrameBar()                                = default;
      };

      template<typename Base, typename Derived>
      class NullableFrameBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename S,
               Channel O,
               Policy M,
               Region A>
      class NullableFrameBar<Base, Derived<S, O, M, A>> : public Base {
        friend Base;

        void warmup() & {}

      public:
        using Base::Base;
        constexpr NullableFrameBar( NullableFrameBar&& )                          = default;
        __PGBAR_CXX14_CNSTXPR NullableFrameBar& operator=( NullableFrameBar&& ) & = default;
        __PGBAR_CXX20_CNSTXPR ~NullableFrameBar()                                 = default;
      };
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER( assets::ReactiveBar, assets::CoreBar, );
      __PGBAR_INHERIT_REGISTER( assets::TickableBar, , assets::TaskCounter );
      __PGBAR_INHERIT_REGISTER( assets::PlainBar,
                                ,
                                __PGBAR_PACK( assets::ReactiveBar, assets::TickableBar ) );
      __PGBAR_INHERIT_REGISTER( assets::FrameBar,
                                ,
                                __PGBAR_PACK( assets::ReactiveBar,
                                              assets::TickableBar,
                                              assets::FrameCounter ) );
      __PGBAR_INHERIT_REGISTER( assets::BoundedFrameBar, , assets::FrameBar );
      __PGBAR_INHERIT_REGISTER( assets::NullableFrameBar, , assets::FrameBar );

      // In pgbar, the configuration type is "first-class type";
      // thus, we hope to be able to automatically generate the progress bar type
      // by specifying the configuration type for a template class.
      // This's why we should define an additional tool type here.
      template<typename Config>
      struct BehaviourFor {
        using type = TemplateSet<>;
      };
      template<typename Config>
      using BehaviourFor_t = typename BehaviourFor<Config>::type;

# define __PGBAR_BIND_BEHAVIOUR( Config, ... ) \
   template<>                                  \
   struct BehaviourFor<Config> {               \
     using type = TemplateSet<__VA_ARGS__>;    \
   }

      __PGBAR_BIND_BEHAVIOUR( config::Line, assets::BoundedFrameBar );
      __PGBAR_BIND_BEHAVIOUR( config::Block, assets::PlainBar );
      __PGBAR_BIND_BEHAVIOUR( config::Spin, assets::NullableFrameBar );
      __PGBAR_BIND_BEHAVIOUR( config::Sweep, assets::NullableFrameBar );
      __PGBAR_BIND_BEHAVIOUR( config::Flow, assets::NullableFrameBar );
    } // namespace traits

    namespace prefabs {
      // Interface type, used to meet the template type arguments of the base class for the derived class.
      template<typename Soul, Channel Outlet, Policy Mode, Region Area>
      class BasicBar
        : public traits::LI<traits::BehaviourFor_t<Soul>>::template type<Indicator,
                                                                         BasicBar<Soul, Outlet, Mode, Area>> {
        using Base = typename traits::LI<
          traits::BehaviourFor_t<Soul>>::template type<Indicator, BasicBar<Soul, Outlet, Mode, Area>>;

      public:
        using Config                     = Soul;
        static constexpr Channel Sink    = Outlet;
        static constexpr Policy Strategy = Mode;
        static constexpr Region Layout   = Area;

        constexpr BasicBar( Soul config ) noexcept( std::is_nothrow_constructible<Base, Soul&&>::value )
          : Base( std::move( config ) )
        {}
# if __PGBAR_CXX20
        template<typename... Args>
          requires( !( std::is_same_v<std::decay_t<Args>, Soul> || ... )
                    && std::is_constructible_v<Soul, Args...> )
# else
        template<typename... Args,
                 typename = typename std::enable_if<
                   traits::AllOf<traits::Not<std::is_same<typename std::decay<Args>::type, Soul>>...,
                                 std::is_constructible<Soul, Args...>>::value>::type>
# endif
        constexpr BasicBar( Args&&... args ) noexcept( std::is_nothrow_constructible<Base, Args...>::value )
          : Base( Soul( std::forward<Args>( args )... ) )
        {}

        BasicBar( const BasicBar& )                               = delete;
        BasicBar& operator=( const BasicBar& ) &                  = delete;
        constexpr BasicBar( BasicBar&& )                          = default;
        __PGBAR_CXX14_CNSTXPR BasicBar& operator=( BasicBar&& ) & = default;
        __PGBAR_CXX20_CNSTXPR virtual ~BasicBar()                 = default;
      };
    } // namespace prefabs

    namespace traits {
      template<typename B>
      struct is_bar {
      private:
        template<typename S, Channel O, Policy M, Region A>
        static constexpr std::true_type check( const prefabs::BasicBar<S, O, M, A>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<B>>,
                decltype( check( std::declval<typename std::remove_cv<B>::type>() ) )>::value;
      };
      template<typename B>
      struct is_iterable_bar : AllOf<is_bar<B>, InstanceOf<B, assets::TaskCounter>> {};
    } // namespace traits
  } // namespace __details

  /**
   * The simplest progress bar, which is what you think it is.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using ProgressBar = __details::prefabs::BasicBar<config::Line, Outlet, Mode, Area>;
  /**
   * A progress bar with a smoother bar, requires an Unicode-supported terminal.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using BlockBar = __details::prefabs::BasicBar<config::Block, Outlet, Mode, Area>;
  /**
   * A progress bar without bar indicator, replaced by a fixed animation component.
   *
   * It's structure is shown below:
   * {LeftBorder}{Lead}{Prefix}{Percent}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using SpinBar = __details::prefabs::BasicBar<config::Spin, Outlet, Mode, Area>;
  /**
   * A progress bar with a sweeping indicator, where the lead moves back and forth within the bar area.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using SweepBar = __details::prefabs::BasicBar<config::Sweep, Outlet, Mode, Area>;
  /**
   * A progress bar with a flowing indicator, where the lead moves in a single direction within the bar area.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using FlowBar = __details::prefabs::BasicBar<config::Flow, Outlet, Mode, Area>;

  namespace __details {
    namespace assets {
      template<types::Size, typename Base>
      struct TupleSlot : public Base {
        using Base::Base;
        __PGBAR_CXX23_CNSTXPR TupleSlot( TupleSlot&& )              = default;
        __PGBAR_CXX14_CNSTXPR TupleSlot& operator=( TupleSlot&& ) & = default;
        constexpr TupleSlot( Base&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        __PGBAR_CXX14_CNSTXPR TupleSlot& operator=( Base&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( rhs ) );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
      };

      template<typename Seq, typename... Bars>
      class TupleBar;
      template<types::Size... Tags, Channel Outlet, Policy Mode, Region Area, typename... Configs>
      class TupleBar<traits::IndexSeq<Tags...>, prefabs::BasicBar<Configs, Outlet, Mode, Area>...> final
        : public TupleSlot<Tags, prefabs::BasicBar<Configs, Outlet, Mode, Area>>... {
        static_assert( sizeof...( Tags ) == sizeof...( Configs ),
                       "pgbar::__details::assets::TupleBar: Unexpected type mismatch" );
        static_assert( sizeof...( Configs ) > 0,
                       "pgbar::__details::assets::TupleBar: The number of progress bars cannot be zero" );

        std::atomic<types::Size> alive_cnt_;
        // cb: CombinedBar
        enum class State : std::uint8_t { Stop, Awake, Refresh };
        std::atomic<State> state_;
        mutable std::mutex sched_mtx_;

        // Bitmask indicating which bars produced output in the current render pass.
        std::atomic<std::bitset<sizeof...( Configs )>> active_mask_;

        template<types::Size Pos>
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR typename std::enable_if<( Pos >= sizeof...( Configs ) )>::type
          do_render() &
        {}
        template<types::Size Pos = 0>
        inline typename std::enable_if<( Pos < sizeof...( Configs ) )>::type do_render() &
        {
          __PGBAR_ASSERT( online() );
          auto& ostream        = io::OStream<Outlet>::itself();
          const auto istty     = console::TermContext<Outlet>::itself().connected();
          const auto hide_done = config::hide_completed();

          bool this_rendered = false;
          auto mask          = active_mask_.load( std::memory_order_relaxed );
          if ( at<Pos>().active() ) {
            this_rendered = true;
            mask.set( Pos );
            if ( istty )
              ostream << console::escodes::linewipe;
            make_frame( at<Pos>() );

            if ( ( !istty || hide_done ) && !at<Pos>().active() )
              mask.reset( Pos );
            active_mask_.store( mask, std::memory_order_release );
          }

          /**
           * Here are the scenarios where a newline character is output:
           * 1. If the output stream is bound to a terminal and the completed progress bar does not need to be
           * hidden, it should be output when active_mask_[Pos] is true.
           * 2. If the output stream is bound to a terminal and the completed progress bar needs to be hidden,
           *    it only be output when Pos-th is still active.
           * 3. If the output stream is not bound to a terminal,
           *    it should be output whenever Pos-th has just been rendered.
           */
          if ( ( this_rendered || mask[Pos] )
               && ( ( !istty && this_rendered ) || ( istty && ( !hide_done || at<Pos>().active() ) ) ) )
            ostream << console::escodes::nextline;
          if ( istty && hide_done ) {
            if ( !at<Pos>().active() )
              ostream << console::escodes::linestart;
            ostream << console::escodes::linewipe;
          }

          return do_render<Pos + 1>(); // tail recursive
        }

        void do_halt( bool forced ) noexcept final
        { // This virtual function is invoked only via the vtable,
          // hence the default arguments from the base class declaration are always used.
          // Any default arguments provided in the derived class are ignored.
          if ( online() ) {
            auto& executor = render::Renderer<Outlet, Mode>::itself();
            __PGBAR_ASSERT( executor.empty() == false );
            std::lock_guard<std::mutex> lock { sched_mtx_ };
            if ( !forced )
              executor.attempt();
            if ( alive_cnt_.fetch_sub( 1, std::memory_order_acq_rel ) == 1 ) {
              state_.store( State::Stop, std::memory_order_release );
              executor.appoint_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
            }
          }
        }
        void do_boot() & final
        {
          std::lock_guard<std::mutex> lock { sched_mtx_ };
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          if ( state_.load( std::memory_order_acquire ) == State::Stop ) {
            if ( !executor.try_appoint( [this]() {
                   auto& ostream    = io::OStream<Outlet>::itself();
                   const auto istty = console::TermContext<Outlet>::itself().connected();
                   switch ( state_.load( std::memory_order_acquire ) ) {
                   case State::Awake: {
                     active_mask_.store( {}, std::memory_order_release );
                     if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
                       if ( istty )
                         ostream << console::escodes::savecursor;
                     do_render();
                     ostream << io::flush;

                     auto expected = State::Awake;
                     state_.compare_exchange_strong( expected, State::Refresh, std::memory_order_release );
                   } break;
                   case State::Refresh: {
                     if ( istty ) {
                       if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
                         ostream << console::escodes::resetcursor;
                       else
                         ostream
                           .append( console::escodes::prevline,
                                    active_mask_.load( std::memory_order_acquire ).count() )
                           .append( console::escodes::linestart );
                     }
                     do_render();
                     ostream << io::flush;
                   } break;
                   default: break;
                   }
                 } ) )
              __PGBAR_UNLIKELY throw exception::InvalidState(
                "pgbar: another progress bar instance is already running" );

            io::OStream<Outlet>::itself() << io::release;
            state_.store( State::Awake, std::memory_order_release );
            try {
              executor.activate();
            } catch ( ... ) {
              state_.store( State::Stop, std::memory_order_release );
              executor.appoint();
              throw;
            }
          } else
            executor.attempt();
          alive_cnt_.fetch_add( 1, std::memory_order_release );
          __PGBAR_ASSERT( alive_cnt_ <= sizeof...( Configs ) );
        }

        template<types::Size Pos>
        using ElementAt_t =
          traits::TypeAt_t<Pos, TupleSlot<Tags, prefabs::BasicBar<Configs, Outlet, Mode, Area>>...>;

        template<typename Tuple, types::Size... Is>
        TupleBar( Tuple&& tup, const traits::IndexSeq<Is...>& )
          noexcept( std::tuple_size<typename std::decay<Tuple>::type>::value == sizeof...( Configs ) )
          : ElementAt_t<Is>( utils::forward_or<Is, ElementAt_t<Is>>( std::forward<Tuple>( tup ) ) )...
          , alive_cnt_ { 0 }
          , state_ { State::Stop }
        {
          static_assert( std::tuple_size<typename std::decay<Tuple>::type>::value <= sizeof...( Is ),
                         "pgbar::__details::assets::TupleBar::ctor: Unexpected tuple size mismatch" );
        }

      public:
        template<types::Size... Is, typename... Cs, Channel O, Policy M, Region A>
        TupleBar( const TupleSlot<Is, prefabs::BasicBar<Cs, O, M, A>>&... ) = delete;

        // SFINAE is used here to prevent infinite recursive matching of errors.
        template<typename Cfg,
                 typename... Cfgs,
                 typename = typename std::enable_if<
                   traits::AllOf<traits::is_config<typename std::decay<Cfg>::type>,
                                 traits::is_config<typename std::decay<Cfgs>::type>...,
                                 traits::StartsWith<traits::TypeList<typename std::decay<Cfg>::type,
                                                                     typename std::decay<Cfgs>::type...>,
                                                    Configs...>>::value>::type>
        TupleBar( Cfg&& cfg, Cfgs&&... cfgs ) noexcept( sizeof...( Cfgs ) + 1 == sizeof...( Configs ) )
          : TupleBar( std::forward_as_tuple( std::forward<Cfg>( cfg ), std::forward<Cfgs>( cfgs )... ),
                      traits::MakeIndexSeq<sizeof...( Cfgs ) + 1>() )
        {}
        template<typename... Cfgs,
                 typename = typename std::enable_if<
                   traits::StartsWith<traits::TypeList<Cfgs...>, Configs...>::value>::type>
        TupleBar( prefabs::BasicBar<Cfgs, Outlet, Mode, Area>&&... bars )
          noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
          : TupleBar( std::forward_as_tuple( std::move( bars )... ),
                      traits::MakeIndexSeq<sizeof...( Cfgs )>() )
        {}
        TupleBar( const TupleBar& )              = delete;
        TupleBar& operator=( const TupleBar& ) & = delete;
        TupleBar( TupleBar&& rhs ) noexcept
          : TupleSlot<Tags, prefabs::BasicBar<Configs, Outlet, Mode, Area>>( std::move( rhs ) )...
          , alive_cnt_ { 0 }
          , state_ { State::Stop }
        {}
        TupleBar& operator=( TupleBar&& ) & = default;
        ~TupleBar()                         = default;

        void halt() noexcept
        {
          if ( online() && !__details::render::Renderer<Outlet, Mode>::itself().empty() )
            (void)std::initializer_list<char> { ( this->ElementAt_t<Tags>::do_reset( true ), '\0' )... };
          __PGBAR_ASSERT( alive_cnt_ == 0 );
          __PGBAR_ASSERT( online() == false );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool online() const noexcept
        {
          return state_.load( std::memory_order_acquire ) != State::Stop;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size active_size() const noexcept
        {
          return active_mask_.load( std::memory_order_acquire ).count();
        }

        void swap( TupleBar& rhs ) noexcept
        {
          __PGBAR_TRUST( this != &rhs );
          __PGBAR_ASSERT( online() == false );
          __PGBAR_ASSERT( rhs.online() == false );
          (void)std::initializer_list<char> {
            ( this->ElementAt_t<Tags>::swap( static_cast<ElementAt_t<Tags>&>( rhs ) ), '\0' )...
          };
        }

        template<types::Size Pos>
        __PGBAR_INLINE_FN ElementAt_t<Pos>& at() & noexcept
        {
          return static_cast<ElementAt_t<Pos>&>( *this );
        }
        template<types::Size Pos>
        __PGBAR_INLINE_FN const ElementAt_t<Pos>& at() const& noexcept
        {
          return static_cast<const ElementAt_t<Pos>&>( *this );
        }
        template<types::Size Pos>
        __PGBAR_INLINE_FN ElementAt_t<Pos>&& at() && noexcept
        {
          return static_cast<ElementAt_t<Pos>&&>( std::move( *this ) );
        }
      };
    } // namespace assets
  } // namespace __details

  template<typename Bar, typename... Bars>
  class MultiBar;
  template<typename Config, typename... Configs, Channel O, Policy M, Region A>
  class MultiBar<__details::prefabs::BasicBar<Config, O, M, A>,
                 __details::prefabs::BasicBar<Configs, O, M, A>...> {
    static_assert( __details::traits::AllOf<__details::traits::is_config<Config>,
                                            __details::traits::is_config<Configs>...>::value,
                   "pgbar::MultiBar: Invalid type" );
    using Self    = MultiBar;
    using Package = __details::assets::TupleBar<__details::traits::MakeIndexSeq<sizeof...( Configs ) + 1>,
                                                __details::prefabs::BasicBar<Config, O, M, A>,
                                                __details::prefabs::BasicBar<Configs, O, M, A>...>;

    template<__details::types::Size Pos>
    using ConfigAt_t = __details::traits::TypeAt_t<Pos, Config, Configs...>;
    template<__details::types::Size Pos>
    using BarAt_t = __details::traits::TypeAt_t<Pos,
                                                __details::prefabs::BasicBar<Config, O, M, A>,
                                                __details::prefabs::BasicBar<Configs, O, M, A>...>;

    Package tuple_;

  public:
    MultiBar() = default;

# if __PGBAR_CXX20
    template<typename Cfg, typename... Cfgs>
      requires( sizeof...( Cfgs ) <= sizeof...( Configs )
                && __details::traits::StartsWith<
                  __details::traits::TypeList<std::decay_t<Cfg>, std::decay_t<Cfgs>...>,
                  Config,
                  Configs...>::value )
# else
    template<typename Cfg,
             typename... Cfgs,
             typename = typename std::enable_if<__details::traits::AllOf<
               std::integral_constant<bool, ( sizeof...( Cfgs ) <= sizeof...( Configs ) )>,
               __details::traits::StartsWith<__details::traits::TypeList<typename std::decay<Cfg>::type,
                                                                         typename std::decay<Cfgs>::type...>,
                                             Config,
                                             Configs...>>::value>::type>
# endif
    MultiBar( Cfg&& cfg, Cfgs&&... cfgs ) noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
      : tuple_ { std::forward<Cfg>( cfg ), std::forward<Cfgs>( cfgs )... }
    {}

    template<typename Cfg, typename... Cfgs
# if __PGBAR_CXX20
             >
      requires( sizeof...( Cfgs ) <= sizeof...( Configs ) && __details::traits::is_config<Cfg>::value
                && ( __details::traits::is_config<Cfgs>::value && ... )
                && __details::traits::
                  StartsWith<__details::traits::TypeList<Cfg, Cfgs...>, Config, Configs...>::value )
# else
             ,
             typename = typename std::enable_if<__details::traits::AllOf<
               std::integral_constant<bool, ( sizeof...( Cfgs ) <= sizeof...( Configs ) )>,
               __details::traits::is_config<Cfg>,
               __details::traits::is_config<Cfgs>...,
               __details::traits::StartsWith<
                 __details::traits::TypeList<Cfg,
                 Cfgs...>,
                 Config,
                 Configs...>>::value>::type>
# endif
    MultiBar( __details::prefabs::BasicBar<Cfg, O, M, A>&& bar,
              __details::prefabs::BasicBar<Cfgs, O, M, A>&&... bars )
      noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
      : tuple_ { std::move( bar ), std::move( bars )... }
    {}

    MultiBar( const Self& )          = delete;
    Self& operator=( const Self& ) & = delete;
    MultiBar( Self&& rhs ) noexcept : tuple_ { std::move( rhs.tuple_ ) }
    {
      __PGBAR_ASSERT( rhs.active() == false );
    }
    Self& operator=( Self&& rhs ) & noexcept
    {
      __PGBAR_TRUST( this != &rhs );
      __PGBAR_ASSERT( active() == false );
      __PGBAR_ASSERT( rhs.active() == false );
      tuple_ = std::move( rhs );
      return *this;
    }
    ~MultiBar() noexcept { reset(); }

    // Check whether a progress bar is running
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept { return tuple_.online(); }
    // Reset all the progress bars.
    __PGBAR_INLINE_FN void reset() noexcept { tuple_.halt(); }
    // Returns the number of progress bars.
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CNSTEVAL __details::types::Size size() const noexcept
    {
      return sizeof...( Configs ) + 1;
    }
    // Returns the number of progress bars which is running.
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size active_size() const noexcept
    {
      return tuple_.active_size();
    }
    // Wait for all progress bars to stop.
    void wait() const noexcept
    {
      __details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for all progress bars to stop or time out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return __details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }

    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN BarAt_t<Pos>& at() & noexcept
    {
      return tuple_.template at<Pos>();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN const BarAt_t<Pos>& at() const& noexcept
    {
      return static_cast<const Package&>( tuple_ ).template at<Pos>();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN BarAt_t<Pos>&& at() && noexcept
    {
      return std::move( tuple_ ).template at<Pos>();
    }

    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick() &
    {
      at<Pos>().tick();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick( std::uint64_t next_step ) &
    {
      at<Pos>().tick( next_step );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick_to( std::uint8_t percentage ) &
    {
      at<Pos>().tick_to( percentage );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void reset()
    {
      at<Pos>().reset();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void wait() const noexcept
    {
      at<Pos>().wait();
    }
    template<__details::types::Size Pos, class Rep, class Period>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool wait_for(
      const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return at<Pos>().wait_for( timeout );
    }
    template<__details::types::Size Pos>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
    {
      return at<Pos>().active();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN ConfigAt_t<Pos>& config() &
    {
      return at<Pos>().config();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN const ConfigAt_t<Pos>& config() const&
    {
      return at<Pos>().config();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN ConfigAt_t<Pos>&& config() &&
    {
      return at<Pos>().config();
    }

    template<__details::types::Size Pos, typename... Args
# if __PGBAR_CXX20
             >
      requires __details::traits::is_iterable_bar<BarAt_t<Pos>>::value
# else
             ,
      typename = typename std::enable_if<__details::traits::is_iterable_bar<BarAt_t<Pos>>::value>::type>
# endif
    __PGBAR_INLINE_FN auto iterate( Args&&... args ) & noexcept(
      noexcept( std::declval<Self&>().template at<Pos>().iterate( std::forward<Args>( args )... ) ) )
      -> decltype( std::declval<Self&>().template at<Pos>().iterate( std::forward<Args>( args )... ) )
    {
      return at<Pos>().iterate( std::forward<Args>( args )... );
    }

    void swap( Self& rhs ) noexcept { tuple_.swap( rhs.tuple_ ); }
    friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

    template<__details::types::Size Pos, typename Mb>
    friend __PGBAR_INLINE_FN constexpr
      typename std::enable_if<std::is_same<typename std::decay<Mb>::type, Self>::value,
                              decltype( std::declval<Mb>().template at<Pos>() )>::type
      get( Mb&& self ) noexcept
    {
      return std::forward<Mb>( self ).template at<Pos>();
    }
  };

# if __PGBAR_CXX17
  // CTAD, only generates the default version,
  // which means the the Outlet is `Channel::Stderr` and Mode is `Policy::Async`.
  template<typename Config, typename... Configs
#  if __PGBAR_CXX20
           >
    requires( __details::traits::is_config<std::decay_t<Config>>::value
              && ( __details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
#  else
           ,
    typename = std::enable_if_t<__details::traits::AllOf<__details::traits::is_config<std::decay_t<Config>>,
                                                         __details::traits::is_config<std::decay_t<Configs>>...>::value>>
#  endif
  MultiBar( Config&&, Configs&&... )
    -> MultiBar<__details::prefabs::BasicBar<Config, Channel::Stderr, Policy::Async, Region::Fixed>,
                __details::prefabs::BasicBar<Configs, Channel::Stderr, Policy::Async, Region::Fixed>...>;
# endif

  // Creates a MultiBar using existing bar instances.
  template<typename Config, typename... Configs, Channel Outlet, Policy Mode, Region Area>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN MultiBar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>,
                                               __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>...>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    typename std::enable_if<__details::traits::AllOf<__details::traits::is_config<Config>,
                                                     __details::traits::is_config<Configs>...>::value,
                            MultiBar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>,
                                     __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>...>>::type
# endif
    make_multi( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar,
                __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars ) noexcept
  {
    return { std::move( bar ), std::move( bars )... };
  }
  // Creates a MultiBar using configuration objects.
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config,
           typename... Configs>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<std::decay_t<Config>>::value
              && ( __details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
  MultiBar<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>,
           __details::prefabs::BasicBar<std::decay_t<Configs>, Outlet, Mode, Area>...>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<typename std::decay<Config>::type>,
                             __details::traits::is_config<typename std::decay<Configs>::type>...>::value,
    MultiBar<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>,
             __details::prefabs::BasicBar<typename std::decay<Configs>::type, Outlet, Mode, Area>...>>::type
# endif
    make_multi( Config&& cfg, Configs&&... cfgs ) noexcept(
      __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Config>,
                                                      std::is_lvalue_reference<Configs>...>>::value )
  {
    return { std::forward<Config>( cfg ), std::forward<Configs>( cfgs )... };
  }

  namespace __details {
    namespace assets {
      template<types::Size Cnt, Channel O, Policy M, Region A, typename C, types::Size... Is>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN
        traits::Repeat_t<prefabs::BasicBar<typename std::decay<C>::type, O, M, A>, MultiBar, Cnt>
        make_multi_helper( C&& cfg, const traits::IndexSeq<Is...>& )
          noexcept( __details::traits::AllOf<std::integral_constant<bool, ( Cnt == 1 )>,
                                             __details::traits::Not<std::is_lvalue_reference<C>>>::value )
      {
        std::array<C, Cnt - 1> cfgs { { ( (void)( Is ), cfg )... } };
        return { std::forward<C>( cfg ), std::move( cfgs[Is] )... };
      }
    }
  } // namespace __details

  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single bar object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<__details::types::Size Cnt, typename Config, Channel Outlet, Policy Mode, Region Area>
# if __PGBAR_CXX20
    requires( Cnt > 0 && __details::traits::is_config<Config>::value )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             __details::traits::is_config<Config>>::value,
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>>::
    type
# endif
    make_multi( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar ) noexcept( Cnt == 1 )
  {
    return __details::assets::make_multi_helper<Cnt, Outlet, Mode>(
      std::move( bar ).config(),
      __details::traits::MakeIndexSeq<Cnt - 1>() );
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single configuration object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<__details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config>
# if __PGBAR_CXX20
    requires( Cnt > 0 && __details::traits::is_config<std::decay_t<Config>>::value )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::traits::
    Repeat_t<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             __details::traits::is_config<typename std::decay<Config>::type>>::value,
    __details::traits::Repeat_t<
      __details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>,
      MultiBar,
      Cnt>>::type
# endif
    make_multi( Config&& cfg )
      noexcept( __details::traits::AllOf<std::integral_constant<bool, ( Cnt == 1 )>,
                                         __details::traits::Not<std::is_lvalue_reference<Config>>>::value )
  {
    return __details::assets::make_multi_helper<Cnt, Outlet, Mode>(
      std::forward<Config>( cfg ),
      __details::traits::MakeIndexSeq<Cnt - 1>() );
  }

  /**
   * Creates a MultiBar with a fixed number of bars using mutiple bar/configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided arguments;
   * **any remaining instances with no corresponding arguments will be default-initialized.**
   */
  template<typename Bar, __details::types::Size Cnt, typename... Objs>
# if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Objs ) <= Cnt && __details::traits::is_bar<Bar>::value
              && ( ( ( std::is_same_v<std::remove_cv_t<Bar>, std::decay_t<Objs>> && ... )
                     && !( std::is_lvalue_reference_v<Objs> || ... ) )
                   || ( std::is_same_v<typename Bar::Config, std::decay_t<Objs>> && ... ) ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::traits::Repeat_t<Bar, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<
      std::integral_constant<bool, ( Cnt > 0 )>,
      std::integral_constant<bool, ( sizeof...( Objs ) <= Cnt )>,
      __details::traits::is_bar<Bar>,
      __details::traits::AnyOf<
        __details::traits::AllOf<
          std::is_same<typename std::remove_cv<Bar>::type, typename std::decay<Objs>::type>...,
          __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Objs>...>>>,
        __details::traits::AllOf<std::is_same<typename Bar::Config, typename std::decay<Objs>::type>...>>>::
      value,
    __details::traits::Repeat_t<Bar, MultiBar, Cnt>>::type
# endif
    make_multi( Objs&&... objs ) noexcept( sizeof...( Objs ) == Cnt )
  {
    return { std::forward<Objs>( objs )... };
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using mutiple configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided configurations;
   * **any remaining instances with no corresponding configurations will be default-initialized.**
   */
  template<typename Config,
           __details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
# if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && __details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             std::integral_constant<bool, ( sizeof...( Configs ) <= Cnt )>,
                             __details::traits::is_config<Config>,
                             std::is_same<Config, typename std::decay<Configs>::type>...>::value,
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>>::
    type
# endif
    make_multi( Configs&&... configs ) noexcept( sizeof...( Configs ) == Cnt )
  {
    return { std::forward<Configs>( configs )... };
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using mutiple bar objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided objects;
   * **any remaining instances with no corresponding configurations will be default-initialized.**
   */
  template<typename Config,
           __details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
# if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && __details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<std::integral_constant<bool, ( Cnt > 0 )>,
                             std::integral_constant<bool, ( sizeof...( Configs ) <= Cnt )>,
                             __details::traits::is_config<Config>,
                             std::is_same<Config, typename std::decay<Configs>::type>&&...>::value,
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>>::
    type
# endif
    make_multi( __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars )
      noexcept( sizeof...( Configs ) == Cnt )
  {
    return { std::move( bars )... };
  }

  template<typename Bar, typename N, typename F, typename... Options>
# if __PGBAR_CXX20
    requires( __details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> && std::is_arithmetic_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<__details::traits::is_iterable_bar<Bar>,
                                                                     std::is_constructible<Bar, Options...>,
                                                                     std::is_arithmetic<N>>::value>::type
# endif
    iterate( N startpoint, N endpoint, N step, F&& unary_fn, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( startpoint, endpoint, step, std::forward<F>( unary_fn ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename F,
           typename... Options>
# if __PGBAR_CXX20
    requires(
      __details::traits::is_config<Config>::value
      && __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> && std::is_arithmetic_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<
    __details::traits::is_config<Config>,
    __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>,
    std::is_arithmetic<N>>::value>::type
# endif
    iterate( N startpoint, N endpoint, N step, F&& unary_fn, Options&&... options )
  {
    iterate<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>(
      startpoint,
      endpoint,
      step,
      std::forward<F>( unary_fn ),
      Config( std::forward<Options>( options )... ) );
  }

  template<typename Bar, typename N, typename F, typename... Options>
# if __PGBAR_CXX20
    requires( __details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> && std::is_floating_point_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<__details::traits::is_iterable_bar<Bar>,
                                                                     std::is_constructible<Bar, Options...>,
                                                                     std::is_floating_point<N>>::value>::type
# endif
    iterate( N endpoint, N step, F&& unary_fn, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( endpoint, step, std::forward<F>( unary_fn ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename F,
           typename... Options>
# if __PGBAR_CXX20
    requires(
      __details::traits::is_config<Config>::value
      && __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> && std::is_floating_point_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<
    __details::traits::is_config<Config>,
    __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>,
    std::is_floating_point<N>>::value>::type
# endif
    iterate( N endpoint, N step, F&& unary_fn, Options&&... options )
  {
    iterate<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>(
      endpoint,
      step,
      std::forward<F>( unary_fn ),
      Config( std::forward<Options>( options )... ) );
  }

  template<typename Bar, typename N, typename F, typename... Options>
# if __PGBAR_CXX20
    requires( __details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> && std::is_integral_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<__details::traits::is_iterable_bar<Bar>,
                                                                     std::is_constructible<Bar, Options...>,
                                                                     std::is_integral<N>>::value>::type
# endif
    iterate( N startpoint, N endpoint, F&& unary_fn, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( startpoint, endpoint, std::forward<F>( unary_fn ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename F,
           typename... Options>
# if __PGBAR_CXX20
    requires(
      __details::traits::is_config<Config>::value
      && __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> && std::is_integral_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<
    __details::traits::is_config<Config>,
    __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>,
    std::is_integral<N>>::value>::type
# endif
    iterate( N startpoint, N endpoint, F&& unary_fn, Options&&... options )
  {
    iterate<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>(
      startpoint,
      endpoint,
      std::forward<F>( unary_fn ),
      Config( std::forward<Options>( options )... ) );
  }

  template<typename Bar, typename N, typename F, typename... Options>
# if __PGBAR_CXX20
    requires( __details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> && std::is_integral_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<__details::traits::is_iterable_bar<Bar>,
                                                                     std::is_constructible<Bar, Options...>,
                                                                     std::is_integral<N>>::value>::type
# endif
    iterate( N endpoint, F&& unary_fn, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( endpoint, std::forward<F>( unary_fn ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename F,
           typename... Options>
# if __PGBAR_CXX20
    requires(
      __details::traits::is_config<Config>::value
      && __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> && std::is_integral_v<N> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<
    __details::traits::is_config<Config>,
    __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>,
    std::is_integral<N>>::value>::type
# endif
    iterate( N endpoint, F&& unary_fn, Options&&... options )
  {
    iterate<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>(
      endpoint,
      std::forward<F>( unary_fn ),
      Config( std::forward<Options>( options )... ) );
  }

  template<typename Bar, typename I, typename F, typename... Options>
# if __PGBAR_CXX20
    requires( __details::traits::is_sized_iterator<I>::value && __details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_sized_iterator<I>,
                             __details::traits::is_iterable_bar<Bar>,
                             std::is_constructible<Bar, Options...>,
                             __details::traits::Not<std::is_arithmetic<I>>>::value>::type
# endif
    iterate( I startpoint, I endpoint, F&& unary_fn, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( std::move( startpoint ), std::move( endpoint ), std::forward<F>( unary_fn ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename I,
           typename F,
           typename... Options>
# if __PGBAR_CXX20
    requires(
      __details::traits::is_sized_iterator<I>::value && __details::traits::is_config<Config>::value
      && __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<
    __details::traits::is_sized_iterator<I>,
    __details::traits::is_config<Config>,
    __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>,
    __details::traits::Not<std::is_arithmetic<I>>>::value>::type
# endif
    iterate( I startpoint, I endpoint, F&& unary_fn, Options&&... options )
  {
    iterate<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>(
      std::move( startpoint ),
      std::move( endpoint ),
      std::forward<F>( unary_fn ),
      Config( std::forward<Options>( options )... ) );
  }

  template<typename Bar, class R, typename F, typename... Options>
# if __PGBAR_CXX20
    requires( __details::traits::is_bounded_range<R>::value && __details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN
    typename std::enable_if<__details::traits::AllOf<__details::traits::is_bounded_range<R>,
                                                     __details::traits::is_iterable_bar<Bar>,
                                                     std::is_constructible<Bar, Options...>>::value>::type
# endif
    iterate( R&& range, F&& unary_fn, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( std::forward<R>( range ), std::forward<F>( unary_fn ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           class R,
           typename F,
           typename... Options>
# if __PGBAR_CXX20
    requires(
      __details::traits::is_bounded_range<R>::value && __details::traits::is_config<Config>::value
      && __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  __PGBAR_INLINE_FN void
# else
  __PGBAR_INLINE_FN typename std::enable_if<__details::traits::AllOf<
    __details::traits::is_bounded_range<R>,
    __details::traits::is_config<Config>,
    __details::traits::is_iterable_bar<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
# endif
    iterate( R&& range, F&& unary_fn, Options&&... options )
  {
    iterate<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>(
      std::forward<R>( range ),
      std::forward<F>( unary_fn ),
      Config( std::forward<Options>( options )... ) );
  }

  namespace slice {
    /**
     * A range that contains a bar object and an unidirectional abstract range,
     * which transforms the iterations in the abstract into a visual display of the object.
     */
    template<typename R, typename B>
    class ProxySpan
# if __PGBAR_CXX20
      : public std::ranges::view_interface<ProxySpan<R, B>>
# endif
    {
      static_assert( __details::traits::is_bounded_range<R>::value,
                     "pgbar::slice::ProxySpan: Only available for bounded ranges" );
      static_assert( __details::traits::is_iterable_bar<B>::value,
                     "pgbar::slice::ProxySpan: Must have a method to configure the iteration "
                     "count for the object's configuration type" );

      B* itr_bar_;
      R itr_range_;

# if __PGBAR_CXX20
      using Stnl = std::ranges::sentinel_t<R>;
# elif __PGBAR_CXX17
      using Stnl = __details::traits::IteratorOf_t<R>;
# endif

    public:
      class iterator;
# if __PGBAR_CXX17
      class sentinel {
        Stnl end_;
        friend class iterator;

      public:
        constexpr sentinel() = default;
        constexpr sentinel( Stnl&& end ) noexcept( std::is_nothrow_move_constructible<Stnl>::value )
          : end_ { std::move( end ) }
        {}
      };
# else
      using sentinel = iterator;
# endif

      class iterator {
        using Iter = __details::traits::IteratorOf_t<R>;
        Iter itr_;
        B* itr_bar_;

      public:
        using iterator_category = typename std::conditional<
          __details::traits::AnyOf<
            std::is_same<typename std::iterator_traits<Iter>::iterator_category, std::input_iterator_tag>,
            std::is_same<typename std::iterator_traits<Iter>::iterator_category,
                         std::output_iterator_tag>>::value,
          typename std::iterator_traits<Iter>::iterator_category,
          std::forward_iterator_tag>::type;
# if __PGBAR_CXX20
        using value_type      = std::iter_value_t<Iter>;
        using difference_type = std::iter_difference_t<Iter>;
        using reference       = std::iter_reference_t<Iter>;
# else
        using value_type      = typename std::iterator_traits<Iter>::value_type;
        using difference_type = typename std::iterator_traits<Iter>::difference_type;
        using reference       = typename std::iterator_traits<Iter>::reference;
# endif
        using pointer = Iter;

        constexpr iterator() noexcept( std::is_nothrow_default_constructible<Iter>::value )
          : itr_ {}, itr_bar_ { nullptr }
        {}
        __PGBAR_CXX17_CNSTXPR iterator( Iter itr ) noexcept( std::is_nothrow_move_constructible<Iter>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { nullptr }
        {}
        __PGBAR_CXX17_CNSTXPR iterator( Iter itr, B& itr_bar )
          noexcept( std::is_nothrow_move_constructible<Iter>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { std::addressof( itr_bar ) }
        {}
        __PGBAR_CXX17_CNSTXPR iterator( iterator&& rhs )
          noexcept( std::is_nothrow_move_constructible<Iter>::value )
          : itr_ { std::move( rhs.itr_ ) }, itr_bar_ { rhs.itr_bar_ }
        {
          rhs.itr_bar_ = nullptr;
        }
        __PGBAR_CXX17_CNSTXPR iterator& operator=( iterator&& rhs ) & noexcept(
          std::is_nothrow_move_assignable<Iter>::value )
        {
          __PGBAR_TRUST( this != &rhs );
          itr_         = std::move( rhs.itr_ );
          itr_bar_     = rhs.itr_bar_;
          rhs.itr_bar_ = nullptr;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR ~iterator() = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() &
        {
          __PGBAR_TRUST( itr_bar_ != nullptr );
          ++itr_;
          itr_bar_->tick();
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) &
        {
          __PGBAR_TRUST( itr_bar_ != nullptr );
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR reference operator*() const
        {
          return *itr_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR pointer operator->() const { return itr_; }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==( const Iter& lhs ) const
        {
          return itr_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=( const Iter& lhs ) const
        {
          return itr_ != lhs;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const iterator& a,
                                                                              const iterator& b )
        {
          return a.itr_ == b.itr_;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b )
        {
          return !( a == b );
        }

# if __PGBAR_CXX17
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==( const sentinel& b ) const
        {
          return itr_ == b.end_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=( const sentinel& b ) const
        {
          return !( *this == b );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator==( const sentinel& a,
                                                                              const iterator& b )
        {
          return b == a;
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN constexpr bool operator!=( const sentinel& a,
                                                                              const iterator& b )
        {
          return !( b == a );
        }
# endif

        constexpr explicit operator bool() const noexcept { return itr_bar_ != nullptr; }
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
        __PGBAR_TRUST( this != &rhs );
        swap( rhs );
        rhs.itr_bar_ = nullptr;
        return *this;
      }
      // Intentional non-virtual destructors.
      __PGBAR_CXX20_CNSTXPR ~ProxySpan() = default;

      // This function will CHANGE the state of the pgbar object it holds.
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator begin() &
      {
        itr_bar_->config().tasks( itr_range_.size() );
        return { itr_range_.begin(), *itr_bar_ };
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR sentinel end() const
      {
        return { itr_range_.end() };
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR bool empty() const noexcept
      {
        return itr_bar_ == nullptr;
      }

      __PGBAR_CXX14_CNSTXPR void swap( ProxySpan<R, B>& lhs ) noexcept
      {
        __PGBAR_TRUST( this != &lhs );
        std::swap( itr_bar_, lhs.itr_bar_ );
        itr_range_.swap( lhs.itr_range_ );
      }
      friend __PGBAR_CXX14_CNSTXPR void swap( ProxySpan<R, B>& a, ProxySpan<R, B>& b ) noexcept
      {
        a.swap( b );
      }
      constexpr explicit operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice

  namespace __details {
    namespace assets {
      template<Channel, Policy, Region>
      class DynamicContext;
    }

    namespace prefabs {
      template<typename C, Channel O, Policy M, Region A>
      class ManagedBar final : public BasicBar<C, O, M, A> {
        using Base    = BasicBar<C, O, M, A>;
        using Context = std::shared_ptr<assets::DynamicContext<O, M, A>>;

        Context context_;

        void do_halt( bool forced ) noexcept final { context_->pop( this, forced ); }
        void do_boot() & final { context_->append( this ); }

      public:
        ManagedBar( Context context, C&& config ) noexcept
          : Base( std::move( config ) ), context_ { std::move( context ) }
        {}
        ManagedBar( Context context, BasicBar<C, O, M, A>&& bar ) noexcept
          : Base( std::move( bar ) ), context_ { std::move( context ) }
        {}
        template<typename... Args>
        ManagedBar( Context context, Args&&... args )
          : Base( std::forward<Args>( args )... ), context_ { std::move( context ) }
        {}

        /**
         * This thing is always wrapped by `std::unique_ptr` under normal circumstances,
         * so there is no need to add move semantics support for it;
         * otherwise, additional null checks would be required in the methods.
         */
        ManagedBar( const ManagedBar& )              = delete;
        ManagedBar& operator=( const ManagedBar& ) & = delete;

        /**
         * The object model of C++ requires that derived classes be destructed first.
         * When the derived class is destructed and the base class destructor attempts to call `do_reset`,
         * the internal virtual function `do_halt` will point to a non-existent derived class.

         * Therefore, here it is necessary to explicitly re-call the base class's `do_reset`
         * to shut down any possible running state.

         * After calling `do_reset`, the object state will switch to Stop,
         * and further calls to `do_reset` will have no effect.
         * So even if the base class destructor calls `do_reset` again, it is safe.
         */
        virtual ~ManagedBar() noexcept { halt(); }

        void halt() noexcept { this->do_reset( true ); }
      };
    } // namespace prefabs

    namespace assets {
      template<Channel Outlet, Policy Mode, Region Area>
      class DynamicContext final {
        struct Slot final {
        private:
          using Self = Slot;

          template<typename Derived>
          static void halt( Indicator* item ) noexcept
          {
            static_assert(
              traits::AllOf<std::is_base_of<Indicator, Derived>, traits::is_bar<Derived>>::value,
              "pgbar::__details::assets::DynamicContext::Slot::halt: Derived must inherit from Indicator" );
            __PGBAR_TRUST( item != nullptr );
            static_cast<Derived*>( item )->halt();
          }
          template<typename Derived>
          static void render( Indicator* item )
          {
            static_assert(
              traits::AllOf<std::is_base_of<Indicator, Derived>, traits::is_bar<Derived>>::value,
              "pgbar::__details::assets::DynamicContext::Slot::render: Derived must inherit from Indicator" );
            __PGBAR_TRUST( item != nullptr );
            make_frame( static_cast<Derived&>( *item ) );
          }

        public:
          void ( *halt_ )( Indicator* ) noexcept;
          void ( *render_ )( Indicator* );
          Indicator* target_;

          template<typename Config>
          Slot( prefabs::ManagedBar<Config, Outlet, Mode, Area>* item ) noexcept
            : halt_ { halt<prefabs::ManagedBar<Config, Outlet, Mode, Area>> }
            , render_ { render<prefabs::BasicBar<Config, Outlet, Mode, Area>> }
            , target_ { item }
          {}
        };

        enum class State : std::uint8_t { Stop, Awake, Refresh };
        std::atomic<State> state_;
        std::vector<Slot> items_;

        // If Area is equal to Region::Fixed,
        // the variable represents the number of lines that need to be discarded;
        // If Area is equal to Region::Relative,
        // the variable represents the number of nextlines output last time.
        std::atomic<types::Size> num_modified_lines_;
        mutable concurrent::SharedMutex res_mtx_;
        mutable std::mutex sched_mtx_;

        void do_render() &
        {
          auto& ostream        = io::OStream<Outlet>::itself();
          const auto istty     = console::TermContext<Outlet>::itself().connected();
          const auto hide_done = config::hide_completed();

          bool any_alive = false, any_rendered = false;
          for ( types::Size i = 0; i < items_.size(); ++i ) {
            bool this_rendered = false;
            // If items_[i].target_ is equal to nullptr,
            // indicating that the i-th object has stopped.
            if ( items_[i].target_ != nullptr ) {
              this_rendered = any_rendered = true;
              if ( istty && !hide_done )
                ostream << console::escodes::linewipe;
              ( *items_[i].render_ )( items_[i].target_ );

              const auto is_alive = items_[i].target_->active();
              any_alive |= is_alive;
              if ( !is_alive )
                items_[i].target_ = nullptr; // mark that it should stop rendering
            }

            /**
             * When Area is equal to Region::Fixed, the row discard policy is as follows:
             * After eliminating the first k consecutive stopped items in the render queue item_
             * (via the eliminate method), the starting area for rendering is moved down by k rows.
             * Therefore, at this point,
             * all remaining items that have not been removed should trigger a line break for rendering.

             * When Area is equal to Region::Relative, the row discard policy is as follows:
             * During the rendering process,
             * count the number of consecutive line breaks output starting from the first rendered item,
             * denoted as n.
             * In the next round of rendering, move up by n rows.
             * Therefore, at this point,
             * it is necessary to track which items in the render queue have been rendered
             * and whether any items have been rendered in the current round of rendering.

             * If the output stream is not bound to a terminal, there is no line discard policy;
             * all rendered items will trigger a newline character to be rendered.
             */
            if __PGBAR_CXX17_CNSTXPR ( Area == Region::Relative )
              if ( !any_rendered && !this_rendered )
                continue;
            if ( ( !istty && this_rendered )
                 || ( istty && ( !hide_done || items_[i].target_ != nullptr ) ) ) {
              ostream << console::escodes::nextline;
              if __PGBAR_CXX17_CNSTXPR ( Area == Region::Relative )
                num_modified_lines_.fetch_add( any_alive, std::memory_order_relaxed );
            }
            if ( istty && hide_done ) {
              if ( items_[i].target_ == nullptr )
                ostream << console::escodes::linestart;
              ostream << console::escodes::linewipe;
            }
          }
        }

        void eliminate() noexcept
        {
          // Search for the first k stopped progress bars and remove them.
          auto itr = std::find_if( items_.cbegin(), items_.cend(), []( const Slot& slot ) noexcept {
            return slot.target_ != nullptr;
          } );
          items_.erase( items_.cbegin(), itr );
          if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
            num_modified_lines_.fetch_add( std::distance( items_.cbegin(), itr ), std::memory_order_release );
        }

      public:
        DynamicContext() noexcept : state_ { State::Stop } {}
        DynamicContext( const DynamicContext& )              = delete;
        DynamicContext& operator=( const DynamicContext& ) & = delete;
        ~DynamicContext() noexcept { halt(); }

        void halt() noexcept
        {
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          if ( state_.load( std::memory_order_acquire ) != State::Stop )
            render::Renderer<Outlet, Mode>::itself().appoint();
          state_.store( State::Stop, std::memory_order_release );

          std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
          for ( types::Size i = 0; i < items_.size(); ++i ) {
            if ( items_[i].target_ != nullptr ) {
              __PGBAR_ASSERT( items_[i].halt_ != nullptr );
              ( *items_[i].halt_ )( items_[i].target_ );
            }
          }
          items_.clear();
        }

        template<typename C>
        void append( prefabs::ManagedBar<C, Outlet, Mode, Area>* item ) & noexcept( false )
        {
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          auto& executor     = render::Renderer<Outlet, Mode>::itself();
          bool activate_flag = false;
          {
            concurrent::SharedLock<concurrent::SharedMutex> lock2 { res_mtx_ };
            activate_flag = items_.empty();
          }
          if ( activate_flag ) {
            if ( !executor.try_appoint( [this]() {
                   auto& ostream        = io::OStream<Outlet>::itself();
                   const auto istty     = console::TermContext<Outlet>::itself().connected();
                   const auto hide_done = config::hide_completed();
                   switch ( state_.load( std::memory_order_acquire ) ) {
                   case State::Awake: {
                     if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed ) {
                       if ( istty )
                         ostream << console::escodes::savecursor;
                     }
                     {
                       concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
                       do_render();
                     }
                     ostream << io::flush;
                     auto expected = State::Awake;
                     state_.compare_exchange_strong( expected, State::Refresh, std::memory_order_release );
                   } break;
                   case State::Refresh: {
                     {
                       concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
                       if ( istty ) {
                         if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed ) {
                           ostream << console::escodes::resetcursor;
                           if ( !hide_done ) {
                             const auto num_discarded = num_modified_lines_.load( std::memory_order_acquire );
                             if ( num_discarded > 0 ) {
                               ostream.append( console::escodes::nextline, num_discarded )
                                 .append( console::escodes::savecursor );
                               num_modified_lines_.fetch_sub( num_discarded, std::memory_order_release );
                             }
                           }
                         } else {
                           ostream
                             .append( console::escodes::prevline,
                                      num_modified_lines_.load( std::memory_order_relaxed ) )
                             .append( console::escodes::linestart );
                           num_modified_lines_.store( 0, std::memory_order_relaxed );
                         }
                       }
                       do_render();
                     }
                     ostream << io::flush;
                   } break;
                   default: return;
                   }
                 } ) )
              __PGBAR_UNLIKELY throw exception::InvalidState(
                "pgbar: another progress bar instance is already running" );

            io::OStream<Outlet>::itself() << io::release;
            num_modified_lines_.store( 0, std::memory_order_relaxed );
            state_.store( State::Awake, std::memory_order_release );
            try {
              {
                std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
                items_.emplace_back( item );
              }
              executor.activate();
            } catch ( ... ) {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              items_.clear();
              state_.store( State::Stop, std::memory_order_release );
              throw;
            }
          } else {
            {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              eliminate();
              items_.emplace_back( item );
            }
            executor.attempt();
          }
        }
        void pop( const Indicator* item, bool forced = false ) noexcept
        {
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          __PGBAR_ASSERT( executor.empty() == false );
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          __PGBAR_ASSERT( size() != 0 );
          if ( !forced )
            executor.attempt();

          bool suspend_flag = false;
          {
            std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
            const auto itr = std::find_if( items_.begin(), items_.end(), [item]( const Slot& slot ) noexcept {
              return item == slot.target_;
            } );
            // Mark target_ as empty,
            // and then search for the first k invalid or destructed progress bars and remove them.
            if ( itr != items_.end() )
              itr->target_ = nullptr;
            eliminate();
            suspend_flag = items_.empty();
          }

          if ( suspend_flag ) {
            state_.store( State::Stop, std::memory_order_release );
            executor.appoint_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
          }
        }

        __PGBAR_NODISCARD types::Size size() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
          return items_.size();
        }
      };
    } // namespace assets
  } // namespace __details

  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  class DynamicBar {
    using Self    = DynamicBar;
    using Context = __details::assets::DynamicContext<Outlet, Mode, Area>;

    std::shared_ptr<Context> core_;

  public:
    DynamicBar()                     = default;
    DynamicBar( const Self& )        = delete;
    Self& operator=( const Self& ) & = delete;
    DynamicBar( Self&& )             = default;
    Self& operator=( Self&& ) &      = default;
    ~DynamicBar()                    = default;

    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
    {
      return core_ != nullptr && core_->size() != 0;
    }
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size size() const noexcept
    {
      return core_ == nullptr ? 0 : core_.use_count() - 1;
    }
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size active_size() const noexcept
    {
      return core_ == nullptr ? 0 : core_->size();
    }
    __PGBAR_INLINE_FN void reset() noexcept
    {
      if ( core_ != nullptr )
        core_->halt();
    }

    // Wait until the indicator is Stop.
    __PGBAR_INLINE_FN void wait() const noexcept
    {
      __details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for the indicator is Stop or timed out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool wait_for(
      const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return __details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }

    template<typename Config>
# if __PGBAR_CXX20
      requires __details::traits::is_config<Config>::value
    __PGBAR_NODISCARD std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<__details::traits::is_config<Config>::value,
                              std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>::type
# endif
      insert( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<__details::prefabs::ManagedBar<Config, Outlet, Mode, Area>>(
        core_,
        std::move( bar ) );
    }
    template<typename Config>
# if __PGBAR_CXX20
      requires __details::traits::is_config<std::decay_t<Config>>::value
    __PGBAR_NODISCARD std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>>
# else
    __PGBAR_NODISCARD typename std::enable_if<
      __details::traits::is_config<typename std::decay<Config>::type>::value,
      std::unique_ptr<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>>::
      type
# endif
      insert( Config&& cfg )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<
        __details::prefabs::ManagedBar<typename std::decay<Config>::type, Outlet, Mode, Area>>(
        core_,
        std::forward<Config>( cfg ) );
    }

    template<typename Bar, typename... Options>
# if __PGBAR_CXX20
      requires( __details::traits::is_bar<Bar>::value && Bar::Sink == Outlet && Bar::Strategy == Mode
                && Bar::Layout == Area && std::is_constructible_v<Bar, Options...> )
    __PGBAR_NODISCARD std::unique_ptr<Bar>
# else
    __PGBAR_NODISCARD typename std::enable_if<
      __details::traits::AllOf<__details::traits::is_bar<Bar>,
                               std::is_constructible<Bar, Options...>,
                               std::integral_constant<bool, ( Bar::Sink == Outlet )>,
                               std::integral_constant<bool, ( Bar::Strategy == Mode )>,
                               std::integral_constant<bool, ( Bar::Layout == Area )>>::value,
      std::unique_ptr<Bar>>::type
# endif
      insert( Options&&... options )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<
        __details::prefabs::ManagedBar<typename Bar::Config, Outlet, Mode, Area>>(
        core_,
        std::forward<Options>( options )... );
    }
    template<typename Config, typename... Options>
# if __PGBAR_CXX20
      requires( __details::traits::is_config<Config>::value && std::is_constructible_v<Config, Options...> )
    __PGBAR_NODISCARD std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<__details::traits::AllOf<__details::traits::is_config<Config>,
                                                       std::is_constructible<Config, Options...>>::value,
                              std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>::type
# endif
      insert( Options&&... options )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<__details::prefabs::ManagedBar<Config, Outlet, Mode, Area>>(
        core_,
        std::forward<Options>( options )... );
    }

    void swap( Self& lhs ) noexcept
    {
      __PGBAR_TRUST( this != &lhs );
      core_.swap( lhs.core_ );
    }
    friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }
  };

  // Creates a tuple of unique_ptr pointing to bars using existing bar instances.
  template<typename Config, typename... Configs, Channel Outlet, Policy Mode, Region Area>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
               std::unique_ptr<__details::prefabs::BasicBar<Configs, Outlet, Mode, Area>>...>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<Config>,
                             __details::traits::is_config<Configs>...>::value,
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
               std::unique_ptr<__details::prefabs::BasicBar<Configs, Outlet, Mode, Area>>...>>::type
# endif
    make_dynamic( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar,
                  __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars )
  {
    DynamicBar<Outlet, Mode, Area> factory;
    return { factory.insert( std::move( bar ) ), factory.insert( std::move( bars ) )... };
  }
  // Creates a tuple of unique_ptr pointing to bars using configuration objects.
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config,
           typename... Configs>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<std::decay_t<Config>>::value
              && ( __details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>>,
               std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Configs>, Outlet, Mode, Area>>...>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<typename std::decay<Config>::type>,
                             __details::traits::is_config<typename std::decay<Configs>::type>...>::value,
    std::tuple<
      std::unique_ptr<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>,
      std::unique_ptr<
        __details::prefabs::BasicBar<typename std::decay<Configs>::type, Outlet, Mode, Area>>...>>::type
# endif
    make_dynamic( Config&& cfg, Configs&&... cfgs )
  {
    DynamicBar<Outlet, Mode, Area> factory;
    return { factory.insert( std::forward<Config>( cfg ) ),
             factory.insert( std::forward<Configs>( cfgs ) )... };
  }

  /**
   * Creates a vector of unique_ptr pointing to bars with a fixed number of BasicBar instances.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<typename Config, Channel Outlet, Policy Mode, Region Area>
# if __PGBAR_CXX20
    requires __details::traits::is_config<Config>::value
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::is_config<Config>::value,
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>>::type
# endif
    make_dynamic( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar,
                  __details::types::Size count )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>> products;
    std::generate_n( std::back_inserter( products ), count - 1, [&factory, &bar]() {
      return factory.insert( bar.config() );
    } );
    products.emplace_back( factory.insert( std::move( bar ) ) );
    return products;
  }
  /**
   * Creates a vector of unique_ptr pointing to bars with a fixed number of BasicBar instances.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config>
# if __PGBAR_CXX20
    requires __details::traits::is_config<std::decay_t<Config>>::value
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>>>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::is_config<typename std::decay<Config>::type>::value,
    std::vector<std::unique_ptr<
      __details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>>>::type
# endif
    make_dynamic( Config&& cfg, __details::types::Size count )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<
      std::unique_ptr<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>>
      products;
    std::generate_n( std::back_inserter( products ), count - 1, [&factory, &cfg]() {
      return factory.insert( cfg );
    } );
    products.emplace_back( factory.insert( std::forward<Config>( cfg ) ) );
    return products;
  }

  /**
   * Creates a vector of unique_ptr with a fixed number of bars using mutiple bar/configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided arguments;
   * **An unmatched count and Bars number will cause an exception `pgbar::exception::InvalidArgument`.**
   */
  template<typename Bar, typename... Objs>
# if __PGBAR_CXX20
    requires( __details::traits::is_bar<Bar>::value
              && ( ( ( std::is_same_v<std::remove_cv_t<Bar>, std::remove_cv_t<Objs>> && ... )
                     && !( std::is_lvalue_reference_v<Objs> || ... ) )
                   || ( std::is_same<typename Bar::Config, std::decay_t<Objs>>::value && ... ) ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN std::vector<std::unique_ptr<Bar>>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<
      __details::traits::is_bar<Bar>,
      __details::traits::AnyOf<
        __details::traits::AllOf<
          std::is_same<typename std::remove_cv<Bar>::type, typename std::remove_cv<Objs>::type>...,
          __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Objs>...>>>,
        __details::traits::AllOf<std::is_same<typename Bar::Config, typename std::decay<Objs>::type>...>>>::
      value,
    std::vector<std::unique_ptr<Bar>>>::type
# endif
    make_dynamic( __details::types::Size count, Objs&&... objs ) noexcept( false )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    else if ( count != sizeof...( Objs ) )
      __PGBAR_UNLIKELY throw exception::InvalidArgument(
        "pgbar: the number of arguments mismatch with the provided count" );

    DynamicBar<Bar::Sink, Bar::Strategy, Bar::Layout> factory;
    std::vector<std::unique_ptr<Bar>> products;
    (void)std::initializer_list<char> {
      ( products.emplace_back( factory.insert( std::forward<Objs>( objs ) ) ), '\0' )...
    };
    std::generate_n( std::back_inserter( products ), count - sizeof...( Objs ), [&factory]() {
      return factory.template insert<Bar>();
    } );
    return products;
  }
  /**
   * Creates a vector of unique_ptr with a fixed number of BasicBar instances using multiple configurations.
   * The ctor sequentially initializes the first few instances corresponding to the provided configurations;
   * **An unmatched count and Bars number will cause an exception `pgbar::exception::InvalidArgument`.**
   */
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( std::is_same_v<std::remove_cv_t<Config>, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<
      __details::traits::is_config<Config>,
      std::is_same<typename std::remove_cv<Config>::type, typename std::decay<Configs>::type>...>::value,
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>>::type
# endif
    make_dynamic( __details::types::Size count, Configs&&... cfgs ) noexcept( false )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    else if ( count != sizeof...( Configs ) )
      __PGBAR_UNLIKELY throw exception::InvalidArgument(
        "pgbar: the number of configs mismatch with the provided count" );

    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>> products;
    (void)std::initializer_list<char> {
      ( products.emplace_back( factory.insert( std::forward<Configs>( cfgs ) ) ), '\0' )...
    };
    std::generate_n( std::back_inserter( products ), count - sizeof...( Configs ) - 1, [&factory]() {
      return factory.template insert<Config>();
    } );
    return products;
  }
  /**
   * Creates a vector of unique_ptr with a fixed number of BasicBar instances using multiple bar objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided objects;
   * **An unmatched count and Bars number will cause an exception `pgbar::exception::InvalidArgument`.**
   */
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
# if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>
# else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<Config>,
                             std::is_same<Config, typename std::decay<Configs>::type>...>::value,
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>>::type
# endif
    make_dynamic( __details::types::Size count,
                  __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars ) noexcept( false )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    else if ( count != sizeof...( Configs ) )
      __PGBAR_UNLIKELY throw exception::InvalidArgument(
        "pgbar: the number of bars mismatch with the provided count" );

    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>> products;
    (void)std::initializer_list<char> { ( products.emplace_back( factory.insert( std::move( bars ) ) ),
                                          '\0' )... };
    std::generate_n( std::back_inserter( products ), count - sizeof...( Configs ) - 1, [&factory]() {
      return factory.template insert<Config>();
    } );
    return products;
  }
} // namespace pgbar

template<typename... Bs>
struct std::tuple_size<pgbar::MultiBar<Bs...>> : std::integral_constant<std::size_t, sizeof...( Bs )> {};

template<std::size_t I, typename... Bs>
struct std::tuple_element<I, pgbar::MultiBar<Bs...>> : pgbar::__details::traits::TypeAt<I, Bs...> {};

# undef __PGBAR_BIND_BEHAVIOUR
# undef __PGBAR_BIND_OPTION
# undef __PGBAR_TRAIT_REGISTER

# undef __PGBAR_EMPTY_COMPONENT
# undef __PGBAR_NONEMPTY_COMPONENT

# undef __PGBAR_PACK
# undef __PGBAR_INHERIT_REGISTER

# undef __PGBAR_CC_STD
# undef __PGBAR_WIN
# undef __PGBAR_UNIX
# undef __PGBAR_UNKNOWN
# undef __PGBAR_CXX23
# undef __PGBAR_TRUST
# undef __PGBAR_ASSUME
# undef __PGBAR_CXX23_CNSTXPR
# undef __PGBAR_INLINE_FN
# undef __PGBAR_NODISCARD
# undef __PGBAR_CXX20
# undef __PGBAR_CNSTEVAL
# undef __PGBAR_CXX20_CNSTXPR
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
