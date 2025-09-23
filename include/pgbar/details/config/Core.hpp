#ifndef __PGBAR_CORE
#define __PGBAR_CORE

#if defined( _MSC_VER ) && defined( _MSVC_LANG ) // for msvc
# define __PGBAR_CC_STD _MSVC_LANG
#else
# define __PGBAR_CC_STD __cplusplus
#endif

#if defined( _MSC_VER )
# define __PGBAR_INLINE_FN __forceinline
#elif defined( __GNUC__ ) || defined( __clang__ )
# define __PGBAR_INLINE_FN __attribute__( ( always_inline ) ) inline
#else
# define __PGBAR_INLINE_FN inline
#endif

#if defined( _WIN32 ) || defined( _WIN64 )
# define __PGBAR_WIN     1
# define __PGBAR_UNIX    0
# define __PGBAR_UNKNOWN 0
#elif defined( __unix__ )
# define __PGBAR_WIN     0
# define __PGBAR_UNIX    1
# define __PGBAR_UNKNOWN 0
#else
# define __PGBAR_WIN     0
# define __PGBAR_UNIX    0
# define __PGBAR_UNKNOWN 1
#endif

#if __PGBAR_CC_STD >= 202302L
# define __PGBAR_CXX23          1
# define __PGBAR_CXX23_CNSTXPR  constexpr
# define __PGBAR_UNREACHABLE    std::unreachable()
# define __PGBAR_ASSUME( expr ) [[assume( expr )]]
#else
# define __PGBAR_CXX23 0
# define __PGBAR_CXX23_CNSTXPR
# if defined( _MSC_VER )
#  define __PGBAR_UNREACHABLE    __assume( 0 )
#  define __PGBAR_ASSUME( expr ) __assume( expr )
# elif defined( __clang__ )
#  define __PGBAR_UNREACHABLE    __builtin_unreachable()
#  define __PGBAR_ASSUME( expr ) __builtin_assume( expr )
# elif defined( __GNUC__ )
#  define __PGBAR_UNREACHABLE __builtin_unreachable()
#  define __PGBAR_ASSUME( expr ) \
    do {                         \
      if ( expr ) {              \
      } else {                   \
        __builtin_unreachable(); \
      }                          \
    } while ( false )
# else
#  define __PGBAR_UNREACHABLE __PGBAR_TRUST( false )
#  define __PGBAR_ASSUME( expr )
# endif
#endif
#if __PGBAR_CC_STD >= 202002L
# define __PGBAR_CXX20         1
# define __PGBAR_UNLIKELY      [[unlikely]]
# define __PGBAR_CXX20_CNSTXPR constexpr
# define __PGBAR_CNSTEVAL      consteval
#else
# define __PGBAR_CXX20 0
# define __PGBAR_UNLIKELY
# define __PGBAR_CXX20_CNSTXPR
# define __PGBAR_CNSTEVAL constexpr
#endif
#if __PGBAR_CC_STD >= 201703L
# define __PGBAR_CXX17         1
# define __PGBAR_CXX17_CNSTXPR constexpr
# define __PGBAR_CXX17_INLINE  inline
# define __PGBAR_FALLTHROUGH   [[fallthrough]]
# define __PGBAR_NODISCARD     [[nodiscard]]
#else
# define __PGBAR_CXX17 0
# define __PGBAR_CXX17_CNSTXPR
# define __PGBAR_CXX17_INLINE
# define __PGBAR_FALLTHROUGH
# if defined( _MSC_VER )
#  define __PGBAR_NODISCARD _Check_return_
# elif defined( __clang__ ) || defined( __GNUC__ )
#  define __PGBAR_NODISCARD __attribute__( ( warn_unused_result ) )
# else
#  define __PGBAR_NODISCARD
# endif
#endif
#if __PGBAR_CC_STD >= 201402L
# define __PGBAR_CXX14         1
# define __PGBAR_CXX14_CNSTXPR constexpr
#else
# define __PGBAR_CXX14 0
# define __PGBAR_CXX14_CNSTXPR
#endif
#if __PGBAR_CC_STD >= 201103L
# define __PGBAR_CXX11 1
#else
# error "The library 'pgbar' requires C++11"
#endif

#ifdef __has_builtin
# define __PGBAR_BUILTIN( builtin ) __has_builtin( builtin )
#else
# define __PGBAR_BUILTIN( _ ) 0
#endif

#ifdef PGBAR_DEBUG
# include <cassert>
# define __PGBAR_ASSERT( expr ) assert( expr )
# undef __PGBAR_INLINE_FN
# define __PGBAR_INLINE_FN
#else
# define __PGBAR_ASSERT( expr )
#endif

// For assertion conditions without side effects.
#define __PGBAR_TRUST( expr ) \
  do {                        \
    __PGBAR_ASSERT( expr );   \
    __PGBAR_ASSUME( expr );   \
  } while ( false )

// Pack multiple macro parameters into a single one.
#define __PGBAR_PACK( ... ) __VA_ARGS__

#include <cstdint>

namespace pgbar {
  // A enum that specifies the type of the output stream.
  enum class Channel : int { Stdout = 1, Stderr = 2 };
  enum class Policy : std::uint8_t { Async, Sync };
  enum class Region : std::uint8_t { Fixed, Relative };
} // namespace pgbar

#endif
