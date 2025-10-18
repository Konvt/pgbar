#ifndef PGBAR_FLOWBAR
#define PGBAR_FLOWBAR

#include "details/prefabs/BasicBar.hpp"
#include "details/prefabs/BasicConfig.hpp"
#include "details/render/AnimatedBuilder.hpp"
#include "details/render/Builder.hpp"
#include "slice/TrackedSpan.hpp"

namespace pgbar {
  namespace _details {
    namespace assets {
      template<typename Base, typename Derived>
      class FlowIndic : public Base {
      protected:
        PGBAR__CXX23_CNSTXPR io::Stringbuf& build_flow( io::Stringbuf& buffer,
                                                        types::Size num_frame_cnt ) const
        {
          if ( this->bar_width_ == 0 )
            return buffer;

          num_frame_cnt = static_cast<types::Size>( num_frame_cnt * this->shift_factor_ );

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->lead_.empty() ) {
            const auto& current_lead = this->lead_[num_frame_cnt % this->lead_.size()];
            if ( current_lead.width() <= this->bar_width_ ) {
              // virtual_point is a value between 0 and this->bar_width - 1
              const auto virtual_point = [this, num_frame_cnt]() noexcept {
                const auto pos = num_frame_cnt % this->bar_width_;
                return !this->reversed_ ? pos : ( this->bar_width_ - 1 - pos ) % this->bar_width_;
              }();
              const auto len_vacancy = this->bar_width_ - virtual_point;

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
              buffer.append( ' ', this->bar_width_ );
          } else if ( this->filler_.empty() )
            buffer.append( ' ', this->bar_width_ );
          else {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ );
            buffer.append( this->filler_, this->bar_width_ / this->filler_.width() )
              .append( ' ', this->bar_width_ % this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        PGBAR__EMPTY_COMPONENT( FlowIndic )
      };
    } // namespace assets

    namespace traits {
      PGBAR__INHERIT_REGISTER( assets::FlowIndic,
                               assets::Filler,
                               assets::BasicAnimation,
                               assets::BasicIndicator,
                               assets::Reversible );
      template<>
      struct OptionFor<assets::FlowIndic>
        : Merge<OptionFor_t<assets::Countable>,
                OptionFor_t<assets::Reversible>,
                OptionFor_t<assets::Frames>,
                OptionFor_t<assets::Filler>,
                OptionFor_t<assets::BasicAnimation>,
                OptionFor_t<assets::BasicIndicator>> {};
    }
  } // namespace _details

  namespace config {
    class Flow : public _details::prefabs::BasicConfig<_details::assets::FlowIndic, Flow> {
      using Base = _details::prefabs::BasicConfig<_details::assets::FlowIndic, Flow>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( _details::traits::InstanceOf<ArgSet, _details::traits::TypeSet>::value,
                       "pgbar::config::Flow::initialize: Invalid template type" );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::BarWidth>::value )
          unpacker( *this, option::BarWidth( 30 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8" " ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8"====" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Ani | Base::Elpsd ) );
      }

    protected:
      PGBAR__NODISCARD PGBAR__INLINE_FN _details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[_details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                 : 0 );
      }

    public:
      using Base::Base;
      Flow( const Flow& )              = default;
      Flow( Flow&& )                   = default;
      Flow& operator=( const Flow& ) & = default;
      Flow& operator=( Flow&& ) &      = default;
      PGBAR__CXX20_CNSTXPR ~Flow()     = default;
    };
  } // namespace config

  namespace _details {
    namespace traits {
      PGBAR__BIND_BEHAVIOUR( config::Flow, assets::NullableFrameBar );
    }

    namespace render {
      template<>
      struct Builder<config::Flow> final : public AnimatedBuilder<config::Flow, Builder<config::Flow>> {
      private:
        using Base = AnimatedBuilder<config::Flow, Builder<config::Flow>>;
        friend Base;

      protected:
        PGBAR__INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                         types::Size num_frame_cnt ) const
        {
          return this->build_flow( buffer, num_frame_cnt );
        }

      public:
        using Base::Base;

        PGBAR__INLINE_FN io::Stringbuf& build( io::Stringbuf& buffer,
                                               types::Size num_frame_cnt,
                                               std::uint64_t num_task_done,
                                               std::uint64_t num_all_tasks,
                                               const std::chrono::steady_clock::time_point& zero_point ) const
        {
          PGBAR__TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_frame_cnt );
        }
      };
    } // namespace render
  } // namespace _details

  /**
   * A progress bar with a flowing indicator, where the lead moves in a single direction within the bar area.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using FlowBar = _details::prefabs::BasicBar<config::Flow, Outlet, Mode, Area>;
} // namespace pgbar

#endif
