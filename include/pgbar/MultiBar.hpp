#ifndef PGBAR_MULTIBAR
#define PGBAR_MULTIBAR

#include "details/prefabs/TupleBar.hpp"

namespace pgbar {
  template<typename Bar, typename... Bars>
  class MultiBar;
  template<Channel O, Policy M, Region A, typename Config, typename... Configs>
  class MultiBar<_details::prefabs::BasicBar<Config, O, M, A>,
                 _details::prefabs::BasicBar<Configs, O, M, A>...> {
    static_assert( _details::traits::AllOf<_details::traits::is_config<Config>,
                                           _details::traits::is_config<Configs>...>::value,
                   "pgbar::MultiBar: Invalid type" );
    using Self = MultiBar;

    template<_details::types::Size Pos>
    using ConfigAt_t = _details::traits::TypeAt_t<Pos, Config, Configs...>;
    template<_details::types::Size Pos>
    using BarAt_t = _details::traits::TypeAt_t<Pos,
                                               _details::prefabs::BasicBar<Config, O, M, A>,
                                               _details::prefabs::BasicBar<Configs, O, M, A>...>;

    _details::prefabs::TupleBar<_details::traits::MakeIndexSeq<sizeof...( Configs ) + 1>,
                                _details::prefabs::BasicBar<Config, O, M, A>,
                                _details::prefabs::BasicBar<Configs, O, M, A>...>
      package_;

  public:
    MultiBar() = default;

#if PGBAR__CXX20
    template<typename Cfg, typename... Cfgs>
      requires( sizeof...( Cfgs ) <= sizeof...( Configs )
                && _details::traits::TpStartsWith<
                  _details::traits::TypeList<std::decay_t<Cfg>, std::decay_t<Cfgs>...>,
                  Config,
                  Configs...>::value )
#else
    template<typename Cfg,
             typename... Cfgs,
             typename = typename std::enable_if<_details::traits::AllOf<
               _details::traits::BoolConstant<( sizeof...( Cfgs ) <= sizeof...( Configs ) )>,
               _details::traits::TpStartsWith<_details::traits::TypeList<typename std::decay<Cfg>::type,
                                                                         typename std::decay<Cfgs>::type...>,
                                              Config,
                                              Configs...>>::value>::type>
#endif
    MultiBar( Cfg&& cfg, Cfgs&&... cfgs ) noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
      : package_ { std::forward<Cfg>( cfg ), std::forward<Cfgs>( cfgs )... }
    {}

    template<typename Cfg, typename... Cfgs
#if PGBAR__CXX20
             >
      requires( sizeof...( Cfgs ) <= sizeof...( Configs ) && _details::traits::is_config<Cfg>::value
                && _details::traits::
                  TpStartsWith<_details::traits::TypeList<Cfg, Cfgs...>, Config, Configs...>::value )
#else
             ,
             typename = typename std::enable_if<_details::traits::AllOf<
               _details::traits::BoolConstant< ( sizeof...( Cfgs ) <= sizeof...( Configs ) )>,
               _details::traits::TpStartsWith<
                 _details::traits::TypeList<Cfg,
                 Cfgs...>,
                 Config,
                 Configs...>>::value>::type>
#endif
    MultiBar( _details::prefabs::BasicBar<Cfg, O, M, A>&& bar,
              _details::prefabs::BasicBar<Cfgs, O, M, A>&&... bars )
      noexcept( sizeof...( Cfgs ) == sizeof...( Configs ) )
      : package_ { std::move( bar ), std::move( bars )... }
    {}

    MultiBar( const Self& )                  = delete;
    Self& operator=( const Self& ) &         = delete;
    MultiBar( Self&& rhs ) noexcept          = default;
    Self& operator=( Self&& rhs ) & noexcept = default;
    ~MultiBar()                              = default;

    // Check whether a progress bar is running
    PGBAR__NODISCARD PGBAR__INLINE_FN bool active() const noexcept { return package_.online(); }
    // Reset all the progress bars.
    PGBAR__INLINE_FN void reset() { package_.shut(); }
    // Abort all the progress bars.
    PGBAR__INLINE_FN void abort() noexcept { package_.kill(); }
    // Returns the number of progress bars.
    PGBAR__NODISCARD PGBAR__INLINE_FN constexpr _details::types::Size size() const noexcept
    {
      return sizeof...( Configs ) + 1;
    }
    // Returns the number of progress bars which is running.
    PGBAR__NODISCARD PGBAR__INLINE_FN _details::types::Size active_size() const noexcept
    {
      return package_.online_count();
    }
    // Wait for all progress bars to stop.
    void wait() const noexcept
    {
      _details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for all progress bars to stop or time out.
    template<class Rep, class Period>
    PGBAR__NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return _details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }

    template<_details::types::Size Pos>
    PGBAR__INLINE_FN BarAt_t<Pos>& at() & noexcept
    {
      return package_.template at<Pos>();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN const BarAt_t<Pos>& at() const& noexcept
    {
      return package_.template at<Pos>();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN BarAt_t<Pos>&& at() && noexcept
    {
      return std::move( package_.template at<Pos>() );
    }

    template<_details::types::Size Pos>
    PGBAR__INLINE_FN void tick() &
    {
      at<Pos>().tick();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN void tick( std::uint64_t next_step ) &
    {
      at<Pos>().tick( next_step );
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN void tick_to( std::uint8_t percentage ) &
    {
      at<Pos>().tick_to( percentage );
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN void reset()
    {
      at<Pos>().reset();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN void abort() noexcept
    {
      at<Pos>().abort();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN void wait() const noexcept
    {
      at<Pos>().wait();
    }
    template<_details::types::Size Pos, class Rep, class Period>
    PGBAR__NODISCARD PGBAR__INLINE_FN bool wait_for(
      const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return at<Pos>().wait_for( timeout );
    }
    template<_details::types::Size Pos>
    PGBAR__NODISCARD PGBAR__INLINE_FN bool active() const noexcept
    {
      return at<Pos>().active();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN ConfigAt_t<Pos>& config() &
    {
      return at<Pos>().config();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN const ConfigAt_t<Pos>& config() const&
    {
      return at<Pos>().config();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN ConfigAt_t<Pos>&& config() &&
    {
      return at<Pos>().config();
    }

    template<_details::types::Size Pos, typename... Args
#if PGBAR__CXX20
             >
      requires _details::traits::is_iterable_bar<BarAt_t<Pos>>::value
#else
             ,
      typename = typename std::enable_if<_details::traits::is_iterable_bar<BarAt_t<Pos>>::value>::type>
#endif
    PGBAR__INLINE_FN auto iterate( Args&&... args ) & noexcept(
      noexcept( std::declval<Self&>().template at<Pos>().iterate( std::forward<Args>( args )... ) ) )
      -> decltype( std::declval<Self&>().template at<Pos>().iterate( std::forward<Args>( args )... ) )
    {
      return at<Pos>().iterate( std::forward<Args>( args )... );
    }

    template<_details::types::Size Pos, typename F
#if PGBAR__CXX20
             >
      requires _details::traits::is_reactive_bar<BarAt_t<Pos>>::value
#else
      ,
      typename = typename std::enable_if<_details::traits::is_reactive_bar<BarAt_t<Pos>>::value>::type>
#endif
    PGBAR__INLINE_FN BarAt_t<Pos>& action( F&& fn ) & noexcept(
      noexcept( std::declval<Self&>().template at<Pos>().action( std::forward<F>( fn ) ) ) )
    {
      return at<Pos>().action( std::forward<F>( fn ) );
    }
    template<_details::types::Size Pos
#if PGBAR__CXX20
             >
      requires _details::traits::is_reactive_bar<BarAt_t<Pos>>::value

#else
             ,
             typename = typename std::enable_if<_details::traits::is_reactive_bar<BarAt_t<Pos>>::value>::type>
#endif
    PGBAR__INLINE_FN BarAt_t<Pos>& action() noexcept
    {
      return at<Pos>().action();
    }

    void swap( Self& lhs ) noexcept { package_.swap( lhs.package_ ); }
    friend void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

    template<_details::types::Size Pos>
    PGBAR__INLINE_FN constexpr BarAt_t<Pos>& get( Self& self ) noexcept
    {
      return self.template at<Pos>();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN constexpr const BarAt_t<Pos>& get( const Self& self ) noexcept
    {
      return self.template at<Pos>();
    }
    template<_details::types::Size Pos>
    PGBAR__INLINE_FN constexpr BarAt_t<Pos>&& get( Self&& self ) noexcept
    {
      return std::move( self ).template at<Pos>();
    }
  };

#if PGBAR__CXX17
  // CTAD, only generates the default version,
  // which means the the Outlet is `Channel::Stderr` and Mode is `Policy::Async`.
  template<typename Config, typename... Configs
# if PGBAR__CXX20
           >
    requires( _details::traits::is_config<std::decay_t<Config>>::value
              && ( _details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
# else
           ,
    typename = std::enable_if_t<_details::traits::AllOf<_details::traits::is_config<std::decay_t<Config>>,
                                                         _details::traits::is_config<std::decay_t<Configs>>...>::value>>
# endif
  MultiBar( Config&&, Configs&&... )
    -> MultiBar<_details::prefabs::BasicBar<Config, Channel::Stderr, Policy::Async, Region::Fixed>,
                _details::prefabs::BasicBar<Configs, Channel::Stderr, Policy::Async, Region::Fixed>...>;
#endif

  // Generates a MultiBar type containing Count instances of the given Bar type.
  template<typename Bar, _details::types::Size Count>
  using MakeMulti_t = _details::traits::FillWith_t<MultiBar, Bar, Count>;

  // Creates a MultiBar using existing bar instances.
  template<typename Config, typename... Configs, Channel O, Policy M, Region A>
#if PGBAR__CXX20
    requires( _details::traits::is_config<Config>::value
              && ( _details::traits::is_config<Configs>::value && ... ) )
  PGBAR__NODISCARD PGBAR__INLINE_FN
    MultiBar<_details::prefabs::BasicBar<Config, O, M, A>, _details::prefabs::BasicBar<Configs, O, M, A>...>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<_details::traits::is_config<Config>,
                                                    _details::traits::is_config<Configs>...>::value,
                            MultiBar<_details::prefabs::BasicBar<Config, O, M, A>,
                                     _details::prefabs::BasicBar<Configs, O, M, A>...>>::type
#endif
    make_multi( _details::prefabs::BasicBar<Config, O, M, A>&& bar,
                _details::prefabs::BasicBar<Configs, O, M, A>&&... bars ) noexcept
  {
    return { std::move( bar ), std::move( bars )... };
  }
  // Creates a MultiBar using configuration objects.
  template<Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config,
           typename... Configs>
#if PGBAR__CXX20
    requires( _details::traits::is_config<std::decay_t<Config>>::value
              && ( _details::traits::is_config<std::decay_t<Configs>>::value && ... ) )
  MultiBar<_details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>,
           _details::prefabs::BasicBar<std::decay_t<Configs>, Outlet, Mode, Area>...>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
    _details::traits::AllOf<_details::traits::is_config<typename std::decay<Config>::type>,
                            _details::traits::is_config<typename std::decay<Configs>::type>...>::value,
    MultiBar<_details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>,
             _details::prefabs::BasicBar<typename std::decay<Configs>::type, Outlet, Mode, Area>...>>::type
#endif
    make_multi( Config&& cfg, Configs&&... cfgs )
      noexcept( _details::traits::Not<_details::traits::AnyOf<std::is_lvalue_reference<Config>,
                                                              std::is_lvalue_reference<Configs>...>>::value )
  {
    return { std::forward<Config>( cfg ), std::forward<Configs>( cfgs )... };
  }

  namespace _details {
    namespace assets {
      template<types::Size Cnt, Channel O, Policy M, Region A, typename B, types::Size... Is>
      PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
        traits::is_bar<typename std::decay<B>::type>::value,
        MakeMulti_t<prefabs::BasicBar<typename std::decay<B>::type::Config, O, M, A>, Cnt>>::type
        make_multi_helper( B&& bar, const traits::IndexSeq<Is...>& )
          noexcept( traits::BoolConstant<( Cnt == 1 )>::value )
      {
        using Bar = typename std::decay<B>::type;
        std::array<typename Bar::Config, Cnt - 1> cfgs { { ( (void)( Is ), bar.config() )... } };
        return { std::forward<B>( bar ), Bar( std::move( cfgs[Is] ) )... };
      }
      template<types::Size Cnt, Channel O, Policy M, Region A, typename C, types::Size... Is>
      PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
        traits::is_config<typename std::decay<C>::type>::value,
        MakeMulti_t<prefabs::BasicBar<typename std::decay<C>::type, O, M, A>, Cnt>>::type
        make_multi_helper( C&& cfg, const traits::IndexSeq<Is...>& ) noexcept(
          traits::AllOf<traits::BoolConstant<( Cnt == 1 )>, traits::Not<std::is_lvalue_reference<C>>>::value )
      {
        std::array<C, Cnt - 1> cfgs { { ( (void)( Is ), cfg )... } };
        return { std::forward<C>( cfg ), std::move( cfgs[Is] )... };
      }
    } // namespace assets
  } // namespace _details

  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single bar object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<_details::types::Size Cnt, typename Config, Channel O, Policy M, Region A>
#if PGBAR__CXX20
    requires( Cnt > 0 && _details::traits::is_config<Config>::value )
  PGBAR__NODISCARD PGBAR__INLINE_FN MakeMulti_t<_details::prefabs::BasicBar<Config, O, M, A>, Cnt>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<_details::traits::BoolConstant<( Cnt > 0 )>,
                                                    _details::traits::is_config<Config>>::value,
                            MakeMulti_t<_details::prefabs::BasicBar<Config, O, M, A>, Cnt>>::type
#endif
    make_multi( _details::prefabs::BasicBar<Config, O, M, A>&& bar ) noexcept( Cnt == 1 )
  {
    return _details::assets::make_multi_helper<Cnt, O, M, A>( std::move( bar ),
                                                              _details::traits::MakeIndexSeq<Cnt - 1>() );
  }
  /**
   * Creates a MultiBar with a fixed number of BasicBar instances using a single configuration object.
   * **All BasicBar instances are initialized using the same configuration.**
   */
  template<_details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename Config>
#if PGBAR__CXX20
    requires( Cnt > 0 && _details::traits::is_config<std::decay_t<Config>>::value )
  PGBAR__NODISCARD PGBAR__INLINE_FN
    MakeMulti_t<_details::prefabs::BasicBar<std::decay_t<Config>, Outlet, Mode, Area>, Cnt>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
    _details::traits::AllOf<_details::traits::BoolConstant<( Cnt > 0 )>,
                            _details::traits::is_config<typename std::decay<Config>::type>>::value,
    MakeMulti_t<_details::prefabs::BasicBar<typename std::decay<Config>::type, Outlet, Mode, Area>,
                Cnt>>::type
#endif
    make_multi( Config&& cfg )
      noexcept( _details::traits::AllOf<_details::traits::BoolConstant<( Cnt == 1 )>,
                                        _details::traits::Not<std::is_lvalue_reference<Config>>>::value )
  {
    return _details::assets::make_multi_helper<Cnt, Outlet, Mode, Area>(
      std::forward<Config>( cfg ),
      _details::traits::MakeIndexSeq<Cnt - 1>() );
  }

  /**
   * Creates a MultiBar with a fixed number of bars using mutiple bar/configuration objects.
   * The ctor sequentially initializes the first few instances corresponding to the provided arguments;
   * **any remaining instances with no corresponding arguments will be default-initialized.**
   */
  template<typename Bar, _details::types::Size Cnt, typename... Objs>
#if PGBAR__CXX20
    requires( Cnt > 0 && sizeof...( Objs ) <= Cnt && _details::traits::is_bar<Bar>::value
              && ( ( ( std::is_same_v<std::remove_cv_t<Bar>, std::decay_t<Objs>> && ... )
                     && !( std::is_lvalue_reference_v<Objs> || ... ) )
                   || ( std::is_same_v<typename Bar::Config, std::decay_t<Objs>> && ... ) ) )
  PGBAR__NODISCARD PGBAR__INLINE_FN MakeMulti_t<Bar, Cnt>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
    _details::traits::AllOf<
      _details::traits::BoolConstant<( Cnt > 0 )>,
      _details::traits::BoolConstant<( sizeof...( Objs ) <= Cnt )>,
      _details::traits::is_bar<Bar>,
      _details::traits::AnyOf<
        _details::traits::AllOf<
          std::is_same<typename std::remove_cv<Bar>::type, typename std::decay<Objs>::type>...,
          _details::traits::Not<_details::traits::AnyOf<std::is_lvalue_reference<Objs>...>>>,
        _details::traits::AllOf<std::is_same<typename Bar::Config, typename std::decay<Objs>::type>...>>>::
      value,
    MakeMulti_t<Bar, Cnt>>::type
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
           _details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
#if PGBAR__CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && _details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  PGBAR__NODISCARD PGBAR__INLINE_FN MakeMulti_t<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>, Cnt>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
    _details::traits::AllOf<_details::traits::BoolConstant<( Cnt > 0 )>,
                            _details::traits::BoolConstant<( sizeof...( Configs ) <= Cnt )>,
                            _details::traits::is_config<Config>,
                            std::is_same<Config, typename std::decay<Configs>::type>...>::value,
    MakeMulti_t<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>, Cnt>>::type
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
           _details::types::Size Cnt,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename... Configs>
#if PGBAR__CXX20
    requires( Cnt > 0 && sizeof...( Configs ) <= Cnt && _details::traits::is_config<Config>::value
              && ( std::is_same_v<Config, std::decay_t<Configs>> && ... ) )
  PGBAR__NODISCARD PGBAR__INLINE_FN MakeMulti_t<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>, Cnt>
#else
  PGBAR__NODISCARD PGBAR__INLINE_FN typename std::enable_if<
    _details::traits::AllOf<_details::traits::BoolConstant<( Cnt > 0 )>,
                            _details::traits::BoolConstant<( sizeof...( Configs ) <= Cnt )>,
                            _details::traits::is_config<Config>,
                            std::is_same<Config, typename std::decay<Configs>::type>&&...>::value,
    MakeMulti_t<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>, Cnt>>::type
#endif
    make_multi( _details::prefabs::BasicBar<Configs, Outlet, Mode, Area>&&... bars )
      noexcept( sizeof...( Configs ) == Cnt )
  {
    return { std::move( bars )... };
  }
} // namespace pgbar

template<typename... Bs>
struct std::tuple_size<pgbar::MultiBar<Bs...>> : std::integral_constant<std::size_t, sizeof...( Bs )> {};

template<std::size_t I, typename... Bs>
struct std::tuple_element<I, pgbar::MultiBar<Bs...>> : pgbar::_details::traits::TypeAt<I, Bs...> {};

#endif
