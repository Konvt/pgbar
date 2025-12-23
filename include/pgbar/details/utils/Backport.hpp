#ifndef PGBAR__UTILS_BACKPORT
#define PGBAR__UTILS_BACKPORT

#include "../core/Core.hpp"
#include "../types/Types.hpp"
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#ifndef __cpp_lib_invoke
# include "../traits/Backport.hpp"
# include "../traits/Util.hpp"
#endif
#ifdef __cpp_lib_ranges
# include <ranges>
#endif

namespace pgbar {
  namespace _details {
    namespace utils {
      // Before C++17, not all std entities had feature macros.
#if PGBAR__CXX14
      template<typename T, typename... Args>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX23_CNSTXPR auto make_unique( Args&&... args )
      {
        return std::make_unique<T>( std::forward<Args>( args )... );
      }
#else
      // picking up the standard's mess...
      template<typename T, typename... Args>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX23_CNSTXPR std::unique_ptr<T> make_unique(
        Args&&... args )
      {
        return std::unique_ptr<T>( ::new T( std::forward<Args>( args )... ) );
      }
#endif

      PGBAR__FORCEINLINE int uncaught_exceptions() noexcept
      {
#ifdef __cpp_lib_uncaught_exceptions
        return std::uncaught_exceptions();
#else
        return static_cast<int>( std::uncaught_exception() );
#endif
      }

      // Available only for objects that constructed by placement new.
      template<typename T>
      PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR void destruct_at( T* location )
        noexcept( std::is_nothrow_destructible<T>::value )
      {
#if PGBAR__CXX17
        std::destroy_at( location );
#else
        static_assert( !std::is_array<T>::value,
                       "pgbar::_details::utils::destruct_at: The program is ill-formed" );
        if PGBAR__CXX17_CNSTXPR ( !std::is_trivially_destructible<T>::value )
          location->~T();
#endif
      }
      template<typename T>
      PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR void destruct_at( T& object )
        noexcept( std::is_nothrow_destructible<T>::value )
      {
        destruct_at( std::addressof( object ) );
      }

      // Available only for buffers that use placement new.
      template<typename To, typename From>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR To* launder_as( From* src ) noexcept
      {
#ifdef __cpp_lib_launder
        return std::launder( reinterpret_cast<To*>( src ) );
#elif defined( __GNUC__ )
# if __GNUC__ >= 7
        return __builtin_launder( reinterpret_cast<To*>( src ) );
# else
        // same as the impl of folly::launder (v2017.09.04)
        __asm__( "" : "+r"( src ) );
# endif
#elif defined( __clang__ )
# if __clang_major__ >= 8
        return __builtin_launder( reinterpret_cast<To*>( src ) );
# endif
#endif
        // By default, we can only trust that reinterpret_cast can give us what we want.
        return reinterpret_cast<To*>( src );
      }
      template<typename To>
      To* launder_as( void* src ) = delete;
      template<typename To>
      To* launder_as( const void* src ) = delete;
      template<typename To>
      To* launder_as( volatile void* src ) = delete;
      template<typename To>
      To* launder_as( const volatile void* src ) = delete;
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wambiguous-ellipsis"
#endif
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args... ) ) = delete;
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args...... ) ) = delete;
#if PGBAR__CXX17
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args... ) noexcept ) = delete;
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args...... ) noexcept ) = delete;
#endif
#ifdef __clang__
# pragma clang diagnostic pop
#endif

#ifdef __cpp_lib_as_const
      template<typename T>
      PGBAR__FORCEINLINE constexpr decltype( auto ) as_const( T&& param ) noexcept
      {
        return std::as_const( std::forward<T>( param ) );
      }
#else
      template<typename T>
      PGBAR__FORCEINLINE constexpr typename std::add_const<T>::type& as_const( T& param ) noexcept
      {
        return param;
      }
      template<typename T>
      void as_const( const T&& ) = delete;
#endif

#ifdef __cpp_lib_invoke
      template<typename Fn, typename... Args>
      PGBAR__FORCEINLINE constexpr decltype( auto ) invoke( Fn&& fn, Args&&... args )
        noexcept( noexcept( std::invoke( std::forward<Fn>( fn ), std::forward<Args>( args )... ) ) )
      // some compilers have provided std::invoke in C++14 but without the std::is_nothrow_invocable,
      // it's strange (e.g. MSVC STL).
      {
        return std::invoke( std::forward<Fn>( fn ), std::forward<Args>( args )... );
      }
#else
      template<typename C, typename MemFn, typename Object, typename... Args>
      PGBAR__FORCEINLINE constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( std::forward<Object>( object ).*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                      std::is_same<C, typename std::decay<Object>::type>>>::value,
          decltype( ( std::forward<Object>( object ).*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( std::forward<Object>( object ).*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemFn, typename Object, typename... Args>
      PGBAR__FORCEINLINE constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( object.get().*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                                              traits::is_instance_of<typename std::decay<Object>::type,
                                                                     std::reference_wrapper>>::value,
                                decltype( ( object.get().*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( object.get().*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemFn, typename Object, typename... Args>
      PGBAR__FORCEINLINE constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( ( *std::forward<Object>( object ) )
                              .*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::Not<traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                                  std::is_same<C, typename std::decay<Object>::type>,
                                                  traits::is_instance_of<typename std::decay<Object>::type,
                                                                         std::reference_wrapper>>>>::value,
          decltype( ( ( *std::forward<Object>( object ) ).*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( ( *std::forward<Object>( object ) ).*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemObj, typename Object>
      PGBAR__FORCEINLINE constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                      std::is_same<C, typename std::decay<Object>::type>>>::value,
          decltype( std::forward<Object>( object ).*member )>::type
      {
        return std::forward<Object>( object ).*member;
      }
      template<typename C, typename MemObj, typename Object>
      PGBAR__FORCEINLINE constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                                              traits::is_instance_of<typename std::decay<Object>::type,
                                                                     std::reference_wrapper>>::value,
                                decltype( object.get().*member )>::type
      {
        return object.get().*member;
      }
      template<typename C, typename MemObj, typename Object>
      PGBAR__FORCEINLINE constexpr auto invoke( MemObj C::* member, Object&& object )
        noexcept( noexcept( ( *std::forward<Object>( object ) ).*member ) ) -> typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::Not<traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                                  std::is_same<C, typename std::decay<Object>::type>,
                                                  traits::is_instance_of<typename std::decay<Object>::type,
                                                                         std::reference_wrapper>>>>::value,
          decltype( ( *std::forward<Object>( object ) ).*member )>::type
      {
        return ( *std::forward<Object>( object ) ).*member;
      }
      template<typename Fn, typename... Args>
      PGBAR__FORCEINLINE constexpr auto invoke( Fn&& fn, Args&&... args )
        noexcept( noexcept( std::forward<Fn>( fn )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::Not<
            traits::AnyOf<std::is_member_function_pointer<typename std::remove_reference<Fn>::type>,
                          std::is_member_object_pointer<typename std::remove_reference<Fn>::type>>>::value,
          decltype( std::forward<Fn>( fn )( std::forward<Args>( args )... ) )>::type
      {
        return std::forward<Fn>( fn )( std::forward<Args>( args )... );
      }
#endif

#ifdef __cpp_lib_ranges
      template<typename Itr, typename Snt>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR auto distance( Itr&& first, Snt&& second )
        noexcept( noexcept( std::ranges::distance( std::forward<Itr>( first ),
                                                   std::forward<Snt>( second ) ) ) )
      {
        return std::ranges::distance( std::forward<Itr>( first ), std::forward<Snt>( second ) );
      }
      template<typename R>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR auto begin( R&& r )
        noexcept( noexcept( std::ranges::begin( std::forward<R>( r ) ) ) )
      {
        return std::ranges::begin( std::forward<R>( r ) );
      }
      template<typename R>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR auto end( R&& r )
        noexcept( noexcept( std::ranges::end( std::forward<R>( r ) ) ) )
      {
        return std::ranges::end( std::forward<R>( r ) );
      }
#else
      template<typename Itr>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX17_CNSTXPR auto distance( Itr first, Itr second )
        noexcept( noexcept( std::distance( std::move( first ), std::move( second ) ) ) )
          -> decltype( std::distance( std::move( first ), std::move( second ) ) )
      {
        return std::distance( std::move( first ), std::move( second ) );
      }
      template<typename R>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR auto begin( R&& r )
        noexcept( noexcept( std::begin( std::forward<R>( r ) ) ) )
          -> decltype( std::begin( std::forward<R>( r ) ) )
      {
        return std::begin( std::forward<R>( r ) );
      }
      template<typename R>
      PGBAR__NODISCARD PGBAR__FORCEINLINE PGBAR__CXX14_CNSTXPR auto end( R&& r )
        noexcept( noexcept( std::end( std::forward<R>( r ) ) ) )
          -> decltype( std::end( std::forward<R>( r ) ) )
      {
        return std::end( std::forward<R>( r ) );
      }
#endif

#ifdef __cpp_lib_ranges
      template<typename Range>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr types::Size size( Range&& rn )
        noexcept( noexcept( std::ranges::size( std::forward<Range>( rn ) ) ) )
      {
        return std::ranges::size( std::forward<Range>( rn ) );
      }
#elif defined( __cpp_lib_nonmember_container_access )
      template<typename Range>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr types::Size size( Range&& rn )
        noexcept( noexcept( std::size( std::forward<Range>( rn ) ) ) )
      {
        return std::size( std::forward<Range>( rn ) );
      }
#else
      template<typename Range>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr types::Size size( Range&& rn )
        noexcept( noexcept( std::forward<Range>( rn ).size() ) )
      {
        return std::forward<Range>( rn ).size();
      }
      template<typename Range, types::Size N>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr types::Size size( const Range ( & )[N] ) noexcept
      {
        return N;
      }
#endif

#if PGBAR__CXX20
      template<typename T, typename... Args>
      PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR T* construct_at( T* location, Args&&... args )
        noexcept( noexcept( std::construct_at( std::declval<T*>(), std::forward<Args>( args )... ) ) )
      {
        return std::construct_at( reinterpret_cast<T*>( location ), std::forward<Args>( args )... );
      }
#else
      template<typename T, typename... Args>
      PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR typename std::enable_if<std::is_array<T>::value, T*>::type
        construct_at( T* location ) noexcept( noexcept( ::new( std::declval<void*>() ) T[1]() ) )
      {
        // static_assert( !std::is_unbounded_array_v<T> );
        return ::new ( location ) T[1]();
      }
      template<typename T, typename... Args>
      PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR typename std::enable_if<!std::is_array<T>::value, T*>::type
        construct_at( T* location, Args&&... args )
          noexcept( noexcept( ::new( std::declval<void*>() ) T( std::forward<Args>( args )... ) ) )
      {
        return ::new ( location ) T( std::forward<Args>( args )... );
      }
#endif
      template<typename T, typename U, typename... Args>
      PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR T* construct_at( U* location, Args&&... args )
        noexcept( noexcept( construct_at( std::declval<T*>(), std::forward<Args>( args )... ) ) )
      {
        static_assert( !std::is_void<T>::value, "pgbar::_details::utils::construct_at: Invalid type" );
        return construct_at( reinterpret_cast<T*>( location ), std::forward<Args>( args )... );
      }

#ifdef __cpp_lib_to_underlying
      template<typename E>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr auto as_val( E enum_val ) noexcept
      {
        return std::to_underlying( enum_val );
      }
#else
      template<typename E>
      PGBAR__NODISCARD PGBAR__FORCEINLINE constexpr typename std::underlying_type<E>::type as_val(
        E enum_val ) noexcept
      {
        return static_cast<typename std::underlying_type<E>::type>( enum_val );
      }

#endif

      // see https://github.com/llvm/llvm-project/issues/101614
#if defined( __cpp_lib_forward_like ) && PGBAR__CONSISTENT_VENDOR
      template<typename As, typename T>
      PGBAR__FORCEINLINE constexpr decltype( auto ) forward_as( T&& param ) noexcept
      {
        return std::forward_like<As>( std::forward<T>( param ) );
      }
#else
      template<typename As, typename T>
      PGBAR__FORCEINLINE constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<std::is_lvalue_reference<As&&>::value
                                  && std::is_const<typename std::remove_reference<As>::type>::value,
                                decltype( as_const( param ) )>::type
      {
        return as_const( param );
      }
      template<typename As, typename T>
      PGBAR__FORCEINLINE constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<std::is_lvalue_reference<As&&>::value
                                  && !std::is_const<typename std::remove_reference<As>::type>::value,
                                T&>::type
      {
        return static_cast<T&>( param );
      }
      template<typename As, typename T>
      PGBAR__FORCEINLINE constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<!std::is_lvalue_reference<As&&>::value
                                  && std::is_const<typename std::remove_reference<As>::type>::value,
                                decltype( std::move( as_const( param ) ) )>::type
      {
        return std::move( as_const( param ) );
      }
      template<typename As, typename T>
      PGBAR__FORCEINLINE constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<!std::is_lvalue_reference<As&&>::value
                                  && !std::is_const<typename std::remove_reference<As>::type>::value,
                                decltype( std::move( param ) )>::type
      {
        return std::move( param );
      }
#endif

      [[noreturn]] PGBAR__FORCEINLINE void unreachable()
      {
#ifdef __cpp_lib_unreachable
        std::unreachable();
#elif defined( __GNUC__ ) || defined( __clang__ )
        __builtin_unreachable();
#else
        PGBAR__TRUST( 0 );
#endif
      }
    } // namespace utils

    namespace traits {
#if PGBAR__CXX17
      template<typename Fn, typename... Args>
      using is_invocable = std::is_invocable<Fn, Args...>;

      template<typename Ret, typename Fn, typename... Args>
      using is_invocable_to = std::is_invocable_r<Ret, Fn, Args...>;
#else
      template<typename Fn, typename... Args>
      struct is_invocable {
      private:
        template<typename F>
        static constexpr auto check( int ) -> typename std::enable_if<
          std::is_void<decltype( utils::invoke( std::declval<F>(), std::declval<Args>()... ),
                                 void() )>::value,
          std::true_type>::type;
        template<typename>
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check<Fn>( 0 ) )::value;
      };

      template<typename Ret, typename Fn, typename... Args>
      struct is_invocable_to {
      private:
        template<typename F>
        static constexpr auto check( int )
          -> std::is_convertible<decltype( utils::invoke( std::declval<F>(), std::declval<Args>()... ) ),
                                 Ret>;
        template<typename>
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check<Fn>( 0 ) )::value;
      };
#endif
    }
  } // namespace _details
} // namespace pgbar
#endif
