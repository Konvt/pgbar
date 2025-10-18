#ifndef PGBAR__RENDERER
#define PGBAR__RENDERER

#include "Runner.hpp"

namespace pgbar {
  namespace _details {
    namespace render {
      template<Channel Tag, Policy Mode>
      class Renderer;
      template<Channel Tag>
      class Renderer<Tag, Policy::Async> final {
        using Self = Renderer;

        static std::atomic<types::TimeUnit> _working_interval;

        /**
         * Unlike synchronous renderers,
         * asynchronous renderers do not require the background thread to be suspended for a long time,
         * so the condition variable is simplified away here.
         */
        enum class State : std::uint8_t { Awake, Active, Attempt, Quit };
        std::atomic<State> state_;
        wrappers::UniqueFunction<void()> task_;
        mutable concurrent::SharedMutex rw_mtx_;

        Renderer() noexcept : state_ { State::Quit } {}

      public:
        // Get the current working interval for all threads.
        PGBAR__NODISCARD static PGBAR__INLINE_FN types::TimeUnit working_interval()
        {
          return _working_interval.load( std::memory_order_acquire );
        }
        // Adjust the thread working interval between this loop and the next loop.
        static PGBAR__INLINE_FN void working_interval( types::TimeUnit new_rate )
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
        ~Renderer() noexcept { appoint(); }

        void activate() & noexcept( false )
        {
          auto expected = State::Quit;
          if ( state_.compare_exchange_strong( expected, State::Awake, std::memory_order_release ) ) {
            PGBAR__ASSERT( task_ != nullptr );
            auto& renderer = Runner<Tag>::itself();
            try {
              renderer.activate();
            } catch ( ... ) {
              /**
               * In Runner::activate, there are only two sources of exceptions,
               * Runner::launch or the background thread;
               * in either case, the Runner retains the assigned task,
               * so the task status is still valid and should be switched back to Quit.
               */
              state_.store( State::Quit, std::memory_order_release );
              throw;
            }
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Awake; } );
            renderer.throw_if();
            // Recheck whether any exceptions have been received in the background thread.
            // If so, abandon this rendering.
          }
        }

        template<typename F>
        void appoint_then( F&& noexpt_fn ) noexcept
        {
          static_assert( noexcept( (void)noexpt_fn() ),
                         "pgbar::_details::render::Renderer::appoint_then: Unsafe functor types" );

          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr ) {
            state_.store( State::Quit, std::memory_order_release );
            Runner<Tag>::itself().appoint();
            task_ = nullptr;
          }
          (void)noexpt_fn();
        }
        void appoint() noexcept
        {
          appoint_then( []() noexcept {} );
        }
        PGBAR__NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          if ( task_ != nullptr || !Runner<Tag>::itself().try_appoint( [this]() {
                 try {
                   while ( state_.load( std::memory_order_acquire ) != State::Quit ) {
                     switch ( state_.load( std::memory_order_acquire ) ) {
                     case State::Awake: {
                       task_();
                       auto expected = State::Awake;
                       state_.compare_exchange_strong( expected, State::Active, std::memory_order_release );
                     }
                       PGBAR__FALLTHROUGH;
                     case State::Active: {
                       task_();
                       std::this_thread::sleep_for( working_interval() );
                     } break;

                     case State::Attempt: {
                       task_();
                       auto expected = State::Attempt;
                       state_.compare_exchange_strong( expected, State::Active, std::memory_order_release );
                     } break;

                     default: return;
                     }
                   }
                 } catch ( ... ) {
                   state_.store( State::Quit, std::memory_order_release );
                   throw;
                 }
               } ) )
            return false;
          state_.store( State::Quit, std::memory_order_release );
          task_.swap( task );
          return true;
        }

        PGBAR__INLINE_FN void execute() & noexcept { /* Empty implementation */ }
        PGBAR__INLINE_FN void attempt() & noexcept
        {
          // Each call should not be discarded.
          std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ };
          auto try_update = [this]( State expected ) noexcept {
            return state_.compare_exchange_strong( expected, State::Attempt, std::memory_order_release );
          };
          if ( try_update( State::Awake ) || try_update( State::Active ) )
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Attempt; } );
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN bool empty() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ };
          return task_ == nullptr;
        }
      };
      template<Channel Tag>
      std::atomic<types::TimeUnit> Renderer<Tag, Policy::Async>::_working_interval {
        std::chrono::duration_cast<types::TimeUnit>( std::chrono::milliseconds( 40 ) )
      };

      template<Channel Tag>
      class Renderer<Tag, Policy::Sync> final {
        using Self = Renderer;

        enum class State : std::uint8_t { Dormant, Finish, Active, Quit };
        std::atomic<State> state_;

        mutable concurrent::SharedMutex res_mtx_;
        mutable std::mutex sched_mtx_;
        mutable std::condition_variable cond_var_;

        wrappers::UniqueFunction<void()> task_;

        Renderer() noexcept : state_ { State::Quit } {}

      public:
        static Self& itself() noexcept
        {
          static Self instance;
          return instance;
        }
        Renderer( const Self& )        = delete;
        Self& operator=( const Self& ) = delete;

        ~Renderer() noexcept { appoint(); }

        // `activate` guarantees to perform the render task at least once.
        void activate() & noexcept( false )
        { /**
           * Since the rendering function is a state machine,
           * `activate` must ensure it runs at least once to trigger state transitions.
           * As asynchronous renderers do not provide an interface that guarantees at-least-once execution,
           * `activate` must explicitly invoke the rendering function during scheduling.
           */
          std::lock_guard<concurrent::SharedMutex> lock1 { res_mtx_ };
          // Internal component does not make validity judgments.
          PGBAR__ASSERT( task_ != nullptr );
          PGBAR__ASSERT( state_ == State::Dormant );
          Runner<Tag>::itself().activate();
          task_();
          // If the rendering task throws some exception, this rendering should be abandoned.
        }

        template<typename F>
        void appoint_then( F&& noexpt_fn ) noexcept
        {
          static_assert( noexcept( (void)noexpt_fn() ),
                         "pgbar::_details::render::Renderer::appoint_then: Unsafe functor types" );

          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          if ( task_ != nullptr ) {
            state_.store( State::Quit, std::memory_order_release );
            {
              std::unique_lock<std::mutex> lock { sched_mtx_ };
              cond_var_.notify_one();
            }
            Runner<Tag>::itself().appoint();
            task_ = nullptr;
          }
          (void)noexpt_fn();
        }
        void appoint() noexcept
        {
          appoint_then( []() noexcept {} );
        }

        PGBAR__NODISCARD bool try_appoint( wrappers::UniqueFunction<void()>&& task ) & noexcept( false )
        {
          std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
          if ( task_ != nullptr || !Runner<Tag>::itself().try_appoint( [this]() {
                 try {
                   while ( state_.load( std::memory_order_acquire ) != State::Quit ) {
                     switch ( state_.load( std::memory_order_acquire ) ) {
                     case State::Dormant: PGBAR__FALLTHROUGH;
                     case State::Finish:  {
                       std::unique_lock<std::mutex> lock { sched_mtx_ };
                       auto expected = State::Finish;
                       state_.compare_exchange_strong( expected, State::Dormant, std::memory_order_release );
                       cond_var_.wait( lock, [this]() noexcept {
                         return state_.load( std::memory_order_acquire ) != State::Dormant;
                       } );
                     } break;
                     case State::Active: {
                       {
                         concurrent::SharedLock<concurrent::SharedMutex> lock1 { res_mtx_ };
                         std::lock_guard<std::mutex> lock2 { sched_mtx_ };
                         task_();
                       }
                       auto expected = State::Active;
                       state_.compare_exchange_strong( expected, State::Finish, std::memory_order_release );
                     } break;
                     default: return;
                     }
                   }
                 } catch ( ... ) {
                   state_.store( State::Quit, std::memory_order_release );
                   throw;
                 }
               } ) )
            return false;
          state_.store( State::Dormant, std::memory_order_release );
          task_.swap( task );
          return true;
        }

        // Assume that the task is not empty and execute it once.
        PGBAR__INLINE_FN void execute() & noexcept( false )
        { /**
           * Although not relevant here,
           * OStream is called non-atomically for each rendering task,
           * so it needs to be locked for the entire duration of the rendering task execution.

           * By the way, although the implementation tries to lock the internal component
           * when it renders the string,
           * that is a reader lock on the configuration data type of the component that stores the string,
           * and it does not prevent concurrent threads from simultaneously
           * trying to concatenate the string in the same OStream.
           */
          concurrent::SharedLock<concurrent::SharedMutex> lock1 { res_mtx_ };
          std::lock_guard<std::mutex> lock2 { sched_mtx_ }; // Fixed locking order.
          PGBAR__ASSERT( task_ != nullptr );
          task_();
          /**
           * In short: The mtx_ here is to prevent multiple progress bars in MultiBar
           * from concurrently attempting to render strings in synchronous rendering mode;
           * each of these progress bars will lock itself before rendering,
           * but they cannot mutually exclusively access the renderer.
           */
        }
        // When the task is not empty, execute at least one task.
        PGBAR__INLINE_FN void attempt() & noexcept
        {
          // The lock here is to ensure that only one thread executes the task_ at any given time.
          // And synchronization semantics require that no call request be dropped.
          concurrent::SharedLock<concurrent::SharedMutex> lock1 { res_mtx_ };
          /**
           * Here, asynchronous threads and forced state checks are used to achieve synchronous semantics;
           * this is because task_ is not exception-free,
           * so if task_ needs to be executed with synchronous semantics in some functions marked as noexcept,
           * the execution operation needs to be dispatched to another thread.
           */
          auto expected = State::Dormant;
          if ( state_.compare_exchange_strong( expected, State::Active, std::memory_order_release ) ) {
            {
              std::lock_guard<std::mutex> lock { sched_mtx_ };
              cond_var_.notify_one();
            }
            concurrent::spin_wait(
              [this]() noexcept { return state_.load( std::memory_order_acquire ) != State::Active; } );
            std::lock_guard<std::mutex> lock { sched_mtx_ };
          }
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN bool empty() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
          return task_ == nullptr;
        }
      };
    } // namespace render
  } // namespace _details
} // namespace pgbar

#endif
