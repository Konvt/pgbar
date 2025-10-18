#ifndef PGBAR__TEMPLATESET
#define PGBAR__TEMPLATESET

#include "Algorithm.hpp"
#include "TemplateList.hpp"

namespace pgbar {
  namespace _details {
    namespace traits {
      template<template<typename...> class... Ts>
      struct TemplateSet : TemplateList<Ts>... {};

      template<template<typename...> class... Es, template<typename...> class T>
      struct TmpContain<TemplateSet<Es...>, T> : std::is_base_of<TemplateList<T>, TemplateSet<Es...>> {};

      template<template<typename...> class... Es, template<typename...> class T>
      struct TmpAppend<TemplateSet<Es...>, T> {
      private:
        template<bool Cond, template<typename...> class NewOne>
        struct _Select;
        template<template<typename...> class NewOne>
        struct _Select<true, NewOne> {
          using type = TemplateSet<Es...>;
        };
        template<template<typename...> class NewOne>
        struct _Select<false, NewOne> {
          using type = TemplateSet<Es..., NewOne>;
        };

      public:
        using type = typename _Select<TmpContain<TemplateSet<Es...>, T>::value, T>::type;
      };

      template<template<typename...> class... Es, template<typename...> class T>
      struct TmpPrepend<TemplateSet<Es...>, T> {
      private:
        template<bool Cond, template<typename...> class NewOne>
        struct _Select;
        template<template<typename...> class NewOne>
        struct _Select<true, NewOne> {
          using type = TemplateSet<Es...>;
        };
        template<template<typename...> class NewOne>
        struct _Select<false, NewOne> {
          using type = TemplateSet<NewOne, Es...>;
        };

      public:
        using type = typename _Select<TmpContain<TemplateSet<Es...>, T>::value, T>::type;
      };
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
