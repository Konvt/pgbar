#ifndef PGBAR_SPINBAR
#define PGBAR_SPINBAR

#include "details/prefabs/BasicBar.hpp"
#include "details/prefabs/BasicConfig.hpp"
#include "details/render/Builder.hpp"
#include "details/render/CommonBuilder.hpp"
#include "slice/TrackedSpan.hpp"

namespace pgbar {
  namespace __details {
    namespace assets {
      template<typename Base, typename Derived>
      class SpinIndic : public Base {
      protected:
        __PGBAR_INLINE_FN __PGBAR_CXX20_CNSTXPR io::Stringbuf& build_spin( io::Stringbuf& buffer,
                                                                           types::Size num_frame_cnt ) const
        {
          if ( this->lead_.empty() )
            return buffer;
          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );
          num_frame_cnt %= this->lead_.size();
          __PGBAR_ASSERT( this->len_longest_lead_ >= this->lead_[num_frame_cnt].width() );

          this->try_reset( buffer );
          return this->try_style( buffer, this->lead_col_ )
              << utils::format<utils::TxtLayout::Left>( this->len_longest_lead_, this->lead_[num_frame_cnt] );
        }

      public:
        __PGBAR_EMPTY_COMPONENT( SpinIndic )
      };
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER( assets::SpinIndic, assets::BasicAnimation );
      template<>
      struct OptionFor<assets::SpinIndic>
        : Merge<OptionFor_t<assets::Countable>, OptionFor_t<assets::BasicAnimation>> {};
    }
  } // namespace __details

  namespace config {
    class Spin : public __details::prefabs::BasicConfig<__details::assets::SpinIndic, Spin> {
      using Base = __details::prefabs::BasicConfig<__details::assets::SpinIndic, Spin>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Spin::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( { u8"/", u8"-", u8"\\", u8"|" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )]
                   ? this->fixed_len_frames() + ( !this->prefix_.empty() )
                   : 0 );
      }

    public:
      using Base::Base;
      Spin( const Spin& )              = default;
      Spin( Spin&& )                   = default;
      Spin& operator=( const Spin& ) & = default;
      Spin& operator=( Spin&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Spin()    = default;
    };
  } // namespace config

  namespace __details {
    namespace traits {
      __PGBAR_BIND_BEHAVIOUR( config::Spin, assets::NullableFrameBar );
    }

    namespace render {
      template<>
      struct Builder<config::Spin> final : public CommonBuilder<config::Spin> {
      private:
        using Self = config::Spin;

      public:
        using CommonBuilder<Self>::CommonBuilder;

        io::Stringbuf& build( io::Stringbuf& buffer,
                              types::Size num_frame_cnt,
                              std::uint64_t num_task_done,
                              std::uint64_t num_all_tasks,
                              const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->l_border_;
          }

          this->build_prefix( buffer );
          this->try_reset( buffer );
          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            if ( !this->prefix_.empty() )
              buffer << ' ';
            this->build_spin( buffer, num_frame_cnt );
            this->try_reset( buffer );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) ).any() ) {
              this->try_style( buffer, this->info_col_ );
              buffer << this->divider_;
            }
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            this->build_percent( buffer, num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() )
              buffer << this->divider_;
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          if ( !this->postfix_.empty() && ( !this->prefix_.empty() || this->visual_masks_.any() ) )
            buffer << this->divider_;
          this->build_postfix( buffer );
          this->try_reset( buffer );
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->r_border_;
          }
          return this->try_reset( buffer );
        }
      };
    } // namespace render
  } // namespace __details

  /**
   * A progress bar without bar indicator, replaced by a fixed animation component.
   *
   * It's structure is shown below:
   * {LeftBorder}{Lead}{Prefix}{Percent}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using SpinBar = __details::prefabs::BasicBar<config::Spin, Outlet, Mode, Area>;
} // namespace pgbar

#endif
