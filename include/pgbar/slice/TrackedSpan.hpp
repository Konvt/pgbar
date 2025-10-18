#ifndef PGBAR_TRACKEDSPAN
#define PGBAR_TRACKEDSPAN

#include "../details/prefabs/BasicBar.hpp"
#include "../details/traits/Backport.hpp"
#if PGBAR__CXX20
# include <ranges>
#endif

namespace pgbar {
  namespace slice {
    /**
     * A range that contains a bar object and an unidirectional abstract range,
     * which transforms the iterations in the abstract into a visual display of the object.
     */
    template<typename R, typename B>
    class TrackedSpan
#if PGBAR__CXX20
      : public std::ranges::view_interface<TrackedSpan<R, B>>
#endif
    {
      static_assert( _details::traits::is_bounded_range<R>::value,
                     "pgbar::slice::TrackedSpan: Only available for bounded ranges" );
      static_assert( _details::traits::is_iterable_bar<B>::value,
                     "pgbar::slice::TrackedSpan: Must have a method to configure the iteration "
                     "count for the object's configuration type" );

      B* itr_bar_;
      R itr_range_;

#if PGBAR__CXX20
      using Sntnl = std::ranges::sentinel_t<R>;
#elif PGBAR__CXX17
      using Sntnl = _details::traits::IteratorOf_t<R>;
#endif

    public:
      class iterator;
#if PGBAR__CXX17
      class sentinel {
        Sntnl end_;
        friend class iterator;

      public:
        constexpr sentinel() = default;
        constexpr sentinel( Sntnl&& end ) noexcept( std::is_nothrow_move_constructible<Sntnl>::value )
          : end_ { std::move( end ) }
        {}
      };
#else
      using sentinel = iterator;
#endif

      class iterator {
        using Iter = _details::traits::IteratorOf_t<R>;
        Iter itr_;
        B* itr_bar_;

      public:
        using iterator_category = typename std::conditional<
          _details::traits::AnyOf<
            std::is_same<typename std::iterator_traits<Iter>::iterator_category, std::input_iterator_tag>,
            std::is_same<typename std::iterator_traits<Iter>::iterator_category,
                         std::output_iterator_tag>>::value,
          typename std::iterator_traits<Iter>::iterator_category,
          std::forward_iterator_tag>::type;
#if PGBAR__CXX20
        using value_type      = std::iter_value_t<Iter>;
        using difference_type = std::iter_difference_t<Iter>;
        using reference       = std::iter_reference_t<Iter>;
#else
        using value_type      = typename std::iterator_traits<Iter>::value_type;
        using difference_type = typename std::iterator_traits<Iter>::difference_type;
        using reference       = typename std::iterator_traits<Iter>::reference;
#endif
        using pointer = Iter;

        constexpr iterator() noexcept( std::is_nothrow_default_constructible<Iter>::value )
          : itr_ {}, itr_bar_ { nullptr }
        {}
        PGBAR__CXX17_CNSTXPR iterator( Iter itr ) noexcept( std::is_nothrow_move_constructible<Iter>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { nullptr }
        {}
        PGBAR__CXX17_CNSTXPR iterator( Iter itr, B& itr_bar )
          noexcept( std::is_nothrow_move_constructible<Iter>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { std::addressof( itr_bar ) }
        {}
        PGBAR__CXX17_CNSTXPR iterator( iterator&& rhs )
          noexcept( std::is_nothrow_move_constructible<Iter>::value )
          : itr_ { std::move( rhs.itr_ ) }, itr_bar_ { rhs.itr_bar_ }
        {
          rhs.itr_bar_ = nullptr;
        }
        PGBAR__CXX17_CNSTXPR iterator& operator=( iterator&& rhs ) & noexcept(
          std::is_nothrow_move_assignable<Iter>::value )
        {
          PGBAR__TRUST( this != &rhs );
          itr_         = std::move( rhs.itr_ );
          itr_bar_     = rhs.itr_bar_;
          rhs.itr_bar_ = nullptr;
          return *this;
        }
        PGBAR__CXX20_CNSTXPR ~iterator() = default;

        PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator& operator++() &
        {
          PGBAR__TRUST( itr_bar_ != nullptr );
          ++itr_;
          itr_bar_->tick();
          return *this;
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator++( int ) &
        {
          PGBAR__TRUST( itr_bar_ != nullptr );
          auto before = *this;
          operator++();
          return before;
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR reference operator*() const { return *itr_; }
        PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR pointer operator->() const { return itr_; }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr bool operator==( const Iter& lhs ) const
        {
          return itr_ == lhs;
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr bool operator!=( const Iter& lhs ) const
        {
          return itr_ != lhs;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator==( const iterator& a,
                                                                            const iterator& b )
        {
          return a.itr_ == b.itr_;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                            const iterator& b )
        {
          return !( a == b );
        }

#if PGBAR__CXX17
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr bool operator==( const sentinel& b ) const
        {
          return itr_ == b.end_;
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr bool operator!=( const sentinel& b ) const
        {
          return !( *this == b );
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator==( const sentinel& a,
                                                                            const iterator& b )
        {
          return b == a;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator!=( const sentinel& a,
                                                                            const iterator& b )
        {
          return !( b == a );
        }
#endif

        constexpr explicit operator bool() const noexcept { return itr_bar_ != nullptr; }
      };

      PGBAR__CXX17_CNSTXPR TrackedSpan( R itr_range, B& itr_bar )
        noexcept( std::is_nothrow_move_constructible<R>::value )
        : itr_bar_ { std::addressof( itr_bar ) }, itr_range_ { std::move( itr_range ) }
      {}
      PGBAR__CXX17_CNSTXPR TrackedSpan( TrackedSpan&& rhs )
        noexcept( std::is_nothrow_move_constructible<R>::value )
        : TrackedSpan( std::move( rhs.itr_range_ ), *rhs.itr_bar_ )
      {
        PGBAR__ASSERT( rhs.empty() == false );
        rhs.itr_bar_ = nullptr;
      }
      PGBAR__CXX17_CNSTXPR TrackedSpan& operator=( TrackedSpan&& rhs ) & noexcept(
        std::is_nothrow_move_assignable<R>::value )
      {
        PGBAR__TRUST( this != &rhs );
        swap( rhs );
        rhs.itr_bar_ = nullptr;
        return *this;
      }
      // Intentional non-virtual destructors.
      PGBAR__CXX20_CNSTXPR ~TrackedSpan() = default;

      // This function will CHANGE the state of the pgbar object it holds.
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR iterator begin() &
      {
        itr_bar_->config().tasks( itr_range_.size() );
        return { itr_range_.begin(), *itr_bar_ };
      }
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR sentinel end() const
      {
        return { itr_range_.end() };
      }
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR bool empty() const noexcept
      {
        return itr_bar_ == nullptr;
      }

      PGBAR__CXX14_CNSTXPR void swap( TrackedSpan<R, B>& lhs ) noexcept
      {
        PGBAR__TRUST( this != &lhs );
        std::swap( itr_bar_, lhs.itr_bar_ );
        itr_range_.swap( lhs.itr_range_ );
      }
      friend PGBAR__CXX14_CNSTXPR void swap( TrackedSpan<R, B>& a, TrackedSpan<R, B>& b ) noexcept
      {
        a.swap( b );
      }
      constexpr explicit operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice
} // namespace pgbar

#endif
