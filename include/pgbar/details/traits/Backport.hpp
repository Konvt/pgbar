#ifndef PGBAR__TRAITS_BACKPORT
#define PGBAR__TRAITS_BACKPORT

#include "../types/Types.hpp"
#include <type_traits>
#if PGBAR__CXX20
# include <concepts>
# include <ranges>
#endif

namespace pgbar {
  namespace _details {
    namespace traits {
#if PGBAR__CXX14
      template<types::Size... Ns>
      using IndexSeq = std::integer_sequence<types::Size, Ns...>;

      template<types::Size N>
      using MakeIndexSeq = std::make_integer_sequence<types::Size, N>;
#else
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
#endif

#if PGBAR__CXX17
      template<bool B>
      using BoolConstant = std::bool_constant<B>;

      template<typename... Preds>
      using AllOf = std::conjunction<Preds...>;

      template<typename... Preds>
      using AnyOf = std::disjunction<Preds...>;

      template<typename Pred>
      using Not = std::negation<Pred>;
#else
      template<bool B>
      using BoolConstant = std::integral_constant<bool, B>;

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
      struct Not : BoolConstant<!bool( Pred::value )> {};
#endif

      /**
       * Check whether the type `T` has a type `iterator` or `const_iterator`, and return it if affirmative.
       * Otherwise, return the type `T` itself.
       */
      template<typename T>
      struct IteratorOf {
#if PGBAR__CXX20
      private:
        // Provide a default fallback to avoid the problem of the type not existing
        // in the immediate context derivation.
        template<typename U>
        static constexpr U check( ... );
        template<typename U>
        static constexpr std::ranges::iterator_t<U> check( int );

      public:
        using type = decltype( check<T>( 0 ) );
#else
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
#endif
      };
      // Get the result type of `IteratorOf`.
      template<typename T>
      using IteratorOf_t = typename IteratorOf<T>::type;

#if PGBAR__CXX20
      template<typename T>
      struct is_sized_iterator
        : BoolConstant<std::movable<T> && std::weakly_incrementable<T> && std::indirectly_readable<T>
                       && std::sized_sentinel_for<T, T>> {};
#else
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
#endif

#if PGBAR__CXX20
      template<typename T>
      struct is_bounded_range : BoolConstant<std::ranges::sized_range<T>> {};
#else
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
#endif
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
