#ifndef PGBAR_FLOWBAR
#define PGBAR_FLOWBAR

#include "details/prefabs/BasicBar.hpp"
#include "details/prefabs/BasicConfig.hpp"
#include "details/render/AnimatedBuilder.hpp"
#include "details/render/Builder.hpp"
#include "details/traits/Util.hpp"
#include "slice/ProxySpan.hpp"

namespace pgbar {
  namespace __details {
    namespace assets {
      template<typename Base, typename Derived>
      class FlowIndic : public Base {
      protected:
        __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_flow( io::Stringbuf& buffer,
                                                         types::Size num_frame_cnt ) const
        {
          if ( this->bar_length_ == 0 )
            return buffer;

          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->lead_.empty() ) {
            const auto& current_lead = this->lead_[num_frame_cnt % this->lead_.size()];
            if ( current_lead.width() <= this->bar_length_ ) {
              // virtual_point is a value between 0 and this->bar_length - 1
              const auto virtual_point = [this, num_frame_cnt]() noexcept {
                const auto pos = num_frame_cnt % this->bar_length_;
                return !this->reversed_ ? pos : ( this->bar_length_ - 1 - pos ) % this->bar_length_;
              }();
              const auto len_vacancy = this->bar_length_ - virtual_point;

              if ( current_lead.width() <= len_vacancy ) {
                const auto len_right_fill = len_vacancy - current_lead.width();

                this->try_reset( buffer );
                this->try_dye( buffer, this->filler_col_ )
                  .append( this->filler_, virtual_point / this->filler_.width() )
                  .append( ' ', virtual_point % this->filler_.width() );
                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ ).append( current_lead );

                this->try_reset( buffer );
                this->try_dye( buffer, this->filler_col_ )
                  .append( ' ', len_right_fill % this->filler_.width() )
                  .append( this->filler_, len_right_fill / this->filler_.width() );
              } else {
                const auto division      = current_lead.split_by( len_vacancy );
                const auto len_left_fill = virtual_point - division.second.second;

                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ ).append( division.first[1], division.first[2] );
                this->try_reset( buffer );
                this->try_dye( buffer, this->filler_col_ )
                  .append( ' ', len_left_fill % this->filler_.width() )
                  .append( this->filler_, len_left_fill / this->filler_.width() );

                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ )
                  .append( division.first[0], division.first[1] )
                  .append( ' ', len_vacancy - division.second.first );
              }
            } else
              buffer.append( ' ', this->bar_length_ );
          } else if ( this->filler_.empty() )
            buffer.append( ' ', this->bar_length_ );
          else {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ );
            buffer.append( this->filler_, this->bar_length_ / this->filler_.width() )
              .append( ' ', this->bar_length_ % this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( FlowIndic )
      };
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER(
        assets::FlowIndic,
        ,
        __PGBAR_PACK( assets::Filler, assets::BasicAnimation, assets::BasicIndicator, assets::Reversible ) );
      template<>
      struct OptionFor<assets::FlowIndic>
        : Coalesce<OptionFor_t<assets::Countable>,
                   OptionFor_t<assets::Reversible>,
                   OptionFor_t<assets::Frames>,
                   OptionFor_t<assets::Filler>,
                   OptionFor_t<assets::BasicAnimation>,
                   OptionFor_t<assets::BasicIndicator>> {};
    }
  } // namespace __details

  namespace config {
    class Flow : public __details::prefabs::BasicConfig<__details::assets::FlowIndic, Flow> {
      using Base = __details::prefabs::BasicConfig<__details::assets::FlowIndic, Flow>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Flow::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8" " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8"====" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[__details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                  : 0 );
      }

    public:
      using Base::Base;
      Flow( const Flow& )              = default;
      Flow( Flow&& )                   = default;
      Flow& operator=( const Flow& ) & = default;
      Flow& operator=( Flow&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Flow()    = default;
    };
  } // namespace config

  namespace __details {
    namespace traits {
      __PGBAR_BIND_BEHAVIOUR( config::Flow, assets::NullableFrameBar );
    }

    namespace render {
      template<>
      struct Builder<config::Flow> final : public AnimatedBuilder<config::Flow, Builder<config::Flow>> {
      private:
        using Base = AnimatedBuilder<config::Flow, Builder<config::Flow>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Size num_frame_cnt ) const
        {
          return this->build_flow( buffer, num_frame_cnt );
        }

      public:
        using Base::Base;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          types::Size num_frame_cnt,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_frame_cnt );
        }
      };
    } // namespace render
  } // namespace __details

  /**
   * A progress bar with a flowing indicator, where the lead moves in a single direction within the bar area.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using FlowBar = __details::prefabs::BasicBar<config::Flow, Outlet, Mode, Area>;
} // namespace pgbar

#endif
