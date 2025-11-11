#ifndef PGBAR_ITERATORSPAN
#define PGBAR_ITERATORSPAN

#include "../details/traits/ConceptTraits.hpp"
#include "../details/utils/Backport.hpp"
#include "../exception/Error.hpp"

namespace pgbar {
  namespace slice {
    /**
     * An undirectional range delimited by a pair of iterators, including pointer types.
     *
     * Accepted iterator types must satisfy subtractable.
     */
    template<typename Itr, typename Snt = Itr>
    class IteratorSpan
#ifdef __cpp_concepts
      : public std::ranges::view_interface<IteratorSpan<Itr, Snt>>
#endif
    {
      static_assert( _details::traits::is_sized_cursor<Itr, Snt>::value,
                     "pgbar::slice::IteratorSpan: Only available for sized iterator and sentinel pair" );
      static_assert(
        std::is_convertible<_details::traits::IterDifference_t<Itr>, _details::types::Size>::value,
        "pgbar::slice::IteratorSpan: The 'difference_type' must be convertible to Size" );

      _details::types::Size size_;
      Itr start_;
      Snt end_;

    public:
#if PGBAR__CXX17
      class sentinel {
        Snt ending_;

      public:
        constexpr sentinel() = default;
        constexpr sentinel( Snt&& endpoint ) noexcept( std::is_nothrow_move_constructible<Snt>::value )
          : ending_ { std::move( endpoint ) }
        {}
      };
#else
      class iterator;
      using sentinel = iterator;
#endif
      class iterator {
        Itr current_;

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

        constexpr iterator() = default;
        constexpr iterator( Itr startpoint ) noexcept( std::is_nothrow_move_constructible<Itr>::value )
          : current_ { std::move( startpoint ) }
        {}
        constexpr iterator( const iterator& )                         = default;
        constexpr iterator( iterator&& )                              = default;
        PGBAR__CXX14_CNSTXPR iterator& operator=( const iterator& ) & = default;
        PGBAR__CXX14_CNSTXPR iterator& operator=( iterator&& ) &      = default;
        PGBAR__CXX20_CNSTXPR ~iterator()                              = default;

        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR iterator& operator++() &
        {
          ++current_;
          return *this;
        }
        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR iterator operator++( int ) &
        {
          auto before = *this;
          operator++();
          return before;
        }

        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR reference operator*() const
        {
          return *current_;
        }
        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR pointer operator->() const noexcept
        {
          return current_;
        }
        PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR reference
          operator[]( _details::types::Size inc ) const noexcept( noexcept( *std::next( current_, inc ) ) )
        {
          return *std::next( current_, inc );
        }

        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const iterator& itr,
                                                                              const Itr& ir )
        {
          return itr.current_ == ir;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const Itr& ir,
                                                                              const iterator& itr )
        {
          return itr == ir;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const iterator& itr,
                                                                              const Itr& ir )
        {
          return !( itr == ir );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const Itr& ir,
                                                                              const iterator& itr )
        {
          return !( itr == ir );
        }
        template<typename Sentinel>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<Sentinel, Snt>,
                                  _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator==( const iterator& itr, const Sentinel& ir )
        {
          return itr.current_ == ir;
        }
        template<typename Sentinel>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<Sentinel, Snt>,
                                  _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator==( const Snt& ir, const iterator& itr )
        {
          return itr == ir;
        }
        template<typename Sentinel>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<Sentinel, Snt>,
                                  _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator!=( const iterator& itr, const Snt& ir )
        {
          return !( itr == ir );
        }
        template<typename Sentinel>
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr typename std::enable_if<
          _details::traits::AllOf<std::is_same<Sentinel, Snt>,
                                  _details::traits::Not<std::is_same<Itr, Snt>>>::value,
          bool>::type
          operator!=( const Snt& ir, const iterator& itr )
        {
          return !( itr == ir );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const iterator& a,
                                                                              const iterator& b )
        {
          return a.current_ == b.current_;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const iterator& a,
                                                                              const iterator& b )
        {
          return !( a == b );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr difference_type operator-( const iterator& a,
                                                                                        const iterator& b )
        {
          return _details::utils::distance( a.current_, b.current_ );
        }
#if PGBAR__CXX17
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const iterator& a,
                                                                              const sentinel& b )
        {
          return a.current_ == b.ending_;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const iterator& a,
                                                                              const sentinel& b )
        {
          return !( a == b );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator==( const sentinel& a,
                                                                              const iterator& b )
        {
          return b == a;
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr bool operator!=( const sentinel& a,
                                                                              const iterator& b )
        {
          return !( b == a );
        }
        PGBAR__NODISCARD friend PGBAR__FORCEINLINE constexpr difference_type operator-( const sentinel& a,
                                                                                        const iterator& b )
        {
          return b - a;
        }
#endif
      };

      PGBAR__CXX17_CNSTXPR IteratorSpan( Itr startpoint, Itr endpoint )
        : start_ { std::move( startpoint ) }, end_ { std::move( endpoint ) }
      {
        const auto length = _details::utils::distance( start_, end_ );
        if ( length < 0 )
          throw exception::InvalidArgument( "pgbar: negative iterator range" );
        size_ = static_cast<_details::types::Size>( length );
      }
      PGBAR__CXX17_CNSTXPR IteratorSpan( const IteratorSpan& )              = default;
      PGBAR__CXX17_CNSTXPR IteratorSpan( IteratorSpan&& )                   = default;
      PGBAR__CXX17_CNSTXPR IteratorSpan& operator=( const IteratorSpan& ) & = default;
      PGBAR__CXX17_CNSTXPR IteratorSpan& operator=( IteratorSpan&& ) &      = default;
      // Intentional non-virtual destructors.
      PGBAR__CXX20_CNSTXPR ~IteratorSpan()                                  = default;

      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr iterator begin() const
        noexcept( _details::traits::AllOf<std::is_nothrow_move_constructible<Itr>,
                                          std::is_nothrow_copy_constructible<Itr>>::value )
      {
        return { this->start_ };
      }
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr sentinel end() const
        noexcept( _details::traits::AllOf<std::is_nothrow_move_constructible<Snt>,
                                          std::is_nothrow_copy_constructible<Snt>>::value )
      {
        return { this->end_ };
      }

      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR typename iterator::reference front()
        const noexcept
      {
        return *start_;
      }
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR typename iterator::reference back()
        const noexcept
      {
        return *std::next( start_, size_ - 1 );
      }
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr _details::types::Size step() const noexcept { return 1; }
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr _details::types::Size size() const noexcept
      {
        return size_;
      }
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr bool empty() const noexcept { return size_ == 0; }

      PGBAR__CXX20_CNSTXPR void swap( IteratorSpan<Itr>& lhs ) noexcept
      {
        PGBAR__TRUST( this != &lhs );
        using std::swap;
        swap( start_, lhs.start_ );
        swap( end_, lhs.end_ );
        swap( size_, lhs.size_ );
      }
      friend PGBAR__CXX20_CNSTXPR void swap( IteratorSpan<Itr>& a, IteratorSpan<Itr>& b ) noexcept
      {
        a.swap( b );
      }

      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR typename iterator::reference operator[](
        _details::types::Size inc ) const noexcept( noexcept( *std::next( start_, inc ) ) )
      {
        return *std::next( start_, inc );
      }
      PGBAR__CXX17_CNSTXPR explicit operator bool() const noexcept { return !empty(); }
    };
  } // namespace slice
} // namespace pgbar

#endif
