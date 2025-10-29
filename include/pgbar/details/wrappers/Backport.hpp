#ifndef PGBAR__WRAPPERS_BACKPORT
#define PGBAR__WRAPPERS_BACKPORT

#include "../traits/Backport.hpp"
#if PGBAR__CXX23
# include <functional>
#else
# include "../utils/Backport.hpp"
# include <cstddef>
# include <new>
#endif

namespace pgbar {
  namespace _details {
    namespace wrappers {
#if PGBAR__CXX23
      template<typename... Signature>
      using UniqueFunction = std::move_only_function<Signature...>;
#else
      template<typename Derived>
      class FnStorageBlock {
      protected:
# if PGBAR__CXX17
        using Data = std::byte;
# else
        using Data = unsigned char;
# endif
        union alignas( std::max_align_t ) AnyFn {
          Data buf_[sizeof( void* ) * 2];
          Data* dptr_;

          constexpr AnyFn() noexcept : dptr_ { nullptr } {}
        };
        struct VTable final {
          const typename Derived::Delegate invoke;
          // The handling of function types by msvc is very strange:
          // it often triggers internal compiler errors for no apparent reason,
          // so here we have to manually write the function pointer type.
          void ( *const destroy )( AnyFn& ) noexcept;
          void ( *const move )( AnyFn& dst, AnyFn& src ) noexcept;
        };

        template<typename T>
        using Inlinable = traits::AllOf<
          std::is_nothrow_move_constructible<T>,
          traits::BoolConstant<( sizeof( AnyFn::buf_ ) >= sizeof( T ) && alignof( AnyFn ) >= alignof( T ) )>>;

        const VTable* vtable_;
        AnyFn callee_;

        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR void destroy_null( AnyFn& ) noexcept {}
        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR void move_null( AnyFn&, AnyFn& ) noexcept {}

        template<typename T>
        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR void destroy_inline( AnyFn& fn ) noexcept
        {
          const auto ptr = utils::launder_as<T>( &fn.buf_ );
          ptr->~T();
        }
        template<typename T>
        static PGBAR__NOINLINE void move_inline( AnyFn& dst, AnyFn& src ) noexcept
        {
          new ( &dst.buf_ ) T( std::move( *utils::launder_as<T>( &src.buf_ ) ) );
          destroy_inline<T>( src );
        }

        template<typename T>
        static PGBAR__NOINLINE PGBAR__CXX20_CNSTXPR void destroy_dynamic( AnyFn& fn ) noexcept
        {
          const auto dptr = utils::launder_as<T>( fn.dptr_ );
          dptr->~T();
# if PGBAR__CXX17
          operator delete( fn.dptr_, std::align_val_t( alignof( T ) ) );
# else
          operator delete( fn.dptr_ );
# endif
        }
        template<typename T>
        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR void move_dynamic( AnyFn& dst, AnyFn& src ) noexcept
        {
          dst.dptr_ = src.dptr_;
          src.dptr_ = &table_null();
        }

        template<typename F>
        static typename std::enable_if<Inlinable<typename std::decay<F>::type>::value>::type
          store_fn( const VTable*( &vtable ), AnyFn& any, F&& fn ) noexcept
        {
          using T             = typename std::decay<F>::type;
          const auto location = new ( &any.buf_ ) T( std::forward<F>( fn ) );
          PGBAR__TRUST( static_cast<void*>( location ) == static_cast<void*>( &any.buf_ ) );
          (void)location;
          vtable = &table_inline<T>();
        }
        template<typename F>
        static typename std::enable_if<!Inlinable<typename std::decay<F>::type>::value>::type
          store_fn( const VTable*( &vtable ), AnyFn& any, F&& fn ) noexcept( false )
        {
          using T   = typename std::decay<F>::type;
          auto dptr = std::unique_ptr<void, void ( * )( void* )>(
# if PGBAR__CXX17
            operator new( sizeof( T ), std::align_val_t( alignof( T ) ) ),
# else
            operator new( sizeof( T ) ),
# endif
            +[]( void* ptr ) { operator delete( ptr ); } );

          const auto location = new ( dptr.get() ) T( std::forward<F>( fn ) );
          PGBAR__ASSERT( static_cast<void*>( location ) == dptr.get() );
          (void)location;

          any.dptr_ = static_cast<Data*>( dptr.release() );
          vtable    = &table_dynamic<T>();
        }

        template<typename T>
        static PGBAR__CXX23_CNSTXPR const VTable& table_inline() noexcept
        {
          static const VTable tbl { Derived::template invoke_inline<T>, destroy_inline<T>, move_inline<T> };
          return tbl;
        }
        template<typename T>
        static PGBAR__CXX23_CNSTXPR const VTable& table_dynamic() noexcept
        {
          static const VTable tbl { Derived::template invoke_dynamic<T>,
                                    destroy_dynamic<T>,
                                    move_dynamic<T> };
          return tbl;
        }
        static PGBAR__CXX23_CNSTXPR const VTable& table_null() noexcept
        {
          static const VTable tbl { Derived::invoke_null, destroy_null, move_null };
          return tbl;
        }

        PGBAR__CXX23_CNSTXPR FnStorageBlock() noexcept : vtable_ { &table_null() } {}

        PGBAR__CXX23_CNSTXPR void reset() noexcept
        {
          PGBAR__TRUST( vtable_ != nullptr );
          vtable_->destroy( callee_ );
          vtable_ = &table_null();
        }
        template<typename F>
        void reset( F&& fn ) noexcept( Inlinable<typename std::decay<F>::type>::value )
        {
          const VTable* vtable = nullptr;
          AnyFn tmp;
          store_fn( vtable, tmp, std::forward<F>( fn ) );
          PGBAR__TRUST( vtable != nullptr );
          reset();
          std::swap( vtable_, vtable );
          PGBAR__TRUST( this->vtable_ != nullptr );
          vtable_->move( callee_, tmp );
        }

      public:
        PGBAR__CXX23_CNSTXPR FnStorageBlock( FnStorageBlock&& rhs ) noexcept : vtable_ { rhs.vtable_ }
        {
          PGBAR__TRUST( rhs.vtable_ != nullptr );
          vtable_->move( callee_, rhs.callee_ );
          rhs.vtable_ = &table_null();
        }
        PGBAR__CXX23_CNSTXPR FnStorageBlock& operator=( FnStorageBlock&& rhs ) & noexcept
        {
          PGBAR__TRUST( this != &rhs );
          PGBAR__TRUST( vtable_ != nullptr );
          PGBAR__TRUST( rhs.vtable_ != nullptr );
          reset();
          std::swap( vtable_, rhs.vtable_ );
          PGBAR__TRUST( this->vtable_ != nullptr );
          PGBAR__TRUST( rhs.vtable_ != nullptr );
          vtable_->move( callee_, rhs.callee_ );
          return *this;
        }
        PGBAR__CXX23_CNSTXPR ~FnStorageBlock() noexcept { reset(); }

        PGBAR__CXX23_CNSTXPR void swap( FnStorageBlock& lhs ) noexcept
        {
          PGBAR__TRUST( vtable_ != nullptr );
          PGBAR__TRUST( lhs.vtable_ != nullptr );
          AnyFn tmp;
          vtable_->move( tmp, callee_ );
          lhs.vtable_->move( callee_, lhs.callee_ );
          vtable_->move( lhs.callee_, tmp );
          std::swap( vtable_, lhs.vtable_ );
          PGBAR__TRUST( this->vtable_ != nullptr );
          PGBAR__TRUST( lhs.vtable_ != nullptr );
        }
        PGBAR__CXX23_CNSTXPR friend void swap( FnStorageBlock& a, FnStorageBlock& b ) noexcept
        {
          return a.swap( b );
        }
        friend constexpr bool operator==( const FnStorageBlock& a, std::nullptr_t ) noexcept
        {
          return !static_cast<bool>( a );
        }
        friend constexpr bool operator!=( const FnStorageBlock& a, std::nullptr_t ) noexcept
        {
          return static_cast<bool>( a );
        }
        constexpr explicit operator bool() const noexcept { return vtable_ != &table_null(); }
      };
      // `CrefInfo` can be any types that contains the `cref` info of the functor.
      // e.g. For the function type `void () const&`, the `CrefInfo` can be: `const int&`.
      template<typename Derived, typename CrefInfo, typename R, bool Noexcept, typename... Args>
      class FnInvokeBlock : public FnStorageBlock<Derived> {
        friend class FnStorageBlock<Derived>;

        using typename FnStorageBlock<Derived>::AnyFn;
        template<typename T>
        using Param_t = typename std::conditional<std::is_scalar<T>::value, T, T&&>::type;
        template<typename T>
        using Fn_t = typename std::
          conditional<std::is_const<typename std::remove_reference<CrefInfo>::type>::value, const T, T>::type;

      protected:
        using Delegate = R ( * )( const AnyFn&, Param_t<Args>... )
# if PGBAR__CXX17
          noexcept( Noexcept )
# endif
          ;

        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR R invoke_null( const AnyFn&, Param_t<Args>... )
          noexcept( Noexcept )
        {
          PGBAR__UNREACHABLE;
          // The standard says this should trigger an undefined behavior.
        }
        template<typename T>
        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR R invoke_inline( const AnyFn& fn, Param_t<Args>... args )
          noexcept( Noexcept )
        {
          const auto ptr = utils::launder_as<Fn_t<T>>( ( &const_cast<AnyFn&>( fn ).buf_ ) );
          return utils::invoke( utils::forward_as<CrefInfo>( *ptr ), std::forward<Args>( args )... );
        }
        template<typename T>
        static PGBAR__NOINLINE PGBAR__CXX14_CNSTXPR R invoke_dynamic( const AnyFn& fn, Param_t<Args>... args )
          noexcept( Noexcept )
        {
          const auto dptr = utils::launder_as<Fn_t<T>>( fn.dptr_ );
          return utils::invoke( utils::forward_as<CrefInfo>( *dptr ), std::forward<Args>( args )... );
        }

        constexpr FnInvokeBlock()                                          = default;
        constexpr FnInvokeBlock( FnInvokeBlock&& )                         = default;
        PGBAR__CXX14_CNSTXPR FnInvokeBlock& operator=( FnInvokeBlock&& ) & = default;
        PGBAR__CXX23_CNSTXPR ~FnInvokeBlock()                              = default;
      };

      // A simplified implementation of std::move_only_function
      template<typename...>
      class UniqueFunction;
      template<typename R, typename... Args>
      class UniqueFunction<R( Args... )>
        : public FnInvokeBlock<UniqueFunction<R( Args... )>, int&, R, false, Args...> {
        // Function types without ref qualifier will be treated as non-const lvalue reference types.
        using Base = FnStorageBlock<UniqueFunction>;

      public:
        UniqueFunction( const UniqueFunction& )              = delete;
        UniqueFunction& operator=( const UniqueFunction& ) & = delete;

        constexpr UniqueFunction()                                           = default;
        constexpr UniqueFunction( UniqueFunction&& )                         = default;
        PGBAR__CXX14_CNSTXPR UniqueFunction& operator=( UniqueFunction&& ) & = default;
        PGBAR__CXX23_CNSTXPR ~UniqueFunction()                               = default;

        constexpr UniqueFunction( std::nullptr_t ) noexcept : UniqueFunction() {}
        template<typename F,
                 typename = typename std::enable_if<
                   traits::AllOf<std::is_constructible<typename std::decay<F>::type, F>,
                                 traits::InvocableTo<R, F, Args...>>::value>::type>
        UniqueFunction( F&& fn ) noexcept( Base::template Inlinable<typename std::decay<F>::type>::value )
        {
          Base::store_fn( this->vtable_, this->callee_, std::forward<F>( fn ) );
          PGBAR__TRUST( this->vtable_ != nullptr );
        }
        // In C++11, `std::in_place_type` does not exist, and we will not use it either.
        // Therefore, we do not provide an overloaded constructor for this type here.
        template<typename F>
        typename std::enable_if<traits::AllOf<std::is_constructible<typename std::decay<F>::type, F>,
                                              traits::InvocableTo<R, F, Args...>>::value,
                                UniqueFunction&>::type
          operator=( F&& fn ) & noexcept( Base::template Inlinable<typename std::decay<F>::type>::value )
        {
          this->reset( std::forward<F>( fn ) );
          return *this;
        }
        PGBAR__CXX23_CNSTXPR UniqueFunction& operator=( std::nullptr_t ) noexcept
        {
          this->reset();
          return *this;
        }

        PGBAR__INLINE_FN R operator()( Args... args )
        {
          PGBAR__TRUST( this->vtable_ != nullptr );
          return ( *this->vtable_->invoke )( this->callee_, std::forward<Args>( args )... );
        }
      };
#endif
    } // namespace wrappers
  } // namespace _details
} // namespace pgbar

#endif
