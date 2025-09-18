#ifndef __PGBAR_TEMPLATELIST
#define __PGBAR_TEMPLATELIST

#include "Algorithm.hpp"
#include "Backport.hpp"
#include <type_traits>

namespace pgbar {
  namespace __details {
    namespace traits {
      // A kind of `std::is_same` that applies to template class types.
      template<template<typename...> class T, template<typename...> class U>
      struct Equal : std::false_type {};
      template<template<typename...> class T>
      struct Equal<T, T> : std::true_type {};

      // A lightweight tuple type that stores multiple template class types.
      template<template<typename...> class... Ts>
      struct TemplateList {};

      template<template<typename...> class T>
      struct TmpContain<TemplateList<>, T> : std::false_type {};
      template<template<typename...> class Head,
               template<typename...> class... Tail,
               template<typename...> class T>
      struct TmpContain<TemplateList<Head, Tail...>, T>
        : AnyOf<Equal<Head, T>, TmpContain<TemplateList<Tail...>, T>> {};

      template<template<typename...> class... Ts, template<typename...> class T>
      struct TmpPrepend<TemplateList<Ts...>, T> {
        using type = TemplateList<T, Ts...>;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
