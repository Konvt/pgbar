#ifndef __PGBAR_COMMONBUILDER
#define __PGBAR_COMMONBUILDER

#include "../io/Stringbuf.hpp"
#include "../utils/Util.hpp"
// #include "../prefabs/BasicConfig.hpp"

namespace pgbar {
  namespace __details {
    namespace render {
      template<typename Config>
      struct CommonBuilder : public Config {
        using Config::Config;
        __PGBAR_CXX23_CNSTXPR CommonBuilder( const CommonBuilder& lhs ) = default;
        __PGBAR_CXX23_CNSTXPR CommonBuilder( CommonBuilder&& rhs )      = default;
        constexpr CommonBuilder( const Config& config )
          noexcept( std::is_nothrow_copy_constructible<Config>::value )
          : Config( config )
        {}
        constexpr CommonBuilder( Config&& config ) noexcept : Config( std::move( config ) ) {}
        __PGBAR_CXX20_CNSTXPR ~CommonBuilder() = default;

#define __PGBAR_METHOD( ParamType, Operation, Noexcept )                        \
  __PGBAR_CXX14_CNSTXPR CommonBuilder& operator=( ParamType config ) & Noexcept \
  {                                                                             \
    Config::operator=( Operation( config ) );                                   \
    return *this;                                                               \
  }
        __PGBAR_METHOD( const CommonBuilder&, , noexcept( std::is_nothrow_copy_assignable<Config>::value ) )
        __PGBAR_METHOD( CommonBuilder&&, std::move, noexcept )
        __PGBAR_METHOD( const Config&, , noexcept( std::is_nothrow_copy_assignable<Config>::value ) )
        __PGBAR_METHOD( Config&&, std::move, noexcept )
#undef __PGBAR_METHOD

      protected:
        /**
         * Builds and only builds the components belows:
         * `CounterMeter`, `SpeedMeter`, `ElapsedTimer` and `CountdownTimer`
         */
        io::Stringbuf& common_build( io::Stringbuf& buffer,
                                     std::uint64_t num_task_done,
                                     std::uint64_t num_all_tasks,
                                     const std::chrono::steady_clock::time_point& zero_point ) const
        {
          __PGBAR_TRUST( num_task_done <= num_all_tasks );
          using Self = Config;
          if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )]
               || this->visual_masks_[utils::as_val( Self::Mask::Sped )]
               || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
               || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] ) {
            if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )] ) {
              buffer << this->build_counter( num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
            }
            const auto time_passed = std::chrono::steady_clock::now() - zero_point;
            if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )] ) {
              buffer << this->build_speed( time_passed, num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
            }
            if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                 && this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
              this->build_hybird( buffer, time_passed, num_task_done, num_all_tasks );
            else if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )] )
              buffer << this->build_elapsed( time_passed );
            else if ( this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
              buffer << this->build_countdown( time_passed, num_task_done, num_all_tasks );
          }
          return buffer;
        }
      };
    } // namespace render
  } // namespace __details
} // namespace pgbar

#endif
