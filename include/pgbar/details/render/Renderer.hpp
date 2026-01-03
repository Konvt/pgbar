#ifndef PGBAR__RENDERER
#define PGBAR__RENDERER

#include "../concurrent/ExceptionBox.hpp"
#include "../concurrent/Util.hpp"
#include "../console/TermContext.hpp"
#include "../utils/ScopeGuard.hpp"
#include "../wrappers/UniqueFunction.hpp"
#include <atomic>
#include <thread>
#ifndef __cpp_lib_atomic_wait
# include <condition_variable>
#endif

namespace pgbar {
  namespace _details {
    namespace render {
      template<Channel Tag>
      class Renderer final {
        static std::atomic<TimeGranule> _working_interval;

        std::atomic<std::uint64_t> quota_      = { 0 };
        concurrent::ExceptionBox box_          = {};
        wrappers::UniqueFunction<void()> task_ = {};
        std::thread runner_                    = {};

#ifndef __cpp_lib_atomic_wait
        mutable std::condition_variable cond_var_ = {};
#endif
        mutable concurrent::SharedMutex res_mtx_ = {};
        mutable std::mutex sched_mtx_            = {};

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
            └─ abort() → Asleep → Dormant

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

          runner_ = std::thread( [this]() {
            try {
              for ( auto state = state_.load( std::memory_order_acquire ); state != State::Dead;
                    state      = state_.load( std::memory_order_acquire ) ) {
                switch ( state ) {
                case State::Asleep:
                  concurrent::atomic_commit_all( state_, State::Asleep, State::Dormant );
                  PGBAR__FALLTHROUGH;
                case State::Dormant: {
#ifdef __cpp_lib_atomic_wait
                  state_.wait( State::Dormant, std::memory_order_acquire );
#else
                  std::unique_lock<std::mutex> lock { sched_mtx_ };
                  cond_var_.wait( lock, [this]() noexcept {
                    return state_.load( std::memory_order_acquire ) != State::Dormant;
                  } );
#endif
                } break;

                case State::Warmup: {
                  task_();
                  concurrent::atomic_commit_all( state_, State::Warmup, State::Loop );
                }
                  PGBAR__FALLTHROUGH;
                case State::Loop: {
                  task_();
                  std::this_thread::sleep_for( working_interval() );
                } break;

                case State::Primed: {
                  task_();
                  quota_.fetch_sub( 1, std::memory_order_release );
                  concurrent::atomic_commit_all( state_, State::Primed, State::Pulse );
                }
                  PGBAR__FALLTHROUGH;
                case State::Pulse: {
#ifdef __cpp_lib_atomic_wait
                  quota_.wait( 0, std::memory_order_acquire );
#else
                  concurrent::spin_with(
                    [this]() noexcept { return quota_.load( std::memory_order_acquire ) > 0; },
                    [this]() noexcept {
                      std::unique_lock<std::mutex> lock { sched_mtx_ };
                      cond_var_.wait( lock, [this]() noexcept {
                        return quota_.load( std::memory_order_acquire ) > 0
                            || state_.load( std::memory_order_acquire ) != State::Pulse;
                      } );
                    },
                    1024 );
#endif
                  do
                    task_();
                  while ( quota_.fetch_sub( 1, std::memory_order_acq_rel ) > 1
                          && state_.load( std::memory_order_acquire ) == State::Pulse );
                } break;

                case State::Shot: {
                  {
                    concurrent::SharedLock<concurrent::SharedMutex> lock1 { res_mtx_ };
                    std::lock_guard<std::mutex> lock2 { sched_mtx_ };
                    task_();
                  }
                  concurrent::atomic_commit_all( state_, State::Shot, State::Idle );
                }
                  PGBAR__FALLTHROUGH;
                case State::Idle: {
#ifdef __cpp_lib_atomic_wait
                  state_.wait( State::Idle, std::memory_order_acquire );
#else
                  std::unique_lock<std::mutex> lock { sched_mtx_ };
                  cond_var_.wait( lock, [this]() noexcept {
                    return state_.load( std::memory_order_acquire ) != State::Idle;
                  } );
#endif
                } break;

                default: return;
                }
              }
            } catch ( ... ) {
              auto dump = box_.try_store( std::current_exception() );
              concurrent::atomic_commit_all( state_, dump ? State::Dormant : State::Dead );
              if ( !dump )
                throw;
            }
          } );
          auto expected = State::Dead;
          state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
        }

        void shutdown() noexcept
        {
          concurrent::atomic_commit_all( state_, State::Dead );
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

        static Renderer& itself() noexcept
        {
          static Renderer instance;
          return instance;
        }

        Renderer( const Renderer& )              = delete;
        Renderer& operator=( const Renderer& ) & = delete;
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
            quota_.store( 0, std::memory_order_release );
#ifdef __cpp_lib_atomic_wait
            state_.notify_one();
#endif
            if PGBAR__CXX17_CNSTXPR ( Mode != Policy::Sync ) {
#ifdef __cpp_lib_atomic_wait
              state_.wait( desired(), std::memory_order_acquire );
#else
              {
                std::lock_guard<std::mutex> lock { sched_mtx_ };
                cond_var_.notify_one();
              }
              concurrent::spin_wait(
                [&]() noexcept { return state_.load( std::memory_order_acquire ) != desired(); } );
#endif
            } else {
              std::lock_guard<concurrent::SharedMutex> lock1 { res_mtx_ };
              std::lock_guard<std::mutex> lock2 { sched_mtx_ };
#ifndef __cpp_lib_atomic_wait
              cond_var_.notify_one();
#endif
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
              quota_.fetch_add( 1, std::memory_order_release );
#ifdef __cpp_lib_atomic_wait
              quota_.notify_one();
#else
              std::lock_guard<std::mutex> lock { sched_mtx_ };
              cond_var_.notify_one();
#endif
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
#ifdef __cpp_lib_atomic_wait
          auto state_transfer = [this]( State expected, State desired ) noexcept {
            if ( concurrent::atomic_commit_one( state_, expected, desired ) )
              state_.wait( desired, std::memory_order_acquire );
          };
          if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Async )
            state_transfer( State::Loop, State::Warmup );
          else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Signal ) {
            quota_.fetch_add( 1, std::memory_order_release );
            auto expected = State::Pulse;
            if ( state_.compare_exchange_strong( expected, State::Primed, std::memory_order_release ) ) {
              quota_.notify_one();
              state_.wait( State::Primed, std::memory_order_acquire );
            }
          } else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync )
            state_transfer( State::Idle, State::Shot );
#else
          auto state_transfer = [this]( State expected, State desired ) noexcept {
            if ( state_.compare_exchange_strong( expected, desired, std::memory_order_release ) ) {
              {
                std::lock_guard<std::mutex> lock { sched_mtx_ };
                cond_var_.notify_one();
              }
              concurrent::spin_wait(
                [&]() noexcept { return state_.load( std::memory_order_acquire ) != desired; } );
            }
          };
          if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Async ) {
            auto expected = State::Loop;
            if ( state_.compare_exchange_strong( expected, State::Warmup, std::memory_order_release ) )
              concurrent::spin_wait(
                [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Warmup; } );
          } else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Signal ) {
            quota_.fetch_add( 1, std::memory_order_release );
            state_transfer( State::Pulse, State::Primed );
          } else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync )
            state_transfer( State::Idle, State::Shot );
#endif
        }

        void abort() noexcept
        {
          auto try_update = [this]( State expected ) noexcept {
            return concurrent::atomic_commit_one( state_, expected, State::Asleep );
          };
          if ( try_update( State::Warmup ) || try_update( State::Loop ) || try_update( State::Primed )
               || try_update( State::Pulse ) || try_update( State::Shot ) || try_update( State::Idle ) ) {
#ifdef __cpp_lib_atomic_wait
            state_.wait( State::Asleep, std::memory_order_acquire );
#else
            {
              std::lock_guard<std::mutex> lock { sched_mtx_ };
              cond_var_.notify_all();
            }
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Asleep; } );
#endif
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

        PGBAR__NODISCARD PGBAR__FORCEINLINE bool interrupted() const noexcept { return !box_.empty(); }
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
