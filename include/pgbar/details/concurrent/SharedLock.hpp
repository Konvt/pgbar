#ifndef PGBAR__SHAREDLOCK
#define PGBAR__SHAREDLOCK

#include "../core/Core.hpp"
#if PGBAR__CXX14
# include <shared_mutex>
#else
# include <mutex>
#endif

namespace pgbar {
  namespace _details {
    namespace concurrent {
#if PGBAR__CXX14
      template<typename Mtx>
      using SharedLock = std::shared_lock<Mtx>;
#else
      template<typename Mtx>
      class SharedLock final {
        using Self = SharedLock;

        Mtx& mtx_;

      public:
        using mutex_type = Mtx;

        SharedLock( mutex_type& m ) noexcept( noexcept( mtx_.lock_shared() ) ) : mtx_ { m }
        {
          mtx_.lock_shared();
        }
        // Internal component, assume that the lock object always holds a mutex when destructing.
        SharedLock( mutex_type& m, std::defer_lock_t ) noexcept : mtx_ { m } {}
        SharedLock( mutex_type& m, std::adopt_lock_t ) noexcept : mtx_ { m } {}
        ~SharedLock() noexcept { mtx_.unlock_shared(); }

        PGBAR__FORCEINLINE void lock() & noexcept { mtx_.lock_shared(); }
        PGBAR__FORCEINLINE bool try_lock() & noexcept { return mtx_.try_lock_shared(); }
        PGBAR__FORCEINLINE void unlock() & noexcept { mtx_.unlock_shared(); }
      };
#endif
    }
  } // namespace _details
} // namespace pgbar
#endif
