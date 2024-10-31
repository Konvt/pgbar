#include "common.hpp"

TEST_CASE( "Left alignment" )
{
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::left>( 10, "pgbar" )
           == "pgbar     " );
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::left>( 1, "pgbar" ) == "pgbar" );
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::left>( 0, "pgbar" ) == "" );
}

TEST_CASE( "Center alignment" )
{
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::center>( 10, "pgbar" )
           == "  pgbar   " );
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::center>( 1, "pgbar" )
           == "pgbar" );
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::center>( 0, "pgbar" ) == "" );
}

TEST_CASE( "Right alignment" )
{
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::right>( 10, "pgbar" )
           == "     pgbar" );
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::right>( 1, "pgbar" )
           == "pgbar" );
  REQUIRE( pgbar::__detail::formatting<pgbar::__detail::TxtLayout::right>( 0, "pgbar" ) == "" );
}
