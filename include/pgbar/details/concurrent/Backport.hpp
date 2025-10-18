#ifndef PGBAR__CONCURRENT_BACKPORT
#define PGBAR__CONCURRENT_BACKPORT

#include "../core/Core.hpp"
#if !PGBAR__CXX17
# include <atomic>
# include <mutex>
#endif
#if PGBAR__CXX14
# include <shared_mutex>
#endif

namespace pgbar {
  namespace _details {
    namespace concurrent {
#if PGBAR__CXX17
      using SharedMutex = std::shared_mutex;
#else
      // A simple `Shared Mutex` implementation for any C++ version.
      class SharedMutex final {
        using Self = SharedMutex;

      protected:
        std::atomic<std::uint64_t> num_readers_;
        std::mutex writer_mtx_;
        /**
         * Although the `lock()` and `unlock()` functions of `std::mutex`
         * in the standard library are not marked as `noexcept`,
         * in practice, it is very rare for exceptions to be thrown,
         * and usually indicate a bug in the program itself.
         * Therefore, it is assumed here that `std::mutex` is `noexcept`.

         * For more details, see: https://stackoverflow.com/questions/17551256
         */

      public:
        SharedMutex( const Self& )       = delete;
        Self& operator=( const Self& ) & = delete;

        SharedMutex() noexcept : num_readers_ { 0 } {}
        ~SharedMutex() = default;

        PGBAR__INLINE_FN void lock() & noexcept
        {
          while ( true ) {
            while ( num_readers_.load( std::memory_order_acquire ) != 0 )
              std::this_thread::yield();

            writer_mtx_.lock();
            if ( num_readers_.load( std::memory_order_acquire ) == 0 )
              break;
            else // unlock it and wait for readers to finish
              writer_mtx_.unlock();
          }
        }
        PGBAR__INLINE_FN bool try_lock() & noexcept
        {
          if ( num_readers_.load( std::memory_order_acquire ) == 0 && writer_mtx_.try_lock() ) {
            if ( num_readers_.load( std::memory_order_acquire ) == 0 )
              return true;
            else
              writer_mtx_.unlock();
          }
          return false;
        }
        PGBAR__INLINE_FN void unlock() & noexcept { writer_mtx_.unlock(); }

        void lock_shared() & noexcept
        {
          writer_mtx_.lock();

          num_readers_.fetch_add( 1, std::memory_order_release );
          PGBAR__ASSERT( num_readers_ > 0 ); // overflow checking

          writer_mtx_.unlock();
        }
        PGBAR__INLINE_FN bool try_lock_shared() & noexcept
        {
          if ( writer_mtx_.try_lock() ) {
            num_readers_.fetch_add( 1, std::memory_order_release );
            PGBAR__ASSERT( num_readers_ > 0 );
            writer_mtx_.unlock();
            return true;
          }
          return false;
        }
        PGBAR__INLINE_FN void unlock_shared() & noexcept
        {
          PGBAR__ASSERT( num_readers_ > 0 ); // underflow checking
          num_readers_.fetch_sub( 1, std::memory_order_release );
        }
      };
#endif

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

        PGBAR__INLINE_FN void lock() & noexcept { mtx_.lock_shared(); }
        PGBAR__INLINE_FN bool try_lock() & noexcept { return mtx_.try_lock_shared(); }
        PGBAR__INLINE_FN void unlock() & noexcept { mtx_.unlock_shared(); }
      };
#endif
    } // namespace concurrent
  } // namespace _details
} // namespace pgbar

#endif
