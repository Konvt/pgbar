#ifndef PGBAR_ITERSPAN
#define PGBAR_ITERSPAN

#include "../details/traits/Backport.hpp"
#include "../exception/Error.hpp"
#include <iterator>

namespace pgbar {
  namespace slice {
    /**
     * An undirectional range delimited by a pair of iterators, including pointer types.
     *
     * Accepted iterator types must satisfy subtractable.
     */
    template<typename I>
    class IterSpan
#if __PGBAR_CXX20
      : public std::ranges::view_interface<IterSpan<I>>
#endif
    {
      static_assert( __details::traits::is_sized_iterator<I>::value,
                     "pgbar::slice::IterSpan: Only available for sized iterator types" );
      static_assert(
        std::is_convertible<typename std::iterator_traits<I>::difference_type, __details::types::Size>::value,
        "pgbar::slice::IterSpan: The 'difference_type' must be convertible to Size" );

      I start_, end_;
      __details::types::Size size_;

#if __PGBAR_CXX20
      using Reference_t = std::iter_reference_t<I>;
#else
      using Reference_t = typename std::iterator_traits<I>::reference;
#endif

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
#if __PGBAR_CXX20
        using value_type      = std::iter_value_t<I>;
        using difference_type = std::iter_difference_t<I>;
        using reference       = std::iter_reference_t<I>;
#else
        using value_type      = typename std::iterator_traits<I>::value_type;
        using difference_type = typename std::iterator_traits<I>::difference_type;
        using reference       = typename std::iterator_traits<I>::reference;
#endif
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
#if __PGBAR_CXX20
        const auto length = std::ranges::distance( start_, end_ );
#else
        const auto length = std::distance( start_, end_ );
#endif
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
  } // namespace slice
} // namespace pgbar

#endif
