#ifndef PGBAR_TRACKEDSPAN
#define PGBAR_TRACKEDSPAN

#include "../details/prefabs/BasicBar.hpp"
#include "../details/traits/ConceptTraits.hpp"
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

      using Itr = _details::traits::IteratorOf_t<R>;
      using Snt = _details::traits::SentinelOf_t<R>;
      class Sentry {
        friend class iterator;

        Snt snt_;

      public:
        constexpr Sentry() = default;
        constexpr Sentry( Snt&& endpoint ) noexcept( std::is_nothrow_move_constructible<Snt>::value )
          : snt_ { std::move( endpoint ) }
        {}
      };

    public:
      class iterator;
#if PGBAR__CXX17
      using sentinel = std::conditional_t<std::is_same_v<Itr, Snt>, iterator, Sentry>;
#else
      using sentinel = iterator;
#endif
      class iterator {
        Itr itr_;
        B* itr_bar_;

      public:
        using iterator_category = typename std::conditional<
          _details::traits::AnyOf<
            std::is_same<_details::traits::IterCategory_t<Itr>, std::input_iterator_tag>,
            std::is_same<_details::traits::IterCategory_t<Itr>, std::output_iterator_tag>>::value,
          _details::traits::IterCategory_t<Itr>,
          std::forward_iterator_tag>::type;
        using value_type      = _details::traits::IterValue_t<Itr>;
        using difference_type = _details::traits::IterDifference_t<Itr>;
        using reference       = _details::traits::IterReference_t<Itr>;
        using pointer         = Itr;

        constexpr iterator() noexcept( std::is_nothrow_default_constructible<Itr>::value )
          : itr_ {}, itr_bar_ { nullptr }
        {}
        PGBAR__CXX17_CNSTXPR iterator( Itr itr ) noexcept( std::is_nothrow_move_constructible<Itr>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { nullptr }
        {}
        PGBAR__CXX17_CNSTXPR iterator( Itr itr, B& itr_bar )
          noexcept( std::is_nothrow_move_constructible<Itr>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { std::addressof( itr_bar ) }
        {}
        PGBAR__CXX17_CNSTXPR iterator( iterator&& rhs )
          noexcept( std::is_nothrow_move_constructible<Itr>::value )
          : itr_ { std::move( rhs.itr_ ) }, itr_bar_ { rhs.itr_bar_ }
        {
          rhs.itr_bar_ = nullptr;
        }
        PGBAR__CXX17_CNSTXPR iterator& operator=( iterator&& rhs ) & noexcept(
          std::is_nothrow_move_assignable<Itr>::value )
        {
          PGBAR__TRUST( this != &rhs );
          itr_         = std::move( rhs.itr_ );
          itr_bar_     = rhs.itr_bar_;
          rhs.itr_bar_ = nullptr;
          return *this;
        }
        PGBAR__CXX20_CNSTXPR ~iterator() = default;

        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR iterator& operator++() &
        {
          PGBAR__TRUST( itr_bar_ != nullptr );
          itr_ = std::next( itr_, 1 );
          itr_bar_->tick();
          return *this;
        }
        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR iterator operator++( int ) &
        {
          PGBAR__TRUST( itr_bar_ != nullptr );
          auto before = *this;
          operator++();
          return before;
        }

        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR reference operator*() const { return *itr_; }
        PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR pointer operator->() const { return itr_; }

        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const iterator& a,
                                                                              const Itr& b )
        {
          return a.itr_ == b;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const Itr& a,
                                                                              const iterator& b )
        {
          return b == a;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const iterator& a,
                                                                              const Itr& b )
        {
          return a.itr_ != b;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const Itr& a,
                                                                              const iterator& b )
        {
          return b != a;
        }
        template<typename S>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<S, Snt>, _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator==( const iterator& a, const S& b )
        {
          return a.itr_ == b;
        }
        template<typename S>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<S, Snt>, _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator==( const S& ir, const iterator& itr )
        {
          return itr == ir;
        }
        template<typename S>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<S, Snt>, _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator!=( const iterator& itr, const S& ir )
        {
          return !( itr == ir );
        }
        template<typename S>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<S, Snt>, _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator!=( const S& snt, const iterator& itr )
        {
          return !( itr == snt );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const iterator& a,
                                                                              const iterator& b )
        {
          return a.itr_ == b.itr_;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b )
        {
          return !( a == b );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const iterator& a,
                                                                              const Sentry& b )
        {
          return a.itr_ == b.snt_;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const iterator& a,
                                                                              const Sentry& b )
        {
          return !( a == b );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const Sentry& a,
                                                                              const iterator& b )
        {
          return b == a;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const Sentry& a,
                                                                              const iterator& b )
        {
          return !( b == a );
        }

        explicit constexpr operator bool() const noexcept { return itr_bar_ != nullptr; }
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
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR iterator begin() &
      {
        itr_bar_->config().tasks( _details::utils::size( itr_range_ ) );
        return { _details::utils::begin( itr_range_ ), *itr_bar_ };
      }
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR sentinel end() const
      {
        return { _details::utils::end( itr_range_ ) };
      }

      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR bool empty() const noexcept
      {
        return itr_bar_ == nullptr;
      }

      PGBAR__CXX14_CNSTXPR void swap( TrackedSpan<R, B>& lhs ) noexcept
      {
        PGBAR__TRUST( this != &lhs );
        using std::swap;
        std::swap( itr_bar_, lhs.itr_bar_ );
        swap( itr_range_, lhs.itr_range_ );
      }
      friend PGBAR__CXX14_CNSTXPR void swap( TrackedSpan<R, B>& a, TrackedSpan<R, B>& b ) noexcept
      {
        a.swap( b );
      }

      explicit constexpr operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice
} // namespace pgbar

#endif
