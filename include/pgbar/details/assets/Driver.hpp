#ifndef PGBAR__DRIVER
#define PGBAR__DRIVER

#include "../../Indicator.hpp"
#include "../../slice/BoundedSpan.hpp"
#include "../../slice/IteratorSpan.hpp"
#include "../../slice/NumericSpan.hpp"
#include "../io/OStream.hpp"
#include "../prefabs/BasicConfig.hpp"
#include "../render/Builder.hpp"
#include "../render/Renderer.hpp"
#include "../traits/Backport.hpp"
#include "../utils/Backport.hpp"
#include <atomic>

namespace pgbar {
  namespace slice {
    template<typename R, typename B>
    class TrackedSpan;
  }

  namespace _details {
    namespace assets {
      template<typename Base, typename Derived>
      class TaskCounter : public Base {
        // Throws the exception::InvalidState if current object is active.
        PGBAR__INLINE_FN void throw_if_active()
        {
          if ( this->active() )
            PGBAR__UNLIKELY throw exception::InvalidState( "pgbar: try to iterate using an active object" );
        }

      protected:
        std::atomic<std::uint64_t> task_cnt_;
        std::uint64_t task_end_;

      public:
        template<
          typename... Args,
          typename = typename std::enable_if<traits::Not<traits::AnyOf<
            std::is_base_of<TaskCounter<Base, Derived>, typename std::decay<Args>::type>...>>::value>::type>
        constexpr TaskCounter( Args&&... args )
          noexcept( std::is_nothrow_constructible<Base, Args...>::value )
          : Base( std::forward<Args>( args )... ), task_cnt_ { 0 }
        {}
        PGBAR__CXX14_CNSTXPR TaskCounter( TaskCounter&& rhs ) noexcept : Base( std::move( rhs ) )
        {
          task_cnt_.store( 0, std::memory_order_relaxed );
        }
        PGBAR__CXX14_CNSTXPR TaskCounter& operator=( TaskCounter&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        PGBAR__CXX20_CNSTXPR ~TaskCounter() = default;

        // Get the progress of the task.
        PGBAR__NODISCARD std::uint64_t progress() const noexcept
        {
          return task_cnt_.load( std::memory_order_acquire );
        }

        /**
         * Visualize unidirectional traversal of a numeric interval defined by parameters.
         *
         * @return Return a range `[startpoint, endpoint)` that moves unidirectionally.
         */
        template<typename N>
#if PGBAR__CXX20
          requires std::is_arithmetic_v<N>
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR slice::TrackedSpan<slice::NumericSpan<N>, Derived>
#else
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR
          typename std::enable_if<std::is_arithmetic<N>::value,
                                  slice::TrackedSpan<slice::NumericSpan<N>, Derived>>::type
#endif
          iterate( N startpoint, N endpoint, N step ) &
        { // default parameter will cause ambiguous overloads
          // so we have to write them all
          throw_if_active();
          return {
            { startpoint, endpoint, step },
            static_cast<Derived&>( *this )
          };
        }
        template<typename N, typename Proc>
#if PGBAR__CXX20
          requires std::is_arithmetic_v<N>
        PGBAR__CXX14_CNSTXPR void
#else
        PGBAR__CXX14_CNSTXPR typename std::enable_if<std::is_arithmetic<N>::value>::type
#endif
          iterate( N startpoint, N endpoint, N step, Proc&& op )
        {
          for ( N e : iterate( startpoint, endpoint, step ) )
            utils::invoke( std::forward<Proc>( op ), e );
        }

        template<typename N>
#if PGBAR__CXX20
          requires std::is_floating_point_v<N>
        PGBAR__NODISCARD slice::TrackedSpan<slice::NumericSpan<N>, Derived>
#else
        PGBAR__NODISCARD typename std::enable_if<std::is_floating_point<N>::value,
                                                 slice::TrackedSpan<slice::NumericSpan<N>, Derived>>::type
#endif
          iterate( N endpoint, N step ) &
        {
          throw_if_active();
          return {
            { {}, endpoint, step },
            static_cast<Derived&>( *this )
          };
        }
        template<typename N, typename Proc>
#if PGBAR__CXX20
          requires std::is_floating_point_v<N>
        void
#else
        typename std::enable_if<std::is_floating_point<N>::value>::type
#endif
          iterate( N endpoint, N step, Proc&& op )
        {
          for ( N e : iterate( endpoint, step ) )
            utils::invoke( std::forward<Proc>( op ), e );
        }

        // Only available for integer types.
        template<typename N>
#if PGBAR__CXX20
          requires std::is_integral_v<N>
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR slice::TrackedSpan<slice::NumericSpan<N>, Derived>
#else
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  slice::TrackedSpan<slice::NumericSpan<N>, Derived>>::type
#endif
          iterate( N startpoint, N endpoint ) &
        {
          throw_if_active();
          return {
            { startpoint, endpoint, 1 },
            static_cast<Derived&>( *this )
          };
        }
        template<typename N, typename Proc>
#if PGBAR__CXX20
          requires std::is_integral_v<N>
        PGBAR__CXX14_CNSTXPR void
#else
        PGBAR__CXX14_CNSTXPR typename std::enable_if<std::is_integral<N>::value>::type
#endif
          iterate( N startpoint, N endpoint, Proc&& op )
        {
          for ( N e : iterate( startpoint, endpoint ) )
            utils::invoke( std::forward<Proc>( op ), e );
        }

        template<typename N>
#if PGBAR__CXX20
          requires std::is_integral_v<N>
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR slice::TrackedSpan<slice::NumericSpan<N>, Derived>
#else
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR
          typename std::enable_if<std::is_integral<N>::value,
                                  slice::TrackedSpan<slice::NumericSpan<N>, Derived>>::type
#endif
          iterate( N endpoint ) &
        {
          throw_if_active();
          return {
            { {}, endpoint, 1 },
            static_cast<Derived&>( *this )
          };
        }
        template<typename N, typename Proc>
#if PGBAR__CXX20
          requires std::is_integral_v<N>
        PGBAR__CXX14_CNSTXPR void
#else
        PGBAR__CXX14_CNSTXPR typename std::enable_if<std::is_integral<N>::value>::type
#endif
          iterate( N endpoint, Proc&& op )
        {
          for ( N e : iterate( endpoint ) )
            utils::invoke( std::forward<Proc>( op ), e );
        }

        // Visualize unidirectional traversal of a iterator interval defined by parameters.
        template<typename I>
#if PGBAR__CXX20
          requires traits::is_sized_iterator<I>::value
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR slice::TrackedSpan<slice::IteratorSpan<I>, Derived>
#else
        PGBAR__NODISCARD PGBAR__CXX14_CNSTXPR
          typename std::enable_if<traits::is_sized_iterator<I>::value,
                                  slice::TrackedSpan<slice::IteratorSpan<I>, Derived>>::type
#endif
          iterate( I startpoint, I endpoint ) & noexcept(
            traits::AnyOf<std::is_pointer<I>, std::is_nothrow_move_constructible<I>>::value )
        {
          throw_if_active();
          return {
            { std::move( startpoint ), std::move( endpoint ) },
            static_cast<Derived&>( *this )
          };
        }
        template<typename I, typename Proc>
#if PGBAR__CXX20
          requires traits::is_sized_iterator<I>::value
        PGBAR__CXX14_CNSTXPR void
#else
        PGBAR__CXX14_CNSTXPR typename std::enable_if<traits::is_sized_iterator<I>::value>::type
#endif
          iterate( I startpoint, I endpoint, Proc&& op )
        {
          for ( auto&& e : iterate( std::move( startpoint ), std::move( endpoint ) ) )
            utils::invoke( std::forward<Proc>( op ), std::forward<decltype( e )>( e ) );
        }

        // Visualize unidirectional traversal of a abstract range interval defined by `container`'s
        // slice.
        template<class R>
#if PGBAR__CXX20
          requires( traits::is_bounded_range<std::remove_reference_t<R>>::value
                    && !std::ranges::view<std::remove_reference_t<R>> )
        PGBAR__NODISCARD PGBAR__CXX17_CNSTXPR
          slice::TrackedSpan<slice::BoundedSpan<std::remove_reference_t<R>>, Derived>
#else
        PGBAR__NODISCARD PGBAR__CXX17_CNSTXPR typename std::enable_if<
          traits::is_bounded_range<typename std::remove_reference<R>::type>::value,
          slice::TrackedSpan<slice::BoundedSpan<typename std::remove_reference<R>::type>, Derived>>::type
#endif
          iterate( R& container ) &
        {
          throw_if_active();
          return { { container }, static_cast<Derived&>( *this ) };
        }
#if PGBAR__CXX20
        template<class R>
          requires( traits::is_bounded_range<R>::value && std::ranges::view<R> )
        PGBAR__NODISCARD PGBAR__CXX17_CNSTXPR slice::TrackedSpan<R, Derived> iterate( R view ) &
        {
          throw_if_active();
          return { std::move( view ), static_cast<Derived&>( *this ) };
        }
#endif
        template<class R, typename Proc>
#if PGBAR__CXX20
          requires traits::is_bounded_range<std::remove_reference_t<R>>::value
        PGBAR__CXX17_CNSTXPR void
#else
        PGBAR__CXX17_CNSTXPR typename std::enable_if<
          traits::is_bounded_range<typename std::remove_reference<R>::type>::value>::type
#endif
          iterate( R&& range, Proc&& op )
        {
          for ( auto&& e : iterate( std::forward<R>( range ) ) )
            utils::invoke( std::forward<Proc>( op ), std::forward<decltype( e )>( e ) );
        }
      };

      template<typename Base, typename Derived>
      class FrameCounter : public Base {
      protected:
        types::Size idx_frame_;

      public:
        using Base::Base;
        constexpr FrameCounter() = default;
        constexpr FrameCounter( FrameCounter&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        PGBAR__CXX14_CNSTXPR FrameCounter& operator=( FrameCounter&& rhs ) & noexcept
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        PGBAR__CXX20_CNSTXPR ~FrameCounter() = default;
      };

      template<typename Base, typename Derived>
      class CoreBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename Soul,
               Channel Outlet,
               Policy Mode,
               Region Area>
      class CoreBar<Base, Derived<Soul, Outlet, Mode, Area>> : public Base {
        static_assert( traits::is_config<Soul>::value,
                       "pgbar::_details::prefabs::CoreBar: Invalid config type" );
        using Self   = CoreBar;
        using Subcls = Derived<Soul, Outlet, Mode, Area>;

        PGBAR__INLINE_FN void make_frame()
        {
          switch ( static_cast<Subcls*>( this )->categorize() ) {
          case StateCategory::Awake: {
            static_cast<Subcls*>( this )->startframe();
          } break;
          case StateCategory::Refresh: {
            static_cast<Subcls*>( this )->refreshframe();
          } break;
          case StateCategory::Finish: {
            static_cast<Subcls*>( this )->endframe();
          } break;
          default: return;
          }
        }
        PGBAR__INLINE_FN friend void make_frame( Self& self ) { self.make_frame(); }

      protected:
        enum class StateCategory : std::uint8_t { Stop, Awake, Refresh, Finish };

        render::Builder<Soul> config_;
        mutable std::mutex mtx_;

        std::chrono::steady_clock::time_point zero_point_;

        // An extension point that performs global resource cleanup related to the progress bar semantics
        // themselves.
        virtual void do_halt( bool forced = false ) noexcept
        {
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          PGBAR__ASSERT( executor.empty() == false );
          if ( !forced )
            executor.attempt();
          executor.dismiss_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
        }
        virtual void do_boot() & noexcept( false )
        {
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          if ( !executor.try_appoint( [this]() {
                 // No exceptions are caught here, this should be done by the thread manager.
                 auto& ostream    = io::OStream<Outlet>::itself();
                 const auto istty = console::TermContext<Outlet>::itself().connected();
                 switch ( static_cast<Subcls*>( this )->categorize() ) {
                 case StateCategory::Awake: {
                   if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
                     if ( istty )
                       ostream << console::escodes::savecursor;
                   static_cast<Subcls*>( this )->startframe();
                   ostream << console::escodes::nextline;
                   ostream << io::flush;
                 } break;
                 case StateCategory::Refresh: {
                   if ( istty ) {
                     if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
                       ostream << console::escodes::resetcursor;
                     else
                       ostream << console::escodes::prevline << console::escodes::linestart
                               << console::escodes::linewipe;
                   }
                   static_cast<Subcls*>( this )->refreshframe();
                   ostream << console::escodes::nextline;
                   ostream << io::flush;
                 } break;
                 case StateCategory::Finish: {
                   if ( istty ) {
                     if PGBAR__CXX17_CNSTXPR ( Area == Region::Fixed )
                       ostream << console::escodes::resetcursor;
                     else
                       ostream << console::escodes::prevline << console::escodes::linestart
                               << console::escodes::linewipe;
                   }
                   static_cast<Subcls*>( this )->endframe();
                   if ( istty && config::hide_completed() )
                     ostream << console::escodes::linestart << console::escodes::linewipe;
                   else
                     ostream << console::escodes::nextline;
                   ostream << io::flush;
                 } break;
                 default: return;
                 }
               } ) )
            PGBAR__UNLIKELY throw exception::InvalidState(
              "pgbar: another progress bar instance is already running" );

          io::OStream<Outlet>::itself() << io::release; // reset the state.
          try {
            executor.activate();
          } catch ( ... ) {
            executor.dismiss();
            throw;
          }
        }

      public:
        CoreBar( const Self& )           = delete;
        Self& operator=( const Self& ) & = delete;

        CoreBar( Soul&& config ) noexcept : config_ { std::move( config ) } {}
        CoreBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) ), config_ { std::move( rhs.config_ ) }
        {}
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        {
          config_ = std::move( rhs.config_ );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        ~CoreBar() noexcept = default;

        PGBAR__NODISCARD PGBAR__INLINE_FN bool active() const noexcept final
        {
          return static_cast<const Subcls*>( this )->categorize() != StateCategory::Stop;
        }
        void reset() final
        {
          std::lock_guard<std::mutex> lock { mtx_ };
          static_cast<Subcls*>( this )->template do_reset<false>();
          PGBAR__ASSERT( active() == false );
        }
        void abort() noexcept final
        {
          std::lock_guard<std::mutex> lock { mtx_ };
          static_cast<Subcls*>( this )->template do_reset<true>();
          PGBAR__ASSERT( active() == false );
        }

        PGBAR__INLINE_FN Soul& config() & noexcept { return config_; }
        PGBAR__INLINE_FN const Soul& config() const& noexcept { return config_; }
        PGBAR__INLINE_FN Soul&& config() && noexcept { return std::move( config_ ); }

        PGBAR__CXX20_CNSTXPR void swap( CoreBar& lhs ) noexcept
        {
          Base::swap( lhs );
          config_.swap( lhs.config_ );
        }
      };

      template<typename Base, typename Derived>
      class ReactiveBar : public Base {
        using Self = ReactiveBar;

        union Callback {
          wrappers::UniqueFunction<void()> on_;
          wrappers::UniqueFunction<void( Derived& )> on_self_;
          std::uint8_t nil_;

          constexpr Callback() noexcept : nil_ {} {}
          PGBAR__CXX23_CNSTXPR ~Callback() noexcept {}
        } hook_;
        enum class Tag : std::uint8_t { Nil, Nullary, Unary } tag_ = Tag::Nil;

        PGBAR__CXX23_CNSTXPR void destroy() noexcept
        {
          switch ( tag_ ) {
          case Tag::Nullary: utils::destruct_at( hook_.on_ ); break;
          case Tag::Unary:   utils::destruct_at( hook_.on_self_ ); break;

          case Tag::Nil: PGBAR__FALLTHROUGH;
          default:       break;
          }
          hook_.nil_ = 0;
          tag_       = Tag::Nil;
        }

        void move_to( ReactiveBar& lhs ) & noexcept
        {
          lhs.destroy();
          switch ( tag_ ) {
          case Tag::Nullary:
            new ( std::addressof( lhs.hook_.on_ ) )
              wrappers::UniqueFunction<void()>( std::move( hook_.on_ ) );
            break;
          case Tag::Unary:
            new ( std::addressof( lhs.hook_.on_self_ ) )
              wrappers::UniqueFunction<void( Derived& )>( std::move( hook_.on_self_ ) );
            break;

          case Tag::Nil: PGBAR__FALLTHROUGH;
          default:       break;
          }
          lhs.tag_ = tag_;
          destroy();
        }

      protected:
        PGBAR__INLINE_FN void react() &
        {
          switch ( tag_ ) {
          case Tag::Nullary: hook_.on_(); break;
          case Tag::Unary:   hook_.on_self_( static_cast<Derived&>( *this ) ); break;

          case Tag::Nil: PGBAR__FALLTHROUGH;
          default:       break;
          }
        }

      public:
        ReactiveBar( const Self& )       = delete;
        Self& operator=( const Self& ) & = delete;

        using Base::Base;
        ReactiveBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) )
        {
          rhs.move_to( *this );
        }
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        { // The thread insecurity here is deliberately designed.
          // Because for a move-only type, transferring ownership simultaneously
          // in multiple locations should not occur.
          Base::operator=( std::move( rhs ) );
          rhs.move_to( *this );
          return *this;
        }
        PGBAR__CXX23_CNSTXPR ~ReactiveBar() noexcept { destroy(); }

        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && std::is_constructible_v<wrappers::UniqueFunction<void()>, F> )
        Derived&
#else
        typename std::enable_if<
          traits::AllOf<traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
                        std::is_constructible<wrappers::UniqueFunction<void()>, F>>::value,
          Derived&>::type
#endif
          action( F&& fn ) & noexcept(
            std::is_nothrow_constructible<wrappers::UniqueFunction<void()>, F>::value )
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          new ( std::addressof( hook_.on_ ) ) wrappers::UniqueFunction<void()>( std::forward<F>( fn ) );
          tag_ = Tag::Nullary;
          return static_cast<Derived&>( *this );
        }
        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && std::is_constructible_v<wrappers::UniqueFunction<void( Derived& )>, F> )
        Derived&
#else
        typename std::enable_if<
          traits::AllOf<traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
                        std::is_constructible<wrappers::UniqueFunction<void( Derived& )>, F>>::value,
          Derived&>::type
#endif
          action( F&& fn ) & noexcept(
            std::is_nothrow_constructible<wrappers::UniqueFunction<void( Derived& )>, F>::value )
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          new ( std::addressof( hook_.on_self_ ) )
            wrappers::UniqueFunction<void( Derived& )>( std::forward<F>( fn ) );
          tag_ = Tag::Unary;
          return static_cast<Derived&>( *this );
        }
        Derived& action() noexcept
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          destroy();
          return static_cast<Derived&>( *this );
        }

        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && (std::is_constructible_v<wrappers::UniqueFunction<void()>, F>
                        || std::is_constructible_v<wrappers::UniqueFunction<void( Derived& )>, F>))
        PGBAR__INLINE_FN friend Derived&
#else
        PGBAR__INLINE_FN friend typename std::enable_if<
          traits::AllOf<
            traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
            traits::AnyOf<std::is_constructible<wrappers::UniqueFunction<void()>, F>,
                          std::is_constructible<wrappers::UniqueFunction<void( Derived& )>, F>>>::value,
          Derived&>::type
#endif
          operator|=( Self& bar, F&& fn ) noexcept( noexcept( bar.action( std::forward<F>( fn ) ) ) )
        {
          return bar.action( std::forward<F>( fn ) );
        }
        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && (std::is_constructible_v<wrappers::UniqueFunction<void()>, F>
                        || std::is_constructible_v<wrappers::UniqueFunction<void( Derived& )>, F>))
        PGBAR__INLINE_FN friend Derived&
#else
        PGBAR__INLINE_FN friend typename std::enable_if<
          traits::AllOf<
            traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
            traits::AnyOf<std::is_constructible<wrappers::UniqueFunction<void()>, F>,
                          std::is_constructible<wrappers::UniqueFunction<void( Derived& )>, F>>>::value,
          Derived&>::type
#endif
          operator|( Self& bar, F&& fn ) noexcept( noexcept( bar.action( std::forward<F>( fn ) ) ) )
        {
          return bar.action( std::forward<F>( fn ) );
        }
        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && (std::is_constructible_v<wrappers::UniqueFunction<void()>, F>
                        || std::is_constructible_v<wrappers::UniqueFunction<void( Derived& )>, F>))
        PGBAR__INLINE_FN friend Derived&
#else
        PGBAR__INLINE_FN friend typename std::enable_if<
          traits::AllOf<
            traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
            traits::AnyOf<std::is_constructible<wrappers::UniqueFunction<void()>, F>,
                          std::is_constructible<wrappers::UniqueFunction<void( Derived& )>, F>>>::value,
          Derived&>::type
#endif
          operator|( F&& fn, Self& bar ) noexcept( noexcept( bar.action( std::forward<F>( fn ) ) ) )
        {
          return bar.action( std::forward<F>( fn ) );
        }
        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && (std::is_constructible_v<wrappers::UniqueFunction<void()>, F>
                        || std::is_constructible_v<wrappers::UniqueFunction<void( Derived& )>, F>))
        PGBAR__INLINE_FN friend Derived&&
#else
        PGBAR__INLINE_FN friend typename std::enable_if<
          traits::AllOf<
            traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
            traits::AnyOf<std::is_constructible<wrappers::UniqueFunction<void()>, F>,
                          std::is_constructible<wrappers::UniqueFunction<void( Derived& )>, F>>>::value,
          Derived&&>::type
#endif
          operator|( Self&& bar, F&& fn ) noexcept( noexcept( bar.action( std::forward<F>( fn ) ) ) )
        {
          return std::move( bar.action( std::forward<F>( fn ) ) );
        }
        template<typename F>
#if PGBAR__CXX20
          requires( !std::is_null_pointer_v<std::decay_t<F>>
                    && (std::is_constructible_v<wrappers::UniqueFunction<void()>, F>
                        || std::is_constructible_v<wrappers::UniqueFunction<void( Derived& )>, F>))
        PGBAR__INLINE_FN friend Derived&&
#else
        PGBAR__INLINE_FN friend typename std::enable_if<
          traits::AllOf<
            traits::Not<std::is_same<typename std::decay<F>::type, std::nullptr_t>>,
            traits::AnyOf<std::is_constructible<wrappers::UniqueFunction<void()>, F>,
                          std::is_constructible<wrappers::UniqueFunction<void( Derived& )>, F>>>::value,
          Derived&&>::type
#endif
          operator|( F&& fn, Self&& bar ) noexcept( noexcept( bar.action( std::forward<F>( fn ) ) ) )
        {
          return std::move( bar.action( std::forward<F>( fn ) ) );
        }

        PGBAR__INLINE_FN friend Derived& operator|=( Self& bar, std::nullptr_t ) noexcept
        {
          return bar.action();
        }
        PGBAR__INLINE_FN friend Derived& operator|( Self& bar, std::nullptr_t ) noexcept
        {
          return bar.action();
        }
        PGBAR__INLINE_FN friend Derived& operator|( std::nullptr_t, Self& bar ) noexcept
        {
          return bar.action();
        }
        PGBAR__INLINE_FN friend Derived&& operator|( Self&& bar, std::nullptr_t ) noexcept
        {
          return std::move( bar.action() );
        }
        PGBAR__INLINE_FN friend Derived&& operator|( std::nullptr_t, Self&& bar ) noexcept
        {
          return std::move( bar.action() );
        }

        void swap( Self& lhs ) noexcept
        {
          Base::swap( lhs );
          switch ( tag_ ) {
          case Tag::Nullary:
            if ( lhs.tag_ == Tag::Nullary )
              hook_.on_.swap( lhs.hook_.on_ );
            else {
              wrappers::UniqueFunction<void()> tmp { std::move( hook_.on_ ) };
              lhs.move_to( *this );
              new ( std::addressof( lhs.hook_.on_ ) ) wrappers::UniqueFunction<void()>( std::move( tmp ) );
              lhs.tag_ = Tag::Nullary;
            }
            break;

          case Tag::Unary:
            if ( lhs.tag_ == Tag::Unary )
              hook_.on_self_.swap( lhs.hook_.on_self_ );
            else {
              wrappers::UniqueFunction<void( Derived& )> tmp { std::move( hook_.on_self_ ) };
              lhs.move_to( *this );
              new ( std::addressof( lhs.hook_.on_self_ ) )
                wrappers::UniqueFunction<void( Derived& )>( std::move( tmp ) );
              lhs.tag_ = Tag::Unary;
            }
            break;

          case Tag::Nil: PGBAR__FALLTHROUGH;
          default:
            if ( lhs.tag_ != Tag::Nil )
              lhs.move_to( *this );
            break;
          }
        }
      };

      template<typename Base, typename Derived>
      class TickableBar : public Base {
        using Self = TickableBar;

      public:
        using Base::Base;
        constexpr TickableBar( Self&& rhs )                  = default;
        PGBAR__CXX14_CNSTXPR Self& operator=( Self&& rhs ) & = default;
        PGBAR__CXX20_CNSTXPR virtual ~TickableBar()          = default;

        PGBAR__INLINE_FN void tick() &
        {
          static_cast<Derived*>( this )->do_tick(
            [this]() noexcept { this->task_cnt_.fetch_add( 1, std::memory_order_release ); } );
        }
        PGBAR__INLINE_FN void tick( std::uint64_t next_step ) &
        {
          static_cast<Derived*>( this )->do_tick( [&]() noexcept {
            const auto task_cnt = this->task_cnt_.load( std::memory_order_acquire );
            this->task_cnt_.fetch_add( task_cnt + next_step > this->task_end_ ? this->task_end_ - task_cnt
                                                                              : next_step );
          } );
        }
        /**
         * Set the iteration step of the progress bar to a specified percentage.
         * Ignore the call if the iteration count exceeds the given percentage.
         * If `percentage` is bigger than 100, it will be set to 100.
         *
         * @param percentage Value range: [0, 100].
         */
        PGBAR__INLINE_FN void tick_to( std::uint8_t percentage ) &
        {
          static_cast<Derived*>( this )->do_tick( [&]() noexcept {
            auto updater = [this]( std::uint64_t target ) noexcept {
              auto current = this->task_cnt_.load( std::memory_order_acquire );
              while ( !this->task_cnt_.compare_exchange_weak( current,
                                                              target,
                                                              std::memory_order_release,
                                                              std::memory_order_acquire )
                      && target <= current ) {}
            };
            if ( percentage <= 100 ) {
              const auto target = static_cast<std::uint64_t>( this->task_end_ * percentage * 0.01 );
              PGBAR__ASSERT( target <= this->task_end_ );
              updater( target );
            } else
              updater( this->task_end_ );
          } );
        }
      };

      template<typename Base, typename Derived>
      class PlainBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename Soul,
               Channel Outlet,
               Policy Mode,
               Region A>
      class PlainBar<Base, Derived<Soul, Outlet, Mode, A>> : public Base {
        using Self = PlainBar;
        template<typename, typename>
        friend class CoreBar;
        template<typename, typename>
        friend class TickableBar;

        using State = typename Base::StateCategory;
        std::atomic<State> state_;

      protected:
        PGBAR__INLINE_FN void startframe() &
        {
          refreshframe();
          auto expected = State::Awake;
          this->state_.compare_exchange_strong( expected, State::Refresh, std::memory_order_release );
        }
        PGBAR__INLINE_FN void refreshframe() &
        {
          PGBAR__ASSERT( this->task_cnt_ <= this->task_end_ );
          this->config_.build( io::OStream<Outlet>::itself(),
                               this->task_cnt_.load( std::memory_order_acquire ),
                               this->task_end_,
                               this->zero_point_ );
        }
        PGBAR__INLINE_FN void endframe() &
        {
          refreshframe();
          this->state_.store( State::Stop, std::memory_order_release );
        }

        PGBAR__INLINE_FN typename Base::StateCategory categorize() const noexcept
        {
          return state_.load( std::memory_order_acquire );
        }

        // Only when "forced" is true will it be noexcept.
        template<bool Forced>
        PGBAR__INLINE_FN void do_reset() noexcept( Forced )
        {
          if ( state_.load( std::memory_order_acquire ) != State::Stop ) {
            if PGBAR__CXX17_CNSTXPR ( Forced )
              state_.store( State::Stop, std::memory_order_release );
            else {
              this->react();
              state_.store( State::Finish, std::memory_order_release );
            }
            this->do_halt( Forced );
          } else
            state_.store( State::Stop, std::memory_order_release );
        }
        template<typename F>
        void do_tick( F&& ticker ) & noexcept( false )
        {
          switch ( state_.load( std::memory_order_acquire ) ) {
          case State::Stop:  PGBAR__FALLTHROUGH;
          case State::Awake: {
            std::lock_guard<std::mutex> lock { this->mtx_ };
            if ( state_.load( std::memory_order_acquire ) == State::Stop ) {
              this->task_end_ = this->config_.tasks();
              if ( this->task_end_ == 0 )
                PGBAR__UNLIKELY throw exception::InvalidState( "pgbar: the number of tasks is zero" );

              if ( config::disable_styling() && !config::intty( Outlet ) )
                this->config_.colored( false ).bolded( false );
              this->task_cnt_.store( 0, std::memory_order_release );
              this->zero_point_ = std::chrono::steady_clock::now();
              state_.store( State::Awake, std::memory_order_release );
              try {
                this->do_boot();
              } catch ( ... ) {
                state_.store( State::Stop, std::memory_order_release );
                throw;
              }
            }
          }
            PGBAR__FALLTHROUGH;
          case State::Refresh: {
            ticker();

            if ( this->task_cnt_.load( std::memory_order_acquire ) >= this->task_end_ )
              PGBAR__UNLIKELY
              {
                if ( this->mtx_.try_lock() ) {
                  std::lock_guard<std::mutex> lock { this->mtx_, std::adopt_lock };
                  do_reset<false>();
                }
              }
            else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync )
              render::Renderer<Outlet, Mode>::itself().execute();
          } break;

          default: PGBAR__UNREACHABLE;
          }
        }

      public:
        PlainBar( const Self& )          = delete;
        Self& operator=( const Self& ) & = delete;

        PlainBar( Soul&& config ) noexcept( std::is_nothrow_constructible<Base, Soul&&>::value )
          : Base( std::move( config ) ), state_ { State::Stop }
        {}
        PlainBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) ), state_ { State::Stop }
        {}
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        ~PlainBar() noexcept
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          do_reset<true>();
        }
      };

      template<typename Base, typename Derived>
      class FrameBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename Soul,
               Channel Outlet,
               Policy Mode,
               Region A>
      class FrameBar<Base, Derived<Soul, Outlet, Mode, A>> : public Base {
        using Self   = FrameBar;
        using Subcls = Derived<Soul, Outlet, Mode, A>;
        template<typename, typename>
        friend class CoreBar;
        template<typename, typename>
        friend class TickableBar;

        enum class State : std::uint8_t { Stop, Awake, ProgressRefresh, ActivityRefresh, Finish };
        std::atomic<State> state_;

      protected:
        PGBAR__INLINE_FN void startframe() &
        {
          this->idx_frame_ = 0;
          refreshframe();
          auto expected = State::Awake;
          state_.compare_exchange_strong( expected,
                                          this->task_end_ == 0 ? State::ActivityRefresh
                                                               : State::ProgressRefresh,
                                          std::memory_order_release );
        }
        PGBAR__INLINE_FN void refreshframe() &
        {
          PGBAR__ASSERT( this->task_cnt_ <= this->task_end_ );
          this->config_.build( io::OStream<Outlet>::itself(),
                               this->idx_frame_,
                               this->task_cnt_.load( std::memory_order_acquire ),
                               this->task_end_,
                               this->zero_point_ );
          ++this->idx_frame_;
        }
        PGBAR__INLINE_FN void endframe() &
        {
          PGBAR__ASSERT( this->task_cnt_ <= this->task_end_ );
          this->config_.build( io::OStream<Outlet>::itself(),
                               this->idx_frame_,
                               this->task_cnt_.load( std::memory_order_acquire ),
                               this->task_end_,
                               this->zero_point_ );
          state_.store( State::Stop, std::memory_order_release );
        }

        PGBAR__INLINE_FN typename Base::StateCategory categorize() const noexcept
        {
          switch ( state_.load( std::memory_order_acquire ) ) {
          case State::Awake:           return Base::StateCategory::Awake;
          case State::ProgressRefresh: PGBAR__FALLTHROUGH;
          case State::ActivityRefresh: return Base::StateCategory::Refresh;
          case State::Finish:          return Base::StateCategory::Finish;
          default:                     break;
          }
          return Base::StateCategory::Stop;
        }

        // Only when "forced" is true will it be noexcept.
        template<bool Forced>
        PGBAR__INLINE_FN void do_reset() noexcept( Forced )
        {
          if ( state_.load( std::memory_order_acquire ) != State::Stop ) {
            if PGBAR__CXX17_CNSTXPR ( Forced )
              state_.store( State::Stop, std::memory_order_release );
            else {
              this->react();
              state_.store( State::Finish, std::memory_order_release );
            }
            this->do_halt( Forced );
          } else
            state_.store( State::Stop, std::memory_order_release );
        }
        template<typename F>
        void do_tick( F&& ticker ) & noexcept( false )
        {
          switch ( this->state_.load( std::memory_order_acquire ) ) {
          case State::Stop:  PGBAR__FALLTHROUGH;
          case State::Awake: {
            std::lock_guard<std::mutex> lock { this->mtx_ };
            if ( this->state_.load( std::memory_order_acquire ) == State::Stop ) {
              this->task_end_ = this->config_.tasks();
              static_cast<Subcls*>( this )->warmup();

              if ( config::disable_styling() && !config::intty( Outlet ) )
                this->config_.colored( false ).bolded( false );
              this->task_cnt_.store( 0, std::memory_order_release );
              this->zero_point_ = std::chrono::steady_clock::now();
              this->state_.store( State::Awake, std::memory_order_release );
              try {
                this->do_boot();
              } catch ( ... ) {
                this->state_.store( State::Stop, std::memory_order_release );
                throw;
              }
            }
            if ( this->state_.load( std::memory_order_acquire ) == State::ActivityRefresh )
              return;
          }
            PGBAR__FALLTHROUGH;
          case State::ProgressRefresh: {
            ticker();

            if ( this->task_cnt_.load( std::memory_order_acquire ) >= this->task_end_ )
              PGBAR__UNLIKELY
              {
                if ( this->mtx_.try_lock() ) {
                  std::lock_guard<std::mutex> lock { this->mtx_, std::adopt_lock };
                  do_reset<false>();
                }
              }
            else if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync )
              render::Renderer<Outlet, Mode>::itself().execute();
          } break;

          case State::ActivityRefresh: {
            if PGBAR__CXX17_CNSTXPR ( Mode == Policy::Sync )
              render::Renderer<Outlet, Mode>::itself().execute();
          } break;

          default: PGBAR__UNREACHABLE;
          }
        }

      public:
        FrameBar( const Self& )          = delete;
        Self& operator=( const Self& ) & = delete;

        FrameBar( Soul&& config ) noexcept( std::is_nothrow_constructible<Base, Soul&&>::value )
          : Base( std::move( config ) ), state_ { State::Stop }
        {}
        FrameBar( Self&& rhs ) noexcept( std::is_nothrow_move_constructible<Base>::value )
          : Base( std::move( rhs ) ), state_ { State::Stop }
        {}
        Self& operator=( Self&& rhs ) & noexcept( std::is_nothrow_move_assignable<Base>::value )
        {
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        ~FrameBar() noexcept
        {
          std::lock_guard<std::mutex> lock { this->mtx_ };
          do_reset<true>();
        }
      };

      template<typename Base, typename Derived>
      class BoundedFrameBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename S,
               Channel O,
               Policy M,
               Region A>
      class BoundedFrameBar<Base, Derived<S, O, M, A>> : public Base {
        friend Base;

        void warmup() &
        {
          if ( this->task_end_ == 0 )
            PGBAR__UNLIKELY throw exception::InvalidState( "pgbar: the number of tasks is zero" );
        }

      public:
        using Base::Base;
        constexpr BoundedFrameBar( BoundedFrameBar&& )                         = default;
        PGBAR__CXX14_CNSTXPR BoundedFrameBar& operator=( BoundedFrameBar&& ) & = default;
        PGBAR__CXX20_CNSTXPR ~BoundedFrameBar()                                = default;
      };

      template<typename Base, typename Derived>
      class NullableFrameBar;
      template<typename Base,
               template<typename, Channel, Policy, Region> class Derived,
               typename S,
               Channel O,
               Policy M,
               Region A>
      class NullableFrameBar<Base, Derived<S, O, M, A>> : public Base {
        friend Base;

        void warmup() & {}

      public:
        using Base::Base;
        constexpr NullableFrameBar( NullableFrameBar&& )                         = default;
        PGBAR__CXX14_CNSTXPR NullableFrameBar& operator=( NullableFrameBar&& ) & = default;
        PGBAR__CXX20_CNSTXPR ~NullableFrameBar()                                 = default;
      };
    } // namespace assets

    namespace traits {
      PGBAR__INHERIT_REGISTER( assets::ReactiveBar, assets::CoreBar );
      PGBAR__INHERIT_REGISTER( assets::TickableBar, assets::TaskCounter, assets::CoreBar );
      PGBAR__INHERIT_REGISTER( assets::PlainBar, assets::TickableBar, assets::ReactiveBar );
      PGBAR__INHERIT_REGISTER( assets::FrameBar,
                               assets::FrameCounter,
                               assets::ReactiveBar,
                               assets::TickableBar );
      PGBAR__INHERIT_REGISTER( assets::BoundedFrameBar, assets::FrameBar );
      PGBAR__INHERIT_REGISTER( assets::NullableFrameBar, assets::FrameBar );

      // In pgbar, the configuration type is "first-class type";
      // thus, we hope to be able to automatically generate the progress bar type
      // by specifying the configuration type for a template class.
      // This's why we should define an additional tool type here.
      template<typename Config>
      struct BehaviourFor {
        using type = C3Container<>;
      };
      template<typename Config>
      using BehaviourFor_t = typename BehaviourFor<Config>::type;

#define PGBAR__BIND_BEHAVIOUR( Config, ... ) \
  template<>                                 \
  struct BehaviourFor<Config> {              \
    using type = C3Container<__VA_ARGS__>;   \
  }
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
