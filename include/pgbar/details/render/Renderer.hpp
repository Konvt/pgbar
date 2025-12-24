#ifndef PGBAR__RENDERER
#define PGBAR__RENDERER

#include "../concurrent/ExceptionBox.hpp"
#include "../concurrent/Util.hpp"
#include "../console/TermContext.hpp"
#include "../utils/ScopeGuard.hpp"
#include "../wrappers/UniqueFunction.hpp"
#include <atomic>
#include <condition_variable>
#include <thread>

namespace pgbar {
  namespace _details {
    namespace render {
      template<Channel Tag>
      class Renderer final {
        using Self = Renderer;

        static std::atomic<TimeGranule> _working_interval;

        std::atomic<std::uint64_t> quota_      = { 0 };
        concurrent::ExceptionBox box_          = {};
        wrappers::UniqueFunction<void()> task_ = {};
        std::thread runner_                    = {};

        mutable std::condition_variable cond_var_ = {};
        mutable concurrent::SharedMutex res_mtx_  = {};
        mutable std::mutex sched_mtx_             = {};

        /***********************************************************
          Dead
            └─ launch() → Dormant
                            ├─ activate<Async>() → Warmup → Loop
                            │                                └ trigger<Async>() → Warmup
                            │
                            ├─ activate<Signal>() → Primed → Pulse
                            │                                  └ trigger<Signal>() → Primed
                            │
                            └─ activate<Sync>() → Idle
                                                   └ trigger<Sync>() → Shot → Idle

          any state
            └─ suspend() → Asleep → Dormant

          any state
            └─ abort() → Halt → Dormant

          any state
            └─ drop() → Dead
        ***********************************************************/
        enum class State : std::uint8_t { Dead, Asleep, Dormant, Warmup, Loop, Primed, Pulse, Shot, Idle };
        std::atomic<State> state_ = { State::Dead };

        void launch() & noexcept( false )
        {
          console::TermContext<Tag>::itself().virtual_term();
          PGBAR__ASSERT( runner_.get_id() == std::thread::id() );
          auto guard = utils::make_scope_fail(
            [this]() noexcept { state_.store( State::Dead, std::memory_order_release ); } );

          runner_       = std::thread( [this]() {
            try {
              for ( auto state = state_.load( std::memory_order_acquire ); state != State::Dead;
                    state      = state_.load( std::memory_order_acquire ) ) {
                switch ( state ) {
                case State::Asleep: {
                  auto expected = State::Asleep;
                  state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
                }
                  PGBAR__FALLTHROUGH;
                case State::Dormant: {
                  std::unique_lock<std::mutex> lock { sched_mtx_ };
                  cond_var_.wait( lock, [this]() noexcept {
                    return state_.load( std::memory_order_acquire ) != State::Dormant;
                  } );
                } break;

                case State::Warmup: {
                  task_();
                  auto expected = State::Warmup;
                  state_.compare_exchange_strong( expected, State::Loop, std::memory_order_release );
                }
                  PGBAR__FALLTHROUGH;
                case State::Loop: {
                  task_();
                  std::this_thread::sleep_for( working_interval() );
                } break;

                case State::Primed: {
                  auto expected = State::Primed;
                  state_.compare_exchange_strong( expected, State::Pulse, std::memory_order_release );
                  task_();
                  quota_.fetch_sub( 1, std::memory_order_release );
                }
                  PGBAR__FALLTHROUGH;
                case State::Pulse: {
                  concurrent::spin_while(
                    [this]() noexcept { return quota_.load( std::memory_order_acquire ) > 0; },
                    [this]() noexcept {
                      std::unique_lock<std::mutex> lock { sched_mtx_ };
                      cond_var_.wait( lock, [this]() noexcept {
                        return quota_.load( std::memory_order_acquire ) > 0
                            || state_.load( std::memory_order_acquire ) != State::Pulse;
                      } );
                    },
                    1024 );
                  do
                    task_();
                  while ( quota_.fetch_sub( 1, std::memory_order_acq_rel ) > 1
                          && state_.load( std::memory_order_acquire ) == State::Pulse );
                } break;

                case State::Shot: {
                  {
                    std::lock_guard<concurrent::SharedMutex> lock1 { res_mtx_ };
                    std::lock_guard<std::mutex> lock2 { sched_mtx_ };
                    task_();
                  }
                  auto expected = State::Shot;
                  state_.compare_exchange_strong( expected, State::Idle, std::memory_order_release );
                }
                  PGBAR__FALLTHROUGH;
                case State::Idle: {
                  std::unique_lock<std::mutex> lock { sched_mtx_ };
                  cond_var_.wait( lock, [this]() noexcept {
                    return state_.load( std::memory_order_acquire ) != State::Idle;
                  } );
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
          auto expected = State::Dead;
          state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
        }

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
          task_   = nullptr;
        }

        Renderer() = default;

      public:
        // Get the current working interval for all threads.
        PGBAR__NODISCARD static PGBAR__FORCEINLINE TimeGranule working_interval() noexcept
        {
          return _working_interval.load( std::memory_order_acquire );
        }
        // Adjust the thread working interval between this loop and the next loop.
        static PGBAR__FORCEINLINE void working_interval( TimeGranule new_rate ) noexcept
        {
          _working_interval.store( new_rate, std::memory_order_release );
        }

        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }

        Renderer( const Self& )          = delete;
        Self& operator=( const Self& ) & = delete;
        ~Renderer() noexcept { shutdown(); }

        // `activate` guarantees to perform the render task at least once.
        template<Policy Mode>
        void activate() & noexcept( false )
        {
          if ( state_.load( std::memory_order_acquire ) == State::Dead ) {
            std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
            if ( runner_.get_id() == std::thread::id() )
              launch();
            else {
              shutdown();
              launch();
            }
          }

          PGBAR__ASSERT( state_ != State::Dead );
          PGBAR__ASSERT( task_ != nullptr );
          // The operations below are all thread safe without locking.
          box_.rethrow();
          auto desired = []() noexcept {
            if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Async )
              return State::Warmup;
            else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Signal )
              return State::Primed;
            else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync )
              return State::Idle;
          };
          auto expected = State::Dormant;
          if ( state_.compare_exchange_strong( expected, desired(), std::memory_order_release ) ) {
            if PGBAR__CXX17_CNSTXPR ( Mode != Policy::Sync ) {
              quota_.store( 0, std::memory_order_release );
              {
                std::lock_guard<std::mutex> lock { sched_mtx_ };
                cond_var_.notify_one();
              }
              concurrent::spin_wait(
                [&]() noexcept { return state_.load( std::memory_order_acquire ) != desired(); } );
            } else {
              {
                std::lock_guard<std::mutex> lock { sched_mtx_ };
                cond_var_.notify_one();
              }
              std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
              task_();
            }
          }
        }

        // Commit a rendering request and ensure it's executed.
        template<Policy Mode>
        PGBAR__FORCEINLINE void commit() &
        {
          if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Signal ) {
            if ( state_.load( std::memory_order_acquire ) != State::Dormant ) {
              PGBAR__ASSERT( state_ != State::Idle );
              quota_.fetch_add( 1, std::memory_order_release );
              std::lock_guard<std::mutex> lock { sched_mtx_ };
              cond_var_.notify_one();
            }
          } else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync ) {
            PGBAR__ASSERT( state_ == State::Idle );
            std::lock_guard<concurrent::SharedMutex> lock1 { res_mtx_ };
            // To ensure that only one thread is rendering the bar to the OStream.
            std::lock_guard<std::mutex> lock2 { sched_mtx_ };
            task_();
          }
        }

        template<Policy Mode>
        void trigger() & noexcept
        {
          if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Async ) {
            auto expected = State::Loop;
            if ( state_.compare_exchange_strong( expected, State::Warmup, std::memory_order_release ) )
              concurrent::spin_wait(
                [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Warmup; } );
          } else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Signal ) {
            quota_.fetch_add( 1, std::memory_order_release );
            auto expected = State::Pulse;
            if ( state_.compare_exchange_strong( expected, State::Primed, std::memory_order_release ) ) {
              {
                std::lock_guard<std::mutex> lock { sched_mtx_ };
                cond_var_.notify_one();
              }
              concurrent::spin_wait(
                [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Primed; } );
            }
          } else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync ) {
            auto expected = State::Idle;
            if ( state_.compare_exchange_strong( expected, State::Shot, std::memory_order_release ) ) {
              {
                std::lock_guard<std::mutex> lock { sched_mtx_ };
                cond_var_.notify_one();
              }
              concurrent::spin_wait(
                [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Shot; } );
            }
          }
        }

        void abort() noexcept
        {
          auto try_update = [this]( State expected ) noexcept {
            return state_.compare_exchange_strong( expected, State::Asleep, std::memory_order_release );
          };
          if ( try_update( State::Warmup ) || try_update( State::Loop ) || try_update( State::Primed )
               || try_update( State::Pulse ) || try_update( State::Shot ) || try_update( State::Idle ) ) {
            {
              std::lock_guard<std::mutex> lock { sched_mtx_ };
              cond_var_.notify_all();
            }
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Asleep; } );
          }
        }

        template<typename F>
        void dismiss_then( F&& noexpt_fn ) noexcept
        {
          static_assert( noexcept( (void)noexpt_fn() ),
                         "pgbar::_details::render::Renderer::dismiss_then: Unsafe functor types" );

          abort();
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          if ( task_ != nullptr )
            task_ = nullptr;
          (void)noexpt_fn();
        }
        void dismiss() noexcept
        {
          dismiss_then( []() noexcept {} );
        }

        void drop() noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          shutdown();
        }

        PGBAR__NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          if ( task_ != nullptr )
            return false;
          task_ = std::move( task );
          return true;
        }

        PGBAR__NODISCARD PGBAR__FORCEINLINE bool aborted() const noexcept { return box_.empty(); }
        PGBAR__NODISCARD PGBAR__FORCEINLINE bool empty() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
          return task_ == nullptr;
        }
      };
      template<Channel Tag>
      std::atomic<TimeGranule> Renderer<Tag>::_working_interval { std::chrono::duration_cast<TimeGranule>(
        std::chrono::milliseconds( 40 ) ) };
    } // namespace render
  } // namespace _details
} // namespace pgbar

#endif
