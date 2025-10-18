#ifndef PGBAR__TYPES
#define PGBAR__TYPES

#include "../core/Core.hpp"
#include <chrono>
#include <string>
#if PGBAR__CXX17
# include <string_view>
#else
# include <type_traits>
#endif

namespace pgbar {
  namespace _details {
    namespace types {
      using Size   = std::size_t;
      using String = std::string;
      using Char   = char;
#if PGBAR__CXX17
      using ROStr  = std::string_view;
      using LitStr = ROStr; // literal strings
#else
      using ROStr  = typename std::add_lvalue_reference<typename std::add_const<String>::type>::type;
      using LitStr = typename std::add_pointer<typename std::add_const<Char>::type>::type;
#endif
#if PGBAR__CXX20
      using LitU8 = std::u8string_view;
#else
      using LitU8 = LitStr;
#endif
      using HexRGB     = std::uint32_t;
      using UCodePoint = char32_t; // Unicode code point
      using Float      = double;
      using TimeUnit   = std::chrono::nanoseconds;
      using Byte       = std::uint8_t;
    } // namespace types
  } // namespace _details
} // namespace pgbar

#endif
