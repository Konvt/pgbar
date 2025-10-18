#ifndef PGBAR__ANIMATEDBUILDER
#define PGBAR__ANIMATEDBUILDER

#include "../io/Stringbuf.hpp"
#include "../utils/Backport.hpp"
#include "CommonBuilder.hpp"
// #include "../prefabs/BasicConfig.hpp"

namespace pgbar {
  namespace _details {
    namespace render {
      template<typename Config, typename Impl>
      struct AnimatedBuilder : public CommonBuilder<Config> {
      private:
        using Self = Config;
        using Base = CommonBuilder<Config>;

      protected:
        template<typename... Args>
        io::Stringbuf& indirect_build( io::Stringbuf& buffer,
                                       std::uint64_t num_task_done,
                                       std::uint64_t num_all_tasks,
                                       types::Float num_percent,
                                       const std::chrono::steady_clock::time_point& zero_point,
                                       Args&&... args ) const
        {
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->l_border_;
          }

          this->build_prefix( buffer );
          this->try_reset( buffer );
          if ( this->visual_masks_.any() )
            this->try_style( buffer, this->info_col_ );
          if ( this->visual_masks_[utils::as_val( Self::Mask::Per )] ) {
            this->build_percent( buffer, num_percent );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Per ) ).any() )
              buffer << this->divider_;
          }
          if ( this->visual_masks_[utils::as_val( Self::Mask::Ani )] ) {
            static_cast<const Impl*>( this )->build_animation( buffer, std::forward<Args>( args )... );
            this->try_reset( buffer );
            auto masks = this->visual_masks_;
            if ( masks.reset( utils::as_val( Self::Mask::Ani ) )
                   .reset( utils::as_val( Self::Mask::Per ) )
                   .any() ) {
              this->try_style( buffer, this->info_col_ );
              buffer << this->divider_;
            }
          }
          this->common_build( buffer, num_task_done, num_all_tasks, zero_point );

          this->build_postfix( buffer );
          this->try_reset( buffer );
          if ( !this->prefix_.empty() || !this->postfix_.empty() || this->visual_masks_.any() ) {
            this->try_style( buffer, this->info_col_ );
            buffer << this->r_border_;
          }
          return this->try_reset( buffer );
        }

      public:
        using Base::Base;
      };
    } // namespace render
  } // namespace _details
} // namespace pgbar

#endif
