#include "common.hpp"

TEST_CASE( "Traversal integer range" )
{
  pgbar::ProgressBar<> bar;
  std::vector<int> ret;
  bar.foreach ( 0, 10, 2, [&ret]( int e ) { ret.push_back( e ); } );

  REQUIRE( ret == std::vector<int> { 0, 2, 4, 6, 8 } );
}

TEST_CASE( "Traversal with startpoint and endpoint" )
{
  pgbar::ProgressBar<> bar;
  std::vector<int> ret;
  bar.foreach ( 0, 5, [&]( int val ) { ret.push_back( val ); } );
  REQUIRE( ret == std::vector<int> { 0, 1, 2, 3, 4 } );
}

TEST_CASE( "Traversal with only endpoint" )
{
  pgbar::ProgressBar<> bar;
  std::vector<int> ret;
  bar.foreach ( 5, [&]( int val ) { ret.push_back( val ); } );
  REQUIRE( ret == std::vector<int> { 0, 1, 2, 3, 4 } );
}

TEST_CASE( "Traversal with endpoint and step" )
{
  pgbar::ProgressBar<> bar;
  std::vector<double> ret;
  bar.foreach ( 5.0, 1.5, [&]( double val ) { ret.push_back( val ); } );
  REQUIRE( ret == std::vector<double> { 0.0, 1.5, 3.0, 4.5 } );
}

TEST_CASE( "Traversal with iterator range" )
{
  pgbar::ProgressBar<> bar;
  std::vector<int> vec = { 1, 2, 3, 4, 5 };
  std::vector<int> ret;
  bar.foreach ( vec.begin(), vec.end(), [&]( int val ) { ret.push_back( val ); } );

  REQUIRE( ret == vec );
}

TEST_CASE( "Traversal with container" )
{
  pgbar::ProgressBar<> bar;
  std::vector<int> vec = { 1, 2, 3, 4, 5 };
  std::vector<int> ret;
  bar.foreach ( vec, [&]( int val ) { ret.push_back( val ); } );

  REQUIRE( ret == vec );
}

TEST_CASE( "Traversal with raw array" )
{
  pgbar::ProgressBar<> bar;
  int vec[] = { 1, 2, 3, 4, 5 };
  std::vector<int> ret;
  bar.foreach ( vec, [&]( int val ) { ret.push_back( val ); } );

  for ( std::size_t i = 0; i < ( sizeof( vec ) / sizeof( int ) ); ++i )
    REQUIRE( ret[i] == vec[i] );
}

TEST_CASE( "Invalid parameters" )
{
  pgbar::ProgressBar<> bar;
  int vec[] = { 1, 2, 3, 4, 5 };
  int *begin = vec, *end = nullptr;

  REQUIRE_THROWS_AS( (void)bar.foreach ( begin, end ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( (void)bar.foreach ( 1, 100, -1 ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( (void)bar.foreach ( 100, 1, 1 ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( (void)bar.foreach ( 1, 100, 0 ), pgbar::exceptions::InvalidArgument );
}
