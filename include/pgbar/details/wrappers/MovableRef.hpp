#ifndef PGBAR__MOVABLEREF
#define PGBAR__MOVABLEREF

#include "../core/Core.hpp"
#include "../traits/Backport.hpp"
#include <utility>

namespace pgbar {
  namespace _details {
    namespace wrappers {
      template<typename T>
      class MovableRef {
        using Self = MovableRef;

        T* ref_;

      public:
        constexpr MovableRef() noexcept : ref_ { nullptr } {}
        PGBAR__CXX14_CNSTXPR MovableRef( const Self& )        = default;
        PGBAR__CXX14_CNSTXPR Self& operator=( const Self& ) & = default;
        PGBAR__CXX14_CNSTXPR MovableRef( Self&& rhs ) noexcept : ref_ { rhs.ref_ } { rhs.ref_ = nullptr; }
        PGBAR__CXX14_CNSTXPR Self& operator=( Self&& rhs ) & noexcept
        {
          ref_     = rhs.ref_;
          rhs.ref_ = nullptr;
          return *this;
        }

        template<typename U,
                 typename = typename std::enable_if<
                   traits::AllOf<traits::Not<std::is_same<typename std::decay<U>::type, Self>>,
                                 std::is_convertible<U&&, T&>>::value>::type>
        PGBAR__CXX17_CNSTXPR MovableRef( U&& x ) noexcept
        {
          T& t = std::forward<U>( x );
          ref_ = std::addressof( t );
        }
        PGBAR__CXX17_CNSTXPR Self& operator=( T& ref ) & noexcept
        {
          ref_ = std::addressof( ref );
          return *this;
        }

        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T& get() & noexcept
        {
          PGBAR__TRUST( ref_ != nullptr );
          return *ref_;
        }
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T& get() const& noexcept
        {
          PGBAR__TRUST( ref_ != nullptr );
          return *ref_;
        }
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T&& get() && noexcept
        {
          PGBAR__TRUST( ref_ != nullptr );
          return std::move( *ref_ );
        }

        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T& operator*() & noexcept { return get(); }
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T& operator*() const& noexcept { return get(); }
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T&& operator*() && noexcept { return std::move( get() ); }

        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T* operator->() const noexcept { return ref_; }

        // make it be invocable
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T& operator()() & noexcept { return get(); }
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T& operator()() const& noexcept { return get(); }
        PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR T&& operator()() && noexcept { return std::move( get() ); }

        PGBAR__CXX14_CNSTXPR operator T&() & noexcept { return get(); }
        PGBAR__CXX14_CNSTXPR operator T&() const& noexcept { return get(); }
        PGBAR__CXX14_CNSTXPR operator T&&() && noexcept { return std::move( get() ); }

        explicit constexpr operator bool() const noexcept { return ref_ != nullptr; }

        PGBAR__CXX20_CNSTXPR void swap( Self& other ) noexcept { std::swap( ref_, other.ref_ ); }
        friend PGBAR__CXX20_CNSTXPR void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        template<typename U>
        PGBAR__NODISCARD friend constexpr
          typename std::enable_if<std::is_same<typename std::decay<U>::type, T>::value, bool>::type
          operator==( const Self& a, const MovableRef<const U>& b ) noexcept
        {
          return a.ref_ == b.ref_;
        }
        template<typename U>
        PGBAR__NODISCARD friend constexpr
          typename std::enable_if<std::is_same<typename std::decay<U>::type, T>::value, bool>::type
          operator!=( const Self& a, const MovableRef<const U>& b ) noexcept
        {
          return !( a == b );
        }
        template<typename U>
        PGBAR__NODISCARD friend constexpr
          typename std::enable_if<std::is_same<typename std::decay<U>::type, T>::value, bool>::type
          operator==( const MovableRef<const U>& a, const Self& b ) noexcept
        {
          return a.ref_ == b.ref_;
        }
        template<typename U>
        PGBAR__NODISCARD friend constexpr
          typename std::enable_if<std::is_same<typename std::decay<U>::type, T>::value, bool>::type
          operator!=( const MovableRef<const U>& a, const Self& b ) noexcept
        {
          return !( b == a );
        }
        PGBAR__NODISCARD friend constexpr bool operator==( const Self& a, const T& b ) noexcept
        {
          return a.ref_ == std::addressof( b );
        }
        template<typename U>
        PGBAR__NODISCARD friend constexpr bool operator!=( const Self& a, const T& b ) noexcept
        {
          return !( a == b );
        }
        PGBAR__NODISCARD friend constexpr bool operator==( const T& a, const Self& b ) noexcept
        {
          return b == a;
        }
        PGBAR__NODISCARD friend constexpr bool operator!=( const T& a, const Self& b ) noexcept
        {
          return !( b == a );
        }
      };

#if PGBAR__CXX17
      template<typename T>
      MovableRef( T& ) -> MovableRef<T>;
#endif

      template<typename T>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR MovableRef<T> mref( T& x ) noexcept
      {
        return MovableRef<T>( x );
      }
      template<typename T>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr MovableRef<T> mref( MovableRef<T> x ) noexcept
      {
        if ( x )
          return MovableRef<T>( *x );
        return MovableRef<T>();
      }
      template<typename T>
      void mref( const T&& ) = delete;
    } // namespace wrappers
  } // namespace _details
} // namespace pgbar

#endif
