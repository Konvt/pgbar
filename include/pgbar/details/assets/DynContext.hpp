#ifndef PGBAR__DYNCONTEXT
#define PGBAR__DYNCONTEXT

#include "../prefabs/ManagedBar.hpp"

namespace pgbar {
  namespace _details {
    namespace assets {
      template<Channel Outlet, Policy Mode, Region Area>
      class DynContext final {
        struct Slot final {
        private:
          template<typename Derived>
          static void render( Indicator* item )
          {
            static_assert(
              traits::AllOf<std::is_base_of<Indicator, Derived>, traits::is_bar<Derived>>::value,
              "pgbar::_details::assets::DynContext::Slot::render: Derived must inherit from Indicator" );
            PGBAR__TRUST( item != nullptr );
            make_frame( static_cast<Derived&>( *item ) );
          }

        public:
          void ( *render_ )( Indicator* );
          Indicator* target_;

          template<typename Config>
          Slot( prefabs::ManagedBar<Config, Outlet, Mode, Area>* item ) noexcept
            : render_ { render<prefabs::BasicBar<Config, Outlet, Mode, Area>> }, target_ { item }
          {}
        };

        std::vector<Slot> items_                       = {};
        // If Area is equal to Region::Fixed,
        // the variable represents the number of lines that need to be discarded;
        // If Area is equal to Region::Relative,
        // the variable represents the number of nextlines output last time.
        std::atomic<std::uint64_t> num_modified_lines_ = { 0 };
        mutable concurrent::SharedMutex res_mtx_       = {};
        mutable std::mutex sched_mtx_                  = {};

        enum class State : std::uint8_t { Stop, Awake, Refresh };
        std::atomic<State> state_ = { State::Stop };

        void do_render() &
        {
          auto& ostream        = io::OStream<Outlet>::itself();
          const auto istty     = console::TermContext<Outlet>::itself().connected();
          const auto hide_done = config::hide_completed();

          bool any_alive = false, any_rendered = false;
          for ( types::Size i = 0; i < items_.size(); ++i ) {
            bool this_rendered = false;
            // If items_[i].target_ is equal to nullptr,
            // indicating that the i-th object has stopped.
            if ( items_[i].target_ != nullptr ) {
              this_rendered = any_rendered = true;
              if ( istty && !hide_done )
                ostream << console::escodes::linewipe;
              ( *items_[i].render_ )( items_[i].target_ );

              const auto is_alive = items_[i].target_->active();
              any_alive |= is_alive;
              if ( !is_alive )
                items_[i].target_ = nullptr; // mark that it should stop rendering
            }

            /**
             * When Area is equal to Region::Fixed, the row discard policy is as follows:
             * After eliminating the first k consecutive stopped items in the render queue item_
             * (via the eliminate method), the starting area for rendering is moved down by k rows.
             * Therefore, at this point,
             * all remaining items that have not been removed should trigger a line break for rendering.

             * When Area is equal to Region::Relative, the row discard policy is as follows:
             * During the rendering process,
             * count the number of consecutive line breaks output starting from the first rendered item,
             * denoted as n.
             * In the next round of rendering, move up by n rows.
             * Therefore, at this point,
             * it is necessary to track which items in the render queue have been rendered
             * and whether any items have been rendered in the current round of rendering.

             * If the output stream is not bound to a terminal, there is no line discard policy;
             * all rendered items will trigger a newline character to be rendered.
             */
            if PGBAR__CXX17_CNSTXPR ( Area == Region::Relative )
              if ( !any_rendered && !this_rendered )
                continue;
            if ( ( !istty && this_rendered )
                 || ( istty && ( !hide_done || items_[i].target_ != nullptr ) ) ) {
              ostream << console::escodes::nextline;
              if PGBAR__CXX17_CNSTXPR ( Area == Region::Relative )
                num_modified_lines_.fetch_add( any_alive, std::memory_order_relaxed );
            }
            if ( istty && hide_done ) {
              if ( items_[i].target_ == nullptr )
                ostream << console::escodes::linestart;
              ostream << console::escodes::linewipe;
            }
          }
        }

        void eliminate() noexcept
        {
          // Search for the first k stopped progress bars and remove them.
          auto itr = std::find_if( items_.cbegin(), items_.cend(), []( const Slot& slot ) noexcept {
            return slot.target_ != nullptr;
          } );
          if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
            num_modified_lines_.fetch_add( utils::distance( items_.cbegin(), itr ),
                                           std::memory_order_release );
          items_.erase( items_.cbegin(), itr );
        }

        template<bool Forced>
        void do_shut() noexcept( Forced )
        {
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
          if ( state_.load( std::memory_order_acquire ) != State::Stop ) {
            for ( types::Size i = 0; i < items_.size(); ++i ) {
              if ( items_[i].target_ != nullptr ) {
                if PGBAR__CXX17_CNSTXPR ( Forced )
                  items_[i].target_->abort();
                else
                  items_[i].target_->reset();
              }
            }
            render::Renderer<Outlet>::itself().dismiss();
          }
          state_.store( State::Stop, std::memory_order_release );
          items_.clear();
        }

      public:
        DynContext()                                 = default;
        DynContext( const DynContext& )              = delete;
        DynContext& operator=( const DynContext& ) & = delete;
        ~DynContext() noexcept { kill(); }

        void shut() { do_shut<false>(); }
        void kill() noexcept { do_shut<true>(); }

        template<typename C>
        void append( prefabs::ManagedBar<C, Outlet, Mode, Area>* item ) & noexcept( false )
        {
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          auto& executor     = render::Renderer<Outlet>::itself();
          bool activate_flag = false;
          {
            concurrent::SharedLock<concurrent::SharedMutex> lock2 { res_mtx_ };
            activate_flag = items_.empty();
          }
          if ( activate_flag ) {
            if ( !executor.try_appoint( [this]() {
                   auto& ostream        = io::OStream<Outlet>::itself();
                   const auto istty     = console::TermContext<Outlet>::itself().connected();
                   const auto hide_done = config::hide_completed();
                   switch ( state_.load( std::memory_order_acquire ) ) {
                   case State::Awake: {
                     if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
                       if ( istty )
                         ostream << console::escodes::savecursor;
                     {
                       concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
                       do_render();
                     }
                     ostream << io::flush;
                     auto expected = State::Awake;
                     state_.compare_exchange_strong( expected, State::Refresh, std::memory_order_release );
                   } break;
                   case State::Refresh: {
                     {
                       concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
                       if ( istty ) {
                         if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed ) {
                           ostream << console::escodes::resetcursor;
                           if ( !hide_done ) {
                             const auto num_discarded = num_modified_lines_.load( std::memory_order_acquire );
                             if ( num_discarded > 0 ) {
                               ostream.append( console::escodes::nextline, num_discarded )
                                 .append( console::escodes::savecursor );
                               num_modified_lines_.fetch_sub( num_discarded, std::memory_order_release );
                             }
                           }
                         } else {
                           ostream
                             .append( console::escodes::prevline,
                                      num_modified_lines_.load( std::memory_order_relaxed ) )
                             .append( console::escodes::linestart );
                           num_modified_lines_.store( 0, std::memory_order_relaxed );
                         }
                       }
                       do_render();
                     }
                     ostream << io::flush;
                   } break;
                   default: return;
                   }
                 } ) )
              PGBAR__UNLIKELY throw exception::InvalidState(
                charcodes::make_literal( "pgbar: another progress bar instance is already running" ) );

            io::OStream<Outlet>::itself() << io::release;
            num_modified_lines_.store( 0, std::memory_order_relaxed );
            state_.store( State::Awake, std::memory_order_release );

            auto guard = utils::make_scope_fail( [this]() noexcept {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              items_.clear();
              state_.store( State::Stop, std::memory_order_release );
            } );
            {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              items_.emplace_back( item );
            }
            executor.template activate<Mode>();
          } else {
            {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              eliminate();
              items_.emplace_back( item );
            }
            executor.template trigger<Mode>();
          }
        }
        void pop( const Indicator* item, bool forced = false ) noexcept
        {
          auto& executor = render::Renderer<Outlet>::itself();
          PGBAR__ASSERT( executor.empty() == false );
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          PGBAR__ASSERT( online_count() != 0 );
          if ( !forced )
            executor.template trigger<Mode>();

          bool suspend_flag = false;
          {
            std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
            const auto itr = std::find_if( items_.begin(), items_.end(), [item]( const Slot& slot ) noexcept {
              return item == slot.target_;
            } );
            // Mark target_ as empty,
            // and then search for the first k invalid or destructed progress bars and remove them.
            if ( itr != items_.end() )
              itr->target_ = nullptr;
            eliminate();
            suspend_flag = items_.empty();
          }

          if ( suspend_flag ) {
            state_.store( State::Stop, std::memory_order_release );
            executor.dismiss_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
          }
        }

        PGBAR__NODISCARD PGBAR__FORCEINLINE types::Size online_count() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
          return items_.size();
        }
      };
    } // namespace assets
  } // namespace _details
} // namespace pgbar

#endif
