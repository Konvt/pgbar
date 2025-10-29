#ifndef PGBAR__TUPLEBAR
#define PGBAR__TUPLEBAR

#include "../assets/TupleSlot.hpp"
#include "../prefabs/BasicBar.hpp"
#include "../traits/Backport.hpp"
#include <initializer_list>
#include <tuple>

namespace pgbar {
  namespace _details {
    namespace prefabs {
      template<typename Seq, typename... Bars>
      class TupleBar;
      template<types::Size... Tags, Channel Outlet, Policy Mode, Region Area, typename... Configs>
      class TupleBar<traits::IndexSeq<Tags...>, prefabs::BasicBar<Configs, Outlet, Mode, Area>...> final
        : public assets::TupleSlot<prefabs::BasicBar<Configs, Outlet, Mode, Area>, Tags>... {
        static_assert( sizeof...( Tags ) == sizeof...( Configs ),
                       "pgbar::_details::prefabs::TupleBar: Unexpected type mismatch" );
        static_assert( sizeof...( Configs ) > 0,
                       "pgbar::_details::prefabs::TupleBar: The number of progress bars cannot be zero" );

        template<types::Size Pos>
        using ElementAt_t =
          traits::TypeAt_t<Pos, assets::TupleSlot<prefabs::BasicBar<Configs, Outlet, Mode, Area>, Tags>...>;

        std::atomic<types::Size> alive_cnt_;
        mutable std::mutex sched_mtx_;
        // The std::bitset isn't TriviallyCopyable, so we cannot use std::atomic<std::bitset>.
        mutable concurrent::SharedMutex res_mtx_;

        enum class State : std::uint8_t { Stop, Awake, Refresh };
        std::atomic<State> state_;

        // Bitmask indicating which bars produced output in the current render pass.
        std::bitset<sizeof...( Configs )> active_mask_;

        template<types::Size Pos>
        PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR typename std::enable_if<( Pos >= sizeof...( Configs ) )>::type
          do_render() &
        {}
        template<types::Size Pos = 0>
        inline typename std::enable_if<( Pos < sizeof...( Configs ) )>::type do_render() &
        {
          PGBAR__ASSERT( online() );
          auto& ostream        = io::OStream<Outlet>::itself();
          const auto istty     = console::TermContext<Outlet>::itself().connected();
          const auto hide_done = config::hide_completed();

          bool this_rendered = false;
          if ( at<Pos>().active() ) {
            this_rendered = true;
            active_mask_.set( Pos );
            if ( istty )
              ostream << console::escodes::linewipe;
            make_frame( at<Pos>() );

            if ( ( !istty || hide_done ) && !at<Pos>().active() )
              active_mask_.reset( Pos );
          }

          /**
           * Here are the scenarios where a newline character is output:
           * 1. If the output stream is bound to a terminal and the completed progress bar does not need to be
           * hidden, it should be output when active_mask_[Pos] is true.
           * 2. If the output stream is bound to a terminal and the completed progress bar needs to be hidden,
           *    it only be output when Pos-th is still active.
           * 3. If the output stream is not bound to a terminal,
           *    it should be output whenever Pos-th has just been rendered.
           */
          if ( ( this_rendered || active_mask_[Pos] )
               && ( ( !istty && this_rendered ) || ( istty && ( !hide_done || at<Pos>().active() ) ) ) )
            ostream << console::escodes::nextline;
          if ( istty && hide_done ) {
            if ( !at<Pos>().active() )
              ostream << console::escodes::linestart;
            ostream << console::escodes::linewipe;
          }

          return do_render<Pos + 1>(); // tail recursive
        }

        void do_halt( bool forced ) noexcept final
        { // This virtual function is invoked only via the vtable,
          // hence the default arguments from the base class declaration are always used.
          // Any default arguments provided in the derived class are ignored.
          if ( online() ) {
            auto& executor = render::Renderer<Outlet, Mode>::itself();
            PGBAR__ASSERT( executor.empty() == false );
            std::lock_guard<std::mutex> lock { sched_mtx_ };
            if ( !forced )
              executor.attempt();
            if ( alive_cnt_.fetch_sub( 1, std::memory_order_acq_rel ) == 1 ) {
              state_.store( State::Stop, std::memory_order_release );
              executor.dismiss_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
            }
          }
        }
        void do_boot() & final
        {
          std::lock_guard<std::mutex> lock { sched_mtx_ };
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          if ( state_.load( std::memory_order_acquire ) == State::Stop ) {
            if ( !executor.try_appoint( [this]() {
                   auto& ostream    = io::OStream<Outlet>::itself();
                   const auto istty = console::TermContext<Outlet>::itself().connected();
                   switch ( state_.load( std::memory_order_acquire ) ) {
                   case State::Awake: {
                     if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
                       if ( istty )
                         ostream << console::escodes::savecursor;
                     {
                       std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
                       active_mask_.reset();
                       do_render();
                     }
                     ostream << io::flush;

                     auto expected = State::Awake;
                     state_.compare_exchange_strong( expected, State::Refresh, std::memory_order_release );
                   } break;
                   case State::Refresh: {
                     {
                       std::lock_guard<concurrent::SharedMutex> lock { res_mtx_ };
                       if ( istty ) {
                         if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
                           ostream << console::escodes::resetcursor;
                         else
                           ostream.append( console::escodes::prevline, active_mask_.count() )
                             .append( console::escodes::linestart );
                       }
                       do_render();
                     }
                     ostream << io::flush;
                   } break;
                   default: break;
                   }
                 } ) )
              PGBAR__UNLIKELY throw exception::InvalidState(
                "pgbar: another progress bar instance is already running" );

            io::OStream<Outlet>::itself() << io::release;
            state_.store( State::Awake, std::memory_order_release );
            try {
              executor.activate();
            } catch ( ... ) {
              state_.store( State::Stop, std::memory_order_release );
              executor.dismiss();
              throw;
            }
          } else
            executor.attempt();
          alive_cnt_.fetch_add( 1, std::memory_order_release );
          PGBAR__ASSERT( alive_cnt_ <= sizeof...( Configs ) );
        }

        template<typename Tuple, types::Size... Is>
        TupleBar( Tuple&& tup, const traits::IndexSeq<Is...>& )
          noexcept( std::tuple_size<typename std::decay<Tuple>::type>::value == sizeof...( Configs ) )
          : ElementAt_t<Is>( utils::pick_or<Is, ElementAt_t<Is>>( std::forward<Tuple>( tup ) ) )...
          , alive_cnt_ { 0 }
          , sched_mtx_ {}
          , res_mtx_ {}
          , state_ { State::Stop }
        {
          static_assert( std::tuple_size<typename std::decay<Tuple>::type>::value <= sizeof...( Is ),
                         "pgbar::_details::prefabs::TupleBar::ctor: Unexpected tuple size mismatch" );
        }

      public:
        template<types::Size... Is, typename... Cs, Channel O, Policy M, Region A>
        TupleBar( const assets::TupleSlot<prefabs::BasicBar<Cs, O, M, A>, Is>&... ) = delete;

        // SFINAE is used here to prevent infinite recursive matching of errors.
        template<typename Cfg,
                 typename... Cfgs,
                 typename = typename std::enable_if<traits::TpStartsWith<
                   traits::TypeList<typename std::decay<Cfg>::type, typename std::decay<Cfgs>::type...>,
                   Configs...>::value>::type>
        TupleBar( Cfg&& cfg, Cfgs&&... cfgs ) noexcept( sizeof...( Cfgs ) + 1 == sizeof...( Configs ) )
          : TupleBar( std::forward_as_tuple( std::forward<Cfg>( cfg ), std::forward<Cfgs>( cfgs )... ),
                      traits::MakeIndexSeq<sizeof...( Cfgs ) + 1>() )
        {}
        template<typename... Cfgs,
                 typename = typename std::enable_if<
                   traits::TpStartsWith<traits::TypeList<Cfgs...>, Configs...>::value>::type>
        TupleBar( prefabs::BasicBar<Cfgs, Outlet, Mode, Area>&&... bars )
          noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
          : TupleBar( std::forward_as_tuple( std::move( bars )... ),
                      traits::MakeIndexSeq<sizeof...( Cfgs )>() )
        {}
        TupleBar( const TupleBar& )              = delete;
        TupleBar& operator=( const TupleBar& ) & = delete;
        TupleBar( TupleBar&& rhs ) noexcept
          : assets::TupleSlot<prefabs::BasicBar<Configs, Outlet, Mode, Area>, Tags>( std::move( rhs ) )...
          , alive_cnt_ { 0 }
          , state_ { State::Stop }
        {
          PGBAR__ASSERT( rhs.online() == false );
        }
        TupleBar& operator=( TupleBar&& rhs ) & noexcept
        { // The thread insecurity here is deliberately designed.
          // After all, for a type where a base class reference can be exposed,
          // we cannot apply the lock within the type to the outside.
          PGBAR__TRUST( this != &rhs );
          PGBAR__ASSERT( online() == false );
          PGBAR__ASSERT( rhs.online() == false );
          (void)std::initializer_list<bool> { (
            assets::TupleSlot<prefabs::BasicBar<Configs, Outlet, Mode, Area>, Tags>::operator=(
              std::move( rhs ) ),
            false )... };
          return *this;
        }
        ~TupleBar() noexcept { kill(); }

        void shut()
        {
          if ( online() && !_details::render::Renderer<Outlet, Mode>::itself().empty() )
            (void)std::initializer_list<bool> { ( this->ElementAt_t<Tags>::reset(), false )... };
          PGBAR__ASSERT( alive_cnt_ == 0 );
          PGBAR__ASSERT( online() == false );
        }
        void kill() noexcept
        {
          if ( online() && !_details::render::Renderer<Outlet, Mode>::itself().empty() )
            (void)std::initializer_list<bool> { ( this->ElementAt_t<Tags>::abort(), false )... };
          PGBAR__ASSERT( alive_cnt_ == 0 );
          PGBAR__ASSERT( online() == false );
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN bool online() const noexcept
        {
          return state_.load( std::memory_order_acquire ) != State::Stop;
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN types::Size online_count() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
          return active_mask_.count();
        }

        void swap( TupleBar& lhs ) noexcept
        { // The thread insecurity here is deliberately designed.
          // The reason can be found in the move assignment.
          PGBAR__TRUST( this != &lhs );
          PGBAR__ASSERT( online() == false );
          PGBAR__ASSERT( lhs.online() == false );
          (void)std::initializer_list<bool> {
            ( this->ElementAt_t<Tags>::swap( static_cast<ElementAt_t<Tags>&>( lhs ) ), false )...
          };
        }

        template<types::Size Pos>
        PGBAR__INLINE_FN ElementAt_t<Pos>& at() & noexcept
        {
          return static_cast<ElementAt_t<Pos>&>( *this );
        }
        template<types::Size Pos>
        PGBAR__INLINE_FN const ElementAt_t<Pos>& at() const& noexcept
        {
          return static_cast<const ElementAt_t<Pos>&>( *this );
        }
        template<types::Size Pos>
        PGBAR__INLINE_FN ElementAt_t<Pos>& at() && noexcept
        {
          return std::move( at<Pos>() );
        }
      };
    } // namespace prefabs
  } // namespace _details
} // namespace pgbar

#endif
