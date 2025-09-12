#ifndef __PGBAR_UTILS_BACKPORT
#define __PGBAR_UTILS_BACKPORT

#include "../config/Core.hpp"
#include <memory>
#include <utility>
#if __PGBAR_CXX17
# include <functional>
# include <new>
#else
# include "../traits/Backport.hpp"
# include "../traits/Util.hpp"
# include "../utils/Backport.hpp"
#endif

namespace pgbar {
  namespace __details {
    namespace utils {
      // Available only for buffers that use placement new.
      template<typename To, typename From>
      __PGBAR_INLINE_FN constexpr To* launder_as( From* src ) noexcept
      {
#if __PGBAR_CXX17
        return std::launder( reinterpret_cast<To*>( src ) );
#else
        /**
         * Before c++17 there was no way to prevent compilers from over-optimizing different types
         * of pointers with rules like strict aliases,
         * so we should trust reinerpret_cast to give us what we want.
         */
        return reinterpret_cast<To*>( src );
#endif
      }

#if __PGBAR_CXX14
      template<typename T, typename... Args>
      __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR auto make_unique( Args&&... args )
      {
        return std::make_unique<T>( std::forward<Args>( args )... );
      }
#else
      // picking up the standard's mess...
      template<typename T, typename... Args>
      __PGBAR_INLINE_FN __PGBAR_CXX23_CNSTXPR std::unique_ptr<T> make_unique( Args&&... args )
      {
        return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
      }
#endif

#if __PGBAR_CXX17
      template<typename Fn, typename... Args>
      __PGBAR_INLINE_FN constexpr decltype( auto ) invoke( Fn&& fn, Args&&... args )
        noexcept( std::is_nothrow_invocable_v<Fn, Args...> )
      {
        return std::invoke( std::forward<Fn>( fn ), std::forward<Args>( args )... );
      }
#else
      template<typename C, typename MemFn, typename Object, typename... Args>
      __PGBAR_INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
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
      __PGBAR_INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
        noexcept( noexcept( ( object.get().*method )( std::forward<Args>( args )... ) ) ) ->
        typename std::enable_if<
          traits::AllOf<std::is_member_function_pointer<MemFn C::*>,
                        traits::InstanceOf<typename std::decay<Object>::type, std::reference_wrapper>>::value,
          decltype( ( object.get().*method )( std::forward<Args>( args )... ) )>::type
      {
        return ( object.get().*method )( std::forward<Args>( args )... );
      }
      template<typename C, typename MemFn, typename Object, typename... Args>
      __PGBAR_INLINE_FN constexpr auto invoke( MemFn C::* method, Object&& object, Args&&... args )
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
      __PGBAR_INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::AnyOf<std::is_base_of<C, typename std::decay<Object>::type>,
                                      std::is_same<C, typename std::decay<Object>::type>>>::value,
          decltype( std::forward<Object>( object ).*member )>::type
      {
        return std::forward<Object>( object ).*member;
      }
      template<typename C, typename MemObj, typename Object>
      __PGBAR_INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object ) noexcept ->
        typename std::enable_if<
          traits::AllOf<std::is_member_object_pointer<MemObj C::*>,
                        traits::InstanceOf<typename std::decay<Object>::type, std::reference_wrapper>>::value,
          decltype( object.get().*member )>::type
      {
        return object.get().*member;
      }
      template<typename C, typename MemObj, typename Object>
      __PGBAR_INLINE_FN constexpr auto invoke( MemObj C::* member, Object&& object )
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
      __PGBAR_INLINE_FN constexpr auto invoke( Fn&& fn, Args&&... args )
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
    } // namespace utils

    namespace traits {
#if __PGBAR_CXX17
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
  } // namespace __details
} // namespace pgbar
#endif
