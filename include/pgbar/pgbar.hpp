// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2025 Konvt
#pragma once

#ifndef __KONVT_PGBAR
# define __KONVT_PGBAR

# if defined( _MSC_VER )
#  define __PGBAR_NODISCARD _Check_return_
#  define __PGBAR_INLINE_FN __forceinline
# elif defined( __GNUC__ ) || defined( __clang__ )
#  define __PGBAR_NODISCARD __attribute__( ( warn_unused_result ) )
#  define __PGBAR_INLINE_FN __attribute__( ( always_inline ) ) inline
# else
#  define __PGBAR_NODISCARD
#  define __PGBAR_INLINE_FN inline
# endif

# if defined( _MSC_VER ) && defined( _MSVC_LANG ) // for msvc
#  define __PGBAR_CC_STD _MSVC_LANG
# else
#  define __PGBAR_CC_STD __cplusplus
# endif

# if defined( _WIN32 ) || defined( _WIN64 )
#  include <windows.h>
#  undef min
#  undef max
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
#  define __PGBAR_CXX23         1
#  define __PGBAR_CXX23_CNSTXPR constexpr
#  define __PGBAR_UNREACHABLE   std::unreachable()
#  include <functional>
# else
#  define __PGBAR_CXX23 0
#  define __PGBAR_CXX23_CNSTXPR
#  define __PGBAR_UNREACHABLE __PGBAR_ASSERT( false )
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
#  include <string_view>
#  define __PGBAR_CXX17         1
#  define __PGBAR_CXX17_CNSTXPR constexpr
#  define __PGBAR_FALLTHROUGH   [[fallthrough]]

#  undef __PGBAR_NODISCARD
#  define __PGBAR_NODISCARD [[nodiscard]]
# else
#  define __PGBAR_CXX17 0
#  define __PGBAR_CXX17_CNSTXPR
#  define __PGBAR_FALLTHROUGH
# endif
# if __PGBAR_CC_STD >= 201402L
#  define __PGBAR_CXX14         1
#  define __PGBAR_CXX14_CNSTXPR constexpr
#  include <shared_mutex>
# else
#  define __PGBAR_CXX14 0
#  define __PGBAR_CXX14_CNSTXPR
# endif
# if __PGBAR_CC_STD >= 201103L
#  define __PGBAR_CXX11 1
# else
#  error "The library 'pgbar' requires C++11"
# endif

# include <algorithm>
# include <array>
# include <atomic>
# include <bitset>
# include <chrono>
# include <cmath>
# include <condition_variable>
# include <cstdint>
# include <cstring>
# include <exception>
# include <initializer_list>
# include <iterator>
# include <limits>
# include <memory>
# include <mutex>
# include <queue>
# include <string>
# include <thread>
# include <type_traits>
# include <utility>
# include <vector>

# ifdef PGBAR_DEBUG
#  include <cassert>
#  define __PGBAR_ASSERT( expr ) assert( expr )
#  undef __PGBAR_INLINE_FN
#  define __PGBAR_INLINE_FN
# else
#  define __PGBAR_ASSERT( expr )
# endif

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
      {
        __PGBAR_ASSERT( mes != nullptr );
      }
      virtual ~Error() noexcept = default;
      virtual const char* what() const noexcept { return message_; }
    };

    // Exception for invalid function arguments.
    class InvalidArgument : public Error {
    public:
      using Error::Error;
      virtual ~InvalidArgument() noexcept = default;
    };

    // Exception for error state of object.
    class InvalidState : public Error {
    public:
      using Error::Error;
      virtual ~InvalidState() noexcept = default;
    };

    // Exception for local system error.
    class SystemError : public Error {
    public:
      using Error::Error;
      virtual ~SystemError() noexcept = default;
    };
  } // namespace exception

  namespace __detail {
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
      using BitwiseSet = std::uint8_t;
    } // namespace types

    namespace constants {
      constexpr types::Char blank = ' ';
      types::ConstStr nil_str     = "";
    }

    namespace traits {
      template<typename E>
      __PGBAR_NODISCARD __PGBAR_CNSTEVAL __PGBAR_INLINE_FN
        typename std::enable_if<std::is_enum<E>::value, typename std::underlying_type<E>::type>::type
        as_val( E enum_val ) noexcept
      {
        return static_cast<typename std::underlying_type<E>::type>( enum_val );
      }

      /**
       * A lightweight tuple type that stores multiple types.
       *
       * `std::tuple` puts some constraints on the input type that are not metaprogramming related,
       * so here is a lightweight tuple type that is used only for template type parameter passing.
       */
      template<typename... Ts>
      struct TypeList;

      template<typename HeadList, typename TailList>
      struct Join;
      template<typename HeadList, typename TailList>
      using Join_t = typename Join<HeadList, TailList>::type;

      template<typename... Head, typename... Tail>
      struct Join<TypeList<Head...>, TypeList<Tail...>> {
        using type = TypeList<Head..., Tail...>;
      };

      template<typename HeadList, typename... TailList>
      struct Merge {
        using type = Join_t<HeadList, typename Merge<TailList...>::type>;
      };
      template<typename HeadList, typename... TailList>
      using Merge_t = typename Merge<HeadList, TailList...>::type;

      template<typename... Ts>
      struct Merge<TypeList<Ts...>> {
        using type = TypeList<Ts...>;
      };

      // Check whether type `T` belongs to the list `TypeGroup`.
      template<typename T, typename TypeGroup>
      struct Belong;
      template<typename T>
      struct Belong<T, TypeList<>> : std::false_type {};
      template<typename T, typename U, typename... Us>
      struct Belong<T, TypeList<U, Us...>>
        : std::conditional<std::is_same<T, U>::value, std::true_type, Belong<T, TypeList<Us...>>>::type {};

      // Check whether type `T` belongs to any of the type groups.
      template<typename T, typename TypeGroup, typename... TypeGroups>
      struct BelongAny
        : std::conditional<Belong<T, TypeGroup>::value, std::true_type, BelongAny<T, TypeGroups...>>::type {};
      template<typename T, typename G>
      struct BelongAny<T, G> : Belong<T, G> {};

      // Check whether all types in `TypeList` belong to any of the type groups `Groups`.
      template<typename TypeList, typename... Groups>
      struct AllBelongAny;
      template<>
      struct AllBelongAny<TypeList<>> : std::true_type {};
      template<typename G, typename... Gs>
      struct AllBelongAny<TypeList<>, G, Gs...> : std::false_type {};
      template<typename T, typename... Ts>
      struct AllBelongAny<TypeList<T, Ts...>> : std::false_type {};
      template<typename T, typename G, typename... Gs>
      struct AllBelongAny<TypeList<T>, G, Gs...> : BelongAny<T, G, Gs...> {};
      template<typename T, typename... Ts, typename G, typename... Gs>
      struct AllBelongAny<TypeList<T, Ts...>, G, Gs...>
        : std::conditional<BelongAny<T, G, Gs...>::value,
                           AllBelongAny<TypeList<Ts...>, G, Gs...>,
                           std::false_type>::type {};

      // A kind of `std::is_same` that applies to template class types.
      template<template<typename...> class T, template<typename...> class U>
      struct Equal : std::false_type {};
      template<template<typename...> class T>
      struct Equal<T, T> : std::true_type {};

      // A lightweight tuple type that stores multiple template class types.
      template<template<typename...> class... Ts>
      struct TemplateList;

      // Insert a new template type into the head of the TemplateList.
      template<typename List, template<typename...> class T>
      struct Prepend;
      // Get the result of the template `Prepend`.
      template<typename List, template<typename...> class T>
      using Prepend_t = typename Prepend<List, T>::type;

      template<template<typename...> class... Ts, template<typename...> class T>
      struct Prepend<TemplateList<Ts...>, T> {
        using type = TemplateList<T, Ts...>;
      };

      // Check whether a TemplateList contains given template `T`.
      template<typename List, template<typename...> class T>
      struct Contain;
      template<template<typename...> class T>
      struct Contain<TemplateList<>, T> : std::false_type {};
      template<template<typename...> class Head,
               template<typename...> class... Tail,
               template<typename...> class T>
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
        using VBs = TemplateList<>; // virtual base
        using NBs = TemplateList<>; // normal base
      };
      // Gets the virtual base list of the template class `Node`.
      template<template<typename...> class Node>
      using InheritFrom_vbt = typename InheritFrom<Node>::VBs;
      // Gets the non-virtual base (also called normal base) list of the template class `Node`.
      template<template<typename...> class Node>
      using InheritFrom_nbt = typename InheritFrom<Node>::NBs;

// Pack multiple macro parameters into a single one.
# define __PGBAR_PACK( ... ) __VA_ARGS__
// A helper macro to register the inheritance structure of a template class.
# define __PGBAR_INHERIT_REGISTER( Node, VBList, NBList ) \
   template<>                                             \
   struct InheritFrom<Node> {                             \
     using VBs = TemplateList<VBList>;                    \
     using NBs = TemplateList<NBList>;                    \
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
       */
      template<template<typename...> class Base, template<typename...> class... Bases>
      struct TopoSort {
      private:
        // VI: Virtual Inherit
        template<bool VI, typename List, typename SortedList, typename VisitedList>
        struct Helper;
        /* Return the virtual base class node that was accessed during the recursive process.
         * pt: path type. */
        template<bool VI, typename List, typename SortedList, typename VisitedList>
        using Helper_pt = typename Helper<VI, List, SortedList, VisitedList>::path;
        /* Return the sorted list.
         * tp: type. */
        template<bool VI, typename List, typename SortedList, typename VisitedList>
        using Helper_tp = typename Helper<VI, List, SortedList, VisitedList>::type;

        template<bool VI, typename SortedList, typename VisitedList>
        struct Helper<VI, TemplateList<>, SortedList, VisitedList> {
          using path = VisitedList;
          using type = SortedList;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename SortedList,
                 typename VisitedList>
        struct Helper<true, TemplateList<Head, Tail...>, SortedList, VisitedList> {
        private:
          using SortVB_t = Helper_tp<true, InheritFrom_vbt<Head>, SortedList, VisitedList>;
          using MarkVB_t = Helper_pt<true, InheritFrom_vbt<Head>, SortedList, VisitedList>;

          using SortTail_t = Helper_tp<true, TemplateList<Tail...>, SortVB_t, MarkVB_t>;
          using MarkTail_t = Helper_pt<true, TemplateList<Tail...>, SortVB_t, MarkVB_t>;

          using SortNB_t = Helper_tp<false, InheritFrom_nbt<Head>, SortTail_t, MarkTail_t>;
          using MarkNB_t = Helper_pt<false, InheritFrom_nbt<Head>, SortTail_t, MarkTail_t>;

        public:
          using path = Prepend_t<MarkNB_t, Head>;
          using type =
            typename std::conditional<Contain<VisitedList, Head>::value,
                                      Helper_tp<true, TemplateList<Tail...>, SortedList, VisitedList>,
                                      Prepend_t<SortNB_t, Head>>::type;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename SortedList,
                 typename VisitedList>
        struct Helper<false, TemplateList<Head, Tail...>, SortedList, VisitedList> {
        private:
          using SortVB_t = Helper_tp<true, InheritFrom_vbt<Head>, SortedList, VisitedList>;
          using MarkVB_t = Helper_pt<true, InheritFrom_vbt<Head>, SortedList, VisitedList>;

          using SortTail_t = Helper_tp<false, TemplateList<Tail...>, SortVB_t, MarkVB_t>;
          using MarkTail_t = Helper_pt<false, TemplateList<Tail...>, SortVB_t, MarkVB_t>;

          using SortNB_t = Helper_tp<false, InheritFrom_nbt<Head>, SortTail_t, MarkTail_t>;
          using MarkNB_t = Helper_pt<false, InheritFrom_nbt<Head>, SortTail_t, MarkTail_t>;

        public:
          using path = MarkNB_t;
          using type = Prepend_t<SortNB_t, Head>;
        };

      public:
        using type = Helper_tp<false, TemplateList<Base, Bases...>, TemplateList<>, TemplateList<>>;
      };
      // Get a list of topological sorting results for the input template classes.
      template<template<typename...> class Base, template<typename...> class... Bases>
      using TopoSort_t = typename TopoSort<Base, Bases...>::type;

      /**
       * Linearization of Inheritance.
       *
       * Topologically sort the input types,
       * then iterate through the resulting sorted list and fill in their template types.
       *
       * It relies on the template `InheritFrom` and `TopoSort` classes to work.
       */
      template<template<typename...> class Base, template<typename...> class... Bases>
      struct LI {
      private:
        template<typename TopoOrder, typename FinalBase, typename... Args>
        struct Helper;
        template<typename TopoOrder, typename FinalBase, typename... Args>
        using Helper_t = typename Helper<TopoOrder, FinalBase, Args...>::type;

        template<typename FinalBase, typename... Args>
        struct Helper<TemplateList<>, FinalBase, Args...> {
          using type = FinalBase;
        };
        template<template<typename...> class Head,
                 template<typename...> class... Tail,
                 typename FinalBase,
                 typename... Args>
        struct Helper<TemplateList<Head, Tail...>, FinalBase, Args...> {
          using type = Head<Helper_t<TemplateList<Tail...>, FinalBase, Args...>, Args...>;
        };

      public:
        template<typename FinalBase, typename... Args>
        using type = Helper_t<TopoSort_t<Base, Bases...>, FinalBase, Args...>;
      };

      // A version that accepts TemplateList.
      template<typename List>
      struct LI_t;
      template<template<typename...> class Base, template<typename...> class... Bases>
      struct LI_t<TemplateList<Base, Bases...>> {
        template<typename FinalBase, typename... Args>
        using type = typename LI<Base, Bases...>::template type<FinalBase, Args...>;
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
      struct IteratorTrait<
        T,
        typename std::enable_if<
          !std::is_const<typename std::remove_reference<T>::type>::value
          && std::is_same<typename std::remove_reference<T>::type::iterator,
                          typename std::remove_reference<T>::type::iterator>::value>::type> {
        using type = typename std::remove_reference<T>::type::iterator;
      };
      template<typename T>
      struct IteratorTrait<
        T,
        typename std::enable_if<
          std::is_const<typename std::remove_reference<T>::type>::value
          && std::is_same<typename std::remove_reference<T>::type::const_iterator,
                          typename std::remove_reference<T>::type::const_iterator>::value>::type> {
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
      template<typename F>
      concept TaskFunctor = requires( F fn ) {
        { fn() } -> std::same_as<void>;
      };
      template<typename F>
      struct is_void_functor : std::bool_constant<TaskFunctor<F>> {};

      template<typename M>
      concept Mutex = requires( M mtx ) {
        requires !std::is_reference_v<M>;
        { mtx.lock() } -> std::same_as<void>;
        { mtx.unlock() } -> std::same_as<void>;
      };
      template<typename M>
      struct is_mutex : std::bool_constant<Mutex<M>> {};
# else
      template<typename, typename = void>
      struct is_void_functor : std::false_type {};
      template<typename F>
      struct is_void_functor<
        F,
        typename std::enable_if<std::is_void<decltype( std::declval<F>()() )>::value>::type>
        : std::true_type {};

      template<typename, typename = void>
      struct is_mutex : std::false_type {};
      template<typename M>
      struct is_mutex<
        M,
        typename std::enable_if<!std::is_reference<M>::value
                                && std::is_void<decltype( std::declval<M&>().lock() )>::value
                                && std::is_void<decltype( std::declval<M&>().unlock() )>::value>::type>
        : std::true_type {};
# endif
    } // namespace traits

    namespace wrappers {
      template<typename I>
      class IterSpanBase {
        static_assert( !std::is_arithmetic<I>::value,
                       "pgbar::__detail::wrappers::IterSpanBase: Only available for iterator types" );
        static_assert( !std::is_void<typename std::iterator_traits<I>::difference_type>::value,
                       "pgbar::__detail::wrappers::IterSpanBase: The 'difference_type' of the given "
                       "iterators cannot be 'void'" );

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
        __PGBAR_CXX20_CNSTXPR virtual ~IterSpanBase() noexcept( std::is_nothrow_destructible<I>::value ) = 0;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR I& start_iter() noexcept { return start_; }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR I& end_iter() noexcept { return end_; }

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
          __PGBAR_ASSERT( this != &lhs );
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
      __PGBAR_CXX20_CNSTXPR IterSpanBase<I>::~IterSpanBase()
        noexcept( std::is_nothrow_destructible<I>::value ) = default;

# if __PGBAR_CXX23
      template<typename... Signature>
      using UniqueFunction = std::move_only_function<Signature...>;
# else
      template<typename... Signature>
      class UniqueFunctionImpl;

      template<typename Ret, typename... Args>
      class UniqueFunctionImpl<Ret( Args... )> final {
        using Self = UniqueFunctionImpl;

        struct AnyFn {
#  if __PGBAR_CXX17
          template<types::Size BufferSize, std::size_t Align>
          __PGBAR_CXX20_CNSTXPR static void swap_fn( AnyFn* const a,
                                                     void* const mem_a,
                                                     AnyFn* const b,
                                                     void* const mem_b ) noexcept
          {
            __PGBAR_ASSERT( a != nullptr );
            __PGBAR_ASSERT( b != nullptr );
            __PGBAR_ASSERT( mem_a != nullptr );
            __PGBAR_ASSERT( mem_b != nullptr );
            __PGBAR_ASSERT( a == mem_a );
            __PGBAR_ASSERT( b == mem_b );

            alignas( Align ) std::byte buffer[BufferSize];
            a->move_to( buffer );
            a->~AnyFn();

            b->move_to( mem_a );
            b->~AnyFn();

            AnyFn* const a_tmp = std::launder( reinterpret_cast<AnyFn*>( &buffer ) );
            a_tmp->move_to( mem_b );
            a_tmp->~AnyFn();
          }
#  endif
          __PGBAR_CXX20_CNSTXPR virtual ~AnyFn() noexcept = default;
          virtual Ret operator()( Args... args ) const    = 0;
#  if __PGBAR_CXX17
          virtual void move_to( void* const dest_mem ) noexcept = 0;
#  endif
        };
        template<typename Fn, typename = void>
        struct FnContainer final : public AnyFn {
        private:
          static_assert( !std::is_reference<Fn>::value,
                         "pgbar::__detail::wrappers::UniqueFunction: Incomplete type" );

          mutable Fn fntor_;

        public:
          __PGBAR_CXX14_CNSTXPR FnContainer( Fn fntor )
            noexcept( std::is_nothrow_move_constructible<Fn>::value )
            : fntor_ { std::move( fntor ) }
          {}

          FnContainer( const FnContainer& )                     = delete;
          FnContainer& operator=( const FnContainer& ) &        = delete;
          __PGBAR_CXX20_CNSTXPR virtual ~FnContainer() noexcept = default;

          Ret operator()( Args... args ) const
            noexcept( noexcept( std::declval<Fn>()( std::declval<Args>()... ) ) ) override
          {
            return fntor_( std::forward<Args>( args )... );
          }
#  if __PGBAR_CXX17
          void move_to( void* const dest_mem ) noexcept override
          {
            __PGBAR_ASSERT( dest_mem != nullptr );
            new ( dest_mem ) FnContainer( std::move( fntor_ ) );
          }
#  endif
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

          FnContainer( const FnContainer& )                     = delete;
          FnContainer& operator=( const FnContainer& ) &        = delete;
          __PGBAR_CXX20_CNSTXPR virtual ~FnContainer() noexcept = default;

          Ret operator()( Args... args ) const
            noexcept( noexcept( std::declval<Fn>()( std::declval<Args>()... ) ) ) override
          {
            return ( ( static_cast<Fn&>( const_cast<FnContainer&>( *this ) ) ) )(
              std::forward<Args>( args )... );
          }
#   if __PGBAR_CXX17
          void move_to( void* const dest_mem ) noexcept override
          {
            __PGBAR_ASSERT( dest_mem != nullptr );
            new ( dest_mem ) FnContainer( std::move( static_cast<Fn&>( *this ) ) );
          }
#   endif
        };
#  endif

#  if __PGBAR_CXX17
        union {
          alignas( 16 ) std::add_pointer_t<Ret( Args... )> fptr_;
          alignas( 16 ) AnyFn* ftor_;
        } data_;
        enum class Tag : std::uint8_t { None, Fptr, FtorInline, FtorDync } tag_;
#  else
        union {
          typename std::add_pointer<Ret( Args... )>::type fptr_;
          AnyFn* ftor_;
        } data_;
        enum class Tag : std::uint8_t { None, Fptr, Ftor } tag_;
#  endif

      public:
        __PGBAR_CXX14_CNSTXPR UniqueFunctionImpl() noexcept
        {
          std::memset( &data_, 0, sizeof data_ );
          tag_ = Tag::None;
        }
        __PGBAR_CXX14_CNSTXPR UniqueFunctionImpl( std::nullptr_t ) noexcept : UniqueFunctionImpl() {}

        template<typename Fn,
                 typename = typename std::enable_if<
                   std::is_function<Fn>::value
                   && std::is_convertible<decltype( ( *std::declval<Fn>() )( std::declval<Args>()... ) ),
                                          Ret>::value>::type>
        __PGBAR_CXX14_CNSTXPR UniqueFunctionImpl( Fn* function_ptr ) noexcept : UniqueFunctionImpl()
        {
          data_.fptr_ = function_ptr;
          tag_        = Tag::Fptr;
        }
        template<typename Fn,
                 typename = typename std::enable_if<
                   std::is_class<Fn>::value
                   && std::is_convertible<decltype( ( std::declval<Fn>() )( std::declval<Args>()... ) ),
                                          Ret>::value>::type>
        __PGBAR_CXX20_CNSTXPR UniqueFunctionImpl( Fn functor ) : UniqueFunctionImpl()
        {
          static_assert( std::is_move_constructible<Fn>::value,
                         "pgbar::__detail::wrappers::UniqueFunctionImpl: Invalid type" );

#  if __PGBAR_CXX17
          if __PGBAR_CXX17_CNSTXPR ( sizeof( FnContainer<Fn> ) <= sizeof data_ ) {
            [[maybe_unused]] const auto object = new ( &data_ ) FnContainer<Fn>( std::move( functor ) );
            __PGBAR_ASSERT( static_cast<void*>( object ) == static_cast<void*>( &data_ ) );
            tag_ = Tag::FtorInline;
          } else {
            data_.ftor_ = new FnContainer<Fn>( std::move( functor ) );
            tag_        = Tag::FtorDync;
          }
#  else
          data_.ftor_ = new FnContainer<Fn>( std::move( functor ) );
          tag_        = Tag::Ftor;
#  endif
        }

        __PGBAR_CXX20_CNSTXPR UniqueFunctionImpl( Self&& rhs ) noexcept : UniqueFunctionImpl()
        {
          operator=( std::move( rhs ) );
        }
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != &rhs );
          swap( rhs );
          rhs = nullptr; // exclusive semantics
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR Self& operator=( std::nullptr_t ) & noexcept
        {
#  if __PGBAR_CXX17
          switch ( tag_ ) {
          case Tag::FtorInline: std::launder( reinterpret_cast<AnyFn*>( &data_ ) )->~AnyFn(); break;
          case Tag::FtorDync:   delete data_.ftor_; break;
          default:              break;
          }
#  else
          if ( tag_ == Tag::Ftor )
            delete data_.ftor_;
#  endif
          tag_ = Tag::None;
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR ~UniqueFunctionImpl() noexcept { ( *this ) = nullptr; }

        __PGBAR_CXX20_CNSTXPR Ret operator()( Args... args ) const
        {
          __PGBAR_ASSERT( tag_ != Tag::None );
#  if __PGBAR_CXX17
          switch ( tag_ ) {
          case Tag::Fptr: return ( *data_.fptr_ )( std::forward<Args>( args )... );
          case Tag::FtorInline:
            return ( *std::launder( reinterpret_cast<const AnyFn*>( &data_ ) ) )(
              std::forward<Args>( args )... );
          case Tag::FtorDync: return ( *data_.ftor_ )( std::forward<Args>( args )... );
          default:            break;
          }
#  else
          if ( tag_ == Tag::Fptr )
            return ( *data_.fptr_ )( std::forward<Args>( args )... );
          return ( *data_.ftor_ )( std::forward<Args>( args )... );
#  endif
          __PGBAR_UNREACHABLE;
        }

        void swap( Self& lhs ) noexcept
        {
#  if __PGBAR_CXX17
          switch ( tag_ ) {
          case Tag::FtorInline: {
            switch ( lhs.tag_ ) {
            case Tag::Fptr:     __PGBAR_FALLTHROUGH;
            case Tag::FtorDync: {
              auto union_copy     = lhs.data_;
              const auto base_ptr = std::launder( reinterpret_cast<AnyFn*>( &data_ ) );
              base_ptr->move_to( &lhs.data_ );
              base_ptr->~AnyFn();
              data_ = union_copy;
            } break;
            case Tag::FtorInline: {
              AnyFn::template swap_fn<sizeof data_, alignof( decltype( data_ ) )>(
                std::launder( reinterpret_cast<AnyFn*>( &data_ ) ),
                &data_,
                std::launder( reinterpret_cast<AnyFn*>( &lhs.data_ ) ),
                &lhs.data_ );
            } break;
            case Tag::None: {
              const auto base_ptr = std::launder( reinterpret_cast<AnyFn*>( &data_ ) );
              base_ptr->move_to( &lhs.data_ );
              base_ptr->~AnyFn();
            } break;
            default: __PGBAR_UNREACHABLE;
            }
          } break;
          default: {
            std::swap( data_, lhs.data_ );
          } break;
          }
#  else
          std::swap( data_, lhs.data_ );
#  endif
          std::swap( tag_, lhs.tag_ );
        }
        friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN friend constexpr bool operator==( const Self& a,
                                                                              std::nullptr_t ) noexcept
        {
          return !static_cast<bool>( a );
        }

        explicit operator bool() const noexcept { return tag_ != Tag::None; }
      };

      // A simplified implementation of std::move_only_function
      template<typename... Signature>
      using UniqueFunction = UniqueFunctionImpl<traits::RemoveNoexcept_t<Signature...>>;
# endif
    } // namespace wrappers
  } // namespace __detail

  namespace iterators {
    /**
     * An undirectional range delimited by an numeric interval [start, end).
     *
     * The `end` can be less than the `start` only if the `step` is negative,
     * otherwise it throws exception `pgbar::exception::InvalidArgument`.
     */
    template<typename N>
    class NumericSpan {
      static_assert( std::is_arithmetic<N>::value,
                     "pgbar::iterators::NumericSpan: Only available for arithmetic types" );

      N start_, end_, step_;

    public:
      class iterator final {
        N itr_start_, itr_step_;
        __detail::types::Size itr_cnt_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = N;
        using difference_type   = value_type;
        using pointer           = void;
        using reference         = value_type;

        constexpr iterator( N startpoint, N step, __detail::types::Size iterated = 0 ) noexcept
          : itr_start_ { startpoint }, itr_step_ { step }, itr_cnt_ { iterated }
        {}

        constexpr iterator() noexcept : iterator( {}, {}, {} ) {}
        __PGBAR_CXX20_CNSTXPR ~iterator() noexcept = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++() noexcept
        {
          ++itr_cnt_;
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int ) noexcept
        {
          auto before = *this;
          operator++();
          return before;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator+=( value_type increment ) noexcept
        {
          itr_cnt_ += increment > 0 ? static_cast<__detail::types::Size>( increment / itr_step_ ) : 0;
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr reference operator*() const noexcept
        {
          return static_cast<reference>( static_cast<__detail::types::Size>( itr_start_ )
                                         + itr_cnt_ * static_cast<__detail::types::Size>( itr_step_ ) );
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
        __PGBAR_UNLIKELY if ( step > 0 && startpoint > endpoint ) throw exception::InvalidArgument(
          "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else __PGBAR_UNLIKELY if ( step < 0 && startpoint < endpoint ) throw exception::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        __PGBAR_UNLIKELY if ( step == 0 ) throw exception::InvalidArgument( "pgbar: 'step' is zero" );

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
      __PGBAR_CXX20_CNSTXPR virtual ~NumericSpan() noexcept = default;

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
      NumericSpan& step( N step ) noexcept( false )
      {
        __PGBAR_UNLIKELY if ( step < 0 && start_ < end_ ) throw exception::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else __PGBAR_UNLIKELY if ( step > 0 && start_ > end_ ) throw exception::InvalidArgument(
          "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else __PGBAR_UNLIKELY if ( step == 0 ) throw exception::InvalidArgument( "pgbar: 'step' is zero" );

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
        __PGBAR_UNLIKELY if ( step_ < 0 && startpoint < end_ ) throw exception::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else __PGBAR_UNLIKELY if ( step_ > 0 && startpoint > end_ ) throw exception::InvalidArgument(
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
        __PGBAR_UNLIKELY if ( step_ < 0 && start_ < endpoint ) throw exception::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else __PGBAR_UNLIKELY if ( step_ > 0 && start_ > endpoint ) throw exception::InvalidArgument(
          "pgbar: 'end' is less than 'start' while 'step' is positive" );

        end_ = endpoint;
        return *this;
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N start_value() const noexcept { return start_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N end_value() const noexcept { return end_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr N step() const noexcept { return step_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR __detail::types::Size size() const noexcept
      {
        if __PGBAR_CXX17_CNSTXPR ( std::is_integral<N>::value )
          return ( ( end_ - start_ + step_ ) - 1 ) / step_;
        else
          return static_cast<__detail::types::Size>( std::ceil( ( end_ - start_ ) / step_ ) );
      }

      __PGBAR_CXX14_CNSTXPR void swap( NumericSpan<N>& lhs ) noexcept
      {
        __PGBAR_ASSERT( this != &lhs );
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
    class IterSpan : public __detail::wrappers::IterSpanBase<I> {
      static_assert( !std::is_pointer<I>::value,
                     "pgbar::iterators::IterSpan<I>: Only available for iterator types" );

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
        __PGBAR_CXX20_CNSTXPR ~iterator() noexcept( std::is_nothrow_destructible<I>::value ) = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++()
        {
          ++current_;
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int )
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
          return &current_;
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

      using __detail::wrappers::IterSpanBase<I>::IterSpanBase;
      __PGBAR_CXX20_CNSTXPR virtual ~IterSpan() noexcept( std::is_nothrow_destructible<I>::value ) = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const
        noexcept( std::is_nothrow_move_constructible<I>::value
                  && std::is_nothrow_copy_constructible<I>::value )
      {
        return iterator( this->start_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator end() const
        noexcept( std::is_nothrow_move_constructible<I>::value
                  && std::is_nothrow_copy_constructible<I>::value )
      {
        return iterator( this->end_ );
      }
    };
    template<typename P>
    class IterSpan<P*> : public __detail::wrappers::IterSpanBase<P*> {
      static_assert( std::is_pointer<P*>::value,
                     "pgbar::iterators::IterSpan<P*>: Only available for pointer types" );

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
          __PGBAR_ASSERT( startpoint != nullptr );
          __PGBAR_ASSERT( endpoint != nullptr );
          reversed_ = endpoint < startpoint;
        }
        __PGBAR_CXX20_CNSTXPR ~iterator() noexcept = default;

        __PGBAR_CXX14_CNSTXPR __PGBAR_INLINE_FN iterator& operator++() noexcept
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
          return &current_;
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
        : __detail::wrappers::IterSpanBase<P*>( startpoint, endpoint )
      {
        __PGBAR_UNLIKELY if ( startpoint == nullptr || endpoint == nullptr ) throw exception::InvalidArgument(
          "pgbar: null pointer cannot generate a range" );
      }
      __PGBAR_CXX20_CNSTXPR virtual ~IterSpan() noexcept = default;

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
  } // namespace iterators

  // A enum that specifies the type of the output stream.
  enum class StreamChannel : std::uint8_t { Stdout, Stderr };

  namespace __detail {
    namespace traits {
      template<typename T>
      struct is_arith_range {
      private:
        template<typename N>
        static std::true_type check( const iterators::NumericSpan<N>& );
        static std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check( std::declval<T>() ) )::value;
      };

      template<typename T>
      struct is_iter_range {
      private:
        template<typename I>
        static std::true_type check( const iterators::IterSpan<I>& );
        static std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check( std::declval<T>() ) )::value;
      };
    } // namespace traits

    namespace console {
      namespace escape {
# ifdef PGBAR_COLORLESS
        constexpr types::LitStr reset_font = "";
        constexpr types::LitStr bold_font  = "";
# else
        constexpr types::LitStr reset_font = "\x1B[0m";
        constexpr types::LitStr bold_font  = "\x1B[1m";
# endif
        constexpr types::LitStr store_cursor   = "\x1B[s";
        constexpr types::LitStr restore_cursor = "\x1B[u";

        // Assembles an ANSI escape code that clears `__n` characters after the cursor.
        __PGBAR_INLINE_FN types::String clear_next( types::Size __n = 1 )
        {
          return "\x1B[" + std::to_string( __n ) + 'X';
        }
      } // namespace escape

      /**
       * Convert a hexidecimal RGB color value to an ANSI escape code.
       *
       * Return nothing if defined `PGBAR_COLORLESS`.
       */
      __PGBAR_CXX20_CNSTXPR types::String rgb2ansi( types::HexRGB rgb )
# ifdef PGBAR_COLORLESS
        noexcept( std::is_nothrow_default_constructible<types::String>::value )
      {
        return {};
      }
# else
        noexcept( false )
      {
        if ( rgb == __PGBAR_DEFAULT )
          return types::String( escape::reset_font );

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
          return "\x1B[38;2;" + std::to_string( ( rgb >> 16 ) & 0xFF ) + ";"
               + std::to_string( ( rgb >> 8 ) & 0xFF ) + ";" + std::to_string( rgb & 0xFF ) + "m";
        }
      }
# endif

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
        if ( hex.front() != '#' || ( hex.size() != 7 && hex.size() != 4 ) )
          throw exception::InvalidArgument( "pgbar: invalid hex color format" );

        for ( std::size_t i = 1; i < hex.size(); i++ ) {
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

      template<StreamChannel StreamType>
      /**
       * Determine if the output stream is binded to the tty based on the platform api.
       *
       * Always returns true if defined `PGBAR_INTTY`,
       * or the local platform is neither `Windows` nor `unix-like`.
       */
      __PGBAR_NODISCARD bool intty() noexcept
      {
# if defined( PGBAR_INTTY ) || __PGBAR_UNKNOWN
        return true;
# elif __PGBAR_WIN
        HANDLE stream_handle;
        if __PGBAR_CXX17_CNSTXPR ( StreamType == StreamChannel::Stdout )
          stream_handle = GetStdHandle( STD_OUTPUT_HANDLE );
        else
          stream_handle = GetStdHandle( STD_ERROR_HANDLE );
        __PGBAR_UNLIKELY if ( stream_handle == INVALID_HANDLE_VALUE ) return false;
        return GetFileType( stream_handle ) == FILE_TYPE_CHAR;

# else
        if __PGBAR_CXX17_CNSTXPR ( StreamType == StreamChannel::Stdout )
          return isatty( STDOUT_FILENO );
        else
          return isatty( STDERR_FILENO );
# endif
      }
    } // namespace console

    namespace charset {
      // A type of wrapper that stores the mapping between Unicode code chart and character width.
      class CodeChart final {
        types::UCodePoint start_, end_;
        types::Size width_;

      public:
        constexpr CodeChart( types::UCodePoint start, types::UCodePoint end, types::Size width ) noexcept
          : start_ { start }, end_ { end }, width_ { width }
        { // This is an internal component, so we assume the arguments are always valid.
          __PGBAR_ASSERT( start_ <= end_ );
        }
        __PGBAR_CXX20_CNSTXPR ~CodeChart() noexcept = default;

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
          auto&& charts = code_charts();
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
            const auto start_point = u8_str.data() + i;
            // After RFC 3629, the maximum length of each standard UTF-8 character is 4 bytes.
            const auto first_byte  = static_cast<types::UCodePoint>( *start_point );
            auto integrity_checker = [start_point, &u8_str]( types::Size expected_len ) -> void {
              __PGBAR_ASSERT( start_point >= u8_str.data() );
              if ( u8_str.size() - ( start_point - u8_str.data() ) < expected_len )
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
              integrity_checker( 2 );
              utf_codepoint =
                ( ( first_byte & 0x1F ) << 6 ) | ( static_cast<types::UCodePoint>( start_point[1] ) & 0x3F );
              i += 2;
            } else if ( ( first_byte & 0xF0 ) == 0xE0 ) {
              integrity_checker( 3 );
              utf_codepoint = ( ( first_byte & 0xF ) << 12 )
                            | ( ( static_cast<types::UCodePoint>( start_point[1] ) & 0x3F ) << 6 )
                            | ( static_cast<types::UCodePoint>( start_point[2] ) & 0x3F );
              i += 3;
            } else if ( ( first_byte & 0xF8 ) == 0xF0 ) {
              integrity_checker( 4 );
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
        __PGBAR_CXX20_CNSTXPR U8String( const Self& )              = default;
        __PGBAR_CXX20_CNSTXPR U8String( Self&& ) noexcept          = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( const Self& ) &     = default;
        __PGBAR_CXX20_CNSTXPR Self& operator=( Self&& ) & noexcept = default;
        __PGBAR_CXX20_CNSTXPR ~U8String() noexcept                 = default;

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
        __PGBAR_CXX14_CNSTXPR types::Size size() const noexcept { return width_; }
        __PGBAR_CXX20_CNSTXPR types::ROStr str() const noexcept { return bytes_; }

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

        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN Self operator+( types::ROStr a, const Self& b )
        {
          auto tmp = types::String( a );
          tmp.append( b.bytes_ );
          return U8String( std::move( tmp ) );
        }
        __PGBAR_NODISCARD friend __PGBAR_INLINE_FN Self operator+( const Self& a, types::ROStr b )
        {
          auto tmp = a.bytes_;
          tmp.append( b );
          return U8String( std::move( tmp ) );
        }

# if __PGBAR_CXX20
        static_assert( sizeof( char8_t ) == sizeof( char ),
                       "pgbar::__detail::chaset::U8String: Unexpected type size mismatch" );

        explicit U8String( std::u8string_view u8_sv ) : U8String()
        {
          bytes_.resize( u8_sv.size() + 1 );
          std::memcpy( bytes_.data(), u8_sv.data(), u8_sv.size() );
          width_ = render_width( bytes_ );
        }

        explicit operator std::u8string() const
        {
          std::u8string ret;
          ret.resize( bytes_.size() + 1 );
          std::memcpy( ret.data(), bytes_.data(), bytes_.size() );
          return ret;
        }
# endif
      };
    } // namespace charset

    namespace io {
      enum class TxtLayout { Left, Right, Center }; // text layout
      // Format the `str`.
      template<TxtLayout Style>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String formatting( types::Size width,
                                                                                          types::Size len_str,
                                                                                          types::ROStr str )
        noexcept( false )
      {
        __PGBAR_UNLIKELY if ( width == 0 ) return {};
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
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String formatting( types::Size width,
                                                                                          types::ROStr __str )
        noexcept( false )
      {
        return formatting<Style>( width, __str.size(), __str );
      }
      template<TxtLayout Style>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::String formatting(
        types::Size width,
        const charset::U8String& __str ) noexcept( false )
      {
        return formatting<Style>( width, __str.size(), __str.str() );
      }

      // A simple string buffer, unrelated to the `std::stringbuf` in the STL.
      class Stringbuf {
        using Self = Stringbuf;

      protected:
        std::vector<types::Char> buffer_;

      public:
        __PGBAR_CXX20_CNSTXPR Stringbuf() noexcept = default;

        __PGBAR_CXX20_CNSTXPR Stringbuf( const Self& lhs ) { operator=( lhs ); }
        __PGBAR_CXX20_CNSTXPR Stringbuf( Self&& rhs ) noexcept : Stringbuf()
        {
          operator=( std::move( rhs ) );
        }
        __PGBAR_CXX20_CNSTXPR __PGBAR_INLINE_FN Self& operator=( const Self& lhs ) &
        {
          __PGBAR_ASSERT( this != &lhs );
          buffer_ = lhs.buffer_;
          return *this;
        }
        __PGBAR_CXX20_CNSTXPR __PGBAR_INLINE_FN Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != &rhs );
          swap( rhs );
          rhs.buffer_.clear();
          rhs.buffer_.shrink_to_fit();
          return *this;
        }

        __PGBAR_CXX20_CNSTXPR virtual ~Stringbuf() noexcept = default;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR bool empty() const noexcept { return buffer_.empty(); }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void clear() & noexcept { buffer_.clear(); }

        // Releases the buffer space completely
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void release() & noexcept
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
          __PGBAR_ASSERT( N != 0 );
          for ( types::Size _ = 0; _ < __num; ++_ )
            buffer_.insert( buffer_.cend(), info, info + N );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( types::ROStr info, types::Size __num = 1 ) &
        {
          for ( types::Size _ = 0; _ < __num; ++_ )
            buffer_.insert( buffer_.cend(), info.data(), info.data() + info.size() );
          return *this;
        }
        template<typename T>
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR
          typename std::enable_if<std::is_same<typename std::decay<T>::type, types::String>::value
                                    || std::is_same<typename std::decay<T>::type, types::ROStr>::value,
                                  Self&>::type
          append( T&& info, types::Size __num = 1 ) &
        {
          for ( types::Size _ = 0; _ < __num; ++_ )
            buffer_.insert( buffer_.cend(), info.cbegin(), info.cend() );
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& append( const charset::U8String& info,
                                                              types::Size __num = 1 )
        {
          return append( info.str(), __num );
        }

        template<typename T>
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR
          typename std::enable_if<std::is_same<typename std::decay<T>::type, types::Char>::value
                                    || std::is_same<typename std::decay<T>::type, types::String>::value
                                    || std::is_same<typename std::decay<T>::type, types::ROStr>::value
                                    || ( std::is_pointer<typename std::decay<T>::type>::value
                                         && std::is_same<typename std::decay<typename std::remove_pointer<
                                                           typename std::decay<T>::type>::type>::type,
                                                         char>::value ),
                                  Self&>::type
          operator<<( Self& stream, T&& info )
        {
          return stream.append( std::forward<T>( info ) );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR Self& operator<<( Self& stream,
                                                                         const charset::U8String& info )
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
          __PGBAR_ASSERT( this != &lhs );
          buffer_.swap( lhs.buffer_ );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( Stringbuf& a, Stringbuf& b ) noexcept { a.swap( b ); }
      };

      template<StreamChannel StreamType>
      class OStream;
      template<StreamChannel StreamType>
      OStream<StreamType>& flush( OStream<StreamType>& stream )
      {
        return stream.flush();
      }
      template<StreamChannel StreamType>
      __PGBAR_CXX20_CNSTXPR OStream<StreamType>& release( OStream<StreamType>& stream ) noexcept
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
      template<StreamChannel StreamType>
      class OStream final : public Stringbuf {
        using Self = OStream;

      public:
        __PGBAR_CXX20_CNSTXPR OStream() noexcept          = default;
        __PGBAR_CXX20_CNSTXPR virtual ~OStream() noexcept = default;

        Self& flush() &
        {
# if __PGBAR_WIN
          DWORD written = 0;
          if __PGBAR_CXX17_CNSTXPR ( StreamType == StreamChannel::Stdout ) {
            auto h_stdout = GetStdHandle( STD_OUTPUT_HANDLE );
            __PGBAR_UNLIKELY if ( h_stdout == INVALID_HANDLE_VALUE ) throw exception::SystemError(
              "pgbar: cannot open the standard output stream" );
            WriteFile( h_stdout, buffer_.data(), static_cast<DWORD>( buffer_.size() ), &written, nullptr );
          } else {
            auto h_stderr = GetStdHandle( STD_ERROR_HANDLE );
            __PGBAR_UNLIKELY if ( h_stderr == INVALID_HANDLE_VALUE ) throw exception::SystemError(
              "pgbar: cannot open the standard error stream" );
            WriteFile( h_stderr, buffer_.data(), static_cast<DWORD>( buffer_.size() ), &written, nullptr );
          }
# elif __PGBAR_UNIX
          if __PGBAR_CXX17_CNSTXPR ( StreamType == StreamChannel::Stdout )
            write( STDOUT_FILENO, buffer_.data(), buffer_.size() );
          else
            write( STDERR_FILENO, buffer_.data(), buffer_.size() );
# else
          if __PGBAR_CXX17_CNSTXPR ( StreamType == StreamChannel::Stdout )
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
          __PGBAR_ASSERT( fnptr != nullptr );
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
        Mutex( const Mutex& )            = delete;
        Mutex& operator=( const Mutex& ) = delete;

        __PGBAR_CXX20_CNSTXPR Mutex() noexcept  = default;
        __PGBAR_CXX20_CNSTXPR ~Mutex() noexcept = default;

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
        SharedMutex( const Self& )     = delete;
        Self& operator=( const Self& ) = delete;

        SharedMutex() noexcept : num_readers_ { 0 } {}
        ~SharedMutex() noexcept = default;

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

        SharedLock( mutex_type& m ) noexcept( noexcept( std::declval<mutex_type&>().lock_shared() ) )
          : mtx_ { m }
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
        ExceptionBox() noexcept  = default;
        ~ExceptionBox() noexcept = default;

        ExceptionBox( ExceptionBox&& rhs ) noexcept : ExceptionBox() { swap( rhs ); }
        ExceptionBox& operator=( ExceptionBox&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != &rhs );
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
        __PGBAR_NODISCARD __PGBAR_INLINE_FN Self& clear() & noexcept
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          exception_ = std::exception_ptr();
          return *this;
        }

        // Pop up the head of the queue element and throw it as an exception.
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
          __PGBAR_ASSERT( this != &lhs );
          std::lock_guard<SharedMutex> lock1 { rw_mtx_ };
          std::lock_guard<SharedMutex> lock2 { lhs.rw_mtx_ };
          using std::swap; // ADL custom point
          swap( exception_, lhs.exception_ );
        }
        friend void swap( ExceptionBox& a, ExceptionBox& b ) noexcept { a.swap( b ); }
      };

      // A reusable stateful thread that performs tasks in a loop.
      class StateThread final {
        using Self = StateThread;

        static types::TimeUnit _working_interval;
        static SharedMutex _rw_mtx;

        /* The state transfer process is:
         *                   activate()                   suspend()
         * Dormant(default) -----------> Awake -> Active ----------> Suspend -> Dormant
         *              dctor
         * (any state) ------> Dead
         *              catch an exception while box_ isn't empty
         * (any state) ------------------------------------------> Dead */
        enum class state : std::uint8_t { Dormant, Awake, Active, Suspend, Halt, Dead };
        struct Handle final {
          wrappers::UniqueFunction<void()> task_;
          std::thread td_;
          ExceptionBox box_;
          std::atomic<state> state_;

          mutable std::condition_variable cond_var_;
          mutable std::mutex mtx_;

          Handle() noexcept( std::is_nothrow_default_constructible<std::condition_variable>::value )
            : task_ { nullptr }, td_ {}, box_ {}, state_ { state::Dead }
          {}
          Handle( const Handle& )              = delete;
          Handle& operator=( const Handle& ) & = delete;
          ~Handle() noexcept                   = default;
        };

        std::unique_ptr<Handle> handle_;

        // Create a new thread object.
        __PGBAR_INLINE_FN void launch() & noexcept( false )
        {
          __PGBAR_ASSERT( handle_ != nullptr );
          __PGBAR_ASSERT( handle_->td_.get_id() == std::thread::id() );

          handle_->state_.store( state::Dormant, std::memory_order_release );
          try {
            const auto self = handle_.get();
            handle_->td_    = std::thread( [self]() -> void {
              while ( self->state_.load( std::memory_order_acquire ) != state::Dead ) {
                try {
                  switch ( self->state_.load( std::memory_order_acquire ) ) {
                  case state::Dormant: {
                    std::unique_lock<std::mutex> lock { self->mtx_ };
                    self->cond_var_.wait( lock, [self]() noexcept -> bool {
                      return self->state_.load( std::memory_order_acquire ) != state::Dormant;
                    } );
                  } break;

                  case state::Awake: { // Intermediate state
                    // Used to tell other threads that the current thread has woken up.
                    auto expected = state::Awake;
                    self->state_.compare_exchange_strong( expected,
                                                          state::Active,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed );
                  }
                    __PGBAR_FALLTHROUGH;
                  case state::Active: {
                    self->task_();
                    std::this_thread::sleep_for( working_interval() );
                  } break;

                  case state::Suspend: {
                    self->task_();
                    auto expected = state::Suspend;
                    self->state_.compare_exchange_strong( expected,
                                                          state::Dormant,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed );
                  } break;

                  case state::Halt: {
                    auto expected = state::Halt;
                    self->state_.compare_exchange_strong( expected,
                                                          state::Dormant,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed );
                  } break;

                  default: return;
                  }
                } catch ( ... ) {
                  // keep thread valid
                  if ( self->box_.empty() ) {
                    auto try_update = [self]( state expected ) noexcept {
                      return self->state_.compare_exchange_strong( expected,
                                                                   state::Dormant,
                                                                   std::memory_order_acq_rel,
                                                                   std::memory_order_relaxed );
                    };
                    try_update( state::Awake ) || try_update( state::Active ) || try_update( state::Suspend );
                    // Avoid deadlock in main thread when the child thread catchs exception.
                    auto exception = std::current_exception();
                    if ( exception )
                      self->box_.store( exception );
                  } else {
                    self->state_.store( state::Dead, std::memory_order_relaxed );
                    throw; // Rethrow it, and let the current thread crash.
                  }
                }
              }
            } );
          } catch ( ... ) {
            handle_->state_.store( state::Dead, std::memory_order_release );
            throw;
          }
        }

        // Stop the thread object and release it.
        __PGBAR_INLINE_FN void shutdown() & noexcept
        {
          __PGBAR_ASSERT( handle_ != nullptr );
          handle_->state_.store( state::Dead, std::memory_order_release );
          {
            std::lock_guard<std::mutex> lock { handle_->mtx_ };
            handle_->cond_var_.notify_all();
          }
          if ( handle_->td_.joinable() )
            handle_->td_.join();
          handle_->td_ = std::thread();
        }

      public:
        // Get the current working interval for all threads.
        __PGBAR_NODISCARD static types::TimeUnit working_interval()
        {
          SharedLock<SharedMutex> lock { _rw_mtx };
          return _working_interval;
        }
        // Adjust the thread working interval between this loop and the next loop.
        static void working_interval( types::TimeUnit new_rate )
        {
          std::lock_guard<SharedMutex> lock { _rw_mtx };
          _working_interval = std::move( new_rate );
        }

        StateThread() noexcept : handle_ { nullptr } {}
        explicit StateThread( wrappers::UniqueFunction<void()> task ) : StateThread()
        {
          appoint( std::move( task ) );
        }
        ~StateThread() noexcept { drop(); }

        StateThread( Self&& rhs ) noexcept : StateThread() { operator=( std::move( rhs ) ); }
        Self& operator=( Self&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != &rhs );
          handle_.swap( rhs.handle_ );
          rhs.drop();
          return *this;
        }

        // Stop the thread object immediately.
        __PGBAR_INLINE_FN void halt() & noexcept
        {
          if ( handle_ == nullptr )
            return;
          const auto self = handle_.get();
          auto try_update = [self]( state expected ) noexcept {
            return self->state_.compare_exchange_strong( expected,
                                                         state::Halt,
                                                         std::memory_order_acq_rel,
                                                         std::memory_order_relaxed );
          };
          if ( try_update( state::Awake ) || try_update( state::Active ) ) {
            do {
              __PGBAR_UNLIKELY if ( handle_->box_.empty() == false ) handle_->box_.rethrow();
            } while ( handle_->state_.load( std::memory_order_acquire ) == state::Halt
                      && handle_->state_.load( std::memory_order_acquire ) != state::Dead );
          } else
            __PGBAR_UNLIKELY if ( handle_->box_.empty() == false ) handle_->box_.rethrow();
        }
        // Stop and release everything.
        __PGBAR_INLINE_FN void drop() noexcept
        {
          if ( handle_ != nullptr ) {
            shutdown();
            handle_.reset();
          }
        }

        // Release the `task` resource.
        __PGBAR_INLINE_FN void appoint() & noexcept
        {
          __PGBAR_ASSERT( active() == false );
          if ( handle_ != nullptr ) {
            halt();
            handle_->task_ = nullptr;
          }
        }
        // Reassign a new task to the thread object.
        __PGBAR_INLINE_FN
        void appoint( wrappers::UniqueFunction<void()> task ) & noexcept( false )
        {
          if ( handle_ == nullptr ) {
# if __PGBAR_CXX14
            handle_ = std::make_unique<Handle>();
# else
            handle_.reset( new Handle() );
# endif
            launch();
          } else if ( handle_->state_.load( std::memory_order_acquire ) == state::Dead ) {
            shutdown();
            launch();
          } else
            halt();
          handle_->task_.swap( task );
        }

        // Launch the thread object and wait for it to start running.
        __PGBAR_INLINE_FN void activate() & noexcept( false )
        {
          __PGBAR_ASSERT( jobless() == false );
          __PGBAR_UNLIKELY if ( handle_->state_.load( std::memory_order_acquire ) == state::Dead )
          {
            shutdown();
            launch();
          }
          auto expected = state::Dormant;
          if ( handle_->state_.compare_exchange_strong( expected,
                                                        state::Awake,
                                                        std::memory_order_acq_rel,
                                                        std::memory_order_relaxed ) ) {
            {
              std::lock_guard<std::mutex> lock { handle_->mtx_ };
              handle_->cond_var_.notify_one();
            }
            // spin wait, ensure that the thread has moved to the new state
            do {
              // avoid deadlock and throw the exception the thread received
              __PGBAR_UNLIKELY if ( handle_->box_.empty() == false ) handle_->box_.rethrow();
            } while ( handle_->state_.load( std::memory_order_acquire ) == state::Awake
                      && handle_->state_.load( std::memory_order_acquire ) != state::Dead );
          } else
            __PGBAR_UNLIKELY if ( handle_->box_.empty() == false ) handle_->box_.rethrow();
        }
        // Stop the thread object and wait for it to sleep.
        __PGBAR_INLINE_FN void suspend() & noexcept( false )
        {
          __PGBAR_UNLIKELY if ( handle_ == nullptr ) return;
          const auto self = handle_.get();
          auto try_update = [self]( state expected ) noexcept {
            return self->state_.compare_exchange_strong( expected,
                                                         state::Suspend,
                                                         std::memory_order_acq_rel,
                                                         std::memory_order_relaxed );
          };
          if ( try_update( state::Awake ) || try_update( state::Active ) ) {
            do {
              __PGBAR_UNLIKELY if ( handle_->box_.empty() == false ) handle_->box_.rethrow();
            } while ( handle_->state_.load( std::memory_order_acquire ) == state::Suspend
                      && handle_->state_.load( std::memory_order_acquire ) != state::Dead );
          } else
            __PGBAR_UNLIKELY if ( handle_->box_.empty() == false ) handle_->box_.rethrow();
        }

        // Check whether there is a `task` assigned to the thread.
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool jobless() const noexcept
        {
          return handle_ == nullptr || handle_->task_ == nullptr;
        }
        // Check whether the thread is running.
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
        {
          if ( jobless() )
            return false;
          const auto current_state = handle_->state_.load( std::memory_order_acquire );
          return current_state != state::Dormant && current_state != state::Dead;
        }
        // Checks whether the thread caught an exception and throws it if it did.
        __PGBAR_INLINE_FN void rethrow_if_exception() const noexcept( false )
        {
          if ( handle_ != nullptr && handle_->box_.empty() == false )
            handle_->box_.rethrow();
        }
      };
      types::TimeUnit StateThread::_working_interval =
        std::chrono::duration_cast<types::TimeUnit>( std::chrono::milliseconds( 40 ) );
      SharedMutex StateThread::_rw_mtx {};

      // A fixed length ring queue.
      template<typename T, types::Size Capacity>
      class RingQueue final {
        using Self = RingQueue;

        static_assert( std::is_default_constructible<T>::value,
                       "pgbar::__detail::concurrent::RingQueue: T must be default constructible" );
        static_assert( std::is_move_constructible<T>::value,
                       "pgbar::__detail::concurrent::RingQueue: T must be move constructible" );
        static_assert( std::is_move_assignable<T>::value,
                       "pgbar::__detail::concurrent::RingQueue: T must be move assignable" );

        std::array<T, Capacity> buffer_;
        types::Size read_idx_, write_idx_, count_;

        mutable SharedMutex rw_mtx_;

      public:
        RingQueue() noexcept { read_idx_ = write_idx_ = count_ = 0; }

        RingQueue( const T& __value, types::Size __n = 1 )
          noexcept( std::is_nothrow_copy_assignable<T>::value )
          : RingQueue()
        {
          __PGBAR_ASSERT( __n <= Capacity );
          for ( types::Size i = 0; i < __n; ++i ) {
            buffer_[write_idx_] = __value;
            write_idx_          = ( write_idx_ + 1 ) % Capacity;
            ++count_;
          }
        }

        RingQueue( const Self& rhs )         = delete;
        Self& operator=( const Self& rhs ) & = delete;

        ~RingQueue() noexcept( std::is_nothrow_destructible<T>::value ) = default;

        // Insert the item into the queue and discard it if the queue is full.
        bool push( T item ) & noexcept( std::is_nothrow_move_assignable<T>::value )
        {
          {
            std::lock_guard<SharedMutex> lock { rw_mtx_ };
            if ( count_ == Capacity )
              return false;
            buffer_[write_idx_] = std::move( item );
            write_idx_          = ( write_idx_ + 1 ) % Capacity;
            ++count_;
          }
          return true;
        }

        /**
         * Remove and return the oldest item from the queue.
         * If the queue is empty, return a default-constructed value.
         */
        __PGBAR_NODISCARD T pop() noexcept( std::is_nothrow_default_constructible<T>::value
                                            && std::is_nothrow_move_assignable<T>::value )
        {
          T item {};
          {
            std::lock_guard<SharedMutex> lock { rw_mtx_ };
            if ( count_ != 0 ) {
              item      = std::move( buffer_[read_idx_] );
              read_idx_ = ( read_idx_ + 1 ) % Capacity;
              --count_;
            }
          }
          return item;
        }

        // Reset the read and write pointers.
        void clear() & noexcept
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          read_idx_ = write_idx_ = count_ = 0;
        }
        // Discard all items from the queue, and reset the read and write pointers.
        void drop()
          noexcept( std::is_nothrow_default_constructible<T>::value
                    && std::is_nothrow_move_assignable<T>::value && std::is_nothrow_destructible<T>::value )
        {
          std::lock_guard<SharedMutex> lock { rw_mtx_ };
          for ( ; count_ > 0; --count_ ) {
            buffer_[read_idx_] = T {};
            read_idx_          = ( read_idx_ + 1 ) % Capacity;
          }
          read_idx_ = write_idx_ = 0;
          __PGBAR_ASSERT( count_ == 0 );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool full() const noexcept
        {
          SharedLock<SharedMutex> lock { rw_mtx_ };
          return count_ == Capacity;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          SharedLock<SharedMutex> lock { rw_mtx_ };
          return count_ == 0;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size size() const noexcept
        {
          SharedLock<SharedMutex> lock { rw_mtx_ };
          return count_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CNSTEVAL types::Size capacity() const noexcept
        {
          return Capacity;
        }
      };

      // Global `StateThread` repository.
      RingQueue<StateThread, 4> thread_repo;
    } // namespace concurrent
  } // namespace __detail

  namespace color {
    constexpr __detail::types::HexRGB None    = __PGBAR_DEFAULT;
    constexpr __detail::types::HexRGB Black   = __PGBAR_BLACK;
    constexpr __detail::types::HexRGB Red     = __PGBAR_RED;
    constexpr __detail::types::HexRGB Green   = __PGBAR_GREEN;
    constexpr __detail::types::HexRGB Yellow  = __PGBAR_YELLOW;
    constexpr __detail::types::HexRGB Blue    = __PGBAR_BLUE;
    constexpr __detail::types::HexRGB Magenta = __PGBAR_MAGENTA;
    constexpr __detail::types::HexRGB Cyan    = __PGBAR_CYAN;
    constexpr __detail::types::HexRGB White   = __PGBAR_WHITE;
  } // namespace color

  namespace option {
# define __PGBAR_OPTIONS( StructName, ValueType )                                  \
 private:                                                                          \
   ValueType data_;                                                                \
                                                                                   \
 public:                                                                           \
   __PGBAR_CXX20_CNSTXPR ~StructName() noexcept = default;                         \
   __PGBAR_CXX14_CNSTXPR ValueType& value() noexcept                               \
   {                                                                               \
     return data_;                                                                 \
   }                                                                               \
   __PGBAR_CXX20_CNSTXPR void swap( StructName& lhs ) noexcept                     \
   {                                                                               \
     __PGBAR_ASSERT( this != &lhs );                                               \
     using std::swap;                                                              \
     swap( data_, lhs.data_ );                                                     \
   }                                                                               \
   friend __PGBAR_CXX20_CNSTXPR void swap( StructName& a, StructName& b ) noexcept \
   {                                                                               \
     a.swap( b );                                                                  \
   }

# define __PGBAR_OPTIONS_HELPER( StructName, ValueType, ParamName ) \
   __PGBAR_OPTIONS( StructName, ValueType )                         \
   constexpr StructName( ValueType ParamName ) noexcept : data_ { ParamName } {}

    // A wrapper that stores the value of the bit option setting.
    struct Style final {
      __PGBAR_OPTIONS_HELPER( Style, __detail::types::BitwiseSet, _settings )
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
      __PGBAR_OPTIONS_HELPER( Tasks, __detail::types::Size, _num_tasks )
    };
    // A wrapper that stores the length of the bar indicator, in the character unit.
    struct BarLength final {
      __PGBAR_OPTIONS_HELPER( BarLength, __detail::types::Size, _num_char )
    };
    /**
     * A wrapper that stores the rate factor of the animation
     * with negative value slowing down the switch per frame and positive value speeding it up.
     *
     * The maximum and minimum of the rate factor is between -128 and 127.
     *
     * If the value is zero, freezes the animation.
     */
    struct Shift final {
      __PGBAR_OPTIONS_HELPER( Shift, std::int8_t, _shift_factor )
    };

# undef __PGBAR_OPTIONS_HELPER
# if __PGBAR_CXX20
#  define __PGBAR_OPTIONS_HELPER( StructName, ParamName )                                                    \
    __PGBAR_OPTIONS( StructName, __detail::charset::U8String )                                               \
    /**                                                                                                      \
     * @throw exception::InvalidArgument                                                                     \
     *                                                                                                       \
     * If the passed parameter is not coding in UTF-8.                                                       \
     */                                                                                                      \
    __PGBAR_CXX20_CNSTXPR StructName( __detail::types::String ParamName ) : data_ { std::move( ParamName ) } \
    {}                                                                                                       \
    StructName( std::u8string_view ParamName ) : data_ { ParamName } {}
# else
#  define __PGBAR_OPTIONS_HELPER( StructName, ParamName )                                                    \
    __PGBAR_OPTIONS( StructName, __detail::charset::U8String )                                               \
    __PGBAR_CXX20_CNSTXPR StructName( __detail::types::String ParamName ) : data_ { std::move( ParamName ) } \
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
# define __PGBAR_OPTIONS_HELPER( StructName, ParamName )                                \
   __PGBAR_OPTIONS( StructName, __detail::types::String )                               \
   __PGBAR_CXX20_CNSTXPR StructName( __detail::types::ROStr ParamName )                 \
     : data_ { __detail::console::rgb2ansi( __detail::console::hex2rgb( ParamName ) ) } \
   {}                                                                                   \
   __PGBAR_CXX20_CNSTXPR StructName( __detail::types::HexRGB ParamName )                \
     : data_ { __detail::console::rgb2ansi( ParamName ) }                               \
   {}

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
    struct LeadColor final {
      __PGBAR_OPTIONS_HELPER( LeadColor, _lead_color )
    };
    // A wrapper that stores the color of the whole infomation indicator.
    struct InfoColor final {
      __PGBAR_OPTIONS_HELPER( InfoColor, _info_color )
    };

# undef __PGBAR_OPTIONS_HELPER

    /**
     * A wrapper that stores the units of the infomation indicator's speed.
     *
     * The structure holds exactly four units in a `std::array`,
     * with each unit being 1,000 times greater than the previous one (from left to right).
     */
    struct SpeedUnit final {
      __PGBAR_OPTIONS( SpeedUnit, __PGBAR_PACK( std::array<__detail::charset::U8String, 4> ) )
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       *
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<__detail::types::String, 4> _units ) : data_ {}
      {
        std::transform(
          std::make_move_iterator( _units.begin() ),
          std::make_move_iterator( _units.end() ),
          data_.begin(),
          []( __detail::types::String&& ele ) { return __detail::charset::U8String( std::move( ele ) ); } );
      }
# if __PGBAR_CXX20
      /**
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      SpeedUnit( std::array<std::u8string_view, 4> _units ) : data_ {}
      {
        std::transform( _units.cbegin(), _units.cend(), data_.begin(), []( std::u8string_view ele ) {
          return __detail::charset::U8String( ele );
        } );
      }
# endif
    };

    // A wrapper that stores the `lead` animated element.
    struct Lead final {
      __PGBAR_OPTIONS( Lead, std::vector<__detail::charset::U8String> )
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( std::vector<__detail::types::String> _leads ) : data_ {}
      {
        std::transform(
          std::make_move_iterator( _leads.begin() ),
          std::make_move_iterator( _leads.end() ),
          std::back_inserter( data_ ),
          []( __detail::types::String&& ele ) { return __detail::charset::U8String( std::move( ele ) ); } );
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( __detail::types::String _lead )
        : data_ { __detail::charset::U8String( std::move( _lead ) ) }
      {}
# if __PGBAR_CXX20
      Lead( std::vector<std::u8string_view> _leads ) : data_ {}
      {
        std::transform( _leads.cbegin(),
                        _leads.cend(),
                        std::back_inserter( data_ ),
                        []( std::u8string_view ele ) { return __detail::charset::U8String( ele ); } );
      }
      Lead( std::u8string_view _lead ) : data_ { __detail::charset::U8String( _lead ) } {}
# endif
    };

# undef __PGBAR_OPTIONS
  } // namespace option

  namespace __detail {
    // The Basic components of the progress bar.
    namespace assets {
# define __PGBAR_MEMBER_METHOD( ClassName, Constexpr )                        \
   Constexpr ClassName( const ClassName& lhs ) : Base( lhs )                  \
   {                                                                          \
     member_copy( lhs );                                                      \
   }                                                                          \
   Constexpr ClassName( ClassName&& rhs ) noexcept : Base( std::move( rhs ) ) \
   {                                                                          \
     member_swap( rhs );                                                      \
   }                                                                          \
   Constexpr ClassName& operator=( const ClassName& lhs )&                    \
   {                                                                          \
     Base::operator=( lhs );                                                  \
     member_copy( lhs );                                                      \
     return *this;                                                            \
   }                                                                          \
   Constexpr ClassName& operator=( ClassName&& rhs )& noexcept                \
   {                                                                          \
     Base::operator=( std::move( rhs ) );                                     \
     member_swap( rhs );                                                      \
     return *this;                                                            \
   }                                                                          \
   __PGBAR_CXX20_CNSTXPR virtual ~ClassName() noexcept = 0;

# define __PGBAR_DEFAULT_METHOD( ClassName )                                                                 \
   constexpr ClassName() noexcept( std::is_nothrow_default_constructible<Base>::value ) = default;           \
   constexpr ClassName( const ClassName& lhs ) noexcept( std::is_nothrow_copy_constructible<Base>::value ) = \
     default;                                                                                                \
   constexpr ClassName( ClassName&& rhs ) noexcept = default;                                                \
   __PGBAR_CXX14_CNSTXPR ClassName& operator=( const ClassName& lhs )& noexcept(                             \
     std::is_nothrow_copy_assignable<Base>::value )                        = default;                        \
   __PGBAR_CXX14_CNSTXPR ClassName& operator=( ClassName&& rhs )& noexcept = default;                        \
   __PGBAR_CXX20_CNSTXPR virtual ~ClassName() noexcept                     = 0;

      template<typename Base, typename Derived>
      class Fonts : public Base {
        enum class Mask : types::Size { Colored = 0, Bolded };
        std::bitset<2> fonts_;

# define __PGBAR_UNPAKING( OptionName, MemberName )                                                 \
   friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unpacking( Fonts& cfg,                       \
                                                                  option::OptionName val ) noexcept \
   {                                                                                                \
     cfg.fonts_[traits::as_val( Mask::OptionName )] = val.value();                                  \
   }
        __PGBAR_UNPAKING( Colored, colored_ )
        __PGBAR_UNPAKING( Bolded, bolded_ )
# undef __PGBAR_UNPAKING

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void member_copy( const Fonts& lhs ) & noexcept
        {
          fonts_ = lhs.fonts_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void member_swap( Fonts& lhs ) & noexcept
        {
          std::swap( fonts_, lhs.fonts_ );
        }

      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::ROStr build_color( types::ROStr ansi_color ) const
        {
          return fonts_[traits::as_val( Mask::Colored )] ? ansi_color : constants::nil_str;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_font( io::Stringbuf& buffer,
                                                                           types::ROStr ansi_color ) const
        {
          return buffer << build_color( ansi_color )
                        << ( fonts_[traits::as_val( Mask::Bolded )] ? console::escape::bold_font
                                                                    : constants::nil_str );
        }

        mutable concurrent::SharedMutex rw_mtx_;

      public:
        constexpr Fonts() noexcept( std::is_nothrow_default_constructible<Base>::value )
          : fonts_ { ( std::numeric_limits<types::BitwiseSet>::max )() }
        {}
        __PGBAR_MEMBER_METHOD( Fonts, __PGBAR_CXX14_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName )                    \
                                                                    \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacking( *this, option::OptionName( ParamName ) );             \
   return static_cast<Derived&>( *this )

        // Enable or disable the color effect.
        Derived& colored( bool _enable ) & { __PGBAR_METHOD( Colored, _enable ); }
        // Enable or disable the bold effect.
        Derived& bolded( bool _enable ) & { __PGBAR_METHOD( Bolded, _enable ); }

# undef __PGBAR_METHOD
# define __PGBAR_METHOD( Offset )                                          \
   concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   return fonts_[traits::as_val( Mask::Offset )]

        // Check whether the color effect is enabled.
        __PGBAR_NODISCARD bool colored() const { __PGBAR_METHOD( Colored ); }
        // Check whether the bold effect is enabled.
        __PGBAR_NODISCARD bool bolded() const { __PGBAR_METHOD( Bolded ); }

# undef __PGBAR_METHOD

        __PGBAR_CXX20_CNSTXPR void swap( Fonts& lhs ) noexcept
        { // CRTP swap.
          static_cast<Derived*>( this )->operator=( std::move( static_cast<Derived&>( lhs ) ) );
        }
        friend __PGBAR_CXX20_CNSTXPR void swap( Fonts& a, Fonts& b ) noexcept { a.swap( b ); }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Fonts<Base, Derived>::~Fonts() noexcept = default;

      template<typename Base, typename Derived>
      class TaskQuantity : public Base {

        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( TaskQuantity& cfg, option::Tasks val )
        {
          cfg.task_range_.end_value( val.value() );
        }

      protected:
        iterators::NumericSpan<types::Size> task_range_;

      public:
        constexpr TaskQuantity() noexcept( std::is_nothrow_default_constructible<Base>::value ) = default;
        __PGBAR_CXX14_CNSTXPR TaskQuantity( const TaskQuantity& lhs )
          noexcept( std::is_nothrow_copy_constructible<Base>::value )
          : Base( lhs )
        {
          task_range_ = lhs.task_range_;
        }
        __PGBAR_CXX17_CNSTXPR TaskQuantity( TaskQuantity&& lhs ) noexcept : Base( std::move( lhs ) )
        {
          task_range_.swap( lhs.task_range_ );
        }
        __PGBAR_CXX14_CNSTXPR TaskQuantity& operator=( const TaskQuantity& lhs ) & noexcept(
          std::is_nothrow_copy_assignable<Base>::value )
        {
          Base::operator=( lhs );
          task_range_ = lhs.task_range_;
          return *this;
        }
        __PGBAR_CXX17_CNSTXPR TaskQuantity& operator=( TaskQuantity&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          task_range_.swap( rhs.task_range_ );
          return *this;
        }

        // Set the number of tasks, passing in zero is no exception.
        Derived& tasks( types::Size param ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacking( *this, option::Tasks( param ) );
          return static_cast<Derived&>( *this );
        }
        // Get the current number of tasks.
        __PGBAR_NODISCARD types::Size tasks() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return task_range_.end_value();
        }

        __PGBAR_CXX20_CNSTXPR virtual ~TaskQuantity() noexcept = 0;
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR TaskQuantity<Base, Derived>::~TaskQuantity() noexcept = default;

      template<typename Base, typename Derived>
      class BasicAnimation : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( BasicAnimation& cfg,
                                                                       option::LeadColor val ) noexcept
        {
          cfg.lead_col_ = std::move( val.value() );
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unpacking( BasicAnimation& cfg,
                                                                       option::Shift val ) noexcept
        {
          cfg.shift_factor_ = val.value() < 0 ? ( 1.0 / ( -val.value() ) ) : val.value();
        }
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( BasicAnimation& cfg,
                                                                       option::Lead val ) noexcept
        {
          if ( std::all_of( val.value().cbegin(),
                            val.value().cend(),
                            []( const charset::U8String& ele ) noexcept { return ele.empty(); } ) ) {
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

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const BasicAnimation& lhs ) &
        {
          shift_factor_      = lhs.shift_factor_;
          lead_col_          = lhs.lead_col_;
          lead_              = lhs.lead_;
          size_longest_lead_ = lhs.size_longest_lead_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( BasicAnimation& lhs ) & noexcept
        {
          using std::swap;
          swap( shift_factor_, lhs.shift_factor_ );
          lead_col_.swap( lhs.lead_col_ );
          lead_.swap( lhs.lead_ );
          swap( size_longest_lead_, lhs.size_longest_lead_ );
        }

      protected:
        types::Float shift_factor_;
        types::String lead_col_;
        std::vector<charset::U8String> lead_;
        types::Size size_longest_lead_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_animation()
          const noexcept
        {
          return size_longest_lead_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR BasicAnimation() noexcept(
          std::is_nothrow_default_constructible<Base>::value
          && std::is_nothrow_default_constructible<types::String>::value
          && std::is_nothrow_default_constructible<std::vector<charset::U8String>>::value ) = default;
        __PGBAR_MEMBER_METHOD( BasicAnimation, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )          \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };  \
   unpacking( *this, option::OptionName( Operation( ParamName ) ) ); \
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
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR BasicAnimation<Base, Derived>::~BasicAnimation() noexcept = default;

      template<typename Base, typename Derived>
      class BasicIndicator : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName )                                                 \
   friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( BasicIndicator& cfg,              \
                                                                  option::OptionName val ) noexcept \
   {                                                                                                \
     cfg.MemberName = std::move( val.value() );                                                     \
   }
        __PGBAR_UNPAKING( Starting, starting_ )
        __PGBAR_UNPAKING( Ending, ending_ )
        __PGBAR_UNPAKING( BarLength, bar_length_ )
        __PGBAR_UNPAKING( StartColor, start_col_ )
        __PGBAR_UNPAKING( EndColor, end_col_ )
        __PGBAR_UNPAKING( FillerColor, filler_col_ )
# undef __PGBAR_UNPAKING

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const BasicIndicator& lhs ) &
        {
          bar_length_ = lhs.bar_length_;
          starting_   = lhs.starting_;
          ending_     = lhs.ending_;
          start_col_  = lhs.start_col_;
          end_col_    = lhs.end_col_;
          filler_col_ = lhs.filler_col_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( BasicIndicator& lhs ) & noexcept
        {
          std::swap( bar_length_, lhs.bar_length_ );
          starting_.swap( lhs.starting_ );
          ending_.swap( lhs.ending_ );
          start_col_.swap( lhs.start_col_ );
          end_col_.swap( lhs.end_col_ );
          filler_col_.swap( lhs.filler_col_ );
        }

      protected:
        types::Size bar_length_;
        charset::U8String starting_, ending_;
        types::String start_col_, end_col_;
        types::String filler_col_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_bar() const noexcept
        {
          return starting_.size() + ending_.size();
        }

      public:
        __PGBAR_CXX20_CNSTXPR BasicIndicator()
          noexcept( std::is_nothrow_default_constructible<Base>::value
                    && std::is_nothrow_default_constructible<types::String>::value
                    && std::is_nothrow_default_constructible<charset::U8String>::value ) = default;
        __PGBAR_MEMBER_METHOD( BasicIndicator, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )          \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };  \
   unpacking( *this, option::OptionName( Operation( ParamName ) ) ); \
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
          __detail::concurrent::SharedLock<__detail::concurrent::SharedMutex> lock { this->rw_mtx_ };
          return bar_length_;
        }

# undef __PGBAR_METHOD
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR BasicIndicator<Base, Derived>::~BasicIndicator() noexcept = default;

      template<typename Base, typename Derived>
      class CharIndicator : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName )                                                 \
   friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( CharIndicator& cfg,               \
                                                                  option::OptionName val ) noexcept \
   {                                                                                                \
     cfg.MemberName = std::move( val.value() );                                                     \
   }
        __PGBAR_UNPAKING( Remains, remains_ )
        __PGBAR_UNPAKING( RemainsColor, remains_col_ )
        __PGBAR_UNPAKING( Filler, filler_ )
# undef __PGBAR_UNPAKING

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const CharIndicator& lhs ) &
        {
          remains_col_ = lhs.remains_col_;
          remains_     = lhs.remains_;
          filler_      = lhs.filler_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( CharIndicator& lhs ) & noexcept
        {
          remains_col_.swap( lhs.remains_col_ );
          remains_.swap( lhs.remains_ );
          filler_.swap( lhs.filler_ );
        }

      protected:
        types::String remains_col_;
        charset::U8String remains_, filler_;

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_char( io::Stringbuf& buffer,
                                                                           types::Size num_frame_cnt,
                                                                           types::Float num_percent ) const
        {
          __PGBAR_ASSERT( num_percent >= 0.0 );
          __PGBAR_ASSERT( num_percent <= 1.0 );

          buffer << console::escape::reset_font << this->build_color( this->start_col_ ) << this->starting_
                 << console::escape::reset_font << this->build_color( this->filler_col_ );

          const auto len_finished = static_cast<types::Size>( std::round( this->bar_length_ * num_percent ) );
          types::Size len_unfinished = this->bar_length_ - len_finished;
          __PGBAR_ASSERT( len_finished + len_unfinished == this->bar_length_ );

          // build filler_
          if ( !filler_.empty() && filler_.size() <= len_finished ) {
            const types::Size fill_num  = len_finished / filler_.size(),
                              remaining = len_finished % filler_.size();
            len_unfinished += remaining;
            buffer.append( filler_, fill_num );
          } else
            len_unfinished += len_finished;
          // build lead_
          buffer << console::escape::reset_font;
          if ( !this->lead_.empty() ) {
            num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
            num_frame_cnt %= this->lead_.size();
            const auto& current_lead = this->lead_[num_frame_cnt];
            if ( current_lead.size() <= len_unfinished ) {
              len_unfinished -= current_lead.size();
              buffer << this->build_color( this->lead_col_ ) << current_lead << console::escape::reset_font;
            }
          }
          // build remains_
          buffer << this->build_color( remains_col_ );
          if ( !this->remains_.empty() && this->remains_.size() <= len_unfinished )
            buffer.append( remains_, len_unfinished / remains_.size() )
              .append( constants::blank, len_unfinished % remains_.size() );
          else
            buffer.append( constants::blank, len_unfinished );

          return buffer << console::escape::reset_font << this->build_color( this->end_col_ )
                        << this->ending_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR CharIndicator()
          noexcept( std::is_nothrow_default_constructible<Base>::value
                    && std::is_nothrow_default_constructible<types::String>::value
                    && std::is_nothrow_default_constructible<charset::U8String>::value ) = default;
        __PGBAR_MEMBER_METHOD( CharIndicator, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )          \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };  \
   unpacking( *this, option::OptionName( Operation( ParamName ) ) ); \
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
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR CharIndicator<Base, Derived>::~CharIndicator() noexcept = default;

      template<typename Base, typename Derived>
      class BlockIndicator : public Base {
      protected:
        const std::array<types::LitStr, 8> filler_ = { "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█" };

        __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_block( io::Stringbuf& buffer,
                                                                            types::Float num_percent ) const
        {
          __PGBAR_ASSERT( num_percent >= 0.0 );
          __PGBAR_ASSERT( num_percent <= 1.0 );

          buffer << console::escape::reset_font << this->build_color( this->start_col_ ) << this->starting_
                 << console::escape::reset_font << this->build_color( this->filler_col_ );

          const auto len_finished = static_cast<types::Size>( std::trunc( this->bar_length_ * num_percent ) );
          const types::Float float_part = ( this->bar_length_ * num_percent ) - len_finished;
          __PGBAR_ASSERT( float_part >= 0.0 );
          __PGBAR_ASSERT( float_part <= 1.0 );
          const types::Size incomplete_block = static_cast<types::Size>( float_part * filler_.size() );
          const types::Size len_unfinished   = this->bar_length_ - len_finished - ( incomplete_block != 0 );
          __PGBAR_ASSERT( len_finished + len_unfinished + ( incomplete_block != 0 ) == this->bar_length_ );

          return buffer.append( filler_.back(), len_finished )
            .append( filler_[incomplete_block], incomplete_block != 0 )
            .append( console::escape::reset_font )
            .append( constants::blank, len_unfinished )
            .append( console::escape::reset_font )
            .append( this->build_color( this->end_col_ ) )
            .append( this->ending_ );
        }

      public:
        __PGBAR_DEFAULT_METHOD( BlockIndicator )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR BlockIndicator<Base, Derived>::~BlockIndicator() noexcept = default;

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

          buffer << console::escape::reset_font;
          return this->build_font( buffer, this->lead_col_ )
              << io::formatting<io::TxtLayout::Left>( this->size_longest_lead_, this->lead_[num_frame_cnt] );
        }

      public:
        __PGBAR_DEFAULT_METHOD( Spinner )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Spinner<Base, Derived>::~Spinner() noexcept = default;

      template<typename Base, typename Derived>
      class Scanner : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( Scanner& cfg,
                                                                       option::Filler val ) noexcept
        {
          cfg.filler_ = std::move( val.value() );
        }

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const Scanner& lhs ) &
        {
          filler_ = lhs.filler_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( Scanner& lhs ) & noexcept
        {
          filler_.swap( lhs.filler_ );
        }

      protected:
        charset::U8String filler_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_scanner(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt ) const
        {
          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
          buffer << console::escape::reset_font << this->build_color( this->start_col_ ) << this->starting_
                 << console::escape::reset_font << this->build_color( this->filler_col_ );

          if ( !this->lead_.empty() ) {
            const auto& current_lead = this->lead_[num_frame_cnt % this->lead_.size()];
            if ( current_lead.size() <= this->bar_length_ ) {
              const auto len_left = [this, num_frame_cnt, &current_lead]() noexcept -> types::Size {
                const types::Size period = ( this->bar_length_ - current_lead.size() - 1 ) * 2;
                const auto remainder     = num_frame_cnt % period;
                return remainder >= ( this->bar_length_ - current_lead.size() ) ? period - remainder
                                                                                : remainder;
              }();
              const types::Size len_right = this->bar_length_ - current_lead.size() - len_left - 1;
              __PGBAR_ASSERT( len_left + len_right + current_lead.size() + 1 == this->bar_length_ );

              buffer.append( filler_, len_left / filler_.size() )
                .append( constants::blank, len_left % filler_.size() )
                .append( console::escape::reset_font )
                .append( this->lead_col_ )
                .append( current_lead )
                .append( console::escape::reset_font )
                .append( this->filler_col_ )
                .append( constants::blank, len_right % filler_.size() )
                .append( filler_, len_right / filler_.size() );
            } else
              buffer.append( constants::blank, this->bar_length_ );
          } else if ( filler_.empty() )
            buffer.append( constants::blank, this->bar_length_ );
          else
            buffer.append( filler_, this->bar_length_ / filler_.size() )
              .append( constants::blank, this->bar_length_ % filler_.size() );

          return buffer << console::escape::reset_font << this->build_color( this->end_col_ )
                        << this->ending_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR Scanner()
          noexcept( std::is_nothrow_default_constructible<Base>::value
                    && std::is_nothrow_default_constructible<charset::U8String>::value ) = default;
        __PGBAR_MEMBER_METHOD( Scanner, __PGBAR_CXX20_CNSTXPR )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& filler( types::String param ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacking( *this, option::Filler( std::move( param ) ) );
          return static_cast<Derived&>( *this );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Scanner<Base, Derived>::~Scanner() noexcept = default;

      template<typename Base, typename Derived>
      class Description : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName )                                                 \
   friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( Description& cfg,                 \
                                                                  option::OptionName val ) noexcept \
   {                                                                                                \
     cfg.MemberName = std::move( val.value() );                                                     \
   }
        __PGBAR_UNPAKING( Description, description_ )
        __PGBAR_UNPAKING( TrueMesg, true_mesg_ )
        __PGBAR_UNPAKING( FalseMesg, false_mesg_ )
        __PGBAR_UNPAKING( DescColor, desc_col_ )
        __PGBAR_UNPAKING( TrueColor, true_col_ )
        __PGBAR_UNPAKING( FalseColor, false_col_ )
# undef __PGBAR_UNPAKING

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const Description& lhs ) &
        {
          desc_col_    = lhs.desc_col_;
          true_col_    = lhs.true_col_;
          false_col_   = lhs.false_col_;
          description_ = lhs.description_;
          true_mesg_   = lhs.true_mesg_;
          false_mesg_  = lhs.false_mesg_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( Description& lhs ) & noexcept
        {
          desc_col_.swap( lhs.desc_col_ );
          true_col_.swap( lhs.true_col_ );
          false_col_.swap( lhs.false_col_ );
          description_.swap( lhs.description_ );
          true_mesg_.swap( lhs.true_mesg_ );
          false_mesg_.swap( lhs.false_mesg_ );
        }

      protected:
        types::String desc_col_;
        types::String true_col_;
        types::String false_col_;

        charset::U8String description_;
        charset::U8String true_mesg_;
        charset::U8String false_mesg_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_description(
          io::Stringbuf& buffer ) const
        {
          if ( description_.empty() )
            return buffer;
          buffer << console::escape::reset_font;
          return this->build_font( buffer, desc_col_ ) << description_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_description( io::Stringbuf& buffer,
                                                                                  bool final_mesg ) const
        {
          if ( ( final_mesg ? true_mesg_ : false_mesg_ ).empty() )
            return build_description( buffer );
          buffer << console::escape::reset_font;
          return this->build_font( buffer, final_mesg ? true_col_ : false_col_ )
              << ( final_mesg ? true_mesg_ : false_mesg_ );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR types::Size fixed_len_description()
          const noexcept
        {
          return std::max( std::max( true_mesg_.size(), false_mesg_.size() ), description_.size() );
        }

      public:
        __PGBAR_CXX20_CNSTXPR Description()
          noexcept( std::is_nothrow_default_constructible<Base>::value
                    && std::is_nothrow_default_constructible<types::String>::value ) = default;
        __PGBAR_MEMBER_METHOD( Description, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )          \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };  \
   unpacking( *this, option::OptionName( Operation( ParamName ) ) ); \
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
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Description<Base, Derived>::~Description() noexcept = default;

      template<typename Base, typename Derived>
      class Segment : public Base {
# define __PGBAR_UNPAKING( OptionName, MemberName, Operation )                                      \
   friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( Segment& cfg,                     \
                                                                  option::OptionName val ) noexcept \
   {                                                                                                \
     cfg.MemberName = Operation( val.value() );                                                     \
   }
        __PGBAR_UNPAKING( Style, visibilities_, )
        __PGBAR_UNPAKING( InfoColor, info_col_, std::move )
        __PGBAR_UNPAKING( Divider, divider_, std::move )
        __PGBAR_UNPAKING( LeftBorder, l_border_, std::move )
        __PGBAR_UNPAKING( RightBorder, r_border_, std::move )
# undef __PGBAR_UNPAKING

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const Segment& lhs ) &
        {
          info_col_ = lhs.info_col_;
          divider_  = lhs.divider_;
          l_border_ = lhs.l_border_;
          r_border_ = lhs.r_border_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( Segment& lhs ) & noexcept
        {
          info_col_.swap( lhs.info_col_ );
          divider_.swap( lhs.divider_ );
          l_border_.swap( lhs.l_border_ );
          r_border_.swap( lhs.r_border_ );
        }

      protected:
        types::String info_col_;
        charset::U8String divider_;
        charset::U8String l_border_, r_border_;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_lborder( io::Stringbuf& buffer ) const
        {
          if ( l_border_.empty() )
            return buffer;
          buffer << console::escape::reset_font;
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
          buffer << console::escape::reset_font;
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
        __PGBAR_CXX20_CNSTXPR Segment()
          noexcept( std::is_nothrow_default_constructible<Base>::value
                    && std::is_nothrow_default_constructible<types::String>::value
                    && std::is_nothrow_default_constructible<charset::U8String>::value ) = default;
        __PGBAR_MEMBER_METHOD( Segment, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( OptionName, ParamName, Operation )          \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };  \
   unpacking( *this, option::OptionName( Operation( ParamName ) ) ); \
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
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Segment<Base, Derived>::~Segment() noexcept = default;

      template<typename Base, typename Derived>
      class PercentMeter : public Base {
# define __PGBAR_DEFAULT_PERCENT " --.--%"
        constexpr static types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_PERCENT ) - 1;

      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_percent( types::Float num_percent ) const
        {
          __PGBAR_ASSERT( num_percent >= 0.0 );
          __PGBAR_ASSERT( num_percent <= 1.0 );

          __PGBAR_UNLIKELY if ( num_percent <= 0.0 ) return { __PGBAR_DEFAULT_PERCENT };

          auto proportion = std::to_string( num_percent * 100.0 );
          proportion.resize( proportion.find( '.' ) + 3 );

          return io::formatting<io::TxtLayout::Right>( _fixed_length, std::move( proportion ) + "%" );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_percent() const noexcept
        {
          return _fixed_length;
        }

      public:
        __PGBAR_DEFAULT_METHOD( PercentMeter )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR PercentMeter<Base, Derived>::~PercentMeter() noexcept = default;

      template<typename Base, typename Derived>
      class SpeedMeter : public Base {
        friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( SpeedMeter& cfg,
                                                                       option::SpeedUnit val ) noexcept
        {
          cfg.units_        = std::move( val.value() );
          cfg.longest_unit_ = std::max( std::max( cfg.units_[0].size(), cfg.units_[1].size() ),
                                        std::max( cfg.units_[2].size(), cfg.units_[3].size() ) );
        }

# define __PGBAR_DEFAULT_SPEED "   inf "
        static constexpr types::Size _fixed_length = sizeof( __PGBAR_DEFAULT_SPEED ) - 1;

        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_copy( const SpeedMeter& lhs ) &
        {
          units_        = lhs.units_;
          longest_unit_ = lhs.longest_unit_;
        }
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void member_swap( SpeedMeter& lhs ) & noexcept
        {
          units_.swap( lhs.units_ );
          std::swap( longest_unit_, lhs.longest_unit_ );
        }

      protected:
        std::array<charset::U8String, 4> units_;
        types::Size longest_unit_;

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_speed( const types::TimeUnit& time_passed,
                                                                       types::Size num_task_done,
                                                                       types::Size num_all_tasks ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          __PGBAR_UNLIKELY if ( num_all_tasks == 0 ) return io::formatting<io::TxtLayout::Right>(
            _fixed_length + longest_unit_,
            "-- " + units_.front() );

          const auto float2string = []( types::Float val ) -> types::String {
            auto str = std::to_string( std::round( val * 100.0 ) / 100.0 );
            str.resize( str.find( '.' ) + 3 ); // Keep two decimal places.
            str.append( 1, constants::blank );
            return str;
          };

          const auto seconds_passed    = std::chrono::duration<types::Float>( time_passed ).count();
          // zero or negetive is invalid
          const types::Float frequency = seconds_passed <= 0.0 ? ( std::numeric_limits<types::Float>::max )()
                                                               : num_task_done / seconds_passed;
          types::String rate_str;
          if ( frequency < 1e3 ) // < 1 Hz => '999.99 Hz'
            rate_str = float2string( frequency ) + units_[0];
          else if ( frequency < 1e6 ) // < 1 kHz => '999.99 kHz'
            rate_str = float2string( frequency / 1e3 ) + units_[1];
          else if ( frequency < 1e9 ) // < 1 MHz => '999.99 MHz'
            rate_str = float2string( frequency / 1e6 ) + units_[2];
          else { // > 999 GHz => infinity
            const types::Float remains                        = frequency / 1e9;
            __PGBAR_UNLIKELY if ( remains > 999.99 ) rate_str = __PGBAR_DEFAULT_SPEED + units_[0];
            else rate_str                                     = float2string( remains ) + units_[3];
          }

          return io::formatting<io::TxtLayout::Right>( _fixed_length + longest_unit_, rate_str );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_speed() const noexcept
        {
          return _fixed_length + longest_unit_;
        }

      public:
        __PGBAR_CXX20_CNSTXPR SpeedMeter()
          noexcept( std::is_nothrow_default_constructible<Base>::value
                    && std::is_nothrow_default_constructible<charset::U8String>::value ) = default;
        __PGBAR_MEMBER_METHOD( SpeedMeter, __PGBAR_CXX20_CNSTXPR )

# define __PGBAR_METHOD( ParamName )                                \
   std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
   unpacking( *this, option::SpeedUnit( std::move( ParamName ) ) ); \
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
        Derived& speed_unit( std::array<types::String, 4> _units ) & { __PGBAR_METHOD( _units ); }
# if __PGBAR_CXX20
        /**
         * @param _units
         * The given each unit will be treated as 1,000 times greater than the previous one
         * (from left to right).
         */
        Derived& speed_unit( std::array<std::u8string_view, 4> _units ) & { __PGBAR_METHOD( _units ); }
# endif

# undef __PGBAR_METHOD
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR SpeedMeter<Base, Derived>::~SpeedMeter() noexcept = default;

      template<typename Base, typename Derived>
      class CounterMeter : public Base {
      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_counter( types::Size num_task_done,
                                                                         types::Size num_all_tasks ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          if ( num_all_tasks == 0 )
            return { "-/-" };
          types::String total_str = std::to_string( num_all_tasks );
          const types::Size size  = total_str.size();
          return io::formatting<io::TxtLayout::Right>( size, std::to_string( num_task_done ) ) + "/"
               + std::move( total_str );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size fixed_len_counter() const noexcept
        {
          return ( ( this->task_range_.end_value() == 0
                       ? 0
                       : static_cast<types::Size>( std::log10( this->task_range_.end_value() ) ) )
                   + 1 )
                 * 2
               + 1;
        }

      public:
        __PGBAR_DEFAULT_METHOD( CounterMeter )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR CounterMeter<Base, Derived>::~CounterMeter() noexcept = default;

      template<typename Base, typename Derived>
      class Timer : public Base {
# define __PGBAR_DEFAULT_TIMER "--:--:--"

      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String time_formatter( types::TimeUnit duration ) const
        {
          const auto time2str = []( std::int64_t num_time ) -> types::String {
            auto ret = std::to_string( num_time );
            if ( ret.size() < 2 )
              return "0" + std::move( ret );
            return ret;
          };
          const auto hours = std::chrono::duration_cast<std::chrono::hours>( duration );
          duration -= hours;
          const auto minutes = std::chrono::duration_cast<std::chrono::minutes>( duration );
          duration -= minutes;
          return ( ( ( hours.count() > 99 ? types::String( "--" ) : time2str( hours.count() ) ) + ":" )
                   + ( time2str( minutes.count() ) + ":" )
                   + time2str( std::chrono::duration_cast<std::chrono::seconds>( duration ).count() ) );
        }

      public:
        __PGBAR_DEFAULT_METHOD( Timer )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR Timer<Base, Derived>::~Timer() noexcept = default;

      template<typename Base, typename Derived>
      class ElapsedTimer : public Base {
      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_elapsed( types::TimeUnit time_passed ) const
        {
          return this->time_formatter( std::move( time_passed ) );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_elapsed() const noexcept
        {
          return sizeof( __PGBAR_DEFAULT_TIMER ) - 1;
        }

      public:
        __PGBAR_DEFAULT_METHOD( ElapsedTimer )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR ElapsedTimer<Base, Derived>::~ElapsedTimer() noexcept = default;

      template<typename Base, typename Derived>
      class CountdownTimer : public Base {
      protected:
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String build_countdown( const types::TimeUnit& time_passed,
                                                                           types::Size num_task_done,
                                                                           types::Size num_all_tasks ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
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
            return this->time_formatter( time_per_task * remaining_tasks );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr types::Size fixed_len_countdown() const noexcept
        {
          return sizeof( __PGBAR_DEFAULT_TIMER ) - 1;
        }

      public:
        __PGBAR_DEFAULT_METHOD( CountdownTimer )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR CountdownTimer<Base, Derived>::~CountdownTimer() noexcept = default;

      template<typename Base, typename Derived>
      class TaskCounter : public Base {
      protected:
        std::atomic<__detail::types::Size> task_cnt_, task_end_;

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
        __PGBAR_CXX20_CNSTXPR virtual ~TaskCounter() noexcept = 0;

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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR iterators::ProxySpan<iterators::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_arithmetic<N>::value,
                                  iterators::ProxySpan<iterators::NumericSpan<N>, Derived>>::type
# endif
          iterate( N startpoint, N endpoint, N step ) &
        { // default parameter will cause ambiguous overloads
          // so we have to write them all
          return { iterators::NumericSpan<typename std::decay<N>::type>( startpoint, endpoint, step ),
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
        __PGBAR_NODISCARD iterators::ProxySpan<iterators::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD
          typename std::enable_if<std::is_floating_point<N>::value,
                                  iterators::ProxySpan<iterators::NumericSpan<N>, Derived>>::type
# endif
          iterate( N endpoint, N step ) &
        {
          return { iterators::NumericSpan<typename std::decay<N>::type>( {}, endpoint, step ),
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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR iterators::ProxySpan<iterators::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  iterators::ProxySpan<iterators::NumericSpan<N>, Derived>>::type
# endif
          iterate( N startpoint, N endpoint ) &
        {
          return { iterators::NumericSpan<typename std::decay<N>::type>( startpoint, endpoint, 1 ),
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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR iterators::ProxySpan<iterators::NumericSpan<N>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  iterators::ProxySpan<iterators::NumericSpan<N>, Derived>>::type
# endif
          iterate( N endpoint ) &
        {
          return { iterators::NumericSpan<typename std::decay<N>::type>( {}, endpoint, 1 ),
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
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR iterators::ProxySpan<iterators::IterSpan<I>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX14_CNSTXPR
          typename std::enable_if<!std::is_arithmetic<I>::value,
                                  iterators::ProxySpan<iterators::IterSpan<I>, Derived>>::type
# endif
          iterate( I startpoint, I endpoint ) & noexcept(
            std::is_pointer<typename std::decay<I>::type>::value
            || std::is_nothrow_move_constructible<typename std::decay<I>::type>::value )
        {
          return { iterators::IterSpan<typename std::decay<I>::type>( std::move( startpoint ),
                                                                      std::move( endpoint ) ),
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
        // iterators.
        template<class R>
# if __PGBAR_CXX20
          requires std::disjunction_v<std::is_class<std::decay_t<R>>,
                                      std::is_array<std::remove_reference_t<R>>>
                && std::is_lvalue_reference_v<R>
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR
          iterators::ProxySpan<iterators::IterSpan<traits::IteratorTrait_t<R>>, Derived>
# else
        __PGBAR_NODISCARD __PGBAR_CXX17_CNSTXPR typename std::enable_if<
          ( std::is_class<typename std::decay<R>::type>::value
            || std::is_array<typename std::remove_reference<R>::type>::value )
            && std::is_lvalue_reference<R>::value,
          iterators::ProxySpan<iterators::IterSpan<traits::IteratorTrait_t<R>>, Derived>>::type
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
          requires std::disjunction_v<std::is_class<std::decay_t<R>>,
                                      std::is_array<std::remove_reference_t<R>>>
                && std::is_lvalue_reference_v<R>
        __PGBAR_CXX17_CNSTXPR void
# else
        __PGBAR_CXX17_CNSTXPR
          typename std::enable_if<( std::is_class<typename std::decay<R>::type>::value
                                    || std::is_array<typename std::remove_reference<R>::type>::value )
                                  && std::is_lvalue_reference<R>::value>::type
# endif
          iterate( R&& container, F&& unary_fn )
        {
          for ( auto&& e : iterate( container ) )
            unary_fn( std::forward<decltype( e )>( e ) );
        }
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR TaskCounter<Base, Derived>::~TaskCounter() noexcept = default;

      template<typename Base, typename Derived>
      class FrameCounter : public Base {
      protected:
        __detail::types::Size idx_frame_;

      public:
        __PGBAR_DEFAULT_METHOD( FrameCounter )
      };
      template<typename Base, typename Derived>
      __PGBAR_CXX20_CNSTXPR FrameCounter<Base, Derived>::~FrameCounter() noexcept = default;
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER( assets::TaskQuantity, assets::Fonts, );
      __PGBAR_INHERIT_REGISTER( assets::BasicAnimation, assets::Fonts, );
      __PGBAR_INHERIT_REGISTER( assets::BasicIndicator, assets::Fonts, );

      __PGBAR_INHERIT_REGISTER( assets::CharIndicator,
                                assets::TaskQuantity,
                                __PGBAR_PACK( assets::BasicAnimation, assets::BasicIndicator ) );
      __PGBAR_INHERIT_REGISTER( assets::BlockIndicator, assets::TaskQuantity, assets::BasicIndicator );

      __PGBAR_INHERIT_REGISTER( assets::Spinner, , assets::BasicAnimation );
      __PGBAR_INHERIT_REGISTER( assets::Scanner,
                                ,
                                __PGBAR_PACK( assets::BasicAnimation, assets::BasicIndicator ) );

      __PGBAR_INHERIT_REGISTER( assets::Description, assets::Fonts, );
      __PGBAR_INHERIT_REGISTER( assets::Segment, assets::Fonts, );

      __PGBAR_INHERIT_REGISTER( assets::PercentMeter, assets::TaskQuantity, );
      __PGBAR_INHERIT_REGISTER( assets::SpeedMeter, assets::TaskQuantity, );
      __PGBAR_INHERIT_REGISTER( assets::CounterMeter, assets::TaskQuantity, );

      __PGBAR_INHERIT_REGISTER( assets::ElapsedTimer, assets::Timer, );
      __PGBAR_INHERIT_REGISTER( assets::CountdownTimer,
                                __PGBAR_PACK( assets::TaskQuantity, assets::Timer ), );

      // Following are the types of `option` that each `asset` can receive.

      using GroupFonts        = TypeList<option::Colored, option::Bolded>;
      using GroupTaskQuantity = TypeList<option::Tasks>;
      using GroupDescription  = TypeList<option::Description,
                                         option::TrueMesg,
                                         option::FalseMesg,
                                         option::DescColor,
                                         option::TrueColor,
                                         option::FalseColor>;
      using GroupSegment =
        TypeList<option::Divider, option::LeftBorder, option::RightBorder, option::InfoColor>;
      using GroupSpeedMeter = TypeList<option::SpeedUnit>;
      using GroupBitOption  = TypeList<option::Style>;

      using GroupBasicAnimation = TypeList<option::Shift, option::Lead, option::LeadColor>;
      using GroupBasicBar       = TypeList<option::Starting,
                                           option::Ending,
                                           option::StartColor,
                                           option::EndColor,
                                           option::BarLength,
                                           option::FillerColor>;

      using GroupCharIndicator  = Merge_t<GroupBasicAnimation,
                                          GroupBasicBar,
                                          TypeList<option::Remains, option::Filler, option::RemainsColor>>;
      using GroupBlockIndicator = GroupBasicBar;
      using GroupSpinner        = GroupBasicAnimation;
      using GroupScanner        = Merge_t<GroupBasicAnimation, GroupBasicBar, TypeList<option::Filler>>;
    } // namespace traits

    namespace render {
      template<typename ConfigType>
      __PGBAR_INLINE_FN void default_initializer( ConfigType& )
      {
        static_assert( false, "pgbar::__detail::render::default_initializer: No implemented" );
      }
      template<typename ConfigType, typename Enable = void>
      struct ConfigInfo;
    } // namespace render
  } // namespace __detail

  namespace config {
    class Core {
      static const bool _stdout_in_tty;
      static const bool _stderr_in_tty;

    public:
      using TimeUnit = __detail::types::TimeUnit;

      // Get the current output interval.
      __PGBAR_NODISCARD static TimeUnit refresh_interval()
      {
        return __detail::concurrent::StateThread::working_interval();
      }
      // Set the new output interval.
      static void refresh_interval( TimeUnit new_rate )
      {
        __detail::concurrent::StateThread::working_interval( std::move( new_rate ) );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN static bool intty( StreamChannel stream_type ) noexcept
      {
        return stream_type == StreamChannel::Stdout ? _stdout_in_tty : _stderr_in_tty;
      }

      constexpr Core() noexcept              = default;
      constexpr Core( const Core& ) noexcept = default;
      constexpr Core( Core&& ) noexcept      = default;
      __PGBAR_CXX17_CNSTXPR Core& operator=( const Core& lhs ) &
      {
        __PGBAR_ASSERT( this != &lhs );
        (void)lhs;
        return *this;
      }
      __PGBAR_CXX17_CNSTXPR Core& operator=( Core&& rhs ) & noexcept
      {
        __PGBAR_ASSERT( this != &rhs );
        (void)rhs;
        return *this;
      }

      __PGBAR_CXX20_CNSTXPR virtual ~Core() noexcept = 0;
    };
    const bool Core::_stdout_in_tty              = __detail::console::intty<StreamChannel::Stdout>();
    const bool Core::_stderr_in_tty              = __detail::console::intty<StreamChannel::Stderr>();
    __PGBAR_CXX20_CNSTXPR Core::~Core() noexcept = default;

    template<template<typename...> class BarType, typename OptionConstraint>
    class BasicConfig
      : public __detail::traits::LI<
          BarType,
          __detail::assets::Description,
          __detail::assets::Segment,
          __detail::assets::PercentMeter,
          __detail::assets::SpeedMeter,
          __detail::assets::CounterMeter,
          __detail::assets::ElapsedTimer,
          __detail::assets::CountdownTimer>::template type<Core, BasicConfig<BarType, OptionConstraint>> {
      // BarType must inherit from BasicIndicator or BasicAnimation
      static_assert( __detail::traits::Contain<__detail::traits::TopoSort_t<BarType>,
                                               __detail::assets::BasicIndicator>::value
                       || __detail::traits::Contain<__detail::traits::TopoSort_t<BarType>,
                                                    __detail::assets::BasicAnimation>::value,
                     "pgbar::config::BasicConfig: Invalid progress bar type" );

      using Self = BasicConfig;
      using Base = typename __detail::traits::LI<BarType,
                                                 __detail::assets::Description,
                                                 __detail::assets::Segment,
                                                 __detail::assets::PercentMeter,
                                                 __detail::assets::SpeedMeter,
                                                 __detail::assets::CounterMeter,
                                                 __detail::assets::ElapsedTimer,
                                                 __detail::assets::CountdownTimer>::template type<Core, Self>;

      template<typename, typename>
      friend struct __detail::render::ConfigInfo;

      friend __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void unpacking( BasicConfig& cfg,
                                                                     option::Style val ) noexcept
      {
        cfg.visual_masks_ = val.value();
      }

    protected:
      enum class Mask : __detail::types::Size { Per = 0, Ani, Cnt, Sped, Elpsd, Cntdwn };
      std::bitset<6> visual_masks_;

    public:
      // Percent Meter
      static constexpr __detail::types::BitwiseSet Per    = 1 << 0;
      // Animation
      static constexpr __detail::types::BitwiseSet Ani    = 1 << 1;
      // Task Progress Counter
      static constexpr __detail::types::BitwiseSet Cnt    = 1 << 2;
      // Speed Meter
      static constexpr __detail::types::BitwiseSet Sped   = 1 << 3;
      // Elapsed Timer
      static constexpr __detail::types::BitwiseSet Elpsd  = 1 << 4;
      // Countdown Timer
      static constexpr __detail::types::BitwiseSet Cntdwn = 1 << 5;
      // Enable all components
      static constexpr __detail::types::BitwiseSet Entire = ~0;

      BasicConfig() { __detail::render::default_initializer( *this ); }
      template<
        typename Arg,
        typename... Args,
        typename = typename std::enable_if<__detail::traits::AllBelongAny<
          __detail::traits::TypeList<typename std::decay<Arg>::type, typename std::decay<Args>::type...>,
          __detail::traits::GroupFonts,
          __detail::traits::GroupTaskQuantity,
          OptionConstraint,
          __detail::traits::GroupDescription,
          __detail::traits::GroupSegment,
          __detail::traits::GroupSpeedMeter,
          __detail::traits::GroupBitOption>::value>::type>
      BasicConfig( Arg&& arg, Args&&... args ) : BasicConfig()
      {
        unpacking( *this, std::forward<Arg>( arg ) );
        static_cast<void>(
          std::initializer_list<char> { ( unpacking( *this, std::forward<Args>( args ) ), '\0' )... } );
      }

      BasicConfig( const Self& ) noexcept( std::is_nothrow_copy_constructible<Base>::value ) = default;
      BasicConfig( Self&& ) noexcept                                                         = default;
      Self& operator=( const Self& lhs ) & noexcept( std::is_nothrow_copy_assignable<Base>::value )
      {
        std::lock_guard<__detail::concurrent::SharedMutex> lock1 { this->rw_mtx_ };
        __detail::concurrent::SharedLock<__detail::concurrent::SharedMutex> lock2 { lhs.rw_mtx_ };
        Base::operator=( lhs );
        visual_masks_ = lhs.visual_masks_;
        return *this;
      }
      Self& operator=( Self&& rhs ) & noexcept
      {
        std::lock_guard<__detail::concurrent::SharedMutex> lock1 { this->rw_mtx_ };
        std::lock_guard<__detail::concurrent::SharedMutex> lock2 { rhs.rw_mtx_ };
        Base::operator=( std::move( rhs ) );
        using std::swap;
        swap( visual_masks_, rhs.visual_masks_ );
        return *this;
      }
      virtual ~BasicConfig() noexcept = default;

      Self& style( __detail::types::BitwiseSet val ) &
      {
        std::lock_guard<__detail::concurrent::SharedMutex> lock { this->rw_mtx_ };
        unpacking( *this, option::Style( val ) );
        return *this;
      }

      __PGBAR_NODISCARD __detail::types::Size fixed_size() const
      {
        __detail::concurrent::SharedLock<__detail::concurrent::SharedMutex> lock { this->rw_mtx_ };
        return __detail::render::ConfigInfo<Self>::fixed_render_size( *this );
      }

      template<typename... Args>
      typename std::enable_if<
        __detail::traits::AllBelongAny<__detail::traits::TypeList<typename std::decay<Args>::type...>,
                                       __detail::traits::GroupFonts,
                                       __detail::traits::GroupTaskQuantity,
                                       OptionConstraint,
                                       __detail::traits::GroupDescription,
                                       __detail::traits::GroupSegment,
                                       __detail::traits::GroupSpeedMeter,
                                       __detail::traits::GroupBitOption>::value,
        Self&>::type
        set( Args&&... args ) &
      {
        std::lock_guard<__detail::concurrent::SharedMutex> lock { this->rw_mtx_ };
        static_cast<void>(
          std::initializer_list<char> { ( unpacking( *this, std::forward<Args>( args ) ), '\0' )... } );
        return *this;
      }
    };

    using CharBar = BasicConfig<__detail::assets::CharIndicator, __detail::traits::GroupCharIndicator>;
    using BlckBar = BasicConfig<__detail::assets::BlockIndicator, __detail::traits::GroupBlockIndicator>;
    using SpinBar = BasicConfig<__detail::assets::Spinner, __detail::traits::GroupSpinner>;
    using ScanBar = BasicConfig<__detail::assets::Scanner, __detail::traits::GroupScanner>;
  } // namespace config

  namespace __detail {
    namespace traits {
      template<typename C>
      struct ConfigTrait;

# define __PGBAR_TRAIT_REGISTER( ConfigType, OptionConstraint, ... ) \
   template<>                                                        \
   struct ConfigTrait<ConfigType> {                                  \
     using Constraint = OptionConstraint;                            \
     using TraitsList = TemplateList<__VA_ARGS__>;                   \
   }

      template<typename C>
      using ConfigTrait_c = typename ConfigTrait<C>::Constraint;
      template<typename C>
      using ConfigTrait_t = typename ConfigTrait<C>::TraitsList;

      __PGBAR_TRAIT_REGISTER( config::CharBar,
                              GroupCharIndicator,
                              assets::TaskCounter,
                              assets::FrameCounter );
      __PGBAR_TRAIT_REGISTER( config::BlckBar, GroupBlockIndicator, assets::TaskCounter );
      __PGBAR_TRAIT_REGISTER( config::SpinBar, GroupSpinner, assets::TaskCounter, assets::FrameCounter );
      __PGBAR_TRAIT_REGISTER( config::ScanBar, GroupScanner, assets::TaskCounter, assets::FrameCounter );
    } // namespace traits

    namespace render {
      template<>
      __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void default_initializer<config::CharBar>(
        config::CharBar& cfg )
      {
        unpacking( cfg, option::Shift( -2 ) );
        unpacking( cfg, option::Lead( ">" ) );
        unpacking( cfg, option::Starting( "[" ) );
        unpacking( cfg, option::Ending( "]" ) );
        unpacking( cfg, option::BarLength( 30 ) );
        unpacking( cfg, option::Filler( "=" ) );
        unpacking( cfg, option::Remains( " " ) );
        unpacking( cfg, option::Divider( " | " ) );
        unpacking( cfg, option::InfoColor( color::Cyan ) );
        unpacking( cfg, option::SpeedUnit( { "Hz", "kHz", "MHz", "GHz" } ) );
        unpacking( cfg, option::Style( config::CharBar::Entire ) );
      }
      template<>
      __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void default_initializer<config::BlckBar>(
        config::BlckBar& cfg )
      {
        unpacking( cfg, option::BarLength( 30 ) );
        unpacking( cfg, option::Divider( " | " ) );
        unpacking( cfg, option::InfoColor( color::Cyan ) );
        unpacking( cfg, option::SpeedUnit( { "Hz", "kHz", "MHz", "GHz" } ) );
        unpacking( cfg, option::Style( config::CharBar::Entire ) );
      }
      template<>
      __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void default_initializer<config::SpinBar>(
        config::SpinBar& cfg )
      {
        unpacking( cfg, option::Shift( -3 ) );
        unpacking( cfg, option::Lead( { "/", "-", "\\", "|" } ) );
        unpacking( cfg, option::Divider( " | " ) );
        unpacking( cfg, option::InfoColor( color::Cyan ) );
        unpacking( cfg, option::SpeedUnit( { "Hz", "kHz", "MHz", "GHz" } ) );
        unpacking( cfg, option::Style( config::SpinBar::Ani | config::SpinBar::Elpsd ) );
      }
      template<>
      __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR void default_initializer<config::ScanBar>(
        config::ScanBar& cfg )
      {
        unpacking( cfg, option::Shift( -3 ) );
        unpacking( cfg, option::Starting( "[" ) );
        unpacking( cfg, option::Ending( "]" ) );
        unpacking( cfg, option::BarLength( 30 ) );
        unpacking( cfg, option::Filler( "-" ) );
        unpacking( cfg, option::Lead( "<==>" ) );
        unpacking( cfg, option::Divider( " | " ) );
        unpacking( cfg, option::InfoColor( color::Cyan ) );
        unpacking( cfg, option::SpeedUnit( { "Hz", "kHz", "MHz", "GHz" } ) );
        unpacking( cfg, option::Style( config::ScanBar::Ani | config::ScanBar::Elpsd ) );
      }

      template<typename ConfigType>
      struct ConfigInfo<ConfigType,
                        typename std::enable_if<std::is_same<ConfigType, config::CharBar>::value
                                                || std::is_same<ConfigType, config::BlckBar>::value
                                                || std::is_same<ConfigType, config::ScanBar>::value>::type> {
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN types::Size fixed_render_size(
          const ConfigType& cfg ) noexcept
        {
          using Self = ConfigType;
          return cfg.fixed_len_description()
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Per )] ? cfg.fixed_len_percent() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Ani )] ? cfg.fixed_len_bar() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Cnt )] ? cfg.fixed_len_counter() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Sped )] ? cfg.fixed_len_speed() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Elpsd )] ? cfg.fixed_len_elapsed() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Cntdwn )] ? cfg.fixed_len_countdown() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Elpsd )]
                       && cfg.visual_masks_[traits::as_val( Self::Mask::Cntdwn )]
                     ? 3
                     : 0 )
               + cfg.fixed_len_segment(
                 cfg.visual_masks_.count()
                 - ( cfg.visual_masks_[traits::as_val( Self::Mask::Cntdwn )]
                     && cfg.visual_masks_[traits::as_val( Self::Mask::Elpsd )] )
                 + ( !cfg.true_mesg_.empty() || !cfg.false_mesg_.empty() || !cfg.description_.empty() ) )
               + 1;
        }
      };
      template<>
      struct ConfigInfo<config::SpinBar, void> {
        __PGBAR_NODISCARD static __PGBAR_INLINE_FN types::Size fixed_render_size(
          const config::SpinBar& cfg ) noexcept
        {
          using Self = config::SpinBar;
          return ( cfg.visual_masks_[traits::as_val( Self::Mask::Ani )]
                     ? cfg.fixed_len_animation() + cfg.fixed_len_description()
                         + ( !cfg.true_mesg_.empty() || !cfg.false_mesg_.empty()
                             || !cfg.description_.empty() )
                     : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Per )] ? cfg.fixed_len_percent() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Cnt )] ? cfg.fixed_len_counter() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Sped )] ? cfg.fixed_len_speed() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Elpsd )] ? cfg.fixed_len_elapsed() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Cntdwn )] ? cfg.fixed_len_countdown() : 0 )
               + ( cfg.visual_masks_[traits::as_val( Self::Mask::Elpsd )]
                       && cfg.visual_masks_[traits::as_val( Self::Mask::Cntdwn )]
                     ? 3
                     : 0 )
               + cfg.fixed_len_segment( cfg.visual_masks_.count()
                                        - ( cfg.visual_masks_[traits::as_val( Self::Mask::Cntdwn )]
                                            && cfg.visual_masks_[traits::as_val( Self::Mask::Elpsd )] ) )
               + 1;
        }
      };

      template<typename ConfigType>
      struct CommonBuilder : public ConfigType {
        using ConfigType::ConfigType;
        CommonBuilder( const ConfigType& config )
          noexcept( std::is_nothrow_copy_constructible<ConfigType>::value )
          : ConfigType( config )
        {}
        CommonBuilder( ConfigType&& config ) noexcept( std::is_nothrow_move_constructible<ConfigType>::value )
          : ConfigType( std::move( config ) )
        {}
        virtual ~CommonBuilder() noexcept = default;

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
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          using Self = ConfigType;
          if ( this->visual_masks_[traits::as_val( Self::Mask::Cnt )]
               || this->visual_masks_[traits::as_val( Self::Mask::Sped )]
               || this->visual_masks_[traits::as_val( Self::Mask::Elpsd )]
               || this->visual_masks_[traits::as_val( Self::Mask::Cntdwn )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            if ( this->visual_masks_[traits::as_val( Self::Mask::Cnt )] ) {
              buffer << this->build_counter( num_task_done, num_all_tasks );
              if ( this->visual_masks_[traits::as_val( Self::Mask::Sped )]
                   || this->visual_masks_[traits::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[traits::as_val( Self::Mask::Cntdwn )] )
                this->build_divider( buffer );
            }
            const auto time_passed = std::chrono::steady_clock::now() - zero_point;
            if ( this->visual_masks_[traits::as_val( Self::Mask::Sped )] ) {
              buffer << this->build_speed( time_passed, num_task_done, num_all_tasks );
              if ( this->visual_masks_[traits::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[traits::as_val( Self::Mask::Cntdwn )] )
                this->build_divider( buffer );
            }
            if ( this->visual_masks_[traits::as_val( Self::Mask::Elpsd )] ) {
              buffer << this->build_elapsed( time_passed );
              if ( this->visual_masks_[traits::as_val( Self::Mask::Cntdwn )] )
                buffer << " < ";
            }
            if ( this->visual_masks_[traits::as_val( Self::Mask::Cntdwn )] )
              buffer << this->build_countdown( time_passed, num_task_done, num_all_tasks );
          }
          return buffer;
        }
      };

      template<typename ConfigType>
      struct Builder;
      template<>
      struct Builder<config::CharBar> final : public CommonBuilder<config::CharBar> {
        using Self = config::CharBar;
        using CommonBuilder<Self>::CommonBuilder;
        virtual ~Builder() noexcept = default;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer );
          if ( !this->description_.empty() && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_char( buffer, num_frame_cnt, num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer, final_mesg );
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_char( buffer, num_frame_cnt, num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size full_render_size() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return ConfigInfo<Self>::fixed_render_size( *this )
               + ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ? this->bar_length_ : 0 );
        }
      };

      template<>
      struct Builder<config::BlckBar> final : public CommonBuilder<config::BlckBar> {
        using Self = config::BlckBar;
        using CommonBuilder<Self>::CommonBuilder;
        virtual ~Builder() noexcept = default;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer );
          if ( !this->description_.empty() && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_block( buffer, num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer, final_mesg );
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_block( buffer, num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size full_render_size() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return ConfigInfo<Self>::fixed_render_size( *this )
               + ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ? this->bar_length_ : 0 );
        }
      };

      template<>
      struct Builder<config::SpinBar> final : public CommonBuilder<config::SpinBar> {
        using Self = config::SpinBar;
        using CommonBuilder<Self>::CommonBuilder;
        virtual ~Builder() noexcept = default;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( this->visual_masks_.any() )
            this->build_lborder( buffer );

          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_spinner( buffer, num_frame_cnt );
            if ( !this->description_.empty() ) {
              buffer << constants::blank;
              this->build_description( buffer );
            }
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( this->visual_masks_.any() )
            this->build_lborder( buffer );

          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            if ( ( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty() ) {
              this->build_spinner( buffer, num_frame_cnt );
              if ( !this->description_.empty() )
                buffer << constants::blank;
            }
            this->build_description( buffer, final_mesg );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size full_render_size() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return ConfigInfo<Self>::fixed_render_size( *this );
        }
      };

      template<>
      struct Builder<config::ScanBar> final : public CommonBuilder<config::ScanBar> {
        using Self = config::ScanBar;
        using CommonBuilder<Self>::CommonBuilder;
        virtual ~Builder() noexcept = default;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer );
          if ( !this->description_.empty() && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_scanner( buffer, num_frame_cnt );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->description_.empty() || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          types::Size num_task_done,
          types::Size num_all_tasks,
          bool final_mesg,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_ASSERT( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_lborder( buffer );

          this->build_description( buffer, final_mesg );
          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               && this->visual_masks_.any() )
            this->build_divider( buffer );
          if ( this->visual_masks_[traits::as_val( Self::Mask::Per )] ) {
            buffer << console::escape::reset_font;
            this->build_font( buffer, this->info_col_ );
            buffer << this->build_percent( num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Per ) ).any() )
              this->build_divider( buffer );
          }
          if ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ) {
            this->build_scanner( buffer, num_frame_cnt );
            auto masks = this->visual_masks_;
            if ( masks.reset( traits::as_val( Self::Mask::Ani ) )
                   .reset( traits::as_val( Self::Mask::Per ) )
                   .any() )
              this->build_divider( buffer );
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( ( !( final_mesg ? this->true_mesg_ : this->false_mesg_ ).empty()
                 || !this->description_.empty() )
               || this->visual_masks_.any() )
            this->build_rborder( buffer );
          return buffer << console::escape::reset_font;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size full_render_size() const
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return ConfigInfo<Self>::fixed_render_size( *this )
               + ( this->visual_masks_[traits::as_val( Self::Mask::Ani )] ? this->bar_length_ : 0 );
        }
      };

      // A manager class used to synchronize the rendering thread and main thread.
      class Renderer final {
        using Self = Renderer;
        concurrent::StateThread state_td_;

      public:
        Renderer( const Self& )        = delete;
        Self& operator=( const Self& ) = delete;

        Renderer() noexcept : state_td_ { concurrent::thread_repo.pop() }
        {
          __PGBAR_ASSERT( state_td_.jobless() );
        }
        ~Renderer() noexcept
        {
          state_td_.appoint();
          __PGBAR_ASSERT( state_td_.active() == false );
          __PGBAR_ASSERT( state_td_.jobless() );
          concurrent::thread_repo.push( std::move( state_td_ ) );
        }

        __PGBAR_INLINE_FN void reset( wrappers::UniqueFunction<void()> task )
        {
          state_td_.appoint( std::move( task ) );
        }
        __PGBAR_INLINE_FN void reset() noexcept { state_td_.halt(); }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept { return state_td_.jobless(); }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept { return state_td_.active(); }

        void activate() & { state_td_.activate(); }
        void suspend() & { state_td_.suspend(); }
      };

      // customization point
      template<typename ConfigType, typename Enable = void>
      struct TickAction;
      template<typename ConfigType, typename Enable = void>
      struct RenderAction;
    } // namespace render
  } // namespace __detail

  using Threadsafe = __detail::concurrent::Mutex;
  // A empty class that satisfies the "Basic lockable" requirement.
  class Threadunsafe final {
  public:
    constexpr Threadunsafe() noexcept              = default;
    __PGBAR_CXX20_CNSTXPR ~Threadunsafe() noexcept = default;
    __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void lock() noexcept {}
    __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR void unlock() noexcept {}
  };

  class Indicator {
  protected:
    enum class state : uint8_t { Begin, Refresh1, Refresh2, Finish, Stopped };
    std::atomic<state> state_;

    __detail::render::Renderer executor_;

    std::chrono::steady_clock::time_point zero_point_;
    __detail::types::Size max_bar_size_;
    bool final_mesg_;

    void unlock_reset( bool final_mesg )
    {
      if ( executor_.active() ) {
        final_mesg_     = final_mesg;
        auto try_update = [this]( state expected ) noexcept {
          return state_.compare_exchange_strong( expected,
                                                 state::Finish,
                                                 std::memory_order_acq_rel,
                                                 std::memory_order_relaxed );
        };
        try_update( state::Begin ) || try_update( state::Refresh1 ) || try_update( state::Refresh2 );
        this->executor_.suspend();
      } else
        state_.store( state::Stopped, std::memory_order_release );
    }

  public:
    Indicator() noexcept : state_ { state::Stopped } {}
    Indicator( Indicator&& ) noexcept : Indicator() {}
    Indicator& operator=( Indicator&& rhs ) & noexcept
    {
      __PGBAR_ASSERT( this != &rhs );
      (void)rhs;
      return *this;
    }

    virtual ~Indicator() noexcept = default;

    virtual Indicator& tick() &                                      = 0;
    virtual Indicator& tick( __detail::types::Size next_step ) &     = 0;
    virtual Indicator& tick_to( __detail::types::Size percentage ) & = 0;
    virtual void reset()                                             = 0;
    virtual void reset( bool final_mesg )                            = 0;

    __PGBAR_NODISCARD bool is_running() const noexcept
    {
      return state_.load( std::memory_order_acquire ) != state::Stopped;
    }

    // Wait until the indicator is stopped.
    void wait() const
    {
      while ( is_running() )
        std::this_thread::yield();
    }
    // Wait for the indicator is stopped or timed out.
    template<class Rep, class Period>
    bool wait_for( const std::chrono::duration<Rep, Period>& time_duration ) const
    {
      for ( const auto ending = std::chrono::steady_clock::now() + time_duration;
            std::chrono::steady_clock::now() < ending; ) {
        if ( !is_running() )
          return true;
        std::this_thread::yield();
      }
      return false;
    }
  };

# if __PGBAR_CXX20
  template<typename ConfigType, __detail::traits::Mutex MutexMode, StreamChannel StreamType>
# else
  template<typename ConfigType, typename MutexMode, StreamChannel StreamType>
# endif
  class BasicBar final
    : public __detail::traits::LI_t<__detail::traits::ConfigTrait_t<ConfigType>>::
        template type<Indicator, BasicBar<ConfigType, MutexMode, StreamType>> {
    using Self = BasicBar;
    using Indicator::state;

    template<typename, typename>
    friend struct __detail::render::TickAction;
    template<typename, typename>
    friend struct __detail::render::RenderAction;

    __detail::render::Builder<ConfigType> config_;
    __detail::io::OStream<StreamType> ostream_;

    __PGBAR_NOUNIQUEADDR mutable MutexMode mtx_;

  public:
    BasicBar( ConfigType config = ConfigType() )
      noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : config_ { std::move( config ) }
    {}
    template<typename Arg,
             typename... Args,
             typename = typename std::enable_if<__detail::traits::AllBelongAny<
               __detail::traits::TypeList<typename std::decay<Arg>::type, typename std::decay<Args>::type...>,
               __detail::traits::GroupFonts,
               __detail::traits::GroupTaskQuantity,
               __detail::traits::ConfigTrait_c<ConfigType>,
               __detail::traits::GroupDescription,
               __detail::traits::GroupSegment,
               __detail::traits::GroupSpeedMeter,
               __detail::traits::GroupBitOption>::value>::type>
    BasicBar( Arg&& arg, Args&&... args )
      : BasicBar( ConfigType( std::forward<Arg>( arg ), std::forward<Args>( args )... ) )
    {}
    BasicBar( Self&& rhs ) noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : BasicBar( std::move( rhs.config_ ) )
    {}
    Self& operator=( Self&& rhs ) & noexcept
    {
      swap( rhs );
      return *this;
    }
    virtual ~BasicBar() noexcept { this->executor_.reset(); }

    Self& tick() & override final
    {
      std::lock_guard<MutexMode> lock { mtx_ };
      __detail::render::TickAction<ConfigType>::template do_tick<StreamType>(
        *this,
        [this]() noexcept -> void { this->task_cnt_.fetch_add( 1, std::memory_order_release ); } );
      return *this;
    }
    Self& tick( __detail::types::Size next_step ) & override final
    {
      std::lock_guard<MutexMode> lock { mtx_ };
      __detail::render::TickAction<ConfigType>::template do_tick<StreamType>(
        *this,
        [this, next_step]() noexcept -> void {
          const auto task_cnt = this->task_cnt_.load( std::memory_order_acquire );
          const auto task_end = this->task_end_.load( std::memory_order_acquire );
          this->task_cnt_.fetch_add( next_step + task_cnt > task_end ? task_end - task_cnt : next_step,
                                     std::memory_order_release );
        } );
      return *this;
    }
    /**
     * Set the iteration step of the progress bar to a specified percentage.
     * Ignore the call if the iteration count exceeds the given percentage.
     * If `percentage` is bigger than 100, it will be set to 100.
     *
     * @param percentage Value range: [0, 100].
     */
    Self& tick_to( __detail::types::Size percentage ) & override final
    {
      std::lock_guard<MutexMode> lock { mtx_ };
      __detail::render::TickAction<ConfigType>::template do_tick<StreamType>(
        *this,
        [this, percentage]() noexcept -> void {
          const auto task_end = this->task_end_.load( std::memory_order_acquire );
          if ( percentage < 100 ) {
            const auto target_progress = static_cast<__detail::types::Size>( task_end * percentage * 0.01 );

            __PGBAR_ASSERT( target_progress <= this->task_end_ );

            if ( target_progress > this->task_cnt_.load( std::memory_order_acquire ) )
              this->task_cnt_.store( target_progress, std::memory_order_release );
          } else
            this->task_cnt_.store( task_end, std::memory_order_release );
        } );
      return *this;
    }

    /**
     * Reset the state of the object,
     * it will immediately TERMINATE the current rendering.
     */
    void reset() override final
    {
      std::lock_guard<MutexMode> lock { mtx_ };
      this->unlock_reset( true );
    }
    void reset( bool final_mesg ) override final
    {
      std::lock_guard<MutexMode> lock { mtx_ };
      this->unlock_reset( final_mesg );
    }

    ConfigType& config() & noexcept { return config_; }
    const ConfigType& config() const& noexcept { return config_; }
    ConfigType config() && noexcept { return std::move( config_ ); }

    __PGBAR_CXX20_CNSTXPR void swap( BasicBar& lhs ) noexcept
    {
      __PGBAR_ASSERT( this != &lhs );
      config_.swap( lhs.config_ );
      ostream_.swap( lhs.ostream_ );
    }
    friend __PGBAR_CXX20_CNSTXPR void swap( BasicBar& a, BasicBar& b ) noexcept { a.swap( b ); }
  };

  /**
   * The simplest progress bar, which is what you think it is.
   *
   * It's structure is shown below:
   * {LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, StreamChannel StreamType = StreamChannel::Stderr>
  using ProgressBar = BasicBar<config::CharBar, MutexMode, StreamType>;
  /**
   * A progress bar with a smoother bar, requires an Unicode-supported terminal.
   *
   * It's structure is shown below:
   * {LeftBorder}{Description}{Percent}{Starting}{BlockBar}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, StreamChannel StreamType = StreamChannel::Stderr>
  using BlockProgressBar = BasicBar<config::BlckBar, MutexMode, StreamType>;
  /**
   * A progress bar without bar indicator, replaced by a fixed animation component.
   *
   * It's structure is shown below:
   * {LeftBorder}{Lead}{Description}{Percent}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, StreamChannel StreamType = StreamChannel::Stderr>
  using SpinnerBar = BasicBar<config::SpinBar, MutexMode, StreamType>;
  /**
   * The indeterminate progress bar.
   *
   * It's structure is shown below:
   * {LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
   */
  template<typename MutexMode = Threadunsafe, StreamChannel StreamType = StreamChannel::Stderr>
  using ScannerBar = BasicBar<config::ScanBar, MutexMode, StreamType>;

  namespace __detail {
    namespace render {
      template<typename ConfigType>
      struct RenderAction<
        ConfigType,
        typename std::enable_if<std::is_same<ConfigType, config::CharBar>::value
                                || std::is_same<ConfigType, config::SpinBar>::value
                                || std::is_same<ConfigType, config::ScanBar>::value>::type> {
        template<typename BarType>
        static void rendering( BarType& bar )
        {
          switch ( bar.state_.load( std::memory_order_acquire ) ) {
          case BarType::state::Begin: {
            __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
            bar.idx_frame_    = 0;
            bar.max_bar_size_ = bar.config_.full_render_size();
            bar.ostream_.reserve( static_cast<types::Size>( bar.max_bar_size_ * 1.2 ) )
              << console::escape::store_cursor;
            bar.config_.build( bar.ostream_,
                               bar.idx_frame_,
                               bar.task_cnt_.load( std::memory_order_acquire ),
                               bar.task_end_.load( std::memory_order_acquire ),
                               bar.zero_point_ );
            bar.ostream_ << io::flush;

            auto expected = BarType::state::Begin;
            if __PGBAR_CXX17_CNSTXPR ( std::is_same<ConfigType, config::CharBar>::value )
              bar.state_.compare_exchange_strong( expected,
                                                  BarType::state::Refresh2,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed );
            else
              bar.state_.compare_exchange_strong( expected,
                                                  bar.task_end_.load( std::memory_order_acquire ) == 0
                                                    ? BarType::state::Refresh1
                                                    : BarType::state::Refresh2,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed );
            /* If the main thread finds that the iteration is complete immediately,
             * it will set the `state_` to `finish`.
             * Therefore we cannot use `store` here. */
          }
            __PGBAR_FALLTHROUGH;

          case BarType::state::Refresh1: __PGBAR_FALLTHROUGH;
          case BarType::state::Refresh2: {
            __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
            bar.max_bar_size_ = std::max( bar.max_bar_size_, bar.config_.full_render_size() );
            bar.ostream_ << console::escape::restore_cursor
                         << console::escape::clear_next( bar.max_bar_size_ );

            bar.config_.build( bar.ostream_,
                               bar.idx_frame_,
                               bar.task_cnt_.load( std::memory_order_acquire ),
                               bar.task_end_.load( std::memory_order_acquire ),
                               bar.zero_point_ );
            bar.ostream_ << io::flush;
            ++bar.idx_frame_;
          } break;

          case BarType::state::Finish: { // intermediate state
            __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
            bar.max_bar_size_ = std::max( bar.max_bar_size_, bar.config_.full_render_size() );
            bar.ostream_ << console::escape::restore_cursor
                         << console::escape::clear_next( bar.max_bar_size_ );

            bar.config_.build( bar.ostream_,
                               bar.idx_frame_,
                               bar.task_cnt_.load( std::memory_order_acquire ),
                               bar.task_end_.load( std::memory_order_acquire ),
                               bar.final_mesg_,
                               bar.zero_point_ )
              << '\n';
            bar.ostream_ << io::flush << io::release;
            bar.state_.store( BarType::state::Stopped, std::memory_order_release );
          } break;

          default: return;
          }
        }
      };

      template<>
      struct RenderAction<config::BlckBar, void> {
        template<typename BarType>
        static void rendering( BarType& bar )
        {
          switch ( bar.state_.load( std::memory_order_acquire ) ) {
          case BarType::state::Begin: {
            bar.max_bar_size_ = bar.config_.full_render_size();
            bar.ostream_.reserve( static_cast<types::Size>( bar.max_bar_size_ * 1.2 ) )
              << console::escape::store_cursor;

            bar.config_.build( bar.ostream_,
                               bar.task_cnt_.load( std::memory_order_acquire ),
                               bar.task_end_.load( std::memory_order_acquire ),
                               bar.zero_point_ );
            bar.ostream_ << io::flush;

            auto expected = BarType::state::Begin;
            bar.state_.compare_exchange_strong( expected,
                                                BarType::state::Refresh2,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed );
          }
            __PGBAR_FALLTHROUGH;

          case BarType::state::Refresh2: {
            __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
            bar.max_bar_size_ = std::max( bar.max_bar_size_, bar.config_.full_render_size() );
            bar.ostream_ << console::escape::restore_cursor
                         << console::escape::clear_next( bar.max_bar_size_ );

            bar.config_.build( bar.ostream_,
                               bar.task_cnt_.load( std::memory_order_acquire ),
                               bar.task_end_.load( std::memory_order_acquire ),
                               bar.zero_point_ );
            bar.ostream_ << io::flush;
          } break;

          case BarType::state::Finish: {
            __PGBAR_ASSERT( bar.task_cnt_ <= bar.task_end_ );
            bar.max_bar_size_ = std::max( bar.max_bar_size_, bar.config_.full_render_size() );
            bar.ostream_ << console::escape::restore_cursor
                         << console::escape::clear_next( bar.max_bar_size_ );

            bar.config_.build( bar.ostream_,
                               bar.task_cnt_.load( std::memory_order_acquire ),
                               bar.task_end_.load( std::memory_order_acquire ),
                               bar.final_mesg_,
                               bar.zero_point_ )
              << '\n';
            bar.ostream_ << io::flush << io::release;
            bar.state_.store( BarType::state::Stopped, std::memory_order_release );
          } break;

          default: return;
          }
        }
      };

      template<typename ConfigType>
      struct TickAction<ConfigType,
                        typename std::enable_if<std::is_same<ConfigType, config::CharBar>::value
                                                || std::is_same<ConfigType, config::BlckBar>::value>::type> {
        template<StreamChannel StreamType, typename BarType, typename F>
        static __PGBAR_INLINE_FN void do_tick( BarType& bar, F&& action )
        {
          static_assert( traits::is_void_functor<F>::value,
                         "pgbar::__detail::render::TickAction::do_tick: Invalid template type error" );

          switch ( bar.state_.load( std::memory_order_acquire ) ) {
          case BarType::state::Stopped: {
            __PGBAR_ASSERT( bar.executor_.active() == false );
            bar.task_end_.store( bar.config_.tasks(), std::memory_order_release );
            __PGBAR_UNLIKELY if ( bar.task_end_.load( std::memory_order_acquire ) == 0 ) throw exception::
              InvalidState( "pgbar: the number of tasks is zero" );

            bar.task_cnt_.store( 0, std::memory_order_release );
            bar.zero_point_ = std::chrono::steady_clock::now();
            bar.state_.store( BarType::state::Begin, std::memory_order_release );

            /* If the standard output stream isn't bound to a tty,
             * we shouldn't activate the render thread.

             * However, in order to maintain semantic consistency,
             * exception checking and task counter updating are always carried out. */
            if ( config::Core::intty( StreamType ) ) {
              if ( bar.executor_.empty() )
                bar.executor_.reset( [&bar]() { RenderAction<ConfigType>::rendering( bar ); } );
              bar.executor_.activate();
            }
          }
            __PGBAR_FALLTHROUGH;
          case BarType::state::Begin:    __PGBAR_FALLTHROUGH;
          case BarType::state::Refresh2: {
            action();

            __PGBAR_UNLIKELY if ( bar.task_cnt_.load( std::memory_order_acquire ) >= bar.task_end_.load(
                                    std::memory_order_acquire ) ) bar.unlock_reset( true );
          } break;

          default: return;
          }
        }
      };

      template<typename ConfigType>
      struct TickAction<ConfigType,
                        typename std::enable_if<std::is_same<ConfigType, config::SpinBar>::value
                                                || std::is_same<ConfigType, config::ScanBar>::value>::type> {
        template<StreamChannel StreamType, typename BarType, typename F>
        static __PGBAR_INLINE_FN void do_tick( BarType& bar, F&& action )
        {
          static_assert( traits::is_void_functor<F>::value,
                         "pgbar::__detail::render::TickAction::do_tick: Invalid template type error" );

          switch ( bar.state_.load( std::memory_order_acquire ) ) {
          case BarType::state::Stopped: {
            __PGBAR_ASSERT( bar.executor_.active() == false );
            bar.task_end_.store( bar.config_.tasks(), std::memory_order_release );
            bar.task_cnt_.store( 0, std::memory_order_release );
            bar.zero_point_ = std::chrono::steady_clock::now();
            bar.state_.store( BarType::state::Begin, std::memory_order_release );

            if ( config::Core::intty( StreamType ) ) {
              if ( bar.executor_.empty() )
                bar.executor_.reset( [&bar]() { RenderAction<ConfigType>::rendering( bar ); } );
              bar.executor_.activate();
            }
          }
            __PGBAR_FALLTHROUGH;

          case BarType::state::Begin: {
            if ( bar.task_end_.load( std::memory_order_acquire ) == 0 )
              return;
          }
            __PGBAR_FALLTHROUGH;

          case BarType::state::Refresh2: {
            action();

            __PGBAR_UNLIKELY if ( bar.task_cnt_.load( std::memory_order_acquire ) >= bar.task_end_.load(
                                    std::memory_order_acquire ) ) bar.unlock_reset( true );
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
        typename std::enable_if<std::is_void<
          decltype( std::declval<B&>().config().tasks( std::declval<types::Size>() ), void() )>::value>::type>
        : std::true_type {};
    }
  } // namespace __detail

  namespace iterators {
    /**
     * A range that contains a bar object and an unidirectional abstract range,
     * which transforms the iterations in the abstract into a visual display of the object.
     */
    template<typename R, typename B>
    class ProxySpan {
      static_assert( __detail::traits::is_arith_range<R>::value || __detail::traits::is_iter_range<R>::value,
                     "pgbar::iterators::ProxySpan: Only available for certain range types" );
      static_assert( __detail::traits::is_iterable_bar<B>::value,
                     "pgbar::iterators::ProxySpan: Must have a method to configure the iteration "
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
        __PGBAR_CXX20_CNSTXPR ~iterator() noexcept( std::is_nothrow_destructible<R>::value ) = default;

        __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator& operator++()
        {
          __PGBAR_ASSERT( itr_bar_ != nullptr );
          ++itr_;
          itr_bar_->tick();
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX14_CNSTXPR iterator operator++( int )
        {
          __PGBAR_ASSERT( itr_bar_ != nullptr );
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
      };

      __PGBAR_CXX17_CNSTXPR ProxySpan( R itr_range, B& itr_bar )
        noexcept( std::is_nothrow_move_constructible<R>::value )
        : itr_bar_ { std::addressof( itr_bar ) }, itr_range_ { std::move( itr_range ) }
      {}
      __PGBAR_CXX17_CNSTXPR ProxySpan( ProxySpan&& rhs )
        noexcept( std::is_nothrow_move_constructible<R>::value )
        : ProxySpan( std::move( rhs.itr_range_ ), *rhs.itr_bar_ )
      {
        __PGBAR_ASSERT( rhs.itr_bar_ != nullptr );
      }
      __PGBAR_CXX17_CNSTXPR ProxySpan& operator=( ProxySpan&& rhs ) & noexcept(
        std::is_nothrow_move_assignable<R>::value )
      {
        __PGBAR_ASSERT( this != &rhs );
        swap( rhs );
        return *this;
      }
      __PGBAR_CXX20_CNSTXPR virtual ~ProxySpan() noexcept( std::is_nothrow_destructible<R>::value ) = default;

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

      __PGBAR_CXX20_CNSTXPR void swap( ProxySpan<R, B>& lhs ) noexcept
      {
        __PGBAR_ASSERT( this != &lhs );
        std::swap( itr_bar_, lhs.itr_bar_ );
        itr_range_.swap( lhs.itr_range_ );
      }
      friend __PGBAR_CXX20_CNSTXPR void swap( ProxySpan<R, B>& a, ProxySpan<R, B>& b ) noexcept
      {
        a.swap( b );
      }
    };
  } // namespace iterators

  namespace traits {
    template<typename T>
    using is_mutex = __detail::traits::is_mutex<T>;

# if __PGBAR_CXX14
    template<typename T>
    constexpr bool is_mutex_v = is_mutex<T>::value;
# endif
  } // namespace traits
} // namespace pgbar

# undef __PGBAR_TRAIT_REGISTER

# undef __PGBAR_DEFAULT_METHOD
# undef __PGBAR_MEMBER_METHOD

# undef __PGBAR_DEFAULT_TIMER
# undef __PGBAR_DEFAULT_SPEED
# undef __PGBAR_DEFAULT_PERCENT
# undef __PGBAR_PACK
# undef __PGBAR_INHERIT_REGISTER

# undef __PGBAR_CXX23
# undef __PGBAR_CXX23_CNSTXPR
# undef __PGBAR_INLINE_FN
# undef __PGBAR_NODISCARD
# undef __PGBAR_CC_STD
# undef __PGBAR_WIN
# undef __PGBAR_UNIX
# undef __PGBAR_UNKNOWN
# undef __PGBAR_CXX20
# undef __PGBAR_CNSTEVAL
# undef __PGBAR_CXX20_CNSTXPR
# undef __PGBAR_NOUNIQUEADDR
# undef __PGBAR_CXX17
# undef __PGBAR_CXX17_CNSTXPR
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
