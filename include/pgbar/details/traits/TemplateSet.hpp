#ifndef __PGBAR_TEMPLATESET
#define __PGBAR_TEMPLATESET

#include "TemplateList.hpp"

namespace pgbar {
  namespace __details {
    namespace traits {
      template<template<typename...> class... Ts>
      struct TemplateSet : TemplateList<Ts>... {};

      template<typename TmpSet, template<typename...> class T>
      struct Contain;
      template<template<typename...> class... Ts, template<typename...> class T>
      struct Contain<TemplateSet<Ts...>, T> : std::is_base_of<TemplateList<T>, TemplateSet<Ts...>> {};

      template<typename TmpSet, template<typename...> class T>
      struct Extend;
      template<typename TmpSet, template<typename...> class T>
      using Extend_t = typename Extend<TmpSet, T>::type;

      template<template<typename...> class... Ts, template<typename...> class T>
      struct Extend<TemplateSet<Ts...>, T> {
      private:
        template<bool Cond, template<typename...> class NewOne>
        struct _Select;
        template<template<typename...> class NewOne>
        struct _Select<true, NewOne> {
          using type = TemplateSet<Ts...>;
        };
        template<template<typename...> class NewOne>
        struct _Select<false, NewOne> {
          using type = TemplateSet<Ts..., NewOne>;
        };

      public:
        using type = typename _Select<Contain<TemplateSet<Ts...>, T>::value, T>::type;
      };
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
