#ifndef PGBAR__TUI
#define PGBAR__TUI

#include "../../option/Option.hpp"
#include "../../slice/NumericSpan.hpp"
#include "../concurrent/Backport.hpp"
#include "../io/Stringbuf.hpp"
#include "../traits/C3.hpp"
#include "../traits/TypeSet.hpp"
#include "../utils/Backport.hpp"
#include "../utils/Util.hpp"
#include <bitset>
#include <limits>

namespace pgbar {
  // The Basic components of the progress bar.
  namespace _details {
    namespace assets {
#define PGBAR__NONEMPTY_COMPONENT( ClassName, Constexpr )         \
  Constexpr ClassName( const ClassName& )              = default; \
  Constexpr ClassName( ClassName&& )                   = default; \
  Constexpr ClassName& operator=( const ClassName& ) & = default; \
  Constexpr ClassName& operator=( ClassName&& ) &      = default; \
  PGBAR__CXX20_CNSTXPR ~ClassName()                    = default;

#define PGBAR__EMPTY_COMPONENT( ClassName )                                  \
  constexpr ClassName()                                           = default; \
  constexpr ClassName( const ClassName& )                         = default; \
  constexpr ClassName( ClassName&& )                              = default; \
  PGBAR__CXX14_CNSTXPR ClassName& operator=( const ClassName& ) & = default; \
  PGBAR__CXX14_CNSTXPR ClassName& operator=( ClassName&& ) &      = default; \
  PGBAR__CXX20_CNSTXPR ~ClassName()                               = default;

      template<typename Derived>
      class CoreConfig {
#define PGBAR__UNPAKING( OptionName, MemberName )                                                 \
  friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR void unpacker( CoreConfig& cfg,                    \
                                                              option::OptionName&& val ) noexcept \
  {                                                                                               \
    cfg.fonts_[utils::as_val( Mask::OptionName )] = val.value();                                  \
  }
        PGBAR__UNPAKING( Colored, colored_ )
        PGBAR__UNPAKING( Bolded, bolded_ )
#undef PGBAR__UNPAKING

      protected:
        mutable concurrent::SharedMutex rw_mtx_;
        enum class Mask : std::uint8_t { Colored = 0, Bolded };
        std::bitset<2> fonts_;

        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR io::Stringbuf& try_dye(
          io::Stringbuf& buffer,
          const console::escodes::RGBColor& rgb ) const
        {
          if ( fonts_[utils::as_val( Mask::Colored )] )
            buffer << rgb;
          return buffer;
        }
        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR io::Stringbuf& try_style(
          io::Stringbuf& buffer,
          const console::escodes::RGBColor& rgb ) const
        {
          try_dye( buffer, rgb );
          if ( fonts_[utils::as_val( Mask::Bolded )] )
            buffer << console::escodes::fontbold;
          return buffer;
        }
        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR io::Stringbuf& try_reset( io::Stringbuf& buffer ) const
        {
          if ( fonts_.any() )
            buffer << console::escodes::fontreset;
          return buffer;
        }

      public:
        CoreConfig() noexcept : fonts_ { static_cast<types::Byte>( ~0 ) } {}
        constexpr CoreConfig( const CoreConfig& lhs ) : fonts_ { lhs.fonts_ } {}
        constexpr CoreConfig( CoreConfig&& rhs ) noexcept : fonts_ { rhs.fonts_ } {}
        PGBAR__CXX14_CNSTXPR CoreConfig& operator=( const CoreConfig& lhs ) &
        {
          fonts_ = lhs.fonts_;
          return *this;
        }
        PGBAR__CXX14_CNSTXPR CoreConfig& operator=( CoreConfig&& rhs ) & noexcept
        {
          fonts_ = rhs.fonts_;
          return *this;
        }
        PGBAR__CXX20_CNSTXPR ~CoreConfig() = default;

#define PGBAR__METHOD( OptionName, ParamName )               \
  std::lock_guard<concurrent::SharedMutex> lock { rw_mtx_ }; \
  unpacker( *this, option::OptionName( ParamName ) );        \
  return static_cast<Derived&>( *this )

        // Enable or disable the color effect.
        Derived& colored( bool _enable ) & noexcept
        {
          PGBAR__METHOD( Colored, _enable );
        }
        // Enable or disable the bold effect.
        Derived& bolded( bool _enable ) & noexcept
        {
          PGBAR__METHOD( Bolded, _enable );
        }

#undef PGBAR__METHOD
#define PGBAR__METHOD( Offset )                                     \
  concurrent::SharedLock<concurrent::SharedMutex> lock { rw_mtx_ }; \
  return fonts_[utils::as_val( Mask::Offset )]

        // Check whether the color effect is enabled.
        PGBAR__NODISCARD bool colored() const noexcept
        {
          PGBAR__METHOD( Colored );
        }
        // Check whether the bold effect is enabled.
        PGBAR__NODISCARD bool bolded() const noexcept
        {
          PGBAR__METHOD( Bolded );
        }

#undef PGBAR__METHOD

        PGBAR__CXX14_CNSTXPR void swap( CoreConfig& lhs ) noexcept
        {
          std::swap( fonts_, lhs.fonts_ );
        }
      };

      template<typename Base, typename Derived>
      class Countable : public Base {
        friend PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void unpacker( Countable& cfg,
                                                                    option::Tasks&& val ) noexcept
        {
          cfg.task_range_ = slice::NumericSpan<std::uint64_t>( val.value() );
        }

      protected:
        slice::NumericSpan<std::uint64_t> task_range_;

      public:
        constexpr Countable() = default;
        PGBAR__NONEMPTY_COMPONENT( Countable, PGBAR__CXX14_CNSTXPR )

        // Set the number of tasks, passing in zero is no exception.
        Derived& tasks( std::uint64_t param ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Tasks( param ) );
          return static_cast<Derived&>( *this );
        }
        // Get the current number of tasks.
        PGBAR__NODISCARD std::uint64_t tasks() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return task_range_.back();
        }

        PGBAR__CXX14_CNSTXPR void swap( Countable& lhs ) noexcept
        {
          task_range_.swap( lhs.task_range_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Reversible : public Base {
        friend PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void unpacker( Reversible& cfg,
                                                                    option::Reversed&& val ) noexcept
        {
          cfg.reversed_ = val.value();
        }

      protected:
        bool reversed_;

      public:
        constexpr Reversible() = default;
        PGBAR__NONEMPTY_COMPONENT( Reversible, PGBAR__CXX14_CNSTXPR )

        Derived& reverse( bool flag ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Reversed( flag ) );
          return static_cast<Derived&>( *this );
        }

        PGBAR__NODISCARD bool reverse() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return reversed_;
        }

        PGBAR__CXX20_CNSTXPR void swap( Reversible& lhs ) noexcept
        {
          std::swap( reversed_, lhs.reversed_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Frames : public Base {
        friend PGBAR__INLINE_FN void unpacker( Frames& cfg, option::LeadColor&& val ) noexcept
        {
          cfg.lead_col_ = val.value();
        }
        friend PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void unpacker( Frames& cfg, option::Lead&& val ) noexcept
        {
          if ( std::all_of( val.value().cbegin(),
                            val.value().cend(),
                            []( const charcodes::U8Raw& ele ) noexcept { return ele.empty(); } ) ) {
            cfg.lead_.clear();
            cfg.len_longest_lead_ = 0;
          } else {
            cfg.lead_ = std::move( val.value() );
            cfg.len_longest_lead_ =
              std::max_element( cfg.lead_.cbegin(),
                                cfg.lead_.cend(),
                                []( const charcodes::U8Raw& a, const charcodes::U8Raw& b ) noexcept {
                                  return a.width() < b.width();
                                } )
                ->width();
          }
        }

      protected:
        std::vector<charcodes::U8Text> lead_;
        console::escodes::RGBColor lead_col_;
        types::Size len_longest_lead_;

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::Size fixed_len_frames() const noexcept
        {
          return len_longest_lead_;
        }

      public:
        PGBAR__CXX20_CNSTXPR Frames() = default;
        PGBAR__NONEMPTY_COMPONENT( Frames, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& lead( std::vector<types::String> _leads ) &
        {
          PGBAR__METHOD( Lead, _leads, std::move );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& lead( types::String _lead ) &
        {
          PGBAR__METHOD( Lead, _lead, std::move );
        }
#if PGBAR__CXX20
        Derived& lead( const std::vector<types::LitU8>& _leads ) &
        {
          PGBAR__METHOD( Lead, _leads, );
        }
        Derived& lead( types::LitU8 _lead ) &
        {
          PGBAR__METHOD( Lead, _lead, );
        }
#endif

        // Set the color of the component `lead`.
        Derived& lead_color( types::HexRGB _lead_color ) &
        {
          PGBAR__METHOD( LeadColor, _lead_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& lead_color( types::ROStr _lead_color ) &
        {
          PGBAR__METHOD( LeadColor, _lead_color, );
        }

#undef PGBAR__METHOD

        PGBAR__CXX20_CNSTXPR void swap( Frames& lhs ) noexcept
        {
          using std::swap;
          lead_col_.swap( lhs.lead_col_ );
          lead_.swap( lhs.lead_ );
          swap( len_longest_lead_, lhs.len_longest_lead_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Filler : public Base {
#define PGBAR__UNPAKING( OptionName, MemberName, Constexpr )                                        \
  friend PGBAR__INLINE_FN Constexpr void unpacker( Filler& cfg, option::OptionName&& val ) noexcept \
  {                                                                                                 \
    cfg.MemberName = std::move( val.value() );                                                      \
  }
        PGBAR__UNPAKING( Filler, filler_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( FillerColor, filler_col_, )
#undef PGBAR__UNPAKING

      protected:
        charcodes::U8Raw filler_;
        console::escodes::RGBColor filler_col_;

      public:
        PGBAR__CXX20_CNSTXPR Filler() = default;
        PGBAR__NONEMPTY_COMPONENT( Filler, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& filler( types::String _filler ) &
        {
          PGBAR__METHOD( Filler, _filler, std::move );
        }
#if PGBAR__CXX20
        Derived& filler( types::LitU8 _filler ) &
        {
          PGBAR__METHOD( Filler, _filler, );
        }
#endif
        Derived& filler_color( types::HexRGB _filler_color ) &
        {
          PGBAR__METHOD( FillerColor, _filler_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& filler_color( types::ROStr _filler_color ) &
        {
          PGBAR__METHOD( FillerColor, _filler_color, );
        }

#undef PGBAR__METHOD
        PGBAR__CXX20_CNSTXPR void swap( Filler& lhs ) noexcept
        {
          filler_.swap( lhs.filler_ );
          filler_col_.swap( lhs.filler_col_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Remains : public Base {
#define PGBAR__UNPAKING( OptionName, MemberName, Constexpr )                                         \
  friend PGBAR__INLINE_FN Constexpr void unpacker( Remains& cfg, option::OptionName&& val ) noexcept \
  {                                                                                                  \
    cfg.MemberName = std::move( val.value() );                                                       \
  }
        PGBAR__UNPAKING( Remains, remains_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( RemainsColor, remains_col_, )
#undef PGBAR__UNPAKING

      protected:
        charcodes::U8Raw remains_;
        console::escodes::RGBColor remains_col_;

      public:
        PGBAR__CXX20_CNSTXPR Remains() = default;
        PGBAR__NONEMPTY_COMPONENT( Remains, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        Derived& remains_color( types::HexRGB _remains_color ) &
        {
          PGBAR__METHOD( RemainsColor, _remains_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& remains_color( types::ROStr _remains_color ) &
        {
          PGBAR__METHOD( RemainsColor, _remains_color, );
        }

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& remains( types::String _remains ) &
        {
          PGBAR__METHOD( Remains, _remains, std::move );
        }
#if PGBAR__CXX20
        Derived& remains( types::LitU8 _remains ) &
        {
          PGBAR__METHOD( Remains, _remains, );
        }
#endif
#undef PGBAR__METHOD

        PGBAR__CXX20_CNSTXPR void swap( Remains& lhs ) noexcept
        {
          remains_col_.swap( lhs.remains_col_ );
          remains_.swap( lhs.remains_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class BasicAnimation : public Base {
        friend PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR void unpacker( BasicAnimation& cfg,
                                                                    option::Shift&& val ) noexcept
        {
          cfg.shift_factor_ = val.value() < 0 ? ( 1.0 / ( -val.value() ) ) : val.value();
        }

      protected:
        types::Float shift_factor_;

      public:
        PGBAR__CXX20_CNSTXPR BasicAnimation() = default;
        PGBAR__NONEMPTY_COMPONENT( BasicAnimation, PGBAR__CXX20_CNSTXPR )

        /**
         * Set the rate factor of the animation with negative value slowing down the switch per frame
         * and positive value speeding it up.
         *
         * The maximum and minimum of the rate factor is between -128 and 127.
         *
         * If the value is zero, freeze the animation.
         */
        Derived& shift( std::int8_t _shift_factor ) & noexcept
        {
          std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ };
          unpacker( *this, option::Shift( _shift_factor ) );
          return static_cast<Derived&>( *this );
        }

        PGBAR__CXX20_CNSTXPR void swap( BasicAnimation& lhs ) noexcept
        {
          using std::swap;
          swap( shift_factor_, lhs.shift_factor_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class BasicIndicator : public Base {
#define PGBAR__UNPAKING( OptionName, MemberName, Constexpr )                                                \
  friend PGBAR__INLINE_FN Constexpr void unpacker( BasicIndicator& cfg, option::OptionName&& val ) noexcept \
  {                                                                                                         \
    cfg.MemberName = std::move( val.value() );                                                              \
  }
        PGBAR__UNPAKING( Starting, starting_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( Ending, ending_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( BarWidth, bar_width_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( StartColor, start_col_, )
        PGBAR__UNPAKING( EndColor, end_col_, )
#undef PGBAR__UNPAKING

      protected:
        types::Size bar_width_;
        charcodes::U8Raw starting_, ending_;
        console::escodes::RGBColor start_col_, end_col_;

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::Size fixed_len_bar() const noexcept
        {
          return starting_.width() + ending_.width();
        }

      public:
        PGBAR__CXX20_CNSTXPR BasicIndicator() = default;
        PGBAR__NONEMPTY_COMPONENT( BasicIndicator, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& starting( types::String _starting ) &
        {
          PGBAR__METHOD( Starting, _starting, std::move );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& ending( types::String _ending ) &
        {
          PGBAR__METHOD( Ending, _ending, std::move );
        }
#if PGBAR__CXX20
        Derived& starting( types::LitU8 _starting ) &
        {
          PGBAR__METHOD( Starting, _starting, );
        }
        Derived& ending( types::LitU8 _ending ) &
        {
          PGBAR__METHOD( Ending, _ending, );
        }
#endif

        Derived& start_color( types::HexRGB _start_color ) &
        {
          PGBAR__METHOD( StartColor, _start_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& start_color( types::ROStr _start_color ) &
        {
          PGBAR__METHOD( StartColor, _start_color, );
        }
        Derived& end_color( types::HexRGB _end_color ) &
        {
          PGBAR__METHOD( EndColor, _end_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& end_color( types::ROStr _end_color ) &
        {
          PGBAR__METHOD( EndColor, _end_color, );
        }
        // Set the width of the bar indicator.
        Derived& bar_width( types::Size _width ) & noexcept
        {
          PGBAR__METHOD( BarWidth, _width, );
        }

        PGBAR__NODISCARD types::Size bar_width() const noexcept
        {
          concurrent::SharedLock<concurrent::SharedMutex> lock { this->rw_mtx_ };
          return bar_width_;
        }

#undef PGBAR__METHOD
        PGBAR__CXX20_CNSTXPR void swap( BasicIndicator& lhs ) noexcept
        {
          std::swap( bar_width_, lhs.bar_width_ );
          starting_.swap( lhs.starting_ );
          ending_.swap( lhs.ending_ );
          start_col_.swap( lhs.start_col_ );
          end_col_.swap( lhs.end_col_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Prefix : public Base {
#define PGBAR__UNPAKING( OptionName, MemberName, Constexpr )                                        \
  friend PGBAR__INLINE_FN Constexpr void unpacker( Prefix& cfg, option::OptionName&& val ) noexcept \
  {                                                                                                 \
    cfg.MemberName = std::move( val.value() );                                                      \
  }
        PGBAR__UNPAKING( Prefix, prefix_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( PrefixColor, prfx_col_, )
#undef PGBAR__UNPAKING

      protected:
        charcodes::U8Raw prefix_;
        console::escodes::RGBColor prfx_col_;

        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR io::Stringbuf& build_prefix( io::Stringbuf& buffer ) const
        {
          if ( prefix_.empty() )
            return buffer;
          this->try_reset( buffer );
          return this->try_style( buffer, prfx_col_ ) << prefix_ << ' ';
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::Size fixed_len_prefix() const noexcept
        {
          return prefix_.width() + !prefix_.empty();
        }

      public:
        PGBAR__CXX20_CNSTXPR Prefix() = default;
        PGBAR__NONEMPTY_COMPONENT( Prefix, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& prefix( types::String _prefix ) &
        {
          PGBAR__METHOD( Prefix, _prefix, std::move );
        }

#if PGBAR__CXX20
        Derived& prefix( types::LitU8 _prefix ) &
        {
          PGBAR__METHOD( Prefix, _prefix, );
        }
#endif

        Derived& prefix_color( types::HexRGB _prfx_color ) &
        {
          PGBAR__METHOD( PrefixColor, _prfx_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& prefix_color( types::ROStr _prfx_color ) &
        {
          PGBAR__METHOD( PrefixColor, _prfx_color, );
        }

#undef PGBAR__METHOD

        PGBAR__CXX20_CNSTXPR void swap( Prefix& lhs ) noexcept
        {
          prfx_col_.swap( lhs.prfx_col_ );
          prefix_.swap( lhs.prefix_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Postfix : public Base {
#define PGBAR__UNPAKING( OptionName, MemberName, Constexpr )                                         \
  friend PGBAR__INLINE_FN Constexpr void unpacker( Postfix& cfg, option::OptionName&& val ) noexcept \
  {                                                                                                  \
    cfg.MemberName = std::move( val.value() );                                                       \
  }
        PGBAR__UNPAKING( Postfix, postfix_, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( PostfixColor, pstfx_col_, )
#undef PGBAR__UNPAKING

      protected:
        charcodes::U8Raw postfix_;
        console::escodes::RGBColor pstfx_col_;

        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR io::Stringbuf& build_postfix( io::Stringbuf& buffer ) const
        {
          if ( postfix_.empty() )
            return buffer;
          this->try_reset( buffer );
          return this->try_style( buffer, pstfx_col_ ) << ' ' << postfix_;
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::Size fixed_len_postfix() const noexcept
        {
          return postfix_.width() + !postfix_.empty();
        }

      public:
        PGBAR__CXX20_CNSTXPR Postfix() = default;
        PGBAR__NONEMPTY_COMPONENT( Postfix, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& postfix( types::String _postfix ) &
        {
          PGBAR__METHOD( Postfix, _postfix, std::move );
        }
#if PGBAR__CXX20
        Derived& postfix( types::LitU8 _postfix ) &
        {
          PGBAR__METHOD( Postfix, _postfix, );
        }
#endif

        Derived& postfix_color( types::HexRGB _pstfx_color ) &
        {
          PGBAR__METHOD( PostfixColor, _pstfx_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& postfix_color( types::ROStr _pstfx_color ) &
        {
          PGBAR__METHOD( PostfixColor, _pstfx_color, );
        }

#undef PGBAR__METHOD

        PGBAR__CXX20_CNSTXPR void swap( Postfix& lhs ) noexcept
        {
          pstfx_col_.swap( lhs.pstfx_col_ );
          postfix_.swap( lhs.postfix_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class Segment : public Base {
#define PGBAR__UNPAKING( OptionName, MemberName, Operation, Constexpr )                              \
  friend PGBAR__INLINE_FN Constexpr void unpacker( Segment& cfg, option::OptionName&& val ) noexcept \
  {                                                                                                  \
    cfg.MemberName = Operation( val.value() );                                                       \
  }
        PGBAR__UNPAKING( InfoColor, info_col_, std::move, )
        PGBAR__UNPAKING( Divider, divider_, std::move, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( LeftBorder, l_border_, std::move, PGBAR__CXX20_CNSTXPR )
        PGBAR__UNPAKING( RightBorder, r_border_, std::move, PGBAR__CXX20_CNSTXPR )
#undef PGBAR__UNPAKING

      protected:
        charcodes::U8Raw divider_;
        charcodes::U8Raw l_border_, r_border_;
        console::escodes::RGBColor info_col_;

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR types::Size fixed_len_segment(
          types::Size num_column ) const noexcept
        {
          switch ( num_column ) {
          case 0:  return 0;
          case 1:  return l_border_.width() + r_border_.width();
          default: return ( num_column - 1 ) * divider_.width() + l_border_.width() + r_border_.width();
          }
        }

      public:
        PGBAR__CXX20_CNSTXPR Segment() = default;
        PGBAR__NONEMPTY_COMPONENT( Segment, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName, Operation )          \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( Operation( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& divider( types::String _divider ) &
        {
          PGBAR__METHOD( Divider, _divider, std::move );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& left_border( types::String _l_border ) &
        {
          PGBAR__METHOD( LeftBorder, _l_border, std::move );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         */
        Derived& right_border( types::String _r_border ) &
        {
          PGBAR__METHOD( RightBorder, _r_border, std::move );
        }
#if PGBAR__CXX20
        Derived& divider( types::LitU8 _divider ) &
        {
          PGBAR__METHOD( Divider, _divider, );
        }
        Derived& left_border( types::LitU8 _l_border ) &
        {
          PGBAR__METHOD( LeftBorder, _l_border, );
        }
        Derived& right_border( types::LitU8 _r_border ) &
        {
          PGBAR__METHOD( RightBorder, _r_border, );
        }
#endif

        Derived& info_color( types::HexRGB _info_color ) &
        {
          PGBAR__METHOD( InfoColor, _info_color, );
        }
        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters is not a valid RGB color string.
         */
        Derived& info_color( types::ROStr _info_color ) &
        {
          PGBAR__METHOD( InfoColor, _info_color, );
        }

#undef PGBAR__METHOD

        PGBAR__CXX20_CNSTXPR void swap( Segment& lhs ) & noexcept
        {
          info_col_.swap( lhs.info_col_ );
          divider_.swap( lhs.divider_ );
          l_border_.swap( lhs.l_border_ );
          r_border_.swap( lhs.r_border_ );
          Base::swap( lhs );
        }
      };

      template<typename Base, typename Derived>
      class PercentMeter : public Base {
#define PGBAR__DEFAULT_PERCENT u8" --.--%"

      protected:
        PGBAR__INLINE_FN io::Stringbuf& build_percent( io::Stringbuf& buffer, types::Float num_percent ) const
        {
          PGBAR__TRUST( num_percent >= 0.0 );
          PGBAR__TRUST( num_percent <= 1.0 );

          if ( num_percent <= 0.0 ) // 0.01%
            PGBAR__UNLIKELY return buffer << PGBAR__DEFAULT_PERCENT;

          auto orig = utils::format( num_percent * 100.0, 2 );
          orig.push_back( '%' );
          return buffer << utils::format<utils::TxtLayout::Right>( fixed_len_percent(), orig );
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr types::Size fixed_len_percent() const noexcept
        {
          return sizeof( PGBAR__DEFAULT_PERCENT ) - 1;
        }

      public:
        PGBAR__EMPTY_COMPONENT( PercentMeter )
      };
#undef PGBAR__DEFAULT_PERCENT

      template<typename Base, typename Derived>
      class SpeedMeter : public Base {
        friend PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void unpacker( SpeedMeter& cfg,
                                                                    option::SpeedUnit&& val ) noexcept
        {
          cfg.units_            = std::move( val.value() );
          cfg.nth_longest_unit_ = static_cast<std::uint8_t>( std::distance(
            cfg.units_.cbegin(),
            std::max_element( cfg.units_.cbegin(),
                              cfg.units_.cend(),
                              []( const charcodes::U8Raw& a, const charcodes::U8Raw& b ) noexcept {
                                return a.width() < b.width();
                              } ) ) );
        }
        friend PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void unpacker( SpeedMeter& cfg,
                                                                    option::Magnitude&& val ) noexcept
        {
          cfg.magnitude_ = val.value();
        }

#define PGBAR__DEFAULT_SPEED u8"   inf " // The width prepared for "999.99 "
        static constexpr types::Size _fixed_width = sizeof( PGBAR__DEFAULT_SPEED ) - 1;

      protected:
        std::array<charcodes::U8Raw, 4> units_;
        std::uint16_t magnitude_;
        std::uint8_t nth_longest_unit_;

        io::Stringbuf& build_speed( io::Stringbuf& buffer,
                                    const types::TimeUnit& time_passed,
                                    std::uint64_t num_task_done,
                                    std::uint64_t num_all_tasks ) const
        {
          PGBAR__TRUST( num_task_done <= num_all_tasks );
          if ( num_all_tasks == 0 )
            PGBAR__UNLIKELY return buffer
              << utils::format<utils::TxtLayout::Right>( fixed_len_speed(), u8"-- " + units_.front() );

          /* Since the cube of the maximum value of std::uint16_t does not exceed
           * the representable range of std::uint64_t,
           * we choose to use std::uint16_t to represent the scaling magnitude. */
          const types::Size tier1 = magnitude_ * magnitude_;
          const types::Size tier2 = tier1 * magnitude_;
          // tier0 is magnitude_ itself

          const auto seconds_passed    = std::chrono::duration<types::Float>( time_passed ).count();
          // zero or negetive is invalid
          const types::Float frequency = seconds_passed <= 0.0 ? ( std::numeric_limits<types::Float>::max )()
                                                               : num_task_done / seconds_passed;

          types::String orig;
          if ( frequency < magnitude_ )
            orig = utils::format( frequency, 2 ) + ' ' + units_[0];
          else if ( frequency < tier1 ) // "kilo"
            orig = utils::format( frequency / magnitude_, 2 ) + ' ' + units_[1];
          else if ( frequency < tier2 ) // "Mega"
            orig = utils::format( frequency / tier1, 2 ) + ' ' + units_[2];
          else { // "Giga" or "infinity"
            const types::Float remains = frequency / tier2;
            if ( remains > magnitude_ )
              PGBAR__UNLIKELY orig = PGBAR__DEFAULT_SPEED + units_[3];
            else
              orig = utils::format( remains, 2 ) + ' ' + units_[3];
          }

          return buffer << utils::format<utils::TxtLayout::Right>( fixed_len_speed(), orig );
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr types::Size fixed_len_speed() const noexcept
        {
          return _fixed_width + units_[nth_longest_unit_].width();
        }

      public:
        PGBAR__CXX20_CNSTXPR SpeedMeter() = default;
        PGBAR__NONEMPTY_COMPONENT( SpeedMeter, PGBAR__CXX20_CNSTXPR )

#define PGBAR__METHOD( OptionName, ParamName )                     \
  std::lock_guard<concurrent::SharedMutex> lock { this->rw_mtx_ }; \
  unpacker( *this, option::OptionName( std::move( ParamName ) ) ); \
  return static_cast<Derived&>( *this )

        /**
         * @throw exception::InvalidArgument
         *
         * If the passed parameters are not coding in UTF-8.
         *
         * @param _units
         * The given each unit will be treated as 1,000 times greater than the previous one
         * (from left to right).
         */
        Derived& speed_unit( std::array<types::String, 4> _units ) &
        {
          PGBAR__METHOD( SpeedUnit, _units );
        }
#if PGBAR__CXX20
        /**
         * @param _units
         * The given each unit will be treated as 1,000 times greater than the previous one
         * (from left to right).
         */
        Derived& speed_unit( std::array<types::LitU8, 4> _units ) &
        {
          PGBAR__METHOD( SpeedUnit, _units );
        }
#endif

        /**
         * @param _magnitude
         * The base magnitude for unit scaling in formatted output.
         *
         * Defines the threshold at which values are converted to higher-order units
         * (e.g. 1000 -> "1k", 1000000 -> "1M").
         */
        Derived& magnitude( std::uint16_t _magnitude ) & noexcept
        {
          PGBAR__METHOD( Magnitude, _magnitude );
        }

#undef PGBAR__METHOD

        PGBAR__CXX20_CNSTXPR void swap( SpeedMeter& lhs ) & noexcept
        {
          units_.swap( lhs.units_ );
          std::swap( nth_longest_unit_, lhs.nth_longest_unit_ );
          Base::swap( lhs );
        }
      };
#undef PGBAR__DEFAULT_SPEED

      template<typename Base, typename Derived>
      class CounterMeter : public Base {
      protected:
        PGBAR__INLINE_FN io::Stringbuf& build_counter( io::Stringbuf& buffer,
                                                       std::uint64_t num_task_done,
                                                       std::uint64_t num_all_tasks ) const
        {
          PGBAR__TRUST( num_task_done <= num_all_tasks );
          if ( num_all_tasks == 0 )
            buffer << "-/-";

          return buffer << utils::format<utils::TxtLayout::Right>( utils::count_digits( num_all_tasks ),
                                                                   utils::format( num_task_done ) )
                        << '/' << utils::format( num_all_tasks );
        }

        PGBAR__NODISCARD PGBAR__INLINE_FN PGBAR__CXX14_CNSTXPR types::Size fixed_len_counter() const noexcept
        {
          return utils::count_digits( this->task_range_.back() ) * 2 + 1;
        }

      public:
        PGBAR__EMPTY_COMPONENT( CounterMeter )
      };

      template<typename Base, typename Derived>
      class Timer : public Base {
        PGBAR__NODISCARD PGBAR__INLINE_FN io::Stringbuf& to_hms( io::Stringbuf& buffer,
                                                                 types::TimeUnit duration ) const
        {
          auto zfill2 = [&]( std::int64_t num_time ) -> io::Stringbuf& {
            PGBAR__TRUST( num_time >= 0 );
            if ( num_time > 99 )
              return buffer.append( 'X', 2 );

            auto ret = utils::format( num_time );
            if ( ret.size() < 2 )
              ret.insert( 0, 1, '0' );
            return buffer << ret;
          };
          const auto hours = std::chrono::duration_cast<std::chrono::hours>( duration );
          duration -= hours;
          const auto minutes = std::chrono::duration_cast<std::chrono::minutes>( duration );
          duration -= minutes;

          zfill2( hours.count() ) << ':';
          zfill2( minutes.count() ) << ':';
          return zfill2( std::chrono::duration_cast<std::chrono::seconds>( duration ).count() );
        }

      protected:
#define PGBAR__ELASPED u8"--:--:--"
        PGBAR__INLINE_FN io::Stringbuf& build_elapsed( io::Stringbuf& buffer,
                                                       types::TimeUnit time_passed ) const
        {
          return to_hms( buffer, time_passed );
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr types::Size fixed_len_elapsed() const noexcept
        {
          return sizeof( PGBAR__ELASPED ) - 1;
        }

#define PGBAR__COUNTDOWN u8"~" PGBAR__ELASPED
        PGBAR__CXX20_CNSTXPR io::Stringbuf& build_countdown( io::Stringbuf& buffer,
                                                             const types::TimeUnit& time_passed,
                                                             std::uint64_t num_task_done,
                                                             std::uint64_t num_all_tasks ) const
        {
          PGBAR__TRUST( num_task_done <= num_all_tasks );
          if ( num_task_done == 0 || num_all_tasks == 0 )
            return buffer << PGBAR__COUNTDOWN;

          auto time_per_task = time_passed / num_task_done;
          if ( time_per_task.count() == 0 )
            time_per_task = std::chrono::nanoseconds( 1 );

          const auto remaining_tasks = num_all_tasks - num_task_done;
          // overflow check
          if ( remaining_tasks > ( std::numeric_limits<std::int64_t>::max )() / time_per_task.count() )
            return buffer << u8"~XX:XX:XX";
          buffer << '~';
          return to_hms( buffer, time_per_task * remaining_tasks );
        }
        PGBAR__NODISCARD PGBAR__INLINE_FN constexpr types::Size fixed_len_countdown() const noexcept
        {
          return sizeof( PGBAR__COUNTDOWN ) - 1;
        }

      public:
        PGBAR__EMPTY_COMPONENT( Timer )
      };
#undef PGBAR__COUNTDOWN
#undef PGBAR__ELASPED
    } // namespace assets

    namespace traits {
      // There is no need to declare the dependency on assets::CoreConfig here.
      // It will be directly passed to the dependency resolution function as the initial base class.
      PGBAR__INHERIT_REGISTER( assets::BasicAnimation, assets::Frames );

      PGBAR__INHERIT_REGISTER( assets::PercentMeter, assets::Countable );
      PGBAR__INHERIT_REGISTER( assets::SpeedMeter, assets::Countable );
      PGBAR__INHERIT_REGISTER( assets::CounterMeter, assets::Countable );

      PGBAR__INHERIT_REGISTER( assets::Timer, assets::Countable );

      template<template<typename...> class Component>
      struct OptionFor {
        using type = TypeSet<>;
      };
      template<template<typename...> class Component>
      using OptionFor_t = typename OptionFor<Component>::type;

#define PGBAR__BIND_OPTION( Component, ... ) \
  template<>                                 \
  struct OptionFor<Component> {              \
    using type = TypeSet<__VA_ARGS__>;       \
  }

      PGBAR__BIND_OPTION( assets::CoreConfig, option::Colored, option::Bolded );
      PGBAR__BIND_OPTION( assets::Countable, option::Tasks );
      PGBAR__BIND_OPTION( assets::Reversible, option::Reversed );
      PGBAR__BIND_OPTION( assets::Frames, option::Lead, option::LeadColor );
      PGBAR__BIND_OPTION( assets::Filler, option::Filler, option::FillerColor );
      PGBAR__BIND_OPTION( assets::Remains, option::Remains, option::RemainsColor );
      PGBAR__BIND_OPTION( assets::BasicIndicator,
                          option::Starting,
                          option::Ending,
                          option::StartColor,
                          option::EndColor,
                          option::BarWidth );
      PGBAR__BIND_OPTION( assets::Prefix, option::Prefix, option::PrefixColor );
      PGBAR__BIND_OPTION( assets::Postfix, option::Postfix, option::PostfixColor );
      PGBAR__BIND_OPTION( assets::Segment,
                          option::Divider,
                          option::LeftBorder,
                          option::RightBorder,
                          option::InfoColor );
      PGBAR__BIND_OPTION( assets::PercentMeter, );
      PGBAR__BIND_OPTION( assets::SpeedMeter, option::SpeedUnit, option::Magnitude );
      template<>
      struct OptionFor<assets::BasicAnimation> : TpAppend<OptionFor_t<assets::Frames>, option::Shift> {};
    } // namespace traits
  } // namespace _details
} // namespace pgbar

#endif
