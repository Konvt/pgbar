#ifndef PGBAR__RUNNER
#define PGBAR__RUNNER

#include "../concurrent/ExceptionBox.hpp"
#include "../concurrent/Util.hpp"
#include "../console/TermContext.hpp"
#include "../wrappers/Backport.hpp"
#include <condition_variable>
#include <thread>

namespace pgbar {
  namespace _details {
    namespace render {
      // A lightweight, single-task asynchronous thread executor that executes one task at a time.
      // It provides basic activate/suspend control but does not guarantee strong synchronization
      // between task execution and state transitions.
      template<Channel Tag>
      class AsyncSlot final {
        using Self = AsyncSlot;

        std::thread runner_;
        wrappers::UniqueFunction<void()> task_;
        concurrent::ExceptionBox box_;

        mutable std::condition_variable cond_var_;
        mutable concurrent::SharedMutex res_mtx_;
        mutable std::mutex sched_mtx_;

        enum class State : std::uint8_t { Dormant, Active, Suspend, Dead };
        std::atomic<State> state_;

        void launch() & noexcept( false )
        {
          console::TermContext<Tag>::itself().virtual_term();

          PGBAR__ASSERT( runner_.get_id() == std::thread::id() );
          try {
            runner_ = std::thread( [this]() {
              try {
                while ( state_.load( std::memory_order_acquire ) != State::Dead ) {
                  switch ( state_.load( std::memory_order_acquire ) ) {
                  case State::Dormant: {
                    std::unique_lock<std::mutex> lock { sched_mtx_ };
                    cond_var_.wait( lock, [this]() noexcept {
                      return state_.load( std::memory_order_acquire ) != State::Dormant;
                    } );
                  } break;
                  case State::Active: {
                    concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
                    task_();
                  } break;
                  case State::Suspend: {
                    // An intermediate state, ensure that the background thread does not access task_ again
                    // when it is suspended; otherwise, a race condition on task_ may occur in dismiss().
                    auto expected = State::Suspend;
                    state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
                  } break;
                  default: return;
                  }
                }
              } catch ( ... ) {
                if ( !box_.try_store( std::current_exception() ) ) {
                  state_.store( State::Dead, std::memory_order_release );
                  throw;
                } else
                  state_.store( State::Dormant, std::memory_order_release );
              }
            } );
          } catch ( ... ) {
            state_.store( State::Dead, std::memory_order_release );
            throw;
          }
          auto expected = State::Dead;
          state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
        }

        // Since the control flow of the child thread has been completely handed over to task_,
        // this function does not guarantee that the child thread can be closed immediately after being
        // called.
        void shutdown() noexcept
        {
          state_.store( State::Dead, std::memory_order_release );
          {
            std::lock_guard<std::mutex> lock { sched_mtx_ };
            cond_var_.notify_all();
          }
          if ( runner_.joinable() )
            runner_.join();
          runner_ = std::thread();
        }

        AsyncSlot() noexcept
          : runner_ {}, task_ {}, box_ {}, cond_var_ {}, res_mtx_ {}, sched_mtx_ {}, state_ { State::Dead }
        {}

      public:
        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }

        AsyncSlot( const Self& )         = delete;
        Self& operator=( const Self& ) & = delete;
        ~AsyncSlot() noexcept { shutdown(); }

        // Only switch the state quantity,
        // it is not guaranteed that the background thread is indeed suspended when the method returns.
        void suspend() noexcept
        {
          auto expected = State::Active;
          if ( state_.compare_exchange_strong( expected, State::Suspend, std::memory_order_release ) ) {
            // Discard the current cycle's captured exception.
            // Reason: Delaying the exception and throwing it on the next `activate()` call is meaningless,
            // as it refers to a previous cycle's failure and will no longer be relevant in the new cycle.
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Suspend; } );
            box_.clear();
          }
        }
        // Only switch the state quantity,
        // it is not guaranteed that the background thread is indeed activated when the method returns.
        void activate() & noexcept( false )
        {
          if ( !online() ) {
            std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
            if ( runner_.get_id() == std::thread::id() )
              launch();
            else if ( state_.load( std::memory_order_acquire ) == State::Dead ) {
              shutdown();
              launch();
            }
          }

          // The operations below are all thread safe without locking.
          box_.rethrow();
          PGBAR__ASSERT( state_ != State::Dead );
          PGBAR__ASSERT( task_ != nullptr );
          auto try_update = [this]( State expected ) noexcept {
            return state_.compare_exchange_strong( expected, State::Active, std::memory_order_release );
          };
          if ( try_update( State::Dormant ) || try_update( State::Suspend ) ) {
            std::lock_guard<std::mutex> lock { sched_mtx_ };
            cond_var_.notify_one();
          }
        }

        void dismiss() noexcept
        {
          suspend();
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          task_ = nullptr;
        }
        PGBAR__NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          if ( task_ != nullptr )
            return false;
          PGBAR__ASSERT( online() == false );
          task_ = std::move( task );
          return true;
        }

        void drop() noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          shutdown();
        }
        void throw_if() noexcept( false ) { box_.rethrow(); }
        PGBAR__NODISCARD bool aborted() const noexcept { return !box_.empty(); }

        PGBAR__NODISCARD PGBAR__INLINE_FN bool empty() const noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          return task_ == nullptr;
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN bool online() const noexcept
        {
          return state_.load( std::memory_order_acquire ) == State::Active;
        }
      };
    } // namespace render
  } // namespace _details
} // namespace pgbar

#endif
