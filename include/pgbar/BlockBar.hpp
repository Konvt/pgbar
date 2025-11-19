#ifndef PGBAR_BLOCKBAR
#define PGBAR_BLOCKBAR

#include "details/prefabs/BasicBar.hpp"
#include "details/prefabs/BasicConfig.hpp"
#include "details/render/AnimatedBuilder.hpp"
#include "details/render/Builder.hpp"
#include "slice/TrackedSpan.hpp"

namespace pgbar {
  namespace _details {
    namespace assets {
      template<typename Base, typename Derived>
      class BlockIndic : public Base {
      protected:
        PGBAR__CXX23_CNSTXPR io::Stringbuf& build_block( io::Stringbuf& buffer,
                                                         types::Float num_percent ) const
        {
          PGBAR__TRUST( num_percent >= 0.0 );
          PGBAR__TRUST( num_percent <= 1.0 );
          if ( this->bar_width_ == 0 )
            return buffer;

          const auto len_finished     = static_cast<types::Size>( this->bar_width_ * num_percent );
          const types::Float fraction = ( this->bar_width_ * num_percent ) - len_finished;
          PGBAR__TRUST( fraction >= 0.0 );
          PGBAR__TRUST( fraction <= 1.0 );
          const auto incomplete_block = static_cast<types::Size>( fraction * this->lead_.size() );
          PGBAR__ASSERT( incomplete_block <= this->lead_.size() );
          types::Size len_vacancy = this->bar_width_ - len_finished;

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->reversed_ ) {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( ' ', len_finished % this->filler_.width() );

            if ( this->bar_width_ != len_finished && !this->lead_.empty()
                 && this->lead_[incomplete_block].width() <= len_vacancy ) {
              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( this->lead_[incomplete_block] );
              len_vacancy -= this->lead_[incomplete_block].width();
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ )
              .append( ' ', len_vacancy % this->remains_.width() )
              .append( this->remains_, len_vacancy / this->remains_.width() );
          } else {
            const auto flag = this->bar_width_ != len_finished && !this->lead_.empty()
                           && this->lead_[incomplete_block].width() <= len_vacancy;
            if ( flag )
              len_vacancy -= this->lead_[incomplete_block].width();

            this->try_reset( buffer );
            this->try_dye( buffer, this->remains_col_ )
              .append( this->remains_, len_vacancy / this->remains_.width() )
              .append( ' ', len_vacancy % this->remains_.width() );

            if ( flag ) {
              this->try_reset( buffer );
              this->try_dye( buffer, this->lead_col_ ).append( this->lead_[incomplete_block] );
            }

            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( ' ', len_finished % this->filler_.width() )
              .append( this->filler_, len_finished / this->filler_.width() );
          }

          this->try_reset( buffer );
          return this->try_dye( buffer, this->end_col_ ) << this->ending_;
        }

      public:
        PGBAR__EMPTY_COMPONENT( BlockIndic )
      };
    } // namespace assets

    namespace traits {
      PGBAR__INHERIT_REGISTER( assets::BlockIndic,
                               assets::Filler,
                               assets::Remains,
                               assets::BasicIndicator,
                               assets::Reversible,
                               assets::Frames,
                               assets::Countable );
      template<>
      struct OptionFor<assets::BlockIndic>
        : Merge<OptionFor_t<assets::Filler>,
                OptionFor_t<assets::Remains>,
                OptionFor_t<assets::Reversible>,
                OptionFor_t<assets::BasicIndicator>,
                OptionFor_t<assets::Frames>,
                OptionFor_t<assets::Countable>> {};
    } // namespace traits
  } // namespace _details

  namespace config {
    class Block : public _details::prefabs::BasicConfig<_details::assets::BlockIndic, Block> {
      using Base = _details::prefabs::BasicConfig<_details::assets::BlockIndic, Block>;
      friend Base;

      template<typename ArgSet>
      static void inject( Base& self )
      {
        static_assert( _details::traits::is_instance_of<ArgSet, _details::traits::TypeSet>::value,
                       "pgbar::config::Block::initialize: Invalid template type" );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Reversed>::value )
          unpacker( self, option::Reversed( false ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Lead>::value )
          unpacker( self,
                    option::Lead( { u8" ",
                                    u8"\u258F",
                                    u8"\u258E",
                                    u8"\u258D",
                                    u8"\u258C",
                                    u8"\u258B",
                                    u8"\u258A",
                                    u8"\u2589" } ) );
        // In some editing environments,
        // directly writing character literals can lead to very strange encoding conversion errors.
        // Therefore, here we use Unicode code points to directly specify the required characters.
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::BarWidth>::value )
          unpacker( self, option::BarWidth( 30 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Filler>::value )
          unpacker( self, option::Filler( u8"\u2588" ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Remains>::value )
          unpacker( self, option::Remains( u8" " ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Divider>::value )
          unpacker( self, option::Divider( u8" | " ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::InfoColor>::value )
          unpacker( self, option::InfoColor( color::Cyan ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::SpeedUnit>::value )
          unpacker( self, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Magnitude>::value )
          unpacker( self, option::Magnitude( 1000 ) );
        if PGBAR__CXX17_CNSTXPR ( !_details::traits::TpContain<ArgSet, option::Style>::value )
          unpacker( self, option::Style( Base::Entire ) );
      }

    protected:
      PGBAR__NODISCARD PGBAR__FORCEINLINE _details::types::Size fixed_render_size() const noexcept
      {
        return this->common_render_size()
             + ( this->visual_masks_[_details::utils::as_val( Base::Mask::Ani )] ? this->fixed_len_bar()
                                                                                 : 0 );
      }

    public:
      using Base::Base;
      Block( const Block& )              = default;
      Block( Block&& )                   = default;
      Block& operator=( const Block& ) & = default;
      Block& operator=( Block&& ) &      = default;
      ~Block()      = default;
    };
  } // namespace config

  namespace _details {
    namespace traits {
      PGBAR__BIND_BEHAVIOUR( config::Block, assets::PlainBar );
    }

    namespace render {
      template<>
      struct Builder<config::Block> final : public AnimatedBuilder<config::Block, Builder<config::Block>> {
      private:
        using Base = AnimatedBuilder<config::Block, Builder<config::Block>>;
        friend Base;

      protected:
        PGBAR__FORCEINLINE io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                           types::Float num_percent ) const
        {
          return this->build_block( buffer, num_percent );
        }

      public:
        using Base::Base;

        PGBAR__FORCEINLINE io::Stringbuf& build(
          io::Stringbuf& buffer,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          PGBAR__TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_percent );
        }
      };
    } // namespace render
  } // namespace _details

  /**
   * A progress bar with a smoother bar, requires an Unicode-supported terminal.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using BlockBar = _details::prefabs::BasicBar<config::Block, Outlet, Mode, Area>;
} // namespace pgbar
#endif
