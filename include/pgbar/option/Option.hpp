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
#define PGBAR__BASE( ValueType ) \
public                           \
  _details::wrappers::OptionWrapper<ValueType>
#define PGBAR__DEFAULT_OPTION( StructName, ValueType, ParamName )            \
  constexpr StructName( ValueType ParamName ) noexcept                       \
    : _details::wrappers::OptionWrapper<ValueType>( std::move( ParamName ) ) \
  {}
#define PGBAR__NULLABLE_OPTION( StructName, ValueType, ParamName ) \
  constexpr StructName() = default;                                \
  PGBAR__DEFAULT_OPTION( StructName, ValueType, ParamName )

    // A wrapper that stores the value of the bit option setting.
    struct Style : PGBAR__BASE( _details::types::Byte ) {
      PGBAR__NULLABLE_OPTION( Style, _details::types::Byte, _settings )
    };

    // A wrapper that stores the value of the color effect setting.
    struct Colored : PGBAR__BASE( bool ) {
      PGBAR__NULLABLE_OPTION( Colored, bool, _enable )
    };

    // A wrapper that stores the value of the font boldness setting.
    struct Bolded : PGBAR__BASE( bool ) {
      PGBAR__NULLABLE_OPTION( Bolded, bool, _enable )
    };

    // A wrapper that stores the number of tasks.
    struct Tasks : PGBAR__BASE( std::uint64_t ) {
      PGBAR__DEFAULT_OPTION( Tasks, std::uint64_t, _num_tasks )
    };

    // A wrapper that stores the flag of direction.
    struct Reversed : PGBAR__BASE( bool ) {
      PGBAR__NULLABLE_OPTION( Reversed, bool, _flag )
    };

    // A wrapper that stores the width of the bar indicator, in the character unit.
    struct BarWidth : PGBAR__BASE( _details::types::Size ) {
      PGBAR__DEFAULT_OPTION( BarWidth, _details::types::Size, _num_char )
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
    struct Shift : PGBAR__BASE( std::int8_t ) {
      PGBAR__DEFAULT_OPTION( Shift, std::int8_t, _shift_factor )
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
    struct Magnitude : PGBAR__BASE( std::uint16_t ) {
      PGBAR__DEFAULT_OPTION( Magnitude, std::uint16_t, _magnitude )
    };

#undef PGBAR__NULLABLE_OPTION
#undef PGBAR__DEFAULT_OPTION
#if PGBAR__CXX20
# define PGBAR__DEFAULT_OPTION( StructName, ParamName )                 \
 private:                                                               \
   using Data = _details::charcodes::U8Raw;                             \
   using Base = _details::wrappers::OptionWrapper<Data>;                \
                                                                        \
 public:                                                                \
   PGBAR__CXX20_CNSTXPR StructName() = default;                         \
   /**                                                                  \
    * @throw exception::InvalidArgument                                 \
    *                                                                   \
    * If the passed parameter is not coding in UTF-8.                   \
    */                                                                  \
   PGBAR__CXX20_CNSTXPR StructName( _details::types::String ParamName ) \
     : Base( Data( std::move( ParamName ) ) )                           \
   {}                                                                   \
   StructName( _details::types::LitU8 ParamName ) : Base( Data( std::move( ParamName ) ) ) {}
#else
# define PGBAR__DEFAULT_OPTION( StructName, ParamName )                 \
   PGBAR__CXX20_CNSTXPR StructName() = default;                         \
   PGBAR__CXX20_CNSTXPR StructName( _details::types::String ParamName ) \
     : _details::wrappers::OptionWrapper<_details::charcodes::U8Raw>(   \
         _details::charcodes::U8Raw( std::move( ParamName ) ) )         \
   {}
#endif

    // A wrapper that stores the characters of the filler in the bar indicator.
    struct Filler : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Filler, _filler )
    };

    // A wrapper that stores the characters of the remains in the bar indicator.
    struct Remains : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Remains, _remains )
    };

    // A wrapper that stores characters located to the left of the bar indicator.
    struct Starting : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Starting, _starting )
    };

    // A wrapper that stores characters located to the right of the bar indicator.
    struct Ending : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Ending, _ending )
    };

    // A wrapper that stores the prefix text.
    struct Prefix : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Prefix, _prefix )
    };

    // A wrapper that stores the postfix text.
    struct Postfix : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Postfix, _postfix )
    };

    // A wrapper that stores the separator component used to separate different infomation.
    struct Divider : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( Divider, _divider )
    };

    // A wrapper that stores the border component located to the left of the whole indicator.
    struct LeftBorder : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( LeftBorder, _l_border )
    };

    // A wrapper that stores the border component located to the right of the whole indicator.
    struct RightBorder : PGBAR__BASE( _details::charcodes::U8Raw ) {
      PGBAR__DEFAULT_OPTION( RightBorder, _r_border )
    };

#undef PGBAR__DEFAULT_OPTION
#define PGBAR__DEFAULT_OPTION( StructName, ParamName )                                               \
private:                                                                                             \
  using Data = _details::console::escodes::RGBColor;                                                 \
  using Base = _details::wrappers::OptionWrapper<Data>;                                              \
                                                                                                     \
public:                                                                                              \
  PGBAR__CXX23_CNSTXPR StructName() = default;                                                       \
  PGBAR__CXX23_CNSTXPR StructName( _details::types::ROStr ParamName ) : Base( Data( ParamName ) ) {} \
  PGBAR__CXX23_CNSTXPR StructName( _details::types::HexRGB ParamName ) : Base( Data( ParamName ) ) {}

    // A wrapper that stores the prefix text color.
    struct PrefixColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( PrefixColor, _prfx_color )
    };

    // A wrapper that stores the postfix text color.
    struct PostfixColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( PostfixColor, _pstfx_color )
    };

    // A wrapper that stores the color of component located to the left of the bar indicator.
    struct StartColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( StartColor, _start_color )
    };

    // A wrapper that stores the color of component located to the right of the bar indicator.
    struct EndColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( EndColor, _end_color )
    };

    // A wrapper that stores the color of the filler in the bar indicator.
    struct FillerColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( FillerColor, _filler_color )
    };

    // A wrapper that stores the color of the remains in the bar indicator.
    struct RemainsColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( RemainsColor, _remains_color )
    };

    // A wrapper that stores the color of the lead in the bar indicator.
    struct LeadColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( LeadColor, _lead_color )
    };

    // A wrapper that stores the color of the whole infomation indicator.
    struct InfoColor : PGBAR__BASE( _details::console::escodes::RGBColor ) {
      PGBAR__DEFAULT_OPTION( InfoColor, _info_color )
    };

#undef PGBAR__DEFAULT_OPTION

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
    struct SpeedUnit : PGBAR__BASE( PGBAR__WRAP( std::array<_details::charcodes::U8Raw, 4> ) ) {
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       *
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      PGBAR__CXX20_CNSTXPR SpeedUnit( std::array<_details::types::String, 4> _units )
      {
        std::transform(
          std::make_move_iterator( _units.begin() ),
          std::make_move_iterator( _units.end() ),
          data_.begin(),
          []( _details::types::String&& ele ) { return _details::charcodes::U8Raw( std::move( ele ) ); } );
      }
#if PGBAR__CXX20
      /**
       * @param _units
       * The given each unit will be treated as 1,000 times greater than the previous one
       * (from left to right).
       */
      PGBAR__CXX20_CNSTXPR SpeedUnit( std::array<_details::types::LitU8, 4> _units )
      {
        std::transform(
          _units.cbegin(),
          _units.cend(),
          data_.begin(),
          []( const _details::types::LitU8& ele ) { return _details::charcodes::U8Raw( ele ); } );
      }
#endif
    };

    // A wrapper that stores the `lead` animated element.
    struct Lead : PGBAR__BASE( std::vector<_details::charcodes::U8Text> ) {
    private:
      using Base = _details::wrappers::OptionWrapper<std::vector<_details::charcodes::U8Text>>;

    public:
      PGBAR__CXX20_CNSTXPR Lead() = default;
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      PGBAR__CXX20_CNSTXPR Lead( std::vector<_details::types::String> _leads )
      {
        std::transform(
          std::make_move_iterator( _leads.begin() ),
          std::make_move_iterator( _leads.end() ),
          std::back_inserter( data_ ),
          []( _details::types::String&& ele ) { return _details::charcodes::U8Text( std::move( ele ) ); } );
      }
      /**
       * @throw exception::InvalidArgument
       *
       * If the passed parameters are not coding in UTF-8.
       */
      PGBAR__CXX20_CNSTXPR Lead( _details::types::String _lead )
        : Base( { _details::charcodes::U8Text( std::move( _lead ) ) } )
      {}
#if PGBAR__CXX20
      PGBAR__CXX20_CNSTXPR Lead( const std::vector<_details::types::LitU8>& _leads )
      {
        std::transform(
          _leads.cbegin(),
          _leads.cend(),
          std::back_inserter( data_ ),
          []( const _details::types::LitU8& ele ) { return _details::charcodes::U8Text( ele ); } );
      }
      Lead( const _details::types::LitU8& _lead ) : Base( { _details::charcodes::U8Text( _lead ) } ) {}
#endif
    };

#undef PGBAR__DEFAULT_OPTION
#undef PGBAR__BASE
  } // namespace option
} // namespace pgbar

#endif
