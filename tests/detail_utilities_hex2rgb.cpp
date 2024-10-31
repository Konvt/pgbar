#include "common.hpp"

TEST_CASE( "Valid hex colors" )
{
  REQUIRE( pgbar::__detail::hex2rgb( "#FFF" ) == 0xFFFFFF );
  REQUIRE( pgbar::__detail::hex2rgb( "#000" ) == 0x000000 );
  REQUIRE( pgbar::__detail::hex2rgb( "#FF5733" ) == 0xFF5733 );
  REQUIRE( pgbar::__detail::hex2rgb( "#33FF57" ) == 0x33FF57 );
  REQUIRE( pgbar::__detail::hex2rgb( "#3357FF" ) == 0x3357FF );
}

TEST_CASE( "Invalid hex colors" )
{
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "#F" ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "#12345678" ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "123456" ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "#1234G6" ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "#      " ), pgbar::exceptions::InvalidArgument );
}

TEST_CASE( "Edge cases" )
{
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "" ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( pgbar::__detail::hex2rgb( "#" ), pgbar::exceptions::InvalidArgument );
}
