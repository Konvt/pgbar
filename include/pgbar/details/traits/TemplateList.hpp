#ifndef __PGBAR_TEMPLATELIST
#define __PGBAR_TEMPLATELIST

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

      // Check whether a TemplateList contains given template `T`.
      template<typename TmpList, template<typename...> class T>
      struct Include;
      template<template<typename...> class T>
      struct Include<TemplateList<>, T> : std::false_type {};
      template<template<typename...> class Head,
               template<typename...> class... Tail,
               template<typename...> class T>
      struct Include<TemplateList<Head, Tail...>, T> {
      private:
        template<bool Cond, typename RestList>
        struct _Select;
        template<typename RestList>
        struct _Select<true, RestList> : std::true_type {};
        template<typename RestList>
        struct _Select<false, RestList> : Include<TemplateList<Tail...>, T> {};

      public:
        static constexpr bool value = _Select<Equal<Head, T>::value, TemplateList<Tail...>>::value;
      };

      // Insert a new template type into the head of the TemplateList.
      template<typename TmpList, template<typename...> class T>
      struct Prepend;
      // Get the result of the template `Prepend`.
      template<typename TmpList, template<typename...> class T>
      using Prepend_t = typename Prepend<TmpList, T>::type;

      template<template<typename...> class... Ts, template<typename...> class T>
      struct Prepend<TemplateList<Ts...>, T> {
        using type = TemplateList<T, Ts...>;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
