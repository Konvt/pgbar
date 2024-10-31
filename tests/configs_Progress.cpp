#include "common.hpp"
#include <random>

TEST_CASE( "Default constructor" )
{
  pgbar::configs::Progress config;

  REQUIRE( config.tasks() == 0 );
  REQUIRE( config.bar_length() != 0 );
  REQUIRE( config.colored() );
}

TEST_CASE( "Copy constructor" )
{
  pgbar::configs::Progress config;
  config.tasks( 114514 ).bar_length( 40 );

  REQUIRE( config.tasks() == 114514 );
  REQUIRE( config.bar_length() == 40 );

  auto copy = config;

  REQUIRE( copy.tasks() == 114514 );
  REQUIRE( copy.bar_length() == 40 );
}

TEST_CASE( "Move constructor" )
{
  pgbar::configs::Progress config;
  config.tasks( 114514 ).bar_length( 40 );

  REQUIRE( config.tasks() == 114514 );
  REQUIRE( config.bar_length() == 40 );

  auto moved = std::move( config );

  REQUIRE( moved.tasks() == 114514 );
  REQUIRE( moved.bar_length() == 40 );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::configs::Progress config1;
  pgbar::configs::Progress config2;
  config1.tasks( 114514 ).bar_length( 40 ).colored( false );
  config2.tasks( 42 ).bar_length( 37 );

  REQUIRE( config1.tasks() == 114514 );
  REQUIRE( config1.bar_length() == 40 );
  REQUIRE_FALSE( config1.colored() );
  REQUIRE( config2.tasks() == 42 );
  REQUIRE( config2.bar_length() == 37 );
  REQUIRE( config2.colored() );

  config1.swap( config2 );

  REQUIRE( config1.tasks() == 42 );
  REQUIRE( config1.bar_length() == 37 );
  REQUIRE( config1.colored() );
  REQUIRE( config2.tasks() == 114514 );
  REQUIRE( config2.bar_length() == 40 );
  REQUIRE_FALSE( config2.colored() );
}

TEST_CASE( "Variable parameters setting" )
{
  pgbar::configs::Progress config { pgbar::options::Tasks( 40 ), pgbar::options::BarLength( 80 ) };

  REQUIRE_NOTHROW( config.styles( 0 ) );

  REQUIRE( config.tasks() == 40 );
  REQUIRE( config.bar_length() == 80 );

  config.set( pgbar::options::Tasks( 2 ),
              pgbar::options::BarLength( 70 ),
              pgbar::options::Styles( pgbar::configs::Progress::Entire ) );

  REQUIRE( config.tasks() == 2 );
  REQUIRE( config.bar_length() == 70 );
}

TEST_CASE( "Multi-threaded visit" )
{
  pgbar::configs::Progress config;

  // Default value
  REQUIRE( pgbar::configs::Global::refresh_interval() == std::chrono::nanoseconds( 25000000 ) );
  REQUIRE( config.tasks() == 0 );
  REQUIRE( config.bar_length() == 30 );

  auto td1 = std::thread( [&config]() {
    auto rd      = std::mt19937( std::random_device()() );
    auto distrib = std::uniform_int_distribution<int>( 40000, 50000 );
    for ( auto _ = 0; _ < 50; ++_ ) {
      pgbar::configs::Global::refresh_interval( std::chrono::nanoseconds( distrib( rd ) ) );
      std::this_thread::sleep_for( pgbar::configs::Global::refresh_interval() );

      config.tasks( distrib( rd ) );
      config.set( pgbar::options::BarLength( distrib( rd ) ) );

      auto __ = config.tasks();
    }
  } );
  auto td2 = std::thread( [&config]() {
    auto rd      = std::mt19937( std::random_device()() );
    auto distrib = std::uniform_int_distribution<int>( 40, 80 );
    for ( auto _ = 0; _ < 50; ++_ ) {
      pgbar::configs::Global::refresh_interval( std::chrono::nanoseconds( distrib( rd ) ) );
      std::this_thread::sleep_for( pgbar::configs::Global::refresh_interval() );

      config.tasks( distrib( rd ) );
      config.set( pgbar::options::BarLength( distrib( rd ) ) );
      auto __ = config.tasks();
    }
  } );

  td1.join();
  td2.join();

  REQUIRE( pgbar::configs::Global::refresh_interval() != std::chrono::nanoseconds( 35000 ) );
  REQUIRE( config.tasks() != 0 );
  REQUIRE( config.bar_length() != 30 );
}
