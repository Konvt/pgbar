#ifndef PGBAR__CORE
#define PGBAR__CORE

#if defined( _WIN32 ) || defined( _WIN64 )
# define PGBAR__WIN     1
# define PGBAR__UNIX    0
# define PGBAR__UNKNOWN 0
#elif defined( __unix__ )
# define PGBAR__WIN     0
# define PGBAR__UNIX    1
# define PGBAR__UNKNOWN 0
#else
# define PGBAR__WIN     0
# define PGBAR__UNIX    0
# define PGBAR__UNKNOWN 1
#endif

#if defined( _MSC_VER ) && defined( _MSVC_LANG ) // for msvc
# define PGBAR__CC_STD _MSVC_LANG
#else
# define PGBAR__CC_STD __cplusplus
#endif

#if PGBAR__CC_STD >= 202302L
# define PGBAR__CXX23          1
# define PGBAR__CXX23_CNSTXPR  constexpr
# define PGBAR__UNREACHABLE    std::unreachable()
# define PGBAR__ASSUME( expr ) [[assume( expr )]]
# include <utility> // for std::unreachable
#else
# define PGBAR__CXX23 0
# define PGBAR__CXX23_CNSTXPR

# if defined( __GNUC__ )
#  define PGBAR__UNREACHABLE __builtin_unreachable()
#  define PGBAR__ASSUME( expr ) \
    do {                        \
      if ( expr ) {             \
      } else {                  \
        PGBAR__UNREACHABLE;     \
      }                         \
    } while ( false )
# elif defined( __clang__ )
#  define PGBAR__UNREACHABLE __builtin_unreachable()
#  if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 6 )
#   define PGBAR__ASSUME( expr ) __builtin_assume( expr )
#  endif
# elif defined( _MSC_VER )
#  define PGBAR__ASSUME( expr ) __assume( expr )
# endif

# ifndef PGBAR__UNREACHABLE
#  define PGBAR__UNREACHABLE PGBAR__TRUST( 0 )
# endif
# ifndef PGBAR__ASSUME
#  define PGBAR__ASSUME( _ ) PGBAR__ASSERT( 0 )
# endif
#endif
#if PGBAR__CC_STD >= 202002L
# define PGBAR__CXX20         1
# define PGBAR__CXX20_CNSTXPR constexpr
# define PGBAR__CNSTEVAL      consteval
# define PGBAR__UNLIKELY      [[unlikely]]
#else
# define PGBAR__CXX20 0
# define PGBAR__CXX20_CNSTXPR
# define PGBAR__CNSTEVAL constexpr
# define PGBAR__UNLIKELY
#endif
#if PGBAR__CC_STD >= 201703L
# define PGBAR__CXX17         1
# define PGBAR__CXX17_CNSTXPR constexpr
# define PGBAR__CXX17_INLINE  inline
# define PGBAR__FALLTHROUGH   [[fallthrough]]
# define PGBAR__NODISCARD     [[nodiscard]]
#else
# define PGBAR__CXX17 0
# define PGBAR__CXX17_CNSTXPR
# define PGBAR__CXX17_INLINE

# if defined( __GNUC__ )
#  define PGBAR__NODISCARD __attribute__( ( warn_unused_result ) )
#  if __GNUC__ >= 7
#   define PGBAR__FALLTHROUGH __attribute__( ( fallthrough ) )
#  endif
# elif defined( __clang__ )
#  if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 5 )
#   define PGBAR__FALLTHROUGH [[clang::fallthrough]]
#  endif
#  if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 9 )
#   define PGBAR__NODISCARD __attribute__( ( warn_unused_result ) )
#  endif
# elif defined( _MSC_VER )
#  define PGBAR__NODISCARD _Check_return_
# endif

# ifndef PGBAR__FALLTHROUGH
#  define PGBAR__FALLTHROUGH ( (void)0 )
# endif
# ifndef PGBAR__NODISCARD
#  define PGBAR__NODISCARD
# endif
#endif
#if PGBAR__CC_STD >= 201402L
# define PGBAR__CXX14         1
# define PGBAR__CXX14_CNSTXPR constexpr
#else
# define PGBAR__CXX14 0
# define PGBAR__CXX14_CNSTXPR
#endif
#if PGBAR__CC_STD >= 201103L
# define PGBAR__CXX11 1
#else
# error "The library 'pgbar' requires C++11"
#endif

#if defined( __GNUC__ ) || defined( __clang__ )
// pgbar does not detect the differences in compiler versions
// which released before the publication of the C++11 standard.
# define PGBAR__INLINE_FN __attribute__( ( always_inline ) ) inline
# define PGBAR__NOINLINE  __attribute__( ( noinline ) )
#elif defined( _MSC_VER )
// For msvc, it is a proprietary compiler implementation,
// and I believe it supports this builtin.
# define PGBAR__INLINE_FN __forceinline
# define PGBAR__NOINLINE  __declspec( noinline )
#else
# define PGBAR__INLINE_FN inline
# define PGBAR__NOINLINE
#endif

#ifdef __has_builtin
# define PGBAR__BUILTIN( builtin ) __has_builtin( builtin )
#else
# define PGBAR__BUILTIN( _ ) 0
#endif

#ifdef PGBAR_DEBUG
# include <cassert>
# define PGBAR__ASSERT( expr ) assert( expr )
# undef PGBAR__INLINE_FN
# define PGBAR__INLINE_FN
#else
# define PGBAR__ASSERT( _ ) ( (void)0 )
#endif

// For assertion conditions without side effects.
#define PGBAR__TRUST( expr ) \
  do {                       \
    PGBAR__ASSERT( expr );   \
    PGBAR__ASSUME( expr );   \
  } while ( false )

// Pack multiple macro parameters into a single one.
#define PGBAR__WRAP( ... ) __VA_ARGS__

#include <cstdint>

namespace pgbar {
  // A enum that specifies the type of the output stream.
  enum class Channel : int { Stdout = 1, Stderr = 2 };
  enum class Policy : std::uint8_t { Async, Sync };
  enum class Region : std::uint8_t { Fixed, Relative };
} // namespace pgbar

#endif
