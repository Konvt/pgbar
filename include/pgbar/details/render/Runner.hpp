#ifndef __PGBAR_RUNNER
#define __PGBAR_RUNNER

#include "../concurrent/ExceptionBox.hpp"
#include "../concurrent/Util.hpp"
#include "../console/TermContext.hpp"
#include "../wrappers/Backport.hpp"
#include <condition_variable>
#include <thread>

namespace pgbar {
  namespace __details {
    namespace render {
      template<Channel Tag>
      class Runner final {
        using Self = Runner;

        enum class State : std::uint8_t { Dormant, Suspend, Active, Dead };
        std::atomic<State> state_;

        std::thread td_;
        concurrent::ExceptionBox box_;
        mutable std::condition_variable cond_var_;
        mutable std::mutex mtx_;
        mutable concurrent::SharedMutex rw_mtx_;

        wrappers::UniqueFunction<void()> task_;

        void launch() & noexcept( false )
        {
          console::TermContext<Tag>::itself().virtual_term();

          __PGBAR_ASSERT( td_.get_id() == std::thread::id() );
          state_.store( State::Dormant, std::memory_order_release );
          try {
            td_ = std::thread( [this]() {
              try {
                while ( state_.load( std::memory_order_acquire ) != State::Dead ) {
                  switch ( state_.load( std::memory_order_acquire ) ) {
                  case State::Dormant: __PGBAR_FALLTHROUGH;
                  case State::Suspend: {
                    std::unique_lock<std::mutex> lock { mtx_ };
                    auto expected = State::Suspend;
                    state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
                    cond_var_.wait( lock, [this]() noexcept {
                      return state_.load( std::memory_order_acquire ) != State::Dormant;
                    } );
                  } break;
                  case State::Active: {
                    task_();
                  } break;
                  default: return;
                  }
                }
              } catch ( ... ) {
                if ( !box_.try_store( std::current_exception() ) ) {
                  state_.store( State::Dead, std::memory_order_release );
                  throw;
                }
              }
            } );
          } catch ( ... ) {
            state_.store( State::Dead, std::memory_order_release );
            throw;
          }
        }

        // Since the control flow of the child thread has been completely handed over to task_,
        // this function does not guarantee that the child thread can be closed immediately after being
        // called.
        void shutdown() noexcept
        {
          state_.store( State::Dead, std::memory_order_release );
          {
            std::lock_guard<std::mutex> lock { mtx_ };
            cond_var_.notify_all();
          }
          if ( td_.joinable() )
            td_.join();
          td_ = std::thread();
        }

        Runner() noexcept : state_ { State::Dead } {}

      public:
        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }

        Runner( const Self& )            = delete;
        Self& operator=( const Self& ) & = delete;
        ~Runner() noexcept { shutdown(); }

        void suspend() noexcept
        {
          auto expected = State::Active;
          if ( state_.compare_exchange_strong( expected, State::Suspend, std::memory_order_release ) ) {
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Suspend; } );
            std::lock_guard<std::mutex> lock { mtx_ };
            // Ensure that the background thread is truely suspended.

            // Discard the current cycle's captured exception.
            // Reason: Delaying the exception and throwing it on the next `activate()` call is meaningless,
            // as it refers to a previous cycle's failure and will no longer be relevant in the new cycle.
            box_.clear();
          }
        }
        void activate() & noexcept( false )
        {
          {
            std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
            if ( td_.get_id() == std::thread::id() )
              launch();
            else if ( state_.load( std::memory_order_acquire ) == State::Dead ) {
              shutdown();
              launch();
            }
          }

          // The operations below are all thread safe without locking.
          // Also, only one thread needs to be able to execute concurrently.
          box_.rethrow();
          __PGBAR_ASSERT( state_ != State::Dead );
          __PGBAR_ASSERT( task_ != nullptr );
          auto expected = State::Dormant;
          if ( state_.compare_exchange_strong( expected, State::Active, std::memory_order_release ) ) {
            std::lock_guard<std::mutex> lock { mtx_ };
            cond_var_.notify_one();
          }
        }

        void appoint() noexcept
        {
          suspend();
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          task_ = nullptr;
        }
        __PGBAR_NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr )
            return false;
          // Under normal circumstances, `online() == false` implies that `task_ == nullptr`.
          __PGBAR_ASSERT( online() == false );
          task_.swap( task );
          return true;
        }

        void drop() noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          shutdown();
        }
        void throw_if() noexcept( false ) { box_.rethrow(); }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ == nullptr;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool online() const noexcept
        {
          return state_.load( std::memory_order_acquire ) == State::Active;
        }
      };
    } // namespace render
  } // namespace __details
} // namespace pgbar

#endif
