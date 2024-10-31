#include "common.hpp"

TEST_CASE( "Default constructor" )
{
  pgbar::__detail::StringBuffer buffer;

  REQUIRE( buffer.empty() );
  REQUIRE( buffer.empty() == buffer.data().empty() );
}

TEST_CASE( "Copy constructor" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.append( 3, 'd' );
  pgbar::__detail::StringBuffer copy = buffer;

  REQUIRE( copy.data() == "ddd" );
}

TEST_CASE( "Move constructor" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.append( 4, 'e' );
  pgbar::__detail::StringBuffer moved = std::move( buffer );

  REQUIRE( moved.data() == "eeee" );
}

TEST_CASE( "Append multiple characters" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.append( 5, 'a' );

  REQUIRE( !buffer.empty() );
  REQUIRE( buffer.data() == "aaaaa" );
}

TEST_CASE( "Clear functionality" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.append( 1, 'a' );

  REQUIRE( !buffer.empty() );
  REQUIRE( buffer.empty() == buffer.data().empty() );

  buffer.clear();

  REQUIRE( buffer.empty() );
  REQUIRE( buffer.empty() == buffer.data().empty() );
}

TEST_CASE( "Append single characters" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.clear();
  buffer << 'b';

  REQUIRE( buffer.data() == "b" );
}

TEST_CASE( "Append multiple strings" )
{
  pgbar::__detail::StringBuffer buffer;
  pgbar::__detail::types::String test_str = "xyz";
  buffer.append( 3, test_str );

  REQUIRE( buffer.data() == "xyzxyzxyz" );
}

TEST_CASE( "Reserve capacity" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.reserve( 100 );
  buffer.append( 10, 'c' );

  REQUIRE( !buffer.empty() );
  REQUIRE( buffer.empty() == buffer.data().empty() );
  REQUIRE( buffer.data().capacity() >= 100 );
}

TEST_CASE( "Release functionality" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.append( 80, 'a' );

  REQUIRE( buffer.data().capacity() >= 80 );

  buffer.release();

  REQUIRE( buffer.empty() );
  REQUIRE( buffer.empty() == buffer.data().empty() );
  // If the `std::string` has SSO, the capacity will never be zero.
  REQUIRE( buffer.data().capacity() < 80 );
}

TEST_CASE( "Friend stream output" )
{
  pgbar::__detail::StringBuffer buffer;
  buffer.append( 5, 'a' );
  std::ostringstream oss;
  oss << buffer;

  REQUIRE( oss.str() == "aaaaa" );
  REQUIRE( buffer.empty() );
  REQUIRE( buffer.empty() == buffer.data().empty() );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::__detail::StringBuffer buffer1;
  pgbar::__detail::StringBuffer buffer2;

  buffer1.append( 5, 'a' );
  buffer2.append( 5, 'b' );

  REQUIRE( buffer1.data() == "aaaaa" );
  REQUIRE( buffer2.data() == "bbbbb" );

  buffer1.swap( buffer2 );

  REQUIRE( buffer1.data() == "bbbbb" );
  REQUIRE( buffer2.data() == "aaaaa" );
}
