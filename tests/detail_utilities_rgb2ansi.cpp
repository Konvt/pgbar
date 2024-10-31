#include "common.hpp"

TEST_CASE( "Predefined colors" )
{
  REQUIRE( pgbar::__detail::rgb2ansi( 0xC105EA11 ) == "\x1B[0m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0xB01DFACE ) == "\x1B[1m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0x000000 ) == "\x1B[30m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0xFF0000 ) == "\x1B[31m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0x00FF00 ) == "\x1B[32m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0xFFFF00 ) == "\x1B[33m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0x0000FF ) == "\x1B[34m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0x800080 ) == "\x1B[35m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0x00FFFF ) == "\x1B[36m" );
  REQUIRE( pgbar::__detail::rgb2ansi( 0xFFFFFF ) == "\x1B[37m" );
}

TEST_CASE( "RGB colors" )
{
  // RGB (255, 87, 51)
  REQUIRE( pgbar::__detail::rgb2ansi( 0xFF5733 ) == "\x1B[38;2;255;87;51m" );
  // RGB (51, 255, 87)
  REQUIRE( pgbar::__detail::rgb2ansi( 0x33FF57 ) == "\x1B[38;2;51;255;87m" );
  // RGB (51, 87, 255)
  REQUIRE( pgbar::__detail::rgb2ansi( 0x3357FF ) == "\x1B[38;2;51;87;255m" );
}

TEST_CASE( "Out of range RGB values" )
{
  // Matches predefined values.
  REQUIRE( pgbar::__detail::rgb2ansi( 0xFFFFFFFF ) == "\x1B[37m" );
  // RGB (52, 86, 120)
  REQUIRE( pgbar::__detail::rgb2ansi( 0x12345678 ) == "\x1B[38;2;52;86;120m" );
  // RGB (187, 204, 255)
  REQUIRE( pgbar::__detail::rgb2ansi( 0xAABBCCFF ) == "\x1B[38;2;187;204;255m" );
}
