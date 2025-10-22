#ifndef PGBAR_BOUNDEDSPAN
#define PGBAR_BOUNDEDSPAN

#include "../details/traits/Backport.hpp"

namespace pgbar {
  namespace slice {
    template<typename R>
    class BoundedSpan
#if PGBAR__CXX20
      : public std::ranges::view_interface<BoundedSpan<R>>
#endif
    {
#if PGBAR__CXX20
      static_assert( _details::traits::is_bounded_range<R>::value && !std::ranges::view<R>,
                     "pgbar::slice::BoundedSpan: Only available for bounded ranges, excluding view types" );
#else
      static_assert( _details::traits::is_bounded_range<R>::value,
                     "pgbar::slice::BoundedSpan: Only available for bounded ranges" );
#endif

      R* rnge_;

    public:
      using iterator = _details::traits::IteratorOf_t<R>;

    private:
#if PGBAR__CXX20
      using Reference_t = std::iter_reference_t<iterator>;

      template<typename Rn>
      static PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR _details::types::Size size( Rn& rn )
      {
        return std::ranges::size( rn );
      }
#else
      using Reference_t = typename std::iterator_traits<iterator>::reference;

# if PGBAR__CXX17
      template<typename Rn>
      static PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR _details::types::Size size( Rn& rn )
      {
        return std::size( rn );
      }
# else
      template<typename Rn>
      static PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR _details::types::Size size( Rn& rn )
      {
        return rn.size();
      }
      template<typename Rn, _details::types::Size N>
      static PGBAR__INLINE_FN constexpr _details::types::Size size( Rn ( & )[N] ) noexcept
      {
        return N;
      }
# endif
#endif
    public:
      PGBAR__CXX17_CNSTXPR BoundedSpan( R& rnge ) noexcept : rnge_ { std::addressof( rnge ) } {}

      PGBAR__CXX17_CNSTXPR BoundedSpan( const BoundedSpan& )              = default;
      PGBAR__CXX17_CNSTXPR BoundedSpan& operator=( const BoundedSpan& ) & = default;
      PGBAR__CXX20_CNSTXPR ~BoundedSpan()                                 = default;

      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR iterator begin() const
      {
#if PGBAR__CXX20
        return std::ranges::begin( *rnge_ );
#else
        return std::begin( *rnge_ );
#endif
      }
#if PGBAR__CXX17
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR auto end() const
#else
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR iterator end() const
#endif
      {
#if PGBAR__CXX20
        return std::ranges::end( *rnge_ );
#else
        return std::end( *rnge_ );
#endif
      }

      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR Reference_t front() const { return *begin(); }
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR Reference_t back() const
      {
        return *std::next( begin(), size() - 1 );
      }
      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr _details::types::Size step() const noexcept
      {
        return 1;
      }
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR _details::types::Size size() const
      {
        return size( *rnge_ );
      }

      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR bool empty() const noexcept
      {
        return rnge_ == nullptr;
      }

      PGBAR__CXX17_CNSTXPR void swap( BoundedSpan<R>& lhs ) noexcept
      {
        PGBAR__TRUST( this != &lhs );
        std::swap( rnge_, lhs.rnge_ );
      }
      friend PGBAR__CXX17_CNSTXPR void swap( BoundedSpan<R>& a, BoundedSpan<R>& b ) noexcept { a.swap( b ); }

      PGBAR__CXX17_CNSTXPR explicit operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice
} // namespace pgbar

#endif
