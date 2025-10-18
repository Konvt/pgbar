#ifndef PGBAR__CONCURRENT_UTIL
#define PGBAR__CONCURRENT_UTIL

#include "../types/Types.hpp"
#include <chrono>
#include <thread>

namespace pgbar {
  namespace _details {
    namespace concurrent {
      // Wait for pred to be true.
      template<typename F>
      PGBAR__INLINE_FN void spin_wait( F&& pred, types::Size threshold ) noexcept( noexcept( pred() ) )
      {
        for ( types::Size cnt = 0; !pred(); ) {
          if ( cnt < threshold )
            ++cnt;
          else
            std::this_thread::yield();
        }
      }
      template<typename F>
      PGBAR__INLINE_FN void spin_wait( F&& pred ) noexcept( noexcept( pred() ) )
      {
        spin_wait( std::forward<F>( pred ), 128 );
      }

      template<typename F, typename Rep, typename Period>
      PGBAR__INLINE_FN bool spin_wait_for( F&& pred,
                                           types::Size threshold,
                                           const std::chrono::duration<Rep, Period>& timeout )
        noexcept( noexcept( pred() ) )
      {
        const auto start = std::chrono::steady_clock::now();
        for ( types::Size cnt; !pred(); ) {
          const auto elapsed = std::chrono::steady_clock::now() - start;
          if ( elapsed >= timeout )
            return false;
          else if ( cnt < threshold ) {
            ++cnt;
            continue;
          }
          std::this_thread::yield();
        }
        return true;
      }
      template<typename F, typename Rep, typename Period>
      PGBAR__INLINE_FN bool spin_wait_for( F&& pred, const std::chrono::duration<Rep, Period>& timeout )
        noexcept( noexcept( pred() ) )
      {
        return spin_wait_for( std::forward<F>( pred ), 128, timeout );
      }
    } // namespace concurrent
  } // namespace _details
} // namespace pgbar

#endif
