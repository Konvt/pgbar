#include "common.hpp"
#include <random>

TEST_CASE( "Default constructor" )
{
  pgbar::SpinnerBar<> bar;

  REQUIRE_FALSE( bar.is_running() );
  REQUIRE( bar.configure().colored() );
  REQUIRE( bar.configure().bolded() );
  REQUIRE_NOTHROW( bar.tick() );
}

TEST_CASE( "Move constructor" )
{
  pgbar::SpinnerBar<> bar { pgbar::options::Colored( false ) };

  REQUIRE_FALSE( bar.configure().colored() );

  auto moved = std::move( bar );

  REQUIRE_FALSE( moved.configure().colored() );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::SpinnerBar<> bar1 { pgbar::options::Colored( false ) };
  pgbar::SpinnerBar<> bar2 { pgbar::options::Bolded( false ) };

  REQUIRE_FALSE( bar1.configure().colored() );
  REQUIRE( bar1.configure().bolded() );
  REQUIRE( bar2.configure().colored() );
  REQUIRE_FALSE( bar2.configure().bolded() );

  bar1.swap( bar2 );

  REQUIRE( bar1.configure().colored() );
  REQUIRE_FALSE( bar1.configure().bolded() );
  REQUIRE_FALSE( bar2.configure().colored() );
  REQUIRE( bar2.configure().bolded() );
}

TEST_CASE( "The iterating progress" )
{
  pgbar::SpinnerBar<> bar;

  bar.tick();

  if ( pgbar::configs::Global::intty() )
    REQUIRE( bar.is_running() );

  bar.reset();
}

TEST_CASE( "Multi-threaded tick" )
{
  constexpr auto iteration   = 100000;
  constexpr auto num_threads = 10;

  pgbar::SpinnerBar<std::ostream, pgbar::Threadsafe> bar;

  REQUIRE_FALSE( bar.is_running() );

  std::vector<std::thread> threads;
  for ( auto _ = 0; _ < num_threads; ++_ ) {
    threads.emplace_back( [&bar, iteration, num_threads]() {
      auto rd      = std::mt19937( std::random_device()() );
      auto distrib = std::uniform_int_distribution<int>( 10, 30 );
      for ( auto _ = 0; _ < iteration / num_threads; ++_ ) {
        bar.tick(); // Only the firstly `tick` will launch the bar.
        std::this_thread::sleep_for( std::chrono::microseconds( distrib( rd ) ) );
      }
    } );
  }

  std::for_each( threads.begin(), threads.end(), []( std::thread& td ) { td.join(); } );
  bar.reset();
}

TEST_CASE( "Color switch" )
{
  pgbar::SpinnerBar<> bar { pgbar::options::FramesColor( "#A90101" ),
                            pgbar::options::TrueFrame( "Successful!" ),
                            pgbar::options::TrueColor( pgbar::colors::Green ) };

  REQUIRE( bar.configure().colored() );

  std::cout << "Colorful version:" << std::endl;
  bar.tick();
  std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
  bar.reset();

  bar.configure().colored( false );

  REQUIRE_FALSE( bar.configure().colored() );

  std::cout << "Colorless version:" << std::endl;
  bar.tick();
  std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
  bar.reset();
}
