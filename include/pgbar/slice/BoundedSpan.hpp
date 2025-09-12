#ifndef PGBAR_BOUNDEDSPAN
#define PGBAR_BOUNDEDSPAN

#include "../details/traits/Backport.hpp"

namespace pgbar {
  namespace slice {
    template<typename R>
    class BoundedSpan
#if __PGBAR_CXX20
      : public std::ranges::view_interface<BoundedSpan<R>>
#endif
    {
#if __PGBAR_CXX20
      static_assert( __details::traits::is_bounded_range<R>::value && !std::ranges::view<R>,
                     "pgbar::slice::BoundedSpan: Only available for bounded ranges, excluding view types" );
#else
      static_assert( __details::traits::is_bounded_range<R>::value,
                     "pgbar::slice::BoundedSpan: Only available for bounded ranges" );
#endif

      R* rnge_;

    public:
      using iterator = __details::traits::IteratorOf_t<R>;

    private:
#if __PGBAR_CXX20
      using Reference_t = std::iter_reference_t<iterator>;

      template<typename Rn>
      static __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR __details::types::Size size( Rn& rn )
      {
        return std::ranges::size( rn );
      }
#else
      using Reference_t = typename std::iterator_traits<iterator>::reference;

# if __PGBAR_CXX17
      template<typename Rn>
      static __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR __details::types::Size size( Rn& rn )
      {
        return std::size( rn );
      }
# else
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
# endif
#endif
    public:
      __PGBAR_CXX17_CNSTXPR BoundedSpan( R& rnge ) noexcept : rnge_ { std::addressof( rnge ) } {}

      __PGBAR_CXX17_CNSTXPR BoundedSpan( const BoundedSpan& )              = default;
      __PGBAR_CXX17_CNSTXPR BoundedSpan& operator=( const BoundedSpan& ) & = default;
      __PGBAR_CXX20_CNSTXPR ~BoundedSpan()                                 = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator begin() const
      {
#if __PGBAR_CXX20
        return std::ranges::begin( *rnge_ );
#else
        return std::begin( *rnge_ );
#endif
      }
#if __PGBAR_CXX17
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR auto end() const
#else
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR iterator end() const
#endif
      {
#if __PGBAR_CXX20
        return std::ranges::end( *rnge_ );
#else
        return std::end( *rnge_ );
#endif
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

      __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CXX17_CNSTXPR bool empty() const noexcept
      {
        return rnge_ == nullptr;
      }

      __PGBAR_CXX17_CNSTXPR void swap( BoundedSpan<R>& lhs ) noexcept
      {
        __PGBAR_TRUST( this != &lhs );
        std::swap( rnge_, lhs.rnge_ );
      }
      friend __PGBAR_CXX17_CNSTXPR void swap( BoundedSpan<R>& a, BoundedSpan<R>& b ) noexcept { a.swap( b ); }

      __PGBAR_CXX17_CNSTXPR explicit operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice
} // namespace pgbar

#endif
