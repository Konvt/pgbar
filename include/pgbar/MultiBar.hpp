#ifndef PGBAR_MULTIBAR
#define PGBAR_MULTIBAR

#include "details/prefabs/TupleBar.hpp"

namespace pgbar {
  template<typename Bar, typename... Bars>
  class MultiBar;
  template<Channel O, Policy M, Region A, typename Config, typename... Configs>
  class MultiBar<__details::prefabs::BasicBar<Config, O, M, A>,
                 __details::prefabs::BasicBar<Configs, O, M, A>...> {
    static_assert( __details::traits::AllOf<__details::traits::is_config<Config>,
                                            __details::traits::is_config<Configs>...>::value,
                   "pgbar::MultiBar: Invalid type" );
    using Self    = MultiBar;
    using Package = __details::prefabs::TupleBar<__details::traits::MakeIndexSeq<sizeof...( Configs ) + 1>,
                                                 __details::prefabs::BasicBar<Config, O, M, A>,
                                                 __details::prefabs::BasicBar<Configs, O, M, A>...>;

    template<__details::types::Size Pos>
    using ConfigAt_t = __details::traits::TypeAt_t<Pos, Config, Configs...>;
    template<__details::types::Size Pos>
    using BarAt_t = __details::traits::TypeAt_t<Pos,
                                                __details::prefabs::BasicBar<Config, O, M, A>,
                                                __details::prefabs::BasicBar<Configs, O, M, A>...>;

    Package tuple_;

  public:
    MultiBar() = default;

#if __PGBAR_CXX20
    template<typename Cfg, typename... Cfgs>
      requires( sizeof...( Cfgs ) <= sizeof...( Configs )
                && __details::traits::StartsWith<
                  __details::traits::TypeList<std::decay_t<Cfg>, std::decay_t<Cfgs>...>,
                  Config,
                  Configs...>::value )
#else
    template<typename Cfg,
             typename... Cfgs,
             typename = typename std::enable_if<__details::traits::AllOf<
               __details::traits::BoolConstant<( sizeof...( Cfgs ) <= sizeof...( Configs ) )>,
               __details::traits::StartsWith<__details::traits::TypeList<typename std::decay<Cfg>::type,
                                                                         typename std::decay<Cfgs>::type...>,
                                             Config,
                                             Configs...>>::value>::type>
#endif
    MultiBar( Cfg&& cfg, Cfgs&&... cfgs ) noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
      : tuple_ { std::forward<Cfg>( cfg ), std::forward<Cfgs>( cfgs )... }
    {}

    template<typename Cfg, typename... Cfgs
#if __PGBAR_CXX20
             >
      requires( sizeof...( Cfgs ) <= sizeof...( Configs ) && __details::traits::is_config<Cfg>::value
                && ( __details::traits::is_config<Cfgs>::value && ... )
                && __details::traits::
                  StartsWith<__details::traits::TypeList<Cfg, Cfgs...>, Config, Configs...>::value )
#else
             ,
             typename = typename std::enable_if<__details::traits::AllOf<
               __details::traits::BoolConstant< ( sizeof...( Cfgs ) <= sizeof...( Configs ) )>,
               __details::traits::is_config<Cfg>,
               __details::traits::is_config<Cfgs>...,
               __details::traits::StartsWith<
                 __details::traits::TypeList<Cfg,
                 Cfgs...>,
                 Config,
                 Configs...>>::value>::type>
#endif
    MultiBar( __details::prefabs::BasicBar<Cfg, O, M, A>&& bar,
              __details::prefabs::BasicBar<Cfgs, O, M, A>&&... bars )
      noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
      : tuple_ { std::move( bar ), std::move( bars )... }
    {}

    MultiBar( const Self& )          = delete;
    Self& operator=( const Self& ) & = delete;
    MultiBar( Self&& rhs ) noexcept : tuple_ { std::move( rhs.tuple_ ) }
    {
      __PGBAR_ASSERT( rhs.active() == false );
    }
    Self& operator=( Self&& rhs ) & noexcept
    {
      __PGBAR_TRUST( this != &rhs );
      __PGBAR_ASSERT( active() == false );
      __PGBAR_ASSERT( rhs.active() == false );
      tuple_ = std::move( rhs );
      return *this;
    }
    ~MultiBar() noexcept { reset(); }

    // Check whether a progress bar is running
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept { return tuple_.online(); }
    // Reset all the progress bars.
    __PGBAR_INLINE_FN void reset() noexcept { tuple_.halt(); }
    // Returns the number of progress bars.
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __PGBAR_CNSTEVAL __details::types::Size size() const noexcept
    {
      return sizeof...( Configs ) + 1;
    }
    // Returns the number of progress bars which is running.
    __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size active_size() const noexcept
    {
      return tuple_.active_size();
    }
    // Wait for all progress bars to stop.
    void wait() const noexcept
    {
      __details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for all progress bars to stop or time out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return __details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }

    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN BarAt_t<Pos>& at() & noexcept
    {
      return tuple_.template at<Pos>();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN const BarAt_t<Pos>& at() const& noexcept
    {
      return static_cast<const Package&>( tuple_ ).template at<Pos>();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN BarAt_t<Pos>&& at() && noexcept
    {
      return std::move( tuple_ ).template at<Pos>();
    }

    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick() &
    {
      at<Pos>().tick();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick( std::uint64_t next_step ) &
    {
      at<Pos>().tick( next_step );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void tick_to( std::uint8_t percentage ) &
    {
      at<Pos>().tick_to( percentage );
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void reset()
    {
      at<Pos>().reset();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN void wait() const noexcept
    {
      at<Pos>().wait();
    }
    template<__details::types::Size Pos, class Rep, class Period>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool wait_for(
      const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return at<Pos>().wait_for( timeout );
    }
    template<__details::types::Size Pos>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool active() const noexcept
    {
      return at<Pos>().active();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN ConfigAt_t<Pos>& config() &
    {
      return at<Pos>().config();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN const ConfigAt_t<Pos>& config() const&
    {
      return at<Pos>().config();
    }
    template<__details::types::Size Pos>
    __PGBAR_INLINE_FN ConfigAt_t<Pos>&& config() &&
    {
      return at<Pos>().config();
    }

    template<__details::types::Size Pos, typename... Args
#if __PGBAR_CXX20
             >
      requires __details::traits::is_iterable_bar<BarAt_t<Pos>>::value
#else
             ,
      typename = typename std::enable_if<__details::traits::is_iterable_bar<BarAt_t<Pos>>::value>::type>
#endif
    __PGBAR_INLINE_FN auto iterate( Args&&... args ) & noexcept(
      noexcept( std::declval<Self&>().template at<Pos>().iterate( std::forward<Args>( args )... ) ) )
      -> decltype( std::declval<Self&>().template at<Pos>().iterate( std::forward<Args>( args )... ) )
    {
      return at<Pos>().iterate( std::forward<Args>( args )... );
    }

    template<__details::types::Size Pos, typename F
#if __PGBAR_CXX20
             >
      requires __details::traits::is_reactive_bar<BarAt_t<Pos>>::value
#else
      ,
      typename = typename std::enable_if<__details::traits::is_reactive_bar<BarAt_t<Pos>>::value>::type>
#endif
    __PGBAR_INLINE_FN BarAt_t<Pos>& action( F&& fn ) & noexcept(
      noexcept( std::declval<Self&>().template at<Pos>().action( std::forward<F>( fn ) ) ) )
    {
      return at<Pos>().action( std::forward<F>( fn ) );
    }
    template<__details::types::Size Pos
#if __PGBAR_CXX20
             >
      requires __details::traits::is_reactive_bar<BarAt_t<Pos>>::value

#else
             ,
             typename =
               typename std::enable_if<__details::traits::is_reactive_bar<BarAt_t<Pos>>::value>::type>
#endif
    __PGBAR_INLINE_FN BarAt_t<Pos>& action() noexcept
    {
      return at<Pos>().action();
    }

    void swap( Self& rhs ) noexcept { tuple_.swap( rhs.tuple_ ); }
    friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

    template<__details::types::Size Pos, typename Mb>
    friend __PGBAR_INLINE_FN constexpr auto get( Mb&& self ) noexcept ->
      typename std::enable_if<std::is_same<typename std::decay<Mb>::type, Self>::value,
                              decltype( std::forward<Mb>( self ).template at<Pos>() )>::type
    {
      return std::forward<Mb>( self ).template at<Pos>();
    }
  };

#if __PGBAR_CXX17
  // CTAD, only generates the default version,
  // which means the the Outlet is `Channel::Stderr` and Mode is `Policy::Async`.
  template<typename Config, typename... Configs
# if __PGBAR_CXX20
           >
    requires( __details::traits::is_config<std::decay_t<Config>>::value
              && ( __details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
# else
           ,
    typename = std::enable_if_t<__details::traits::AllOf<__details::traits::is_config<std::decay_t<Config>>,
                                                         __details::traits::is_config<std::decay_t<Configs>>...>::value>>
# endif
  MultiBar( Config&&, Configs&&... )
    -> MultiBar<__details::prefabs::BasicBar<Config, Channel::Stderr, Policy::Async, Region::Fixed>,
                __details::prefabs::BasicBar<Configs, Channel::Stderr, Policy::Async, Region::Fixed>...>;
#endif

  // Creates a MultiBar using existing bar instances.
  template<typename Config, typename... Configs, Channel O, Policy M, Region A>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    MultiBar<__details::prefabs::BasicBar<Config, O, M, A>, __details::prefabs::BasicBar<Configs, O, M, A>...>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    typename std::enable_if<__details::traits::AllOf<__details::traits::is_config<Config>,
                                                     __details::traits::is_config<Configs>...>::value,
                            MultiBar<__details::prefabs::BasicBar<Config, O, M, A>,
                                     __details::prefabs::BasicBar<Configs, O, M, A>...>>::type
#endif
    make_multi( __details::prefabs::BasicBar<Config, O, M, A>&& bar,
                __details::prefabs::BasicBar<Configs, O, M, A>&&... bars ) noexcept
  {
    return { std::move( bar ), std::move( bars )... };
  }
  // Creates a MultiBar using configuration objects.
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config,
           typename... Configs>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<std::decay_t<Config>>::value
              && ( __details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
  MultiBar<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>,
           __details::prefabs::BasicBar<std::decay_t<Configs>, Outlet, Mode, Area>...>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<typename std::decay<Config>::type>,
                             __details::traits::is_config<typename std::decay<Configs>::type>...>::value,
    MultiBar<__details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>,
             __details::prefabs::BasicBar<typename std::decay<Configs>::type, Outlet, Mode, Area>...>>::type
#endif
    make_multi( Config&& cfg, Configs&&... cfgs ) noexcept(
      __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Config>,
                                                      std::is_lvalue_reference<Configs>...>>::value )
  {
    return { std::forward<Config>( cfg ), std::forward<Configs>( cfgs )... };
  }

  namespace __details {
    namespace assets {
      template<types::Size Cnt, Channel O, Policy M, Region A, typename C, types::Size... Is>
      __PGBAR_NODISCARD __PGBAR_INLINE_FN
        traits::Repeat_t<prefabs::BasicBar<typename std::decay<C>::type, O, M, A>, MultiBar, Cnt>
        make_multi_helper( C&& cfg, const traits::IndexSeq<Is...>& ) noexcept(
          traits::AllOf<traits::BoolConstant<( Cnt == 1 )>, traits::Not<std::is_lvalue_reference<C>>>::value )
      {
        std::array<C, Cnt - 1> cfgs { { ( (void)( Is ), cfg )... } };
        return { std::forward<C>( cfg ), std::move( cfgs[Is] )... };
      }
    } // namespace assets
  } // namespace __details

  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single bar object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<__details::types::Size Cnt, typename Config, Channel O, Policy M, Region A>
#if __PGBAR_CXX20
    requires( Cnt > 0 && __details::traits::is_config<Config>::value )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, O, M, A>, MultiBar, Cnt>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::BoolConstant<( Cnt > 0 )>,
                             __details::traits::is_config<Config>>::value,
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, O, M, A>, MultiBar, Cnt>>::type
#endif
    make_multi( __details::prefabs::BasicBar<Config, O, M, A>&& bar ) noexcept( Cnt == 1 )
  {
    return __details::assets::make_multi_helper<Cnt, O, M>( std::move( bar ).config(),
                                                            __details::traits::MakeIndexSeq<Cnt - 1>() );
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single configuration object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<__details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config>
#if __PGBAR_CXX20
    requires( Cnt > 0 && __details::traits::is_config<std::decay_t<Config>>::value )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::traits::
    Repeat_t<__details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>, MultiBar, Cnt>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::BoolConstant<( Cnt > 0 )>,
                             __details::traits::is_config<typename std::decay<Config>::type>>::value,
    __details::traits::Repeat_t<
      __details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>,
      MultiBar,
      Cnt>>::type
#endif
    make_multi( Config&& cfg )
      noexcept( __details::traits::AllOf<__details::traits::BoolConstant<( Cnt == 1 )>,
                                         __details::traits::Not<std::is_lvalue_reference<Config>>>::value )
  {
    return __details::assets::make_multi_helper<Cnt, Outlet, Mode>(
      std::forward<Config>( cfg ),
      __details::traits::MakeIndexSeq<Cnt - 1>() );
  }

  /**
   * Creates a MultiBar with a fixed number of bars using mutiple bar/configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided arguments;
   * **any remaining instances with no corresponding arguments will be default-initialized.**
   */
  template<typename Bar, __details::types::Size Cnt, typename... Objs>
#if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Objs ) <= Cnt && __details::traits::is_bar<Bar>::value
              && ( ( ( std::is_same_v<std::remove_cv_t<Bar>, std::decay_t<Objs>> && ... )
                     && !( std::is_lvalue_reference_v<Objs> || ... ) )
                   || ( std::is_same_v<typename Bar::Config, std::decay_t<Objs>> && ... ) ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::traits::Repeat_t<Bar, MultiBar, Cnt>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<
      __details::traits::BoolConstant<( Cnt > 0 )>,
      __details::traits::BoolConstant<( sizeof...( Objs ) <= Cnt )>,
      __details::traits::is_bar<Bar>,
      __details::traits::AnyOf<
        __details::traits::AllOf<
          std::is_same<typename std::remove_cv<Bar>::type, typename std::decay<Objs>::type>...,
          __details::traits::Not<__details::traits::AnyOf<std::is_lvalue_reference<Objs>...>>>,
        __details::traits::AllOf<std::is_same<typename Bar::Config, typename std::decay<Objs>::type>...>>>::
      value,
    __details::traits::Repeat_t<Bar, MultiBar, Cnt>>::type
#endif
    make_multi( Objs&&... objs ) noexcept( sizeof...( Objs ) == Cnt )
  {
    return { std::forward<Objs>( objs )... };
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using mutiple configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided configurations;
   * **any remaining instances with no corresponding configurations will be default-initialized.**
   */
  template<typename Config,
           __details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
#if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && __details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::BoolConstant<( Cnt > 0 )>,
                             __details::traits::BoolConstant<( sizeof...( Configs ) <= Cnt )>,
                             __details::traits::is_config<Config>,
                             std::is_same<Config, typename std::decay<Configs>::type>...>::value,
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>>::
    type
#endif
    make_multi( Configs&&... configs ) noexcept( sizeof...( Configs ) == Cnt )
  {
    return { std::forward<Configs>( configs )... };
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using mutiple bar objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided objects;
   * **any remaining instances with no corresponding configurations will be default-initialized.**
   */
  template<typename Config,
           __details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
#if __PGBAR_CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && __details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::BoolConstant<( Cnt > 0 )>,
                             __details::traits::BoolConstant<( sizeof...( Configs ) <= Cnt )>,
                             __details::traits::is_config<Config>,
                             std::is_same<Config, typename std::decay<Configs>::type>&&...>::value,
    __details::traits::Repeat_t<__details::prefabs::BasicBar<Config, Outlet, Mode, Area>, MultiBar, Cnt>>::
    type
#endif
    make_multi( __details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars )
      noexcept( sizeof...( Configs ) == Cnt )
  {
    return { std::move( bars )... };
  }
} // namespace pgbar

template<typename... Bs>
struct std::tuple_size<pgbar::MultiBar<Bs...>> : std::integral_constant<std::size_t, sizeof...( Bs )> {};

template<std::size_t I, typename... Bs>
struct std::tuple_element<I, pgbar::MultiBar<Bs...>> : pgbar::__details::traits::TypeAt<I, Bs...> {};

#endif
