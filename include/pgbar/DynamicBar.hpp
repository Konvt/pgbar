#ifndef PGBAR_DYNAMICBAR
#define PGBAR_DYNAMICBAR

#include "details/assets/DynamicContext.hpp"
#include <tuple>

namespace pgbar {
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
  template<typename Config, typename... Configs, Channel O, Policy M, Region A>
#if __PGBAR_CXX20
    requires( __details::traits::is_config<Config>::value
              && ( __details::traits::is_config<Configs>::value && ... ) )
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<Config, O, M, A>>,
               std::unique_ptr<__details::prefabs::BasicBar<Configs, O, M, A>>...>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN typename std::enable_if<
    __details::traits::AllOf<__details::traits::is_config<Config>,
                             __details::traits::is_config<Configs>...>::value,
    std::tuple<std::unique_ptr<__details::prefabs::BasicBar<Config, O, M, A>>,
               std::unique_ptr<__details::prefabs::BasicBar<Configs, O, M, A>>...>>::type
#endif
    make_dynamic( __details::prefabs::BasicBar<Config, O, M, A>&& bar,
                  __details::prefabs::BasicBar<Configs, O, M, A>&&... bars )
  {
    DynamicBar<O, M, A> factory;
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
  template<typename Config, Channel O, Policy M, Region A>
#if __PGBAR_CXX20
    requires __details::traits::is_config<Config>::value
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, O, M, A>>>
#else
  __PGBAR_NODISCARD __PGBAR_INLINE_FN
    typename std::enable_if<__details::traits::is_config<Config>::value,
                            std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, O, M, A>>>>::type
#endif
    make_dynamic( __details::prefabs::BasicBar<Config, O, M, A>&& bar, __details::types::Size count )
  {
    if ( count == 0 )
      __PGBAR_UNLIKELY return {};
    DynamicBar<O, M, A> factory;
    std::vector<std::unique_ptr<__details::prefabs::BasicBar<Config, O, M, A>>> products;
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
