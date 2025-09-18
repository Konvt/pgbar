#ifndef PGBAR_PROGRESSBAR
#define PGBAR_PROGRESSBAR

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
      class CharIndic : public Base {
      protected:
        __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_char( io::Stringbuf& buffer,
                                                         types::Float num_percent,
                                                         types::Size num_frame_cnt ) const
        {
          __PGBAR_TRUST( num_percent >= 0.0 );
          __PGBAR_TRUST( num_percent <= 1.0 );
          if ( this->bar_length_ == 0 )
            return buffer;

          const auto len_finished = static_cast<types::Size>( std::round( this->bar_length_ * num_percent ) );
          types::Size len_vacancy = this->bar_length_ - len_finished;

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->reversed_ ) {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( ' ', len_finished % this->filler_.width() );

            if ( !this->lead_.empty() ) {
              num_frame_cnt =
                static_cast<types::Size>( num_frame_cnt * this->shift_factor_ ) % this->lead_.size();
              const auto& current_lead = this->lead_[num_frame_cnt];
              if ( current_lead.width() <= len_vacancy ) {
                this->try_reset( buffer );
                this->try_dye( buffer, this->lead_col_ ).append( current_lead );
                len_vacancy -= current_lead.width();
              }
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ );
            buffer.append( ' ', len_vacancy % this->remains_.width() )
              .append( this->remains_, len_vacancy / this->remains_.width() );
          } else {
            const auto flag = [this, num_frame_cnt, &len_vacancy]() noexcept {
              if ( !this->lead_.empty() ) {
                const auto offset =
                  static_cast<types::Size>( num_frame_cnt * this->shift_factor_ ) % this->lead_.size();
                if ( this->lead_[offset].width() <= len_vacancy ) {
                  len_vacancy -= this->lead_[offset].width();
                  return true;
                }
              }
              return false;
            }();

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ );
            buffer.append( ' ', len_vacancy % this->remains_.width() )
              .append( this->remains_, len_vacancy / this->remains_.width() );

            if ( flag ) {
              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( this->lead_[num_frame_cnt] );
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( ' ', len_finished % this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        __PGBAR_EMPTY_COMPONENT( CharIndic )
      };
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER( assets::CharIndic,
                                assets::Countable,
                                __PGBAR_PACK( assets::Filler,
                                              assets::Remains,
                                              assets::BasicAnimation,
                                              assets::BasicIndicator,
                                              assets::Reversible ) );
      template<>
      struct OptionFor<assets::CharIndic>
        : Merge<OptionFor_t<assets::Countable>,
                OptionFor_t<assets::Reversible>,
                OptionFor_t<assets::Frames>,
                OptionFor_t<assets::Filler>,
                OptionFor_t<assets::Remains>,
                OptionFor_t<assets::BasicAnimation>,
                OptionFor_t<assets::BasicIndicator>> {};
    }
  } // namespace __details

  namespace config {
    class Line : public __details::prefabs::BasicConfig<__details::assets::CharIndic, Line> {
      using Base = __details::prefabs::BasicConfig<__details::assets::CharIndic, Line>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        // The types in the tuple are never repeated.
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Line::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Shift>::value )
          unpacker( *this, option::Shift( -2 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Lead>::value )
          unpacker( *this, option::Lead( u8">" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Starting>::value )
          unpacker( *this, option::Starting( u8"[" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Ending>::value )
          unpacker( *this, option::Ending( u8"]" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8"=" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Remains>::value )
          unpacker( *this, option::Remains( u8" " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::TpContain<ArgSet, option::Style>::value )
          unpacker( *this, option::Style( Base::Entire ) );
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
      Line( const Line& )              = default;
      Line( Line&& )                   = default;
      Line& operator=( const Line& ) & = default;
      Line& operator=( Line&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Line()    = default;
    };
  } // namespace config

  namespace __details {
    namespace traits {
      __PGBAR_BIND_BEHAVIOUR( config::Line, assets::BoundedFrameBar );
    }

    namespace render {
      template<>
      struct Builder<config::Line> final : public AnimatedBuilder<config::Line, Builder<config::Line>> {
      private:
        using Base = AnimatedBuilder<config::Line, Builder<config::Line>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Float num_percent,
                                                          types::Size num_frame_cnt ) const
        {
          return this->build_char( buffer, num_percent, num_frame_cnt );
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
          return this->indirect_build( buffer,
                                       num_task_done,
                                       num_all_tasks,
                                       num_percent,
                                       zero_point,
                                       num_percent,
                                       num_frame_cnt );
        }
      };
    } // namespace render
  } // namespace __details

  /**
   * The simplest progress bar, which is what you think it is.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using ProgressBar = __details::prefabs::BasicBar<config::Line, Outlet, Mode, Area>;
} // namespace pgbar

#endif
