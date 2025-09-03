#include "pgbar/pgbar.hpp"
#include <chrono>
#include <random>
#include <thread>
using namespace std;

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  auto mbar = pgbar::make_multi(
    pgbar::config::Flow( pgbar::option::Style( pgbar::config::Flow::Entire ),
                         pgbar::option::Filler( "━" ),
                         pgbar::option::FillerColor( pgbar::color::Red ),
                         pgbar::option::Lead( "━━" ),
                         pgbar::option::LeadColor( pgbar::color::White ),
                         pgbar::option::InfoColor( "#F5B0B6" ),
                         pgbar::option::Starting(),
                         pgbar::option::Ending() ),
    pgbar::config::Block( pgbar::option::Lead( { " ", "▁", "▂", "▃", "▄", "▅", "▆", "▇" } ),
                          pgbar::option::InfoColor( "#F7A699" ) ),
    pgbar::config::Block( pgbar::option::Filler( "⠿" ),
                          pgbar::option::Lead( { " ", "⠄", "⠆", "⠇", "⠧", "⠷" } ),
                          pgbar::option::InfoColor( "#7DD4DF" ) ),
    pgbar::config::Block( pgbar::option::Lead( { " ", "▖", "▞", "▛" } ),
                          pgbar::option::InfoColor( "#8AB7EB" ) ) );
  mbar.config<0>().tasks( ( std::tuple_size<decltype( mbar )>::value - 1 ) * 2 );

  vector<thread> pool;
  pool.emplace_back( [&]() {
    mt19937 rd { random_device {}() };
    mbar.config<1>().tasks( 30000 );
    mbar.tick<0>();
    for ( size_t i = 0; i < 30000; ++i ) {
      mbar.tick<1>();
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<>( 1, 1025 )( rd ) ) );
    }
    mbar.tick<0>();
  } );
  pool.emplace_back( [&]() {
    mt19937 rd { random_device {}() };
    mbar.tick<0>();
    mbar.iterate<2>( 10000, [&rd]( int ) {
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<>( 10, 1100 )( rd ) ) );
    } );
    mbar.tick<0>();
  } );
  pool.emplace_back( [&]() {
    mt19937 rd { random_device {}() };
    mbar.tick<0>();
    for ( auto _ : mbar.iterate<3>( 80000 ) )
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<>( 1, 1005 )( rd ) ) );
    mbar.tick<0>();
  } );

  for ( auto& td : pool ) {
    if ( td.joinable() )
      td.join();
  }
  mbar.wait();
}
