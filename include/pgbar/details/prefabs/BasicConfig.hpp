#ifndef PGBAR__BASICCONFIG
#define PGBAR__BASICCONFIG

#include "../assets/TUI.hpp"
#include <atomic>
#include <initializer_list>
#include <mutex>

namespace pgbar {
  namespace _details {
    namespace prefabs {
      template<template<typename...> class BarType, typename Derived>
      class BasicConfig
        : public traits::LI_t<BarType,
                              assets::PercentMeter,
                              assets::SpeedMeter,
                              assets::CounterMeter,
                              assets::Timer,
                              assets::Prefix,
                              assets::Postfix,
                              assets::Segment>::template type<assets::CoreConfig<Derived>, Derived> {
        // In fact, all the dependent components of BasicConfig can be fully injected from the outside.
        // This is not done here just to reduce repetitive code

        // BarType must inherit from BasicIndicator or BasicAnimation
        static_assert(
          traits::AnyOf<traits::TmpContain<traits::C3_t<BarType>, assets::BasicIndicator>,
                        traits::TmpContain<traits::C3_t<BarType>, assets::BasicAnimation>>::value,
          "pgbar::_details::prefabs::BasicConfig: Invalid progress bar type" );

        using Self = BasicConfig;
        using Base =
          typename traits::LI_t<BarType,
                                assets::PercentMeter,
                                assets::SpeedMeter,
                                assets::CounterMeter,
                                assets::Timer,
                                assets::Prefix,
                                assets::Postfix,
                                assets::Segment>::template type<assets::CoreConfig<Derived>, Derived>;
        using PermittedSet = traits::Merge_t<traits::TypeSet<option::Style>,
                                             traits::OptionFor_t<BarType>,
                                             traits::OptionFor_t<assets::SpeedMeter>,
                                             traits::OptionFor_t<assets::Timer>,
                                             traits::OptionFor_t<assets::CoreConfig>,
                                             traits::OptionFor_t<assets::Prefix>,
                                             traits::OptionFor_t<assets::Postfix>,
                                             traits::OptionFor_t<assets::Segment>>;

        friend PGBAR__FORCEINLINE PGBAR__CXX20_CNSTXPR void unpack( Self& cfg, option::Style&& val ) noexcept
        {
          cfg.visual_masks_ = val.value();
        }

        template<bool Enable>
        class Modifier final {
          friend Self;
          Self& self_;
          std::atomic<bool> owner_;

          Modifier( Self& self ) noexcept : self_ { self }, owner_ { true } { self_.rw_mtx_.lock(); }
          Modifier( Self& self, std::adopt_lock_t ) noexcept : self_ { self }, owner_ { true } {}
#if !PGBAR__CXX17
          // There was not standard NRVO support before C++17.
          Modifier( Modifier&& rhs ) noexcept : self_ { rhs.self_ }, owner_ { true }
          {
            rhs.owner_.store( false, std::memory_order_release );
          }
#endif
        public:
          ~Modifier() noexcept
          {
            if ( owner_.load( std::memory_order_acquire ) )
              self_.rw_mtx_.unlock();
          }
          Modifier( const Modifier& )              = delete;
          Modifier& operator=( const Modifier& ) & = delete;

#define PGBAR__METHOD( MethodName, EnumName )                       \
  Modifier&& MethodName()&& noexcept                                \
  {                                                                 \
    if ( owner_.load( std::memory_order_acquire ) )                 \
      self_.visual_masks_.set( utils::as_val( EnumName ), Enable ); \
    return static_cast<Modifier&&>( *this );                        \
  }
          PGBAR__METHOD( percent, Mask::Per )
          PGBAR__METHOD( animation, Mask::Ani )
          PGBAR__METHOD( counter, Mask::Cnt )
          PGBAR__METHOD( speed, Mask::Sped )
          PGBAR__METHOD( elapsed, Mask::Elpsd )
          PGBAR__METHOD( countdown, Mask::Cntdwn )
#undef PGBAR__METHOD
          Modifier&& entire() && noexcept
          {
            if ( owner_.load( std::memory_order_acquire ) ) {
              if PGBAR__CXX17_CNSTXPR ( Enable )
                self_.visual_masks_.set();
              else
                self_.visual_masks_.reset();
            }
            return static_cast<Modifier&&>( *this );
          }

          Modifier<!Enable> negate() && noexcept
          {
            auto negate = Modifier<!Enable>( this->self_, std::adopt_lock );
            owner_.store( false, std::memory_order_release );
            return negate;
          }
        };

      protected:
        enum class Mask : std::uint8_t { Per = 0, Ani, Cnt, Sped, Elpsd, Cntdwn };
        std::bitset<6> visual_masks_;

        PGBAR__NODISCARD PGBAR__FORCEINLINE types::Size common_render_size() const noexcept
        {
          return this->fixed_len_prefix() + this->fixed_len_postfix()
               + ( visual_masks_[utils::as_val( Mask::Per )] ? this->fixed_len_percent() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Cnt )] ? this->fixed_len_counter() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Sped )] ? this->fixed_len_speed() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Elpsd )] ? this->fixed_len_elapsed() : 0 )
               + ( visual_masks_[utils::as_val( Mask::Cntdwn )] ? this->fixed_len_countdown() : 0 )
               + this->fixed_len_segment( this->visual_masks_.count() );
        }

      public:
        // Percent Meter
        static constexpr types::Bit8 Per    = 1 << 0;
        // Animation
        static constexpr types::Bit8 Ani    = 1 << 1;
        // Task Progress Counter
        static constexpr types::Bit8 Cnt    = 1 << 2;
        // Speed Meter
        static constexpr types::Bit8 Sped   = 1 << 3;
        // Elapsed Timer
        static constexpr types::Bit8 Elpsd  = 1 << 4;
        // Countdown Timer
        static constexpr types::Bit8 Cntdwn = 1 << 5;
        // Enable all components
        static constexpr types::Bit8 Entire = ~0;

#ifdef __cpp_concepts
        template<typename... Args>
          requires( traits::Distinct<traits::TypeList<std::decay_t<Args>...>>::value
                    && ( traits::TpContain<PermittedSet, std::decay_t<Args>>::value && ... ) )
#else
        template<typename... Args,
                 typename = typename std::enable_if<traits::AllOf<
                   traits::Distinct<traits::TypeList<typename std::decay<Args>::type...>>,
                   traits::TpContain<PermittedSet, typename std::decay<Args>::type>...>::value>::type>
#endif
        PGBAR__CXX23_CNSTXPR BasicConfig( Args&&... args )
        {
          Derived::template inject<traits::TypeSet<typename std::decay<Args>::type...>>( *this );
          (void)std::initializer_list<bool> { ( unpack( *this, std::forward<Args>( args ) ), false )... };
        }

        BasicConfig( const Self& other )
          noexcept( traits::AllOf<std::is_nothrow_default_constructible<Base>,
                                  std::is_nothrow_copy_assignable<Base>>::value )
        {
          std::lock_guard<concurrent::SharedMutex> lock { other.rw_mtx_ };
          // Here we are calling the operator= of Base, which is lock-free.
          Base::operator=( other );
          visual_masks_ = other.visual_masks_;
        }
        BasicConfig( Self&& rhs ) noexcept
        {
          // static_assert(
          //   std::is_nothrow_default_constructible<Base>::value,
          //   "To ensure that the move ctor is strictly noexcept, "
          //   "it is necessary to require that the default constructor of the base class is noexcept." );
          std::lock_guard<concurrent::SharedMutex> lock { rhs.rw_mtx_ };
          Base::operator=( std::move( rhs ) );
          using std::swap;
          swap( visual_masks_, rhs.visual_masks_ );
        }
        Self& operator=( const Self& other ) & noexcept( std::is_nothrow_copy_assignable<Base>::value )
        {
          PGBAR__TRUST( this != &other );
          concurrent::SharedLock<concurrent::SharedMutex> lock1 { other.rw_mtx_, std::defer_lock };
          std::lock( this->rw_mtx_, lock1 );
          std::lock_guard<concurrent::SharedMutex> lock2 { this->rw_mtx_, std::adopt_lock };

          visual_masks_ = other.visual_masks_;
          Base::operator=( other );
          return *this;
        }
        Self& operator=( Self&& rhs ) & noexcept
        {
          PGBAR__ASSERT( this != &rhs );
          std::lock( this->rw_mtx_, rhs.rw_mtx_ );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_, std::adopt_lock };
          std::lock_guard<concurrent::SharedMutex> lock2 { rhs.rw_mtx_, std::adopt_lock };

          using std::swap;
          swap( visual_masks_, rhs.visual_masks_ );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
        /**
         * Note: Because there are no exposed public base classes for config type,
         * there should be no scenarios for managing derived objects using base class references.
         * So the destructor here is deliberately set to be non-virtual.
         */
        ~BasicConfig() = default;

        Derived& style( types::Bit8 val ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpack( *this, option::Style( val ) );
          return static_cast<Derived&>( *this );
        }

        template<typename Arg, typename... Args>
#ifdef __cpp_concepts
          requires( traits::Distinct<traits::TypeList<Arg, Args...>>::value
                    && traits::TpContain<PermittedSet, Arg>::value
                    && ( traits::TpContain<PermittedSet, Args>::value && ... ) )
        Derived&
#else
        typename std::enable_if<traits::AllOf<traits::Distinct<traits::TypeList<Arg, Args...>>,
                                              traits::TpContain<PermittedSet, Arg>,
                                              traits::TpContain<PermittedSet, Args>...>::value,
                                Derived&>::type
#endif
          with( Arg arg, Args... args ) &
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpack( *this, std::move( arg ) );
          (void)std::initializer_list<bool> { ( unpack( *this, std::move( args ) ), false )... };
          return static_cast<Derived&>( *this );
        }

        PGBAR__NODISCARD types::Size fixed_width() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return static_cast<const Derived*>( this )->fixed_render_size();
        }

        PGBAR__NODISCARD Modifier<true> enable() & noexcept { return Modifier<true>( *this ); }
        PGBAR__NODISCARD Modifier<false> disable() & noexcept { return Modifier<false>( *this ); }

        PGBAR__CXX23_CNSTXPR void swap( Self& other ) noexcept
        {
          std::lock( this->rw_mtx_, other.rw_mtx_ );
          std::lock_guard<concurrent::SharedMutex> lock1 { this->rw_mtx_, std::adopt_lock };
          std::lock_guard<concurrent::SharedMutex> lock2 { other.rw_mtx_, std::adopt_lock };
          using std::swap;
          swap( visual_masks_, other.visual_masks_ );
          Base::swap( other );
        }
        friend PGBAR__CXX23_CNSTXPR void swap( Self& a, Self& b ) noexcept { a.swap( b ); }

        template<typename Option>
        friend auto operator|=( Self& cfg, Option&& opt )
#ifdef __cpp_concepts
          requires traits::TpContain<PermittedSet, std::decay_t<Option>>::value
#else
          -> typename std::enable_if<
            traits::TpContain<PermittedSet, typename std::decay<Option>::type>::value>::type
#endif
        {
          cfg.with( std::forward<Option>( opt ) );
        }
        template<typename Option>
#ifdef __cpp_concepts
          requires traits::TpContain<PermittedSet, Option>::value
        friend Derived&
#else
        friend typename std::enable_if<traits::TpContain<PermittedSet, Option>::value, Derived&>::type
#endif
          operator|( Self& cfg, Option&& opt )
        {
          return cfg.with( std::forward<Option>( opt ) );
        }
        template<typename Option>
#ifdef __cpp_concepts
          requires traits::TpContain<PermittedSet, std::decay_t<Option>>::value
        friend Derived&&
#else
        friend
          typename std::enable_if<traits::TpContain<PermittedSet, typename std::decay<Option>::type>::value,
                                  Derived&&>::type
#endif
          operator|( Self&& cfg, Option&& opt )
        {
          return std::move( cfg.with( std::forward<Option>( opt ) ) );
        }
      };
    } // namespace prefabs

    namespace traits {
      template<typename C>
      struct is_config {
      private:
        template<template<typename...> class B, typename D>
        static constexpr std::true_type check( const prefabs::BasicConfig<B, D>& );
        static constexpr std::false_type check( ... );

      public:
        static constexpr bool value =
          AllOf<Not<std::is_reference<C>>,
                decltype( check( std::declval<typename std::remove_cv<C>::type>() ) )>::value;
      };
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
