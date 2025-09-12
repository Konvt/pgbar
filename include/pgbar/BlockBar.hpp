#ifndef PGBAR_BLOCKBAR
#define PGBAR_BLOCKBAR

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
      class BlockIndic : public Base {
      protected:
        __PGBAR_CXX23_CNSTXPR io::Stringbuf& build_block( io::Stringbuf& buffer,
                                                          types::Float num_percent ) const
        {
          __PGBAR_TRUST( num_percent >= 0.0 );
          __PGBAR_TRUST( num_percent <= 1.0 );
          if ( this->bar_length_ == 0 )
            return buffer;

          const auto len_finished     = static_cast<types::Size>( this->bar_length_ * num_percent );
          const types::Float fraction = ( this->bar_length_ * num_percent ) - len_finished;
          __PGBAR_TRUST( fraction >= 0.0 );
          __PGBAR_TRUST( fraction <= 1.0 );
          const auto incomplete_block = static_cast<types::Size>( fraction * this->lead_.size() );
          __PGBAR_ASSERT( incomplete_block <= this->lead_.size() );
          types::Size len_vacancy = this->bar_length_ - len_finished;

          this->try_reset( buffer );
          this->try_dye( buffer, this->start_col_ ) << this->starting_;

          if ( !this->reversed_ ) {
            this->try_reset( buffer );
            this->try_dye( buffer, this->filler_col_ )
              .append( this->filler_, len_finished / this->filler_.width() )
              .append( ' ', len_finished % this->filler_.width() );

            if ( this->bar_length_ != len_finished && !this->lead_.empty()
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
            const auto flag = this->bar_length_ != len_finished && !this->lead_.empty()
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
        __PGBAR_EMPTY_COMPONENT( BlockIndic )
      };
    } // namespace assets

    namespace traits {
      __PGBAR_INHERIT_REGISTER( assets::BlockIndic,
                                assets::Countable,
                                __PGBAR_PACK( assets::Filler,
                                              assets::Remains,
                                              assets::Frames,
                                              assets::BasicIndicator,
                                              assets::Reversible ) );
      template<>
      struct OptionFor<assets::BlockIndic>
        : Coalesce<OptionFor_t<assets::Countable>,
                   OptionFor_t<assets::Reversible>,
                   OptionFor_t<assets::Frames>,
                   OptionFor_t<assets::Filler>,
                   OptionFor_t<assets::Remains>,
                   OptionFor_t<assets::BasicAnimation>,
                   OptionFor_t<assets::BasicIndicator>> {};
    }
  } // namespace __details

  namespace config {
    class Block : public __details::prefabs::BasicConfig<__details::assets::BlockIndic, Block> {
      using Base = __details::prefabs::BasicConfig<__details::assets::BlockIndic, Block>;
      friend Base;

      template<typename ArgSet>
      void initialize()
      {
        static_assert( __details::traits::InstanceOf<ArgSet, __details::traits::TypeSet>::value,
                       "pgbar::config::Block::initialize: Invalid template type" );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Reversed>::value )
          unpacker( *this, option::Reversed( false ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Lead>::value )
          unpacker( *this,
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
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::BarLength>::value )
          unpacker( *this, option::BarLength( 30 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Filler>::value )
          unpacker( *this, option::Filler( u8"\u2588" ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Remains>::value )
          unpacker( *this, option::Remains( u8" " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Divider>::value )
          unpacker( *this, option::Divider( u8" | " ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::InfoColor>::value )
          unpacker( *this, option::InfoColor( color::Cyan ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::SpeedUnit>::value )
          unpacker( *this, option::SpeedUnit( { u8"Hz", u8"kHz", u8"MHz", u8"GHz" } ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Magnitude>::value )
          unpacker( *this, option::Magnitude( 1000 ) );
        if __PGBAR_CXX17_CNSTXPR ( !__details::traits::Exist<ArgSet, option::Style>::value )
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
      Block( const Block& )              = default;
      Block( Block&& )                   = default;
      Block& operator=( const Block& ) & = default;
      Block& operator=( Block&& ) &      = default;
      __PGBAR_CXX20_CNSTXPR ~Block()     = default;
    };
  } // namespace config

  namespace __details {
    namespace traits {
      __PGBAR_BIND_BEHAVIOUR( config::Block, assets::PlainBar );
    }

    namespace render {
      template<>
      struct Builder<config::Block> final : public AnimatedBuilder<config::Block, Builder<config::Block>> {
      private:
        using Base = AnimatedBuilder<config::Block, Builder<config::Block>>;
        friend Base;

      protected:
        __PGBAR_INLINE_FN io::Stringbuf& build_animation( io::Stringbuf& buffer,
                                                          types::Float num_percent ) const
        {
          return this->build_block( buffer, num_percent );
        }

      public:
        using Base::Base;

        __PGBAR_INLINE_FN io::Stringbuf& build(
          io::Stringbuf& buffer,
          std::uint64_t num_task_done,
          std::uint64_t num_all_tasks,
          const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          const auto num_percent = static_cast<types::Float>( num_task_done ) / num_all_tasks;

          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return this
            ->indirect_build( buffer, num_task_done, num_all_tasks, num_percent, zero_point, num_percent );
        }
      };
    } // namespace render
  } // namespace __details

  /**
   * A progress bar with a smoother bar, requires an Unicode-supported terminal.
   *
   * It's structure is shown below:
   * {LeftBorder}{Prefix}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{Postfix}{RightBorder}
   */
  template<Channel Outlet = Channel::Stderr, Policy Mode = Policy::Async, Region Area = Region::Fixed>
  using BlockBar = __details::prefabs::BasicBar<config::Block, Outlet, Mode, Area>;
} // namespace pgbar

#endif
