#ifndef PGBAR__CONCEPTTRAITS
#define PGBAR__CONCEPTTRAITS
// Only contains trait entities unrelated to template metaprogramming here.

#include "Backport.hpp"
#include <iterator>
#ifdef __cpp_lib_concepts
# include <concepts>
#endif
#ifdef __cpp_lib_ranges
# include <ranges>
#endif

namespace pgbar {
  namespace _details {
    namespace traits {
      template<typename I>
      using IterCategory_t = typename std::iterator_traits<I>::iterator_category;
      template<typename I>
      using IterPointer_t = typename std::iterator_traits<I>::pointer;
#ifdef __cpp_lib_ranges
      template<typename I>
      using IterValue_t = std::iter_value_t<I>;
      template<typename I>
      using IterDifference_t = std::iter_difference_t<I>;
      template<typename I>
      using IterReference_t = std::iter_reference_t<I>;
#else
      template<typename I>
      using IterValue_t = typename std::iterator_traits<I>::value_type;
      template<typename I>
      using IterDifference_t = typename std::iterator_traits<I>::difference_type;
      template<typename I>
      using IterReference_t = typename std::iterator_traits<I>::reference;
#endif

      template<typename T>
      struct IteratorOf {
      private:
        // Provide a default fallback to avoid the problem of the type not existing
        // in the immediate context derivation.
        template<typename U>
        static constexpr U check( ... );
#ifdef __cpp_lib_ranges
        template<typename U>
        static constexpr std::ranges::iterator_t<U> check( int );
#else
        template<typename U>
        static constexpr decltype( std::begin( std::declval<U>() ) ) check( int );
#endif
      public:
        using type = decltype( check<T>( 0 ) );
      };
      // Get the result type of `IteratorOf`.
      template<typename T>
      using IteratorOf_t = typename IteratorOf<T>::type;

      template<typename T>
      struct SentinelOf {
      private:
        template<typename U>
        static constexpr U check( ... );
#ifdef __cpp_lib_ranges
        template<typename U>
        static constexpr std::ranges::sentinel_t<T> check( int );
#else
        template<typename U>
        static constexpr decltype( std::end( std::declval<T&>() ) ) check( int );
#endif

      public:
        using type = decltype( check<T>( 0 ) );
      };
      template<typename T>
      using SentinelOf_t = typename SentinelOf<T>::type;

#ifdef __cpp_lib_concepts
      template<typename Itr, typename Snt = Itr>
      struct is_sized_cursor
        : BoolConstant<std::movable<Itr> && std::weakly_incrementable<Itr> && std::indirectly_readable<Itr>
                       && std::sized_sentinel_for<Snt, Itr>> {};
#else
      template<typename Itr, typename Snt = Itr>
      struct is_sized_cursor {
      private:
        template<typename, typename>
        static constexpr std::false_type check( ... );
        template<typename I, typename S>
        static constexpr typename std::enable_if<
          AllOf<std::is_signed<IterDifference_t<I>>,
                std::is_same<decltype( ++std::declval<I>() ), I&>,
                std::is_void<decltype( std::declval<I>()++, void() )>,
                std::is_same<decltype( *std::declval<I>() ), IterReference_t<I>>,
                std::is_same<decltype( std::distance( std::declval<I>(), std::declval<S>() ) ),
                             IterDifference_t<I>>,
                std::is_convertible<decltype( std::declval<I>() != std::declval<S>() ), bool>>::value,
          std::true_type>::type
          check( int );

      public:
        static constexpr bool value = AllOf<Not<std::is_reference<Itr>>,
                                            Not<std::is_reference<Snt>>,
                                            std::is_move_constructible<Itr>,
                                            decltype( check<Itr, Snt>( 0 ) )>::value;
      };
      template<typename P>
      struct is_sized_cursor<P*, P*> : std::true_type {};
#endif

#ifdef __cpp_lib_ranges
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
          AllOf<std::is_convertible<decltype( std::declval<U>().begin() != std::declval<U>().end() ), bool>,
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
