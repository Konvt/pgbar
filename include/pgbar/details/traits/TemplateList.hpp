#ifndef PGBAR__TEMPLATELIST
#define PGBAR__TEMPLATELIST

namespace pgbar {
  namespace _details {
    namespace traits {
      // A lightweight tuple type that stores multiple template class types.
      template<template<typename...> class... Ts>
      struct TemplateList {};
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
