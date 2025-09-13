#ifndef PGBAR_DYNAMICBAR
#define PGBAR_DYNAMICBAR

#include "details/prefabs/ManagedBar.hpp"
#include <tuple>

namespace pgbar {
  namespace __details {
    namespace assets {
      template<Channel Outlet, Policy Mode, Region Area>
      class DynamicContext final {
        struct Slot final {
        private:
          using Self = Slot;

          template<typename Derived>
          static void halt( Indicator* item ) noexcept
          {
            static_assert(
              traits::AllOf<std::is_base_of<Indicator, Derived>, traits::is_bar<Derived>>::value,
              "pgbar::__details::assets::DynamicContext::Slot::halt: Derived must inherit from Indicator" );
            __PGBAR_TRUST( item != nullptr );
            static_cast<Derived*>( item )->halt();
          }
          template<typename Derived>
          static void render( Indicator* item )
          {
            static_assert(
              traits::AllOf<std::is_base_of<Indicator, Derived>, traits::is_bar<Derived>>::value,
              "pgbar::__details::assets::DynamicContext::Slot::render: Derived must inherit from Indicator" );
            __PGBAR_TRUST( item != nullptr );
            make_frame( static_cast<Derived&>( *item ) );
          }

        public:
          void ( *halt_ )( Indicator* ) noexcept;
          void ( *render_ )( Indicator* );
          Indicator* target_;

          template<typename Config>
          Slot( prefabs::ManagedBar<Config, Outlet, Mode, Area>* item ) noexcept
            : halt_ { halt<prefabs::ManagedBar<Config, Outlet, Mode, Area>> }
            , render_ { render<prefabs::BasicBar<Config, Outlet, Mode, Area>> }
            , target_ { item }
          {}
        };

        enum class State : std::uint8_t { Stop, Awake, Refresh };
        std::atomic<State> state_;
        std::vector<Slot> items_;

        // If Area is equal to Region::Fixed,
        // the variable represents the number of lines that need to be discarded;
        // If Area is equal to Region::Relative,
        // the variable represents the number of nextlines output last time.
        std::atomic<types::Size> num_modified_lines_;
        mutable concurrent::SharedMutex res_mtx_;
        mutable std::mutex sched_mtx_;

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
            if __PGBAR_CXX17_CNSTXPR ( Area == Region::Relative )
              if ( !any_rendered && !this_rendered )
                continue;
            if ( ( !istty && this_rendered )
                 || ( istty && ( !hide_done || items_[i].target_ != nullptr ) ) ) {
              ostream << console::escodes::nextline;
              if __PGBAR_CXX17_CNSTXPR ( Area == Region::Relative )
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
          items_.erase( items_.cbegin(), itr );
          if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed )
            num_modified_lines_.fetch_add( std::distance( items_.cbegin(), itr ), std::memory_order_release );
        }

      public:
        DynamicContext() noexcept : state_ { State::Stop } {}
        DynamicContext( const DynamicContext& )              = delete;
        DynamicContext& operator=( const DynamicContext& ) & = delete;
        ~DynamicContext() noexcept { halt(); }

        void halt() noexcept
        {
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          if ( state_.load( std::memory_order_acquire ) != State::Stop )
            render::Renderer<Outlet, Mode>::itself().appoint();
          state_.store( State::Stop, std::memory_order_release );

          std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
          for ( types::Size i = 0; i < items_.size(); ++i ) {
            if ( items_[i].target_ != nullptr ) {
              __PGBAR_ASSERT( items_[i].halt_ != nullptr );
              ( *items_[i].halt_ )( items_[i].target_ );
            }
          }
          items_.clear();
        }

        template<typename C>
        void append( prefabs::ManagedBar<C, Outlet, Mode, Area>* item ) & noexcept( false )
        {
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          auto& executor     = render::Renderer<Outlet, Mode>::itself();
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
                     if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed ) {
                       if ( istty )
                         ostream << console::escodes::savecursor;
                     }
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
                         if __PGBAR_CXX17_CNSTXPR ( Area == Region::Fixed ) {
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
              __PGBAR_UNLIKELY throw exception::InvalidState(
                "pgbar: another progress bar instance is already running" );

            io::OStream<Outlet>::itself() << io::release;
            num_modified_lines_.store( 0, std::memory_order_relaxed );
            state_.store( State::Awake, std::memory_order_release );
            try {
              {
                std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
                items_.emplace_back( item );
              }
              executor.activate();
            } catch ( ... ) {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              items_.clear();
              state_.store( State::Stop, std::memory_order_release );
              throw;
            }
          } else {
            {
              std::lock_guard<concurrent::SharedMutex> lock2 { res_mtx_ };
              eliminate();
              items_.emplace_back( item );
            }
            executor.attempt();
          }
        }
        void pop( const Indicator* item, bool forced = false ) noexcept
        {
          auto& executor = render::Renderer<Outlet, Mode>::itself();
          __PGBAR_ASSERT( executor.empty() == false );
          std::lock_guard<std::mutex> lock1 { sched_mtx_ };
          __PGBAR_ASSERT( size() != 0 );
          if ( !forced )
            executor.attempt();

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
            executor.appoint_then( []() noexcept { io::OStream<Outlet>::itself().release(); } );
          }
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size size() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { res_mtx_ };
          return items_.size();
        }
      };
    } // namespace assets
  } // namespace __details

  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  class DynamicBar {
    using Self    = DynamicBar;
    using Context = __details::assets::DynamicContext<Outlet, Mode, Area>;

    std::shared_ptr<Context> core_;

  public:
    DynamicBar()                     = default;
    DynamicBar( const Self& )        = delete;
    Self& operator=( const Self& ) & = delete;
    DynamicBar( Self&& )             = default;
    Self& operator=( Self&& ) &      = default;
    ~DynamicBar()                    = default;

    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
    {
      return core_ != nullptr && core_->size() != 0;
    }
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size size() const noexcept
    {
      return core_ != nullptr ? core_.use_count() - 1 : 0;
    }
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size active_size() const noexcept
    {
      return core_ != nullptr ? core_->size() : 0;
    }
    __PGBAR_INLINE_FN void reset() noexcept
    {
      if ( core_ != nullptr )
        core_->halt();
    }

    // Wait until the indicator is Stop.
    void wait() const noexcept
    {
      __details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for the indicator is Stop or timed out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return __details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }

    template<typename Config>
#if __PGBAR_CXX20
      requires __details::traits::is_config<Config>::value
    __PGBAR_NODISCARD std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
#else
    __PGBAR_NODISCARD
      typename std::enable_if<__details::traits::is_config<Config>::value,
                              std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>::type
#endif
      insert( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<__details::prefabs::ManagedBar<Config, Outlet, Mode, Area>>(
        core_,
        std::move( bar ) );
    }
    template<typename Config>
#if __PGBAR_CXX20
      requires __details::traits::is_config<Config>::value
    __PGBAR_NODISCARD std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
#else
    __PGBAR_NODISCARD
      typename std::enable_if<__details::traits::is_config<Config>::value,
                              std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>::type
#endif
      insert( Config cfg )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<__details::prefabs::ManagedBar<Config, Outlet, Mode, Area>>(
        core_,
        std::move( cfg ) );
    }

    template<typename Bar, typename... Options>
#if __PGBAR_CXX20
      requires( __details::traits::is_bar<Bar>::value && Bar::Sink == Outlet && Bar::Strategy == Mode
                && Bar::Layout == Area && std::is_constructible_v<Bar, Options...> )
    __PGBAR_NODISCARD std::unique_ptr<Bar>
#else
    __PGBAR_NODISCARD typename std::enable_if<
      __details::traits::AllOf<__details::traits::is_bar<Bar>,
                               std::is_constructible<Bar, Options...>,
                               __details::traits::BoolConstant<( Bar::Sink == Outlet )>,
                               __details::traits::BoolConstant<( Bar::Strategy == Mode )>,
                               __details::traits::BoolConstant<( Bar::Layout == Area )>>::value,
      std::unique_ptr<Bar>>::type
#endif
      insert( Options&&... options )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<
        __details::prefabs::ManagedBar<typename Bar::Config, Outlet, Mode, Area>>(
        core_,
        std::forward<Options>( options )... );
    }
    template<typename Config, typename... Options>
#if __PGBAR_CXX20
      requires( __details::traits::is_config<Config>::value && std::is_constructible_v<Config, Options...> )
    __PGBAR_NODISCARD std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
#else
    __PGBAR_NODISCARD
      typename std::enable_if<__details::traits::AllOf<__details::traits::is_config<Config>,
                                                       std::is_constructible<Config, Options...>>::value,
                              std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>::type
#endif
      insert( Options&&... options )
    {
      if ( core_ == nullptr )
        __PGBAR_UNLIKELY core_ = std::make_shared<Context>();
      return __details::utils::make_unique<__details::prefabs::ManagedBar<Config, Outlet, Mode, Area>>(
        core_,
        std::forward<Options>( options )... );
    }

    void swap( Self& lhs ) noexcept
    {
      __PGBAR_TRUST( this != &lhs );
      core_.swap( lhs.core_ );
    }
    friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }
  };

  // Creates a tuple of unique_ptr pointing to bars using existing bar instances.
  template<typename Config, typename... Configs, Channel Outlet, Policy Mode, Region Area>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
               std::unique_ptr<__details::prefabs::BasicBar<Configs, Outlet, Mode, Area>>...>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<Config>,
                             __details::traits::is_config<Configs>...>::value,
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
               std::unique_ptr<__details::prefabs::BasicBar<Configs, Outlet, Mode, Area>>...>>::type
#endif
    make_dynamic( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar,
                  __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars )
  {
    DynamicBar<Outlet, Mode, Area> factory;
    return { factory.insert( std::move( bar ) ), factory.insert( std::move( bars ) )... };
  }
  // Creates a tuple of unique_ptr pointing to bars using configuration objects.
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config,
           typename... Configs>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<std::decay_t<Config>>::value
              && ( __details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>>,
               std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Configs>, Outlet, Mode, Area>>...>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<typename std::decay<Config>::type>,
                             __details::traits::is_config<typename std::decay<Configs>::type>...>::value,
    std::tuple<
      std::unique_ptr<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>,
      std::unique_ptr<
        __details::prefabs::BasicBar<typename std::decay<Configs>::type, Outlet, Mode, Area>>...>>::type
#endif
    make_dynamic( Config&& cfg, Configs&&... cfgs )
  {
    DynamicBar<Outlet, Mode, Area> factory;
    return { factory.insert( std::forward<Config>( cfg ) ),
             factory.insert( std::forward<Configs>( cfgs ) )... };
  }

  /**
   * Creates a vector of unique_ptr pointing to bars with a fixed number of BasicBar instances.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<typename Config, Channel Outlet, Policy Mode, Region Area>
#if __PGBAR_CXX20
    requires __details::traits::is_config<Config>::value
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::is_config<Config>::value,
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>>::type
#endif
    make_dynamic( __details::prefabs::BasicBar<Config, Outlet, Mode, Area>&& bar,
                  __details::types::Size count )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>> products;
    std::generate_n( std::back_inserter( products ), count - 1, [&factory, &bar]() {
      return factory.insert( bar.config() );
    } );
    products.emplace_back( factory.insert( std::move( bar ) ) );
    return products;
  }
  /**
   * Creates a vector of unique_ptr pointing to bars with a fixed number of BasicBar instances.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config>
#if __PGBAR_CXX20
    requires __details::traits::is_config<std::decay_t<Config>>::value
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>>>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::is_config<typename std::decay<Config>::type>::value,
    std::vector<std::unique_ptr<
      __details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>>>::type
#endif
    make_dynamic( Config&& cfg, __details::types::Size count )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<
      std::unique_ptr<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>>>
      products;
    std::generate_n( std::back_inserter( products ), count - 1, [&factory, &cfg]() {
      return factory.insert( cfg );
    } );
    products.emplace_back( factory.insert( std::forward<Config>( cfg ) ) );
    return products;
  }

  /**
   * Creates a vector of unique_ptr with a fixed number of bars using mutiple bar/configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided arguments;
   * **An unmatched count and Bars number will cause an exception `pgbar::exception::InvalidArgument`.**
   */
  template<typename Bar, typename... Objs>
#if __PGBAR_CXX20
    requires( __details::traits::is_bar<Bar>::value
              && ( ( ( std::is_same_v<std::remove_cv_t<Bar>, std::remove_cv_t<Objs>> && ... )
                     && !( std::is_lvalue_reference_v<Objs> || ... ) )
                   || ( std::is_same<typename Bar::Config, std::decay_t<Objs>>::value && ... ) ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN std::vector<std::unique_ptr<Bar>>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<
      __details::traits::is_bar<Bar>,
      __details::traits::AnyOf<
        __details::traits::AllOf<
          std::is_same<typename std::remove_cv<Bar>::type, typename std::remove_cv<Objs>::type>...,
          __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Objs>...>>>,
        __details::traits::AllOf<std::is_same<typename Bar::Config, typename std::decay<Objs>::type>...>>>::
      value,
    std::vector<std::unique_ptr<Bar>>>::type
#endif
    make_dynamic( __details::types::Size count, Objs&&... objs ) noexcept( false )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    else if ( count < sizeof...( Objs ) )
      __PGBAR_UNLIKELY throw exception::InvalidArgument(
        "pgbar: the number of provided objects exceeds the specified count" );

    DynamicBar<Bar::Sink, Bar::Strategy, Bar::Layout> factory;
    std::vector<std::unique_ptr<Bar>> products;
    (void)std::initializer_list<bool> {
      ( products.emplace_back( factory.insert( std::forward<Objs>( objs ) ) ), false )...
    };
    std::generate_n( std::back_inserter( products ), count - sizeof...( Objs ), [&factory]() {
      return factory.template insert<Bar>();
    } );
    return products;
  }
  /**
   * Creates a vector of unique_ptr with a fixed number of BasicBar instances using multiple configurations.
   * The ctor sequentially initializes the first few instances corresponding to the provided configurations;
   * **An unmatched count and Bars number will cause an exception `pgbar::exception::InvalidArgument`.**
   */
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( std::is_same_v<std::remove_cv_t<Config>, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<
      __details::traits::is_config<Config>,
      std::is_same<typename std::remove_cv<Config>::type, typename std::decay<Configs>::type>...>::value,
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>>::type
#endif
    make_dynamic( __details::types::Size count, Configs&&... cfgs ) noexcept( false )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    else if ( count < sizeof...( Configs ) )
      __PGBAR_UNLIKELY throw exception::InvalidArgument(
        "pgbar: the number of provided configs exceeds the specified count" );

    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>> products;
    (void)std::initializer_list<bool> {
      ( products.emplace_back( factory.insert( std::forward<Configs>( cfgs ) ) ), false )...
    };
    std::generate_n( std::back_inserter( products ), count - sizeof...( Configs ) - 1, [&factory]() {
      return factory.template insert<Config>();
    } );
    return products;
  }
  /**
   * Creates a vector of unique_ptr with a fixed number of BasicBar instances using multiple bar objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided objects;
   * **An unmatched count and Bars number will cause an exception `pgbar::exception::InvalidArgument`.**
   */
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<Config>,
                             std::is_same<Config, typename std::decay<Configs>::type>...>::value,
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>>>::type
#endif
    make_dynamic( __details::types::Size count,
                  __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars ) noexcept( false )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    else if ( count < sizeof...( Configs ) )
      __PGBAR_UNLIKELY throw exception::InvalidArgument(
        "pgbar: the number of provided bars exceeds the specified count" );

    DynamicBar<Outlet, Mode, Area> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>>> products;
    (void)std::initializer_list<bool> { ( products.emplace_back( factory.insert( std::move( bars ) ) ),
                                          false )... };
    std::generate_n( std::back_inserter( products ), count - sizeof...( Configs ) - 1, [&factory]() {
      return factory.template insert<Config>();
    } );
    return products;
  }
} // namespace pgbar

#endif
