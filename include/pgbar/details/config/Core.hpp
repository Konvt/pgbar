#ifndef __PGBAR_CORE
#define __PGBAR_CORE

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

#if defined( _MSC_VER ) && defined( _MSVC_LANG ) // for msvc
# define __PGBAR_CC_STD _MSVC_LANG
#else
# define __PGBAR_CC_STD __cplusplus
#endif

#if __PGBAR_CC_STD >= 202302L
# define __PGBAR_CXX23          1
# define __PGBAR_CXX23_CNSTXPR  constexpr
# define __PGBAR_UNREACHABLE    std::unreachable()
# define __PGBAR_ASSUME( expr ) [[assume( expr )]]
# include <utility> // for std::unreachable
#else
# define __PGBAR_CXX23 0
# define __PGBAR_CXX23_CNSTXPR

# if defined( __GNUC__ )
#  define __PGBAR_UNREACHABLE __builtin_unreachable()
#  define __PGBAR_ASSUME( expr ) \
    do {                         \
      if ( expr ) {              \
      } else {                   \
        __PGBAR_UNREACHABLE;     \
      }                          \
    } while ( false )
# elif defined( __clang__ )
#  define __PGBAR_UNREACHABLE __builtin_unreachable()
#  if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 6 )
#   define __PGBAR_ASSUME( expr ) __builtin_assume( expr )
#  endif
# elif defined( _MSC_VER )
#  define __PGBAR_ASSUME( expr ) __assume( expr )
# endif

# ifndef __PGBAR_UNREACHABLE
#  define __PGBAR_UNREACHABLE __PGBAR_TRUST( 0 )
# endif
# ifndef __PGBAR_ASSUME
#  define __PGBAR_ASSUME( _ ) __PGBAR_ASSERT( 0 )
# endif
#endif
#if __PGBAR_CC_STD >= 202002L
# define __PGBAR_CXX20         1
# define __PGBAR_CXX20_CNSTXPR constexpr
# define __PGBAR_CNSTEVAL      consteval
# define __PGBAR_UNLIKELY      [[unlikely]]
#else
# define __PGBAR_CXX20 0
# define __PGBAR_CXX20_CNSTXPR
# define __PGBAR_CNSTEVAL constexpr
# define __PGBAR_UNLIKELY
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

# if defined( __GNUC__ )
#  define __PGBAR_NODISCARD __attribute__( ( warn_unused_result ) )
#  if __GNUC__ >= 7
#   define __PGBAR_FALLTHROUGH __attribute__( ( fallthrough ) )
#  endif
# elif defined( __clang__ )
#  if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 5 )
#   define __PGBAR_FALLTHROUGH [[clang::fallthrough]]
#  endif
#  if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 9 )
#   define __PGBAR_NODISCARD __attribute__( ( warn_unused_result ) )
#  endif
# elif defined( _MSC_VER )
#  define __PGBAR_NODISCARD _Check_return_
# endif

# ifndef __PGBAR_FALLTHROUGH
#  define __PGBAR_FALLTHROUGH ( (void)0 )
# endif
# ifndef __PGBAR_NODISCARD
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

#if defined( __GNUC__ ) || defined( __clang__ )
// pgbar does not detect the differences in compiler versions
// which released before the publication of the C++11 standard.
# define __PGBAR_INLINE_FN __attribute__( ( always_inline ) ) inline
#elif defined( _MSC_VER )
// For msvc, it is a proprietary compiler implementation,
// and I believe it supports this builtin.
# define __PGBAR_INLINE_FN __forceinline
#else
# define __PGBAR_INLINE_FN inline
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
# define __PGBAR_ASSERT( _ ) ( (void)0 )
#endif

// For assertion conditions without side effects.
#define __PGBAR_TRUST( expr ) \
  do {                        \
    __PGBAR_ASSERT( expr );   \
    __PGBAR_ASSUME( expr );   \
  } while ( false )

// Pack multiple macro parameters into a single one.
#define __PGBAR_WRAP( ... ) __VA_ARGS__

#include <cstdint>

namespace pgbar {
  // A enum that specifies the type of the output stream.
  enum class Channel : int { Stdout = 1, Stderr = 2 };
  enum class Policy : std::uint8_t { Async, Sync };
  enum class Region : std::uint8_t { Fixed, Relative };
} // namespace pgbar

#endif
