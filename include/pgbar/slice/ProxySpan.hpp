#ifndef PGBAR_PROXYSPAN
#define PGBAR_PROXYSPAN

#include "../details/prefabs/BasicBar.hpp"
#include "../details/traits/Backport.hpp"
#if __PGBAR_CXX20
# include <ranges>
#endif

namespace pgbar {
  namespace slice {
    /**
     * A range that contains a bar object and an unidirectional abstract range,
     * which transforms the iterations in the abstract into a visual display of the object.
     */
    template<typename R, typename B>
    class ProxySpan
#if __PGBAR_CXX20
      : public std::ranges::view_interface<ProxySpan<R, B>>
#endif
    {
      static_assert( __details::traits::is_bounded_range<R>::value,
                     "pgbar::slice::ProxySpan: Only available for bounded ranges" );
      static_assert( __details::traits::is_iterable_bar<B>::value,
                     "pgbar::slice::ProxySpan: Must have a method to configure the iteration "
                     "count for the object's configuration type" );

      B* itr_bar_;
      R itr_range_;

#if __PGBAR_CXX20
      using Sntnl = std::ranges::sentinel_t<R>;
#elif __PGBAR_CXX17
      using Sntnl = __details::traits::IteratorOf_t<R>;
#endif

    public:
      class iterator;
#if __PGBAR_CXX17
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
#if __PGBAR_CXX20
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

#if __PGBAR_CXX17
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
#endif

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
} // namespace pgbar

#endif
