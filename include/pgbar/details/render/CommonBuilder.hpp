#ifndef PGBAR__COMMONBUILDER
#define PGBAR__COMMONBUILDER

#include "../io/Stringbuf.hpp"
#include "../utils/Backport.hpp"
// #include "../prefabs/BasicConfig.hpp"

namespace pgbar {
  namespace _details {
    namespace render {
      template<typename Config>
      struct CommonBuilder : public Config {
        using Config::Config;
        PGBAR__CXX23_CNSTXPR CommonBuilder( const CommonBuilder& lhs )              = default;
        PGBAR__CXX23_CNSTXPR CommonBuilder( CommonBuilder&& rhs )                   = default;
        PGBAR__CXX23_CNSTXPR CommonBuilder& operator=( const CommonBuilder& lhs ) & = default;
        PGBAR__CXX23_CNSTXPR CommonBuilder& operator=( CommonBuilder&& rhs ) &      = default;
        constexpr CommonBuilder( const Config& config )
          noexcept( std::is_nothrow_copy_constructible<Config>::value )
          : Config( config )
        {}
        constexpr CommonBuilder( Config&& config ) noexcept : Config( std::move( config ) ) {}
        PGBAR__CXX20_CNSTXPR ~CommonBuilder() = default;

#define PGBAR__METHOD( ParamType, Operation, Noexcept )                        \
  PGBAR__CXX14_CNSTXPR CommonBuilder& operator=( ParamType config ) & Noexcept \
  {                                                                            \
    Config::operator=( Operation( config ) );                                  \
    return *this;                                                              \
  }
        PGBAR__METHOD( const Config&, , noexcept( std::is_nothrow_copy_assignable<Config>::value ) )
        PGBAR__METHOD( Config&&, std::move, noexcept )
#undef PGBAR__METHOD

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
          PGBAR__TRUST( num_task_done <= num_all_tasks );
          using Self = Config;
          if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )]
               || this->visual_masks_[utils::as_val( Self::Mask::Sped )]
               || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
               || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] ) {
            if ( this->visual_masks_[utils::as_val( Self::Mask::Cnt )] ) {
              this->build_counter( buffer, num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
            }
            const auto time_passed = std::chrono::steady_clock::now() - zero_point;
            if ( this->visual_masks_[utils::as_val( Self::Mask::Sped )] ) {
              this->build_speed( buffer, time_passed, num_task_done, num_all_tasks );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )]
                   || this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
            }
            if ( this->visual_masks_[utils::as_val( Self::Mask::Elpsd )] ) {
              this->build_elapsed( buffer, time_passed );
              if ( this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
                buffer << this->divider_;
            }
            if ( this->visual_masks_[utils::as_val( Self::Mask::Cntdwn )] )
              this->build_countdown( buffer, time_passed, num_task_done, num_all_tasks );
          }
          return buffer;
        }
      };
    } // namespace render
  } // namespace _details
} // namespace pgbar

#endif
