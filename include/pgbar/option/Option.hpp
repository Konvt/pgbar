#ifndef PGBAR_OPTION
#define PGBAR_OPTION

#include "../details/charcodes/U8Text.hpp"
#include "../details/console/escodes/Escodes.hpp"
#include "../details/wrappers/OptionWrapper.hpp"
#include <array>

namespace pgbar {
  namespace option {
    // The purpose of generating code with macros here is to annotate each type and method to provide more
    // friendly IDE access.
#define __PGBAR_BASE( ValueType ) \
public                            \
  __details::wrappers::OptionWrapper<ValueType>
#define __PGBAR_DEFAULT_OPTION( StructName, ValueType, ParamName )            \
  constexpr StructName( ValueType ParamName ) noexcept                        \
    : __details::wrappers::OptionWrapper<ValueType>( std::move( ParamName ) ) \
  {}
#define __PGBAR_NULLABLE_OPTION( StructName, ValueType, ParamName ) \
  constexpr StructName() = default;                                 \
  __PGBAR_DEFAULT_OPTION( StructName, ValueType, ParamName )

    // A wrapper that stores the value of the bit option setting.
    struct Style : __PGBAR_BASE( __details::types::Byte ) {
      __PGBAR_NULLABLE_OPTION( Style, __details::types::Byte, _settings )
    };

    // A wrapper that stores the value of the color effect setting.
    struct Colored : __PGBAR_BASE( bool ) {
      __PGBAR_NULLABLE_OPTION( Colored, bool, _enable )
    };

    // A wrapper that stores the value of the font boldness setting.
    struct Bolded : __PGBAR_BASE( bool ) {
      __PGBAR_NULLABLE_OPTION( Bolded, bool, _enable )
    };

    // A wrapper that stores the number of tasks.
    struct Tasks : __PGBAR_BASE( std::uint64_t ) {
      __PGBAR_DEFAULT_OPTION( Tasks, std::uint64_t, _num_tasks )
    };

    // A wrapper that stores the flag of direction.
    struct Reversed : __PGBAR_BASE( bool ) {
      __PGBAR_NULLABLE_OPTION( Reversed, bool, _flag )
    };

    // A wrapper that stores the length of the bar indicator, in the character unit.
    struct BarLength : __PGBAR_BASE( __details::types::Size ) {
      __PGBAR_DEFAULT_OPTION( BarLength, __details::types::Size, _num_char )
    };

    /**
     * A wrapper that stores the rate factor for animation frame transitions.
     *
     * Controls the speed of per-frame animation updates:
     *
     * - Positive values accelerate the transition (higher -> faster).
     *
     * - Negative values decelerate the transition (lower -> slower).
     *
     * - Zero freezes the animation completely.
     *
     * The effective range is between -128 (slowest) and 127 (fastest).
     */
    struct Shift : __PGBAR_BASE( std::int8_t ) {
      __PGBAR_DEFAULT_OPTION( Shift, std::int8_t, _shift_factor )
    };

    /**
     * A wrapper that stores the base magnitude for unit scaling in formatted output.
     *
     * Defines the threshold at which values are converted to higher-order units
     * (e.g. 1000 -> "1k", 1000000 -> "1M").
     *
     * The effective range is between 1 and 65535.
     *
     * - A zero value implies no scaling (raw numeric display).
     *
     * - Typical usage: 1000 (decimal) or 1024 (binary) scaling.
     */
    struct Magnitude : __PGBAR_BASE( std::uint16_t ) {
      __PGBAR_DEFAULT_OPTION( Magnitude, std::uint16_t, _magnitude )
    };

#undef __PGBAR_NULLABLE_OPTION
#undef __PGBAR_DEFAULT_OPTION
#if __PGBAR_CXX20
# define __PGBAR_DEFAULT_OPTION( StructName, ParamName )                  \
 private:                                                                 \
   using Data = __details::charcodes::U8Raw;                              \
   using Base = __details::wrappers::OptionWrapper<Data>;                 \
                                                                          \
 public:                                                                  \
   __PGBAR_CXX20_CNSTXPR StructName() = default;                          \
   /**                                                                    \
    * @throw exception::InvalidArgument                                   \
    *                                                                     \
    * If the passed parameter is not coding in UTF-8.                     \
    */                                                                    \
   __PGBAR_CXX20_CNSTXPR StructName( __details::types::String ParamName ) \
     : Base( Data( std::move( ParamName ) ) )                             \
   {}                                                                     \
   StructName( __details::types::LitU8 ParamName ) : Base( Data( std::move( ParamName ) ) ) {}
#else
# define __PGBAR_DEFAULT_OPTION( StructName, ParamName )                  \
   __PGBAR_CXX20_CNSTXPR StructName() = default;                          \
   __PGBAR_CXX20_CNSTXPR StructName( __details::types::String ParamName ) \
     : __details::wrappers::OptionWrapper<__details::charcodes::U8Raw>(   \
         __details::charcodes::U8Raw( std::move( ParamName ) ) )          \
   {}
#endif

    // A wrapper that stores the characters of the filler in the bar indicator.
    struct Filler : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Filler, _filler )
    };

    // A wrapper that stores the characters of the remains in the bar indicator.
    struct Remains : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Remains, _remains )
    };

    // A wrapper that stores characters located to the left of the bar indicator.
    struct Starting : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Starting, _starting )
    };

    // A wrapper that stores characters located to the right of the bar indicator.
    struct Ending : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Ending, _ending )
    };

    // A wrapper that stores the prefix text.
    struct Prefix : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Prefix, _prefix )
    };

    // A wrapper that stores the postfix text.
    struct Postfix : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Postfix, _postfix )
    };

    // A wrapper that stores the separator component used to separate different infomation.
    struct Divider : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( Divider, _divider )
    };

    // A wrapper that stores the border component located to the left of the whole indicator.
    struct LeftBorder : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( LeftBorder, _l_border )
    };

    // A wrapper that stores the border component located to the right of the whole indicator.
    struct RightBorder : __PGBAR_BASE( __details::charcodes::U8Raw ) {
      __PGBAR_DEFAULT_OPTION( RightBorder, _r_border )
    };

#undef __PGBAR_DEFAULT_OPTION
#define __PGBAR_DEFAULT_OPTION( StructName, ParamName )                                                \
private:                                                                                               \
  using Data = __details::console::escodes::RGBColor;                                                  \
  using Base = __details::wrappers::OptionWrapper<Data>;                                               \
                                                                                                       \
public:                                                                                                \
  __PGBAR_CXX23_CNSTXPR StructName() = default;                                                        \
  __PGBAR_CXX23_CNSTXPR StructName( __details::types::ROStr ParamName ) : Base( Data( ParamName ) ) {} \
  __PGBAR_CXX23_CNSTXPR StructName( __details::types::HexRGB ParamName ) : Base( Data( ParamName ) ) {}

    // A wrapper that stores the prefix text color.
    struct PrefixColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( PrefixColor, _prfx_color )
    };

    // A wrapper that stores the postfix text color.
    struct PostfixColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( PostfixColor, _pstfx_color )
    };

    // A wrapper that stores the color of component located to the left of the bar indicator.
    struct StartColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( StartColor, _start_color )
    };

    // A wrapper that stores the color of component located to the right of the bar indicator.
    struct EndColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( EndColor, _end_color )
    };

    // A wrapper that stores the color of the filler in the bar indicator.
    struct FillerColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( FillerColor, _filler_color )
    };

    // A wrapper that stores the color of the remains in the bar indicator.
    struct RemainsColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( RemainsColor, _remains_color )
    };

    // A wrapper that stores the color of the lead in the bar indicator.
    struct LeadColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( LeadColor, _lead_color )
    };

    // A wrapper that stores the color of the whole infomation indicator.
    struct InfoColor : __PGBAR_BASE( __details::console::escodes::RGBColor ) {
      __PGBAR_DEFAULT_OPTION( InfoColor, _info_color )
    };

#undef __PGBAR_DEFAULT_OPTION

    /**
     * A wrapper that stores ordered units for information rate formatting (e.g. B/s, kB/s).
     *
     * Encapsulates four consecutive scaling units where each unit is scaled by the
     * configured magnitude factor (default 1,000x if no `option::Magnitude` is explicitly set).
     *
     * Unit order MUST be ascending: [base_unit, scaled_unit_1, scaled_unit_2, scaled_unit_3].
     *
     * Example:
     *
     * - magnitude=1000: ["B/s", "kB/s", "MB/s", "GB/s"]
     *
     * - magnitude=1024: ["B/s", "KiB/s", "MiB/s", "GiB/s"]
     *
     * Scaling logic: value >= magnitude -> upgrade to next unit tier.
     *
     * @throw exception::InvalidArgument
     *   Thrown if any input string fails UTF-8 validation or the array size mismatches.
     */
    struct SpeedUnit : __PGBAR_BASE( __PGBAR_WRAP( std::array<__details::charcodes::U8Raw, 4> ) ) {
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       *
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<__details::types::String, 4> _units )
      {
        std::transform(
          std::make_move_iterator( _units.begin() ),
          std::make_move_iterator( _units.end() ),
          data_.begin(),
          []( __details::types::String&& ele ) { return __details::charcodes::U8Raw( std::move( ele ) ); } );
      }
#if __PGBAR_CXX20
      /**
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      __PGBAR_CXX20_CNSTXPR SpeedUnit( std::array<__details::types::LitU8, 4> _units )
      {
        std::transform(
          _units.cbegin(),
          _units.cend(),
          data_.begin(),
          []( const __details::types::LitU8& ele ) { return __details::charcodes::U8Raw( ele ); } );
      }
#endif
    };

    // A wrapper that stores the `lead` animated element.
    struct Lead : __PGBAR_BASE( std::vector<__details::charcodes::U8Text> ) {
    private:
      using Base = __details::wrappers::OptionWrapper<std::vector<__details::charcodes::U8Text>>;

    public:
      __PGBAR_CXX20_CNSTXPR Lead() = default;
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( std::vector<__details::types::String> _leads )
      {
        std::transform(
          std::make_move_iterator( _leads.begin() ),
          std::make_move_iterator( _leads.end() ),
          std::back_inserter( data_ ),
          []( __details::types::String&& ele ) { return __details::charcodes::U8Text( std::move( ele ) ); } );
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      __PGBAR_CXX20_CNSTXPR Lead( __details::types::String _lead )
        : Base( { __details::charcodes::U8Text( std::move( _lead ) ) } )
      {}
#if __PGBAR_CXX20
      __PGBAR_CXX20_CNSTXPR Lead( const std::vector<__details::types::LitU8>& _leads )
      {
        std::transform(
          _leads.cbegin(),
          _leads.cend(),
          std::back_inserter( data_ ),
          []( const __details::types::LitU8& ele ) { return __details::charcodes::U8Text( ele ); } );
      }
      Lead( const __details::types::LitU8& _lead ) : Base( { __details::charcodes::U8Text( _lead ) } ) {}
#endif
    };

#undef __PGBAR_DEFAULT_OPTION
#undef __PGBAR_BASE
  } // namespace option
} // namespace pgbar

#endif
