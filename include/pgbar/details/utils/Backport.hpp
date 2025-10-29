#ifndef PGBAR__UTILS_BACKPORT
#define PGBAR__UTILS_BACKPORT

#include "../core/Core.hpp"
#include <memory>
#include <utility>
#if PGBAR__CXX17
# include <functional>
# include <new>
#else
# include "../traits/Backport.hpp"
# include "../traits/Util.hpp"
# include "../utils/Backport.hpp"
#endif

namespace pgbar {
  namespace _details {
    namespace utils {
#if PGBAR__CXX14
      template<typename T, typename... Args>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX23_CNSTXPR auto make_unique( Args&&... args )
      {
        return std::make_unique<T>( std::forward<Args>( args )... );
      }
#else
      // picking up the standard's mess...
      template<typename T, typename... Args>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX23_CNSTXPR std::unique_ptr<T> make_unique( Args&&... args )
      {
        return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
      }
#endif

      // Available only for objects that constructed by placement new.
      template<typename T>
      PGBAR__CXX20_CNSTXPR void destruct_at( T& object ) noexcept( std::is_nothrow_destructible<T>::value )
      {
#if PGBAR__CXX17
        std::destroy_at( std::addressof( object ) );
#else
        static_assert( !std::is_array<T>::value,
                       "pgbar::_details::utils::destruct_at: The program is ill-formed" );
        object.~T();
#endif
      }

      // Available only for buffers that use placement new.
      template<typename To, typename From>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX17_CNSTXPR To* launder_as( From* src ) noexcept
      {
#if PGBAR__CXX17
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
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args... ) ) = delete;
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args...... ) ) = delete;
#if PGBAR__CXX17
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args... ) noexcept ) = delete;
      template<typename To, typename R, typename... Args>
      To* launder_as( R ( * )( Args...... ) noexcept ) = delete;

      template<typename T>
      constexpr decltype( auto ) as_const( T&& param ) noexcept
      {
        return std::as_const( std::forward<T>( param ) );
      }

      template<typename Fn, typename... Args>
      PGBAR__INLINE_FN constexpr decltype( auto ) invoke( Fn&& fn, Args&&... args )
        noexcept( std::is_nothrow_invocable_v<Fn, Args...> )
      {
        return std::invoke( std::forward<Fn>( fn ), std::forward<Args>( args )... );
      }
#else
      template<typename T>
      constexpr typename std::add_const<T>::type& as_const( T& param ) noexcept
      {
        return param;
      }
      template<typename T>
      void as_const( const T&& ) = delete;

      template<typename C, typename MemFn, typename Object, typename... Args>
      PGBAR__INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
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
      PGBAR__INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( object.get().*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::InstanceOf<typename std::decay<Object>::type, std::reference_wrapper>>::value,
          decltype( ( object.get().*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( object.get().*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemFn, typename Object, typename... Args>
      PGBAR__INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( ( *std::forward<Object>( object ) )
                              .*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::Not<traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                                  std::is_same<C, typename std::decay<Object>::type>,
                                                  traits::InstanceOf<typename std::decay<Object>::type,
                                                                     std::reference_wrapper>>>>::value,
          decltype( ( ( *std::forward<Object>( object ) ).*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( ( *std::forward<Object>( object ) ).*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemObj, typename Object>
      PGBAR__INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                      std::is_same<C, typename std::decay<Object>::type>>>::value,
          decltype( std::forward<Object>( object ).*member )>::type
      {
        return std::forward<Object>( object ).*member;
      }
      template<typename C, typename MemObj, typename Object>
      PGBAR__INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::InstanceOf<typename std::decay<Object>::type, std::reference_wrapper>>::value,
          decltype( object.get().*member )>::type
      {
        return object.get().*member;
      }
      template<typename C, typename MemObj, typename Object>
      PGBAR__INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object )
        noexcept( noexcept( ( *std::forward<Object>( object ) ).*member ) ) -> typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::Not<traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                                  std::is_same<C, typename std::decay<Object>::type>,
                                                  traits::InstanceOf<typename std::decay<Object>::type,
                                                                     std::reference_wrapper>>>>::value,
          decltype( ( *std::forward<Object>( object ) ).*member )>::type
      {
        return ( *std::forward<Object>( object ) ).*member;
      }
      template<typename Fn, typename... Args>
      PGBAR__INLINE_FN constexpr auto invoke( Fn&& fn, Args&&... args )
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

#if PGBAR__CXX23
      template<typename E>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CNSTEVAL auto as_val( E enum_val ) noexcept
      {
        return std::to_underlying( enum_val );
      }

      template<typename As, typename T>
      constexpr decltype( auto ) forward_as( T&& param ) noexcept
      {
        return std::forward_like<As>( std::forward<T>( param ) );
      }
#else
      template<typename E>
      PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CNSTEVAL typename std::underlying_type<E>::type as_val(
        E enum_val ) noexcept
      {
        return static_cast<typename std::underlying_type<E>::type>( enum_val );
      }

      template<typename As, typename T>
      constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<std::is_lvalue_reference<As&&>::value
                                  && std::is_const<typename std::remove_reference<As>::type>::value,
                                decltype( as_const( param ) )>::type
      {
        return as_const( param );
      }
      template<typename As, typename T>
      constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<std::is_lvalue_reference<As&&>::value
                                  && !std::is_const<typename std::remove_reference<As>::type>::value,
                                T&>::type
      {
        return static_cast<T&>( param );
      }
      template<typename As, typename T>
      constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<!std::is_lvalue_reference<As&&>::value
                                  && std::is_const<typename std::remove_reference<As>::type>::value,
                                decltype( std::move( as_const( param ) ) )>::type
      {
        return std::move( as_const( param ) );
      }
      template<typename As, typename T>
      constexpr auto forward_as( T&& param ) noexcept ->
        typename std::enable_if<!std::is_lvalue_reference<As&&>::value
                                  && !std::is_const<typename std::remove_reference<As>::type>::value,
                                decltype( std::move( param ) )>::type
      {
        return std::move( param );
      }
#endif
    } // namespace utils

    namespace traits {
#if PGBAR__CXX17
      template<typename Ret, typename Fn, typename... Args>
      using InvocableTo = std::is_invocable_r<Ret, Fn, Args...>;
#else
      template<typename Ret, typename Fn, typename... Args>
      struct InvocableTo {
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
