#include "pgbar/pgbar.hpp"
#include <chrono>
#include <random>
#include <thread>
#include <vector>
using namespace std;

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  auto bar =
    pgbar::make_multi( pgbar::config::Line( pgbar::option::Prefix( "Eating something...." ),
                                            pgbar::option::Filler( "⠇" ),
                                            pgbar::option::Lead( { "⠈", "⠐", "⠠", "⢀", "⡀", "⠄", "⠂", "⠁" } ),
                                            pgbar::option::Shift( 1 ),
                                            pgbar::option::TrueMesg( "✔ Mission Accomplished" ),
                                            pgbar::option::TrueColor( pgbar::color::Green ),
                                            pgbar::option::FalseMesg( "❌ Mission failed" ),
                                            pgbar::option::FalseColor( pgbar::color::Red ),
                                            pgbar::option::InfoColor( "#7D7" ) ),
                       pgbar::config::Block( pgbar::option::Prefix( "Picking something..." ),
                                             pgbar::option::Lead( { "⠄", "⠆", "⠇", "⠧", "⠷", "⠿" } ),
                                             pgbar::option::TrueMesg( "✔ Mission Complete" ),
                                             pgbar::option::TrueColor( pgbar::color::Green ),
                                             pgbar::option::FalseMesg( "❌ Mission failed" ),
                                             pgbar::option::FalseColor( pgbar::color::Red ),
                                             pgbar::option::InfoColor( "#7BD" ) ) );

  vector<thread> pool;
  pool.emplace_back( [&bar]() {
    mt19937 rd { random_device {}() };
    bar.iterate<0>( 50000, [&rd]( int ) {
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<int>( 700, 1100 )( rd ) ) );
    } );
  } );
  pool.emplace_back( [&bar]() {
    mt19937 rd { random_device {}() };
    bar.config<1>().tasks( 10000 );
    const size_t terminate_val = 5000 + uniform_int_distribution<int>( 10, 1000 )( rd );
    for ( size_t i = 0; i < terminate_val; ++i ) {
      bar.tick<1>();
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<int>( 1100, 3000 )( rd ) ) );
    }
    bar.reset<1>( false );
  } );

  for ( auto& td : pool )
    td.join();
  bar.wait();
}
