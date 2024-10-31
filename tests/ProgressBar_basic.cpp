#include "common.hpp"
#include <random>

TEST_CASE( "Default constructor" )
{
  pgbar::ProgressBar<> bar;

  REQUIRE_FALSE( bar.is_running() );
  REQUIRE( bar.progress() == 0 );
  REQUIRE( bar.configure().colored() );
  REQUIRE( bar.configure().bolded() );
  REQUIRE( bar.configure().fixed_size() != 0 );
  REQUIRE( bar.configure().tasks() == 0 );
  if ( pgbar::configs::Global::intty() )
    REQUIRE_THROWS_AS( bar.tick(), pgbar::exceptions::InvalidState );
  else
    REQUIRE_NOTHROW( bar.tick() );
}

TEST_CASE( "Move constructor" )
{
  pgbar::ProgressBar<> bar { pgbar::options::BarLength( 40 ) };

  REQUIRE( bar.configure().bar_length() == 40 );

  auto moved = std::move( bar );

  REQUIRE( moved.configure().bar_length() == 40 );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::ProgressBar<> bar1 { pgbar::options::BarLength( 40 ) };
  pgbar::ProgressBar<> bar2 { pgbar::options::Tasks( 100 ) };

  REQUIRE( bar1.configure().bar_length() == 40 );
  REQUIRE( bar2.configure().tasks() == 100 );

  bar1.swap( bar2 );

  REQUIRE( bar1.configure().tasks() == 100 );
  REQUIRE( bar2.configure().bar_length() == 40 );
}

TEST_CASE( "The iterating progress" )
{
  pgbar::ProgressBar<> bar { pgbar::options::Tasks( 10 ) };

  REQUIRE( bar.configure().tasks() == 10 );

  bar.tick( 4 );

  if ( pgbar::configs::Global::intty() ) {
    REQUIRE( bar.progress() == 4 );
    REQUIRE( bar.is_running() );
  } else
    REQUIRE( bar.progress() == 0 );

  bar.tick_to( 80 );

  if ( pgbar::configs::Global::intty() )
    REQUIRE( bar.progress() == 8 );
  else
    REQUIRE( bar.progress() == 0 );

  bar.reset();
  for ( auto i = 0; i < 10; ++i ) {
    bar.tick();

    if ( pgbar::configs::Global::intty() )
      REQUIRE( bar.progress() == static_cast<std::size_t>( i + 1 ) );
    else
      REQUIRE( bar.progress() == 0 );
  }

  if ( pgbar::configs::Global::intty() )
    REQUIRE( bar.progress() == 10 );
  else
    REQUIRE( bar.progress() == 0 );
}

TEST_CASE( "Multi-threaded tick" )
{
  constexpr auto iteration   = 100000;
  constexpr auto num_threads = 10;

  pgbar::ProgressBar<std::ostream, pgbar::Threadsafe> bar { pgbar::options::Tasks( iteration ) };

  REQUIRE_FALSE( bar.is_running() );
  REQUIRE( bar.progress() == 0 );

  std::vector<std::thread> threads;
  for ( auto _ = 0; _ < num_threads; ++_ ) {
    threads.emplace_back( [&bar, iteration, num_threads]() {
      auto rd      = std::mt19937( std::random_device()() );
      auto distrib = std::uniform_int_distribution<int>( 10, 30 );
      for ( auto _ = 0; _ < iteration / num_threads; ++_ ) {
        bar.tick();
        std::this_thread::sleep_for( std::chrono::microseconds( distrib( rd ) ) );
      }
    } );
  }

  std::for_each( threads.begin(), threads.end(), []( std::thread& td ) { td.join(); } );
}

TEST_CASE( "Color switch" )
{
  constexpr auto iteration = 114514;
  pgbar::ProgressBar<> bar {
    pgbar::options::Tasks( iteration ),     pgbar::options::TodoChar( "=" ),
    pgbar::options::DoneChar( "-" ),        pgbar::options::TodoColor( "#A90101" ),
    pgbar::options::DoneColor( "#01DD27" ), pgbar::options::StatusColor( "#94F516" ),
    pgbar::options::StartPoint( " " ),      pgbar::options::EndPoint( "" ),
    pgbar::options::LeftStatus( "=> " )
  };

  REQUIRE( bar.configure().colored() );

  std::cout << "Colorful version:" << std::endl;
  for ( auto _ = 0; _ < iteration; ++_ ) {
    bar.tick();
    std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
  }

  bar.configure().colored( false );

  REQUIRE_FALSE( bar.configure().colored() );

  std::cout << "Colorless version:" << std::endl;
  for ( auto _ = 0; _ < iteration; ++_ ) {
    bar.tick();
    std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
  }
}
