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

  auto mbar = pgbar::make_multi<pgbar::BlockBar<>, 3>();

  vector<thread> pool;
  pool.emplace_back( thread( [&mbar]() {
    mbar.iterate<0>( 500, []( int ) {
      auto rd = mt19937( random_device {}() );
      this_thread::sleep_for( chrono::milliseconds( uniform_int_distribution<>( 10, 70 )( rd ) ) );
    } );
  } ) );
  pool.emplace_back( thread( [&mbar]() {
    mbar.config<1>().tasks( 300 );
    auto rd = mt19937( random_device {}() );
    for ( int i = 0; i < 300; ++i ) {
      mbar.tick<1>();
      this_thread::sleep_for( chrono::milliseconds( uniform_int_distribution<>( 20, 120 )( rd ) ) );
    }
  } ) );
  pool.emplace_back( thread( [&mbar]() { mbar.iterate<2>( 2147483647, []( int ) {} ); } ) );

  for ( auto& td : pool ) {
    if ( td.joinable() )
      td.join();
  }
  mbar.wait();
}
