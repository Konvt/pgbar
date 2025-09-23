#ifndef __PGBAR_TEMPLATELIST
#define __PGBAR_TEMPLATELIST

namespace pgbar {
  namespace __details {
    namespace traits {
      // A lightweight tuple type that stores multiple template class types.
      template<template<typename...> class... Ts>
      struct TemplateList {};
    } // namespace traits
  } // namespace __details
} // namespace pgbar

#endif
