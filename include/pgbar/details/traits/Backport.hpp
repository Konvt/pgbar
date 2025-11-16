#ifndef PGBAR__TRAITS_BACKPORT
#define PGBAR__TRAITS_BACKPORT

#include "../core/Core.hpp"
#include "../types/Types.hpp"
#include <type_traits>

namespace pgbar {
  namespace _details {
    namespace traits {
      // Before C++17, not all std entities had feature macros.
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
      struct _concat_seq;
      template<typename HeadSeq, typename TailSeq>
      using _concat_seq_t = typename _concat_seq<HeadSeq, TailSeq>::type;

      template<types::Size... HeadI, types::Size... TailI>
      struct _concat_seq<IndexSeq<HeadI...>, IndexSeq<TailI...>> {
        using type = IndexSeq<HeadI..., ( sizeof...( HeadI ) + TailI )...>;
      };

      // Internal implementation, it should not be used outside of this preprocessing block.
      template<types::Size N>
      struct _make_index_seq_helper {
        using type = _concat_seq_t<typename _make_index_seq_helper<N / 2>::type,
                                   typename _make_index_seq_helper<N - N / 2>::type>;
      };
      template<>
      struct _make_index_seq_helper<0> {
        using type = IndexSeq<>;
      };
      template<>
      struct _make_index_seq_helper<1> {
        using type = IndexSeq<0>;
      };

      template<types::Size N>
      using MakeIndexSeq = typename _make_index_seq_helper<N>::type;
#endif

#if PGBAR__CXX14
      template<typename T>
      using is_final = std::is_final<T>;
#elif defined( __GNUC__ ) || defined( __clang__ ) || defined( _MSC_VER )
      template<typename T>
      using is_final = std::integral_constant<bool, __is_final( T )>;
#else
      template<typename T>
      using is_final = std::integral_constant<bool, true>;
#endif

#ifdef __cpp_lib_bool_constant
      template<bool B>
      using BoolConstant = std::bool_constant<B>;
#else
      template<bool B>
      using BoolConstant = std::integral_constant<bool, B>;
#endif

#if PGBAR__CXX17
      template<typename... Preds>
      using AllOf = std::conjunction<Preds...>;

      template<typename... Preds>
      using AnyOf = std::disjunction<Preds...>;

      template<typename Pred>
      using Not = std::negation<Pred>;
#else
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
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
