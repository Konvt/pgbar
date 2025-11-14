#ifndef PGBAR__TRAITS_UTIL
#define PGBAR__TRAITS_UTIL

#include "Backport.hpp"
#include "TypeList.hpp"

namespace pgbar {
  namespace _details {
    namespace traits {
      template<template<typename...> class Template, typename T, types::Size N>
      struct FillWith {
      private:
        template<typename List>
        struct Helper;
        template<typename... Elements>
        struct Helper<TypeList<Elements...>> {
          using type = Template<Elements...>;
        };

      public:
        using type = typename Helper<TpFill_t<T, N>>::type;
      };
      template<template<typename...> class Template, typename T, types::Size N>
      using FillWith_t = typename FillWith<Template, T, N>::type;

      template<typename T>
      struct is_pointer_like {
        template<typename>
        static constexpr std::false_type check( ... );
        template<typename U>
        static constexpr typename std::enable_if<
          Not<AnyOf<std::is_void<decltype( *std::declval<U&>() )>,
                    std::is_void<decltype( std::declval<U&>().operator->() )>,
                    std::is_void<decltype( static_cast<bool>( std::declval<U&>() ) )>>>::value,
          std::true_type>::type
          check( int );

      public:
        static constexpr bool value = AllOf<Not<std::is_reference<T>>, decltype( check<T>( 0 ) )>::value;
      };
      template<typename P>
      struct is_pointer_like<P*> : std::true_type {};

      template<typename T>
      struct Pointee {
        using type = typename std::remove_reference<decltype( *std::declval<T&>() )>::type;
      };
      template<typename T>
      using Pointee_t = typename Pointee<T>::type;

      template<typename P>
      struct Pointee<P*> {
        using type = P;
      };

      // Check whether the type Instance is an instantiated version of Tmp or whether it inherits from Tmp
      // itself.
      template<typename Instance, template<typename...> class Tmp>
      struct is_instance_of {
      private:
        template<typename... Args>
        static constexpr std::true_type check( const Tmp<Args...>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<Instance>>,
                decltype( check( std::declval<typename std::remove_cv<Instance>::type>() ) )>::value;
      };
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
