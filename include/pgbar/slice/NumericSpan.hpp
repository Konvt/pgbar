#ifndef PGBAR_NUMERICSPAN
#define PGBAR_NUMERICSPAN

#include "../details/types/Types.hpp"
#include "../exception/Error.hpp"
#include <cmath>
#if PGBAR__CXX20
# include <ranges>
#endif

namespace pgbar {
  namespace slice {
    /**
     * An bidirectional range delimited by an numeric interval [start, end).
     *
     * The `end` can be less than the `start` only if the `step` is negative,
     * otherwise it throws exception `pgbar::exception::InvalidArgument`.
     */
    template<typename N>
    class NumericSpan
#if PGBAR__CXX20
      : public std::ranges::view_interface<NumericSpan<N>>
#endif
    {
      static_assert( std::is_arithmetic<N>::value,
                     "pgbar::slice::NumericSpan: Only available for arithmetic types" );

      N start_, end_, step_;

    public:
      class iterator {
        N itr_start_, itr_step_;
        _details::types::Size itr_cnt_;

      public:
#if PGBAR__CXX20
        using iterator_category = std::random_access_iterator_tag;
#else
        using iterator_category = std::bidirectional_iterator_tag;
#endif
        using value_type      = N;
        using difference_type = std::ptrdiff_t;
        using pointer         = value_type*;
        using reference       = value_type;

        constexpr iterator( N startpoint, N step, _details::types::Size iterated = 0 ) noexcept
          : itr_start_ { startpoint }, itr_step_ { step }, itr_cnt_ { iterated }
        {}
        constexpr iterator() noexcept : iterator( {}, 1, {} ) {}
        constexpr iterator( const iterator& )                         = default;
        PGBAR__CXX14_CNSTXPR iterator& operator=( const iterator& ) & = default;
        PGBAR__CXX20_CNSTXPR ~iterator()                              = default;

        PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator& operator++() & noexcept
        {
          ++itr_cnt_;
          return *this;
        }
        PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator++( int ) & noexcept
        {
          auto before = *this;
          operator++();
          return before;
        }
        PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator& operator--() & noexcept
        {
          --itr_cnt_;
          return *this;
        }
        PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator--( int ) & noexcept
        {
          auto before = *this;
          operator--();
          return before;
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr reference operator*() const noexcept
        {
          return static_cast<reference>( itr_start_
                                         + itr_cnt_ * static_cast<_details::types::Size>( itr_step_ ) );
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr reference operator[](
          _details::types::Size inc ) const noexcept
        {
          return *( *this + inc );
        }

        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator+( const iterator& itr,
                                                                         value_type increment ) noexcept
        {
          return { itr.itr_start_,
                   itr.itr_step_,
                   itr.itr_cnt_ + static_cast<_details::types::Size>( increment / itr.itr_step_ ) };
        }
        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator+( value_type increment,
                                                                         const iterator& itr ) noexcept
        {
          return itr + increment;
        }
        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator-( const iterator& itr,
                                                                         value_type increment ) noexcept
        {
          return { itr.itr_start_,
                   itr.itr_step_,
                   itr.itr_cnt_ - static_cast<_details::types::Size>( increment / itr.itr_step_ ) };
        }
        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator operator-( value_type increment,
                                                                         const iterator& itr ) noexcept
        {
          return itr - increment;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr difference_type operator-(
          const iterator& a,
          const iterator& b ) noexcept
        {
          return static_cast<difference_type>( *a - *b );
        }
        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator& operator+=( iterator& itr,
                                                                           value_type increment ) noexcept
        {
          itr.itr_cnt_ += static_cast<_details::types::Size>( increment / itr.itr_step_ );
          return itr;
        }
        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR iterator& operator-=( iterator& itr,
                                                                           value_type increment ) noexcept
        {
          itr.itr_cnt_ -= static_cast<_details::types::Size>( increment / itr.itr_step_ );
          return itr;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator==( const iterator& itr,
                                                                            value_type num ) noexcept
        {
          return *itr == num;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator!=( const iterator& itr,
                                                                            value_type num ) noexcept
        {
          return !( itr == num );
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator==( const iterator& a,
                                                                            const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ == b.itr_cnt_;
        }
#if PGBAR__CXX20
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr auto operator<=>( const iterator& a,
                                                                             const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ <=> b.itr_cnt_;
        }
#else
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator!=( const iterator& a,
                                                                            const iterator& b ) noexcept
        {
          return !( a == b );
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator<( const iterator& a,
                                                                           const iterator& b ) noexcept
        {
          return a.itr_start_ == b.itr_start_ && a.itr_step_ == b.itr_step_ && a.itr_cnt_ < b.itr_cnt_;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator>( const iterator& a,
                                                                           const iterator& b ) noexcept
        {
          return b < a;
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator<=( const iterator& a,
                                                                            const iterator& b ) noexcept
        {
          return !( b < a );
        }
        PGBAR__NODISCARD friend PGBAR__INLINE_FN constexpr bool operator>=( const iterator& a,
                                                                            const iterator& b ) noexcept
        {
          return !( a < b );
        }
#endif
      };

      constexpr NumericSpan() noexcept : start_ {}, end_ {}, step_ { 1 } {}
      /**
       * @throw exception::InvalidArgument
       *
       * If the `startpoint` is greater than `endpoint` while `step` is positive,
       * or the `startpoint` is less than `endpoint` while `step` is negative.
       */
      PGBAR__CXX20_CNSTXPR NumericSpan( N startpoint, N endpoint, N step ) noexcept( false ) : NumericSpan()
      {
        if ( step > 0 && startpoint > endpoint )
          PGBAR__UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else if ( step < 0 && startpoint < endpoint )
          PGBAR__UNLIKELY throw exception::InvalidArgument(
            "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        if ( step == 0 )
          PGBAR__UNLIKELY throw exception::InvalidArgument( "pgbar: 'step' is zero" );

        start_ = startpoint;
        step_  = step;
        end_   = endpoint;
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the `startpoint` is greater than `endpoint`.
       */
      PGBAR__CXX20_CNSTXPR NumericSpan( N startpoint, N endpoint ) : NumericSpan( startpoint, endpoint, 1 ) {}
      /**
       * @throw exception::InvalidArgument
       *
       * If the `endpoint` is less than zero.
       */
      PGBAR__CXX20_CNSTXPR explicit NumericSpan( N endpoint ) : NumericSpan( {}, endpoint, 1 ) {}
      constexpr NumericSpan( const NumericSpan& )                         = default;
      PGBAR__CXX14_CNSTXPR NumericSpan& operator=( const NumericSpan& ) & = default;
      PGBAR__CXX20_CNSTXPR ~NumericSpan()                                 = default;

      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr iterator begin() const noexcept
      {
        return iterator( start_, step_ );
      }
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX23_CNSTXPR iterator end() const noexcept
      {
        return iterator( start_, step_, size() );
      }

      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr N front() const noexcept { return start_; }
      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr N back() const noexcept { return end_; }
      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr N step() const noexcept { return step_; }
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX23_CNSTXPR _details::types::Size size() const noexcept
      {
        PGBAR__TRUST( step_ != 0 );
        if PGBAR__CXX17_CNSTXPR ( std::is_unsigned<N>::value )
          return ( ( end_ - start_ + step_ ) - 1 ) / step_;
        else if PGBAR__CXX17_CNSTXPR ( std::is_integral<N>::value ) {
          if ( step_ > 0 )
            return ( ( end_ - start_ + step_ - 1 ) / step_ );
          else
            return ( ( start_ - end_ - step_ ) - 1 ) / ( -step_ );
        } else
          return static_cast<_details::types::Size>( std::ceil( ( end_ - start_ ) / step_ ) );
      }

      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr bool empty() const noexcept { return size() == 0; }
      PGBAR__NODISCARD PGBAR__INLINE_FN constexpr typename iterator::reference operator[](
        _details::types::Size inc ) const noexcept
      {
        return *( begin() + inc );
      }
      PGBAR__CXX14_CNSTXPR void swap( NumericSpan<N>& lhs ) noexcept
      {
        PGBAR__TRUST( this != &lhs );
        using std::swap;
        swap( start_, lhs.start_ );
        swap( end_, lhs.end_ );
        swap( step_, lhs.step_ );
      }
      friend PGBAR__CXX14_CNSTXPR void swap( NumericSpan<N>& a, NumericSpan<N>& b ) noexcept { a.swap( b ); }

      constexpr explicit operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice
} // namespace pgbar

#endif
