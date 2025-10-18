#ifndef PGBAR__BASICBAR
#define PGBAR__BASICBAR

#include "../assets/Driver.hpp"
#include "../traits/C3.hpp"
#include "../traits/Util.hpp"

namespace pgbar {
  namespace _details {
    namespace prefabs {
      // Interface type, used to meet the template type arguments of the base class for the derived class.
      template<typename Soul, Channel Outlet, Policy Mode, Region Area>
      class BasicBar
        : public traits::LI<traits::BehaviourFor_t<Soul>>::template type<Indicator,
                                                                         BasicBar<Soul, Outlet, Mode, Area>> {
        using Base = typename traits::LI<
          traits::BehaviourFor_t<Soul>>::template type<Indicator, BasicBar<Soul, Outlet, Mode, Area>>;

      public:
        using Config                     = Soul;
        static constexpr Channel Sink    = Outlet;
        static constexpr Policy Strategy = Mode;
        static constexpr Region Layout   = Area;

        constexpr BasicBar( Soul config ) noexcept( std::is_nothrow_constructible<Base, Soul&&>::value )
          : Base( std::move( config ) )
        {}
#if PGBAR__CXX20
        template<typename... Args>
          requires( !( std::is_same_v<std::decay_t<Args>, Soul> || ... )
                    && std::is_constructible_v<Soul, Args...> )
#else
        template<typename... Args,
                 typename = typename std::enable_if<
                   traits::AllOf<traits::Not<std::is_same<typename std::decay<Args>::type, Soul>>...,
                                 std::is_constructible<Soul, Args...>>::value>::type>
#endif
        constexpr BasicBar( Args&&... args ) noexcept( std::is_nothrow_constructible<Base, Args...>::value )
          : Base( Soul( std::forward<Args>( args )... ) )
        {}

        BasicBar( const BasicBar& )                      = delete;
        BasicBar& operator=( const BasicBar& ) &         = delete;
        constexpr BasicBar( BasicBar&& )                 = default;
        BasicBar& operator=( BasicBar&& rhs ) & noexcept = default;
        PGBAR__CXX20_CNSTXPR virtual ~BasicBar()         = default;

        void swap( BasicBar& lhs ) noexcept
        {
          PGBAR__TRUST( this != &lhs );
          PGBAR__ASSERT( this->active() == false );
          PGBAR__ASSERT( this->active() == false );
          Base::swap( lhs );
        }
        friend void swap( BasicBar& a, BasicBar& b ) noexcept { a.swap( b ); }
      };
    } // namespace prefabs

    namespace traits {
      template<typename B>
      struct is_bar {
      private:
        template<typename S, Channel O, Policy M, Region A>
        static constexpr std::true_type check( const prefabs::BasicBar<S, O, M, A>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<B>>,
                decltype( check( std::declval<typename std::remove_cv<B>::type>() ) )>::value;
      };
      template<typename B>
      struct is_iterable_bar : AllOf<is_bar<B>, InstanceOf<B, assets::TaskCounter>> {};
      template<typename B>
      struct is_reactive_bar : AllOf<is_bar<B>, InstanceOf<B, assets::ReactiveBar>> {};
    } // namespace traits
  } // namespace _details

  template<typename Bar, typename N, typename Proc, typename... Options>
#if PGBAR__CXX20
    requires( std::is_arithmetic_v<N> && _details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<std::is_arithmetic<N>,
                                                    _details::traits::is_iterable_bar<Bar>,
                                                    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, N step, Proc&& op, Options&&... options )
  {
    Bar( std::forward<Options>( options )... )
      .iterate( startpoint, endpoint, step, std::forward<Proc>( op ) );
  }
  template<typename Bar, typename N, typename Proc, typename Act, typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_arithmetic_v<N> && _details::traits::is_iterable_bar<Bar>::value
      && _details::traits::is_reactive_bar<Bar>::value
      && std::is_same_v<std::remove_reference_t<decltype( std::declval<Bar>() | std::declval<Act>() )>, Bar>
      && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_arithmetic<N>,
    _details::traits::is_iterable_bar<Bar>,
    _details::traits::is_reactive_bar<Bar>,
    std::is_same<typename std::remove_reference<decltype( std::declval<Bar>() | std::declval<Act>() )>::type,
                 Bar>,
    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, N step, Proc&& op, Act&& act, Options&&... options )
  {
    ( std::forward<Act>( act ) | Bar( std::forward<Options>( options )... ) )
      .iterate( startpoint, endpoint, step, std::forward<Proc>( op ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_arithmetic_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_arithmetic<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, N step, Proc&& op, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( startpoint,
                                                                      endpoint,
                                                                      step,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Options>( options )... );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename Act,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_arithmetic_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_same_v<std::remove_reference_t<
                          decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                                    | std::declval<Act>() )>,
                        _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_arithmetic<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_same<typename std::remove_reference<
                   decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                             | std::declval<Act>() )>::type,
                 _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, N step, Proc&& op, Act&& act, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( startpoint,
                                                                      endpoint,
                                                                      step,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Act>( act ),
                                                                      std::forward<Options>( options )... );
  }

  template<typename Bar, typename N, typename Proc, typename... Options>
#if PGBAR__CXX20
    requires( std::is_floating_point_v<N> && _details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<std::is_floating_point<N>,
                                                    _details::traits::is_iterable_bar<Bar>,
                                                    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N endpoint, N step, Proc&& op, Options&&... options )
  {
    auto bar = Bar( std::forward<Options>( options )... );
    bar.iterate( endpoint, step, std::forward<Proc>( op ) );
  }
  template<typename Bar, typename N, typename Proc, typename Act, typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_floating_point_v<N> && _details::traits::is_iterable_bar<Bar>::value
      && _details::traits::is_reactive_bar<Bar>::value
      && std::is_same_v<std::remove_reference_t<decltype( std::declval<Bar>() | std::declval<Act>() )>, Bar>
      && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_floating_point<N>,
    _details::traits::is_iterable_bar<Bar>,
    _details::traits::is_reactive_bar<Bar>,
    std::is_same<typename std::remove_reference<decltype( std::declval<Bar>() | std::declval<Act>() )>::type,
                 Bar>,
    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N endpoint, N step, Proc&& op, Act&& act, Options&&... options )
  {
    auto bar = std::forward<Act>( act ) | Bar( std::forward<Options>( options )... );
    bar.iterate( endpoint, step, std::forward<Proc>( op ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_floating_point_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_floating_point<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N endpoint, N step, Proc&& op, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( endpoint,
                                                                      step,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Options>( options )... );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename Act,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_floating_point_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_same_v<std::remove_reference_t<
                          decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                                    | std::declval<Act>() )>,
                        _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_floating_point<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_same<typename std::remove_reference<
                   decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                             | std::declval<Act>() )>::type,
                 _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N endpoint, N step, Proc&& op, Act&& act, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( endpoint,
                                                                      step,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Act>( act ),
                                                                      std::forward<Options>( options )... );
  }

  template<typename Bar, typename N, typename Proc, typename... Options>
#if PGBAR__CXX20
    requires( std::is_integral_v<N> && _details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<std::is_integral<N>,
                                                    _details::traits::is_iterable_bar<Bar>,
                                                    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, Proc&& op, Options&&... options )
  {
    Bar( std::forward<Options>( options )... ).iterate( startpoint, endpoint, std::forward<Proc>( op ) );
  }
  template<typename Bar, typename N, typename Proc, typename Act, typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_integral_v<N> && _details::traits::is_iterable_bar<Bar>::value
      && _details::traits::is_reactive_bar<Bar>::value
      && std::is_same_v<std::remove_reference_t<decltype( std::declval<Bar>() | std::declval<Act>() )>, Bar>
      && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_integral<N>,
    _details::traits::is_iterable_bar<Bar>,
    std::is_same<typename std::remove_reference<decltype( std::declval<Bar>() | std::declval<Act>() )>::type,
                 Bar>,
    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, Proc&& op, Act&& act, Options&&... options )
  {
    ( std::forward<Act>( act ) | Bar( std::forward<Options>( options )... ) )
      .iterate( startpoint, endpoint, std::forward<Proc>( op ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_integral_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_integral<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, Proc&& op, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( startpoint,
                                                                      endpoint,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Options>( options )... );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename Act,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_integral_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_same_v<std::remove_reference_t<
                          decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                                    | std::declval<Act>() )>,
                        _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_integral<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_same<typename std::remove_reference<
                   decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                             | std::declval<Act>() )>::type,
                 _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N startpoint, N endpoint, Proc&& op, Act&& act, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( startpoint,
                                                                      endpoint,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Act>( act ),
                                                                      std::forward<Options>( options )... );
  }

  template<typename Bar, typename N, typename Proc, typename... Options>
#if PGBAR__CXX20
    requires( std::is_integral_v<N> && _details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<std::is_integral<N>,
                                                    _details::traits::is_iterable_bar<Bar>,
                                                    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N endpoint, Proc&& op, Options&&... options )
  {
    Bar( std::forward<Options>( options )... ).iterate( endpoint, std::forward<Proc>( op ) );
  }
  template<typename Bar, typename N, typename Proc, typename Act, typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_integral_v<N> && _details::traits::is_iterable_bar<Bar>::value
      && _details::traits::is_reactive_bar<Bar>::value
      && std::is_same_v<std::remove_reference_t<decltype( std::declval<Bar>() | std::declval<Act>() )>, Bar>
      && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_integral<N>,
    _details::traits::is_iterable_bar<Bar>,
    _details::traits::is_reactive_bar<Bar>,
    std::is_same<typename std::remove_reference<decltype( std::declval<Bar>() | std::declval<Act>() )>::type,
                 Bar>,
    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( N endpoint, Proc&& op, Act&& act, Options&&... options )
  {
    ( std::forward<Act>( act ) | Bar( std::forward<Options>( options )... ) )
      .iterate( endpoint, std::forward<Proc>( op ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_integral_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_integral<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N endpoint, Proc&& op, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( endpoint,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Options>( options )... );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename N,
           typename Proc,
           typename Act,
           typename... Options>
#if PGBAR__CXX20
    requires(
      std::is_integral_v<N> && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_same_v<std::remove_reference_t<
                          decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                                    | std::declval<Act>() )>,
                        _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    std::is_integral<N>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_same<typename std::remove_reference<
                   decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                             | std::declval<Act>() )>::type,
                 _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( N endpoint, Proc&& op, Act&& act, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( endpoint,
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Act>( act ),
                                                                      std::forward<Options>( options )... );
  }

  template<typename Bar, typename I, typename Proc, typename... Options>
#if PGBAR__CXX20
    requires( _details::traits::is_sized_iterator<I>::value && _details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<_details::traits::is_sized_iterator<I>,
                                                    _details::traits::is_iterable_bar<Bar>,
                                                    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( I startpoint, I endpoint, Proc&& op, Options&&... options )
  {
    Bar( std::forward<Options>( options )... )
      .iterate( std::move( startpoint ), std::move( endpoint ), std::forward<Proc>( op ) );
  }
  template<typename Bar, typename I, typename Proc, typename Act, typename... Options>
#if PGBAR__CXX20
    requires(
      _details::traits::is_sized_iterator<I>::value && _details::traits::is_iterable_bar<Bar>::value
      && _details::traits::is_reactive_bar<Bar>::value
      && std::is_same_v<std::remove_reference_t<decltype( std::declval<Bar>() | std::declval<Act>() )>, Bar>
      && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    _details::traits::is_sized_iterator<I>,
    _details::traits::is_iterable_bar<Bar>,
    _details::traits::is_reactive_bar<Bar>,
    std::is_same<typename std::remove_reference<decltype( std::declval<Bar>() | std::declval<Act>() )>::type,
                 Bar>,
    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( I startpoint, I endpoint, Proc&& op, Act&& act, Options&&... options )
  {
    ( std::forward<Act>( act ) | Bar( std::forward<Options>( options )... ) )
      .iterate( std::move( startpoint ), std::move( endpoint ), std::forward<Proc>( op ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename I,
           typename Proc,
           typename... Options>
#if PGBAR__CXX20
    requires(
      _details::traits::is_sized_iterator<I>::value && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    _details::traits::is_sized_iterator<I>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( I startpoint, I endpoint, Proc&& op, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( std::move( startpoint ),
                                                                      std::move( endpoint ),
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Options>( options )... );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           typename I,
           typename Proc,
           typename Act,
           typename... Options>
#if PGBAR__CXX20
    requires(
      _details::traits::is_sized_iterator<I>::value && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_same_v<std::remove_reference_t<
                          decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                                    | std::declval<Act>() )>,
                        _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    _details::traits::is_sized_iterator<I>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_same<typename std::remove_reference<
                   decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                             | std::declval<Act>() )>::type,
                 _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( I startpoint, I endpoint, Proc&& op, Act&& act, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( std::move( startpoint ),
                                                                      std::move( endpoint ),
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Act>( act ),
                                                                      std::forward<Options>( options )... );
  }

  template<typename Bar, class R, typename Proc, typename... Options>
#if PGBAR__CXX20
    requires( _details::traits::is_bounded_range<R>::value && _details::traits::is_iterable_bar<Bar>::value
              && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN
    typename std::enable_if<_details::traits::AllOf<_details::traits::is_bounded_range<R>,
                                                    _details::traits::is_iterable_bar<Bar>,
                                                    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( R&& range, Proc&& op, Options&&... options )
  {
    Bar( std::forward<Options>( options )... ).iterate( std::forward<R>( range ), std::forward<Proc>( op ) );
  }
  template<typename Bar, class R, typename Proc, typename Act, typename... Options>
#if PGBAR__CXX20
    requires(
      _details::traits::is_bounded_range<R>::value && _details::traits::is_iterable_bar<Bar>::value
      && _details::traits::is_reactive_bar<Bar>::value
      && std::is_same_v<std::remove_reference_t<decltype( std::declval<Bar>() | std::declval<Act>() )>, Bar>
      && std::is_constructible_v<Bar, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    _details::traits::is_bounded_range<R>,
    _details::traits::is_iterable_bar<Bar>,
    _details::traits::is_reactive_bar<Bar>,
    std::is_same<typename std::remove_reference<decltype( std::declval<Bar>() | std::declval<Act>() )>::type,
                 Bar>,
    std::is_constructible<Bar, Options...>>::value>::type
#endif
    iterate( R&& range, Proc&& op, Act&& act, Options&&... options )
  {
    ( std::forward<Act>( act ) | Bar( std::forward<Options>( options )... ) )
      .iterate( std::forward<R>( range ), std::forward<Proc>( op ) );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           class R,
           typename Proc,
           typename... Options>
#if PGBAR__CXX20
    requires(
      _details::traits::is_bounded_range<R>::value && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    _details::traits::is_bounded_range<R>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( R&& range, Proc&& op, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( std::forward<R>( range ),
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Options>( options )... );
  }
  template<typename Config,
           Channel Outlet = Channel::Stderr,
           Policy Mode    = Policy::Async,
           Region Area    = Region::Fixed,
           class R,
           typename Proc,
           typename Act,
           typename... Options>
#if PGBAR__CXX20
    requires(
      _details::traits::is_bounded_range<R>::value && _details::traits::is_config<Config>::value
      && _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>::value
      && std::is_same_v<std::remove_reference_t<
                          decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                                    | std::declval<Act>() )>,
                        _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>
      && std::is_constructible_v<Config, Options...> )
  PGBAR__INLINE_FN void
#else
  PGBAR__INLINE_FN typename std::enable_if<_details::traits::AllOf<
    _details::traits::is_bounded_range<R>,
    _details::traits::is_config<Config>,
    _details::traits::is_iterable_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    _details::traits::is_reactive_bar<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_same<typename std::remove_reference<
                   decltype( std::declval<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>()
                             | std::declval<Act>() )>::type,
                 _details::prefabs::BasicBar<Config, Outlet, Mode, Area>>,
    std::is_constructible<Config, Options...>>::value>::type
#endif
    iterate( R&& range, Proc&& op, Act&& act, Options&&... options )
  {
    iterate<_details::prefabs::BasicBar<Config, Outlet, Mode, Area>>( std::forward<R>( range ),
                                                                      std::forward<Proc>( op ),
                                                                      std::forward<Act>( act ),
                                                                      std::forward<Options>( options )... );
  }
} // namespace pgbar

#endif
