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

      // Check whether the type Instance is an instantiated version of Tmp or whether it inherits from Tmp
      // itself.
      template<typename Instance, template<typename...> class Tmp>
      struct InstanceOf {
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
