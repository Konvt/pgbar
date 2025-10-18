#ifndef PGBAR_SWEEPBAR
#define PGBAR_SWEEPBAR

#include "details/prefabs/BasicBar.hpp"
#include "details/prefabs/BasicConfig.hpp"
#include "details/render/AnimatedBuilder.hpp"
#include "details/render/Builder.hpp"
#include "slice/TrackedSpan.hpp"

namespace pgbar {
  namespace _details {
    namespace assets {
      template<typename Base, typename Derived>
      class SweepIndic : public Base {
      protected:
        PGBAR__CXX20_CNSTXPR io::Stringbuf& build_sweep( io::Stringbuf& buffer,
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
              // virtual_point is a value between 1 and this->bar_width
              const auto virtual_point = [this, num_frame_cnt]() noexcept -> types::Size {
                if ( this->bar_width_ == 1 )
                  return 1;
                const auto period = 2 * this->bar_width_ - 2;
                const auto pos    = num_frame_cnt % period;
                return pos < this->bar_width_ ? pos + 1 : 2 * this->bar_width_ - pos - 1;
              }();
              const auto len_left_fill = [this, virtual_point, &current_lead]() noexcept -> types::Size {
                const auto len_half_lead = ( current_lead.width() / 2 ) + current_lead.width() % 2;
                if ( virtual_point <= len_half_lead )
                  return 0;
                const auto len_unreached = this->bar_width_ - virtual_point;
                if ( len_unreached <= len_half_lead - current_lead.width() % 2 )
                  return this->bar_width_ - current_lead.width();

                return virtual_point - len_half_lead;
              }();
              const auto len_right_fill = this->bar_width_ - ( len_left_fill + current_lead.width() );
              PGBAR__ASSERT( len_left_fill + len_right_fill + current_lead.width() == this->bar_width_ );

              this->try_reset( buffer );
              this->try_dye( buffer, this->filler_col_ )
                .append( this->filler_, len_left_fill / this->filler_.width() )
                .append( ' ', len_left_fill % this->filler_.width() );

              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( current_lead );

              this->try_reset( buffer );
              this->try_dye( buffer, this->filler_col_ )
                .append( ' ', len_right_fill % this->filler_.width() )
                .append( this->filler_, len_right_fill / this->filler_.width() );
            } else
              buffer.append( ' ', this->bar_width_ );
          } else if ( this->filler_.empty() )
            buffer.append( ' ', this->bar_width_ );
          else {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, this->bar_width_ / this->filler_.width() )
              .append( ' ', this->bar_width_ % this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        PGBAR__EMPTY_COMPONENT( SweepIndic )
      };
    } // namespace assets

    namespace traits {
      PGBAR__INHERIT_REGISTER( assets::SweepIndic,
                               assets::Filler,
                               assets::BasicIndicator,
                               assets::BasicAnimation );
      template<>
      struct OptionFor<assets::SweepIndic>
        : Merge<OptionFor_t<assets::Filler>,
                OptionFor_t<assets::BasicIndicator>,
                OptionFor_t<assets::BasicAnimation>,
                OptionFor_t<assets::Countable>> {};
    }
  } // namespace _details

  namespace config {
    class Sweep : public _details::prefabs::BasicConfig<_details::assets::SweepIndic, Sweep> {
      using Base = _details::prefabs::BasicConfig<_details::assets::SweepIndic, Sweep>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( _details::traits::InstanceOf<ArgSet, _details::traits::TypeSet>::value,
                       "pgbar::config::Sweep::initialize: Invalid template type" );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -3 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::BarWidth>::value )
          unpacker( *this, option::BarWidth( 30 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8"-" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8"<=>" ) );
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
      Sweep( const Sweep& )              = default;
      Sweep( Sweep&& )                   = default;
      Sweep& operator=( const Sweep& ) & = default;
      Sweep& operator=( Sweep&& ) &      = default;
      PGBAR__CXX20_CNSTXPR ~Sweep()      = default;
    };
  } // namespace config

  namespace _details {
    namespace traits {
      PGBAR__BIND_BEHAVIOUR( config::Sweep, assets::NullableFrameBar );
    }

    namespace render {
      template<>
      struct Builder<config::Sweep> final : public AnimatedBuilder<config::Sweep, Builder<config::Sweep>> {
      private:
        using Base = AnimatedBuilder<config::Sweep, Builder<config::Sweep>>;
        friend Base;

      protected:
        PGBAR__INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                         types::Size num_frame_cnt ) const
        {
          return this->build_sweep( buffer, num_frame_cnt );
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
   * A progress bar with a sweeping indicator, where the lead moves back and forth within the bar area.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using SweepBar = _details::prefabs::BasicBar<config::Sweep, Outlet, Mode, Area>;
} // namespace pgbar

#endif
