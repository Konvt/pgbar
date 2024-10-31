#include "common.hpp"
#include <random>

TEST_CASE( "Default constructor" )
{
  pgbar::configs::Spinner config;

  REQUIRE( config.colored() );
  REQUIRE( config.bolded() );
}

TEST_CASE( "Copy constructor" )
{
  pgbar::configs::Spinner config;
  config.colored( false ).bolded( false );

  REQUIRE_FALSE( config.colored() );
  REQUIRE_FALSE( config.bolded() );

  auto copy = config;

  REQUIRE_FALSE( copy.colored() );
  REQUIRE_FALSE( copy.bolded() );
}

TEST_CASE( "Move constructor" )
{
  pgbar::configs::Spinner config;
  config.colored( false ).bolded( false );

  REQUIRE_FALSE( config.colored() );
  REQUIRE_FALSE( config.bolded() );

  auto moved = std::move( config );

  REQUIRE_FALSE( moved.colored() );
  REQUIRE_FALSE( moved.bolded() );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::configs::Spinner config1;
  pgbar::configs::Spinner config2;
  config1.colored( false ).bolded( false );

  REQUIRE_FALSE( config1.colored() );
  REQUIRE_FALSE( config1.bolded() );
  REQUIRE( config2.colored() );
  REQUIRE( config2.bolded() );

  config1.swap( config2 );

  REQUIRE( config1.colored() );
  REQUIRE( config1.bolded() );
  REQUIRE_FALSE( config2.colored() );
  REQUIRE_FALSE( config2.bolded() );
}

TEST_CASE( "Variable parameters setting" )
{
  pgbar::configs::Spinner config { pgbar::options::Colored( false ) };

  REQUIRE_NOTHROW( config.true_frame( "" ) );
  REQUIRE_NOTHROW( config.false_frame( "" ) );
  REQUIRE_FALSE( config.colored() );

  config.set( pgbar::options::Bolded( false ) );

  REQUIRE_FALSE( config.bolded() );
}

TEST_CASE( "Invalid parameters" )
{
  REQUIRE_THROWS_AS( pgbar::configs::Spinner( pgbar::options::Frames( {} ) ),
                     pgbar::exceptions::InvalidArgument );

  pgbar::configs::Spinner config;

  REQUIRE_THROWS_AS( config.frames( {} ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( config.set( pgbar::options::Frames( {} ) ),
                     pgbar::exceptions::InvalidArgument );
}

TEST_CASE( "Multi-threaded visit" )
{
  pgbar::configs::Spinner config;

  // Default value
  REQUIRE( config.colored() );
  REQUIRE( config.bolded() );

  auto td1 = std::thread( [&config]() {
    auto rd      = std::mt19937( std::random_device()() );
    auto distrib = std::uniform_int_distribution<int>( 0, 16777215 );
    for ( auto _ = 0; _ < 50; ++_ ) {
      std::this_thread::sleep_for( pgbar::configs::Global::refresh_interval() );

      config.frames( { ".", "..", "..." } );
      config.set( pgbar::options::TrueColor( distrib( rd ) ) );
    }
  } );
  auto td2 = std::thread( [&config]() {
    auto rd      = std::mt19937( std::random_device()() );
    auto distrib = std::uniform_int_distribution<int>( 0, 16777215 );
    for ( auto _ = 0; _ < 50; ++_ ) {
      std::this_thread::sleep_for( pgbar::configs::Global::refresh_interval() );

      config.true_color( distrib( rd ) );
      config.set( pgbar::options::Frames( { "/", "|", "\\", "-" } ) );
    }
  } );

  td1.join();
  td2.join();
}
