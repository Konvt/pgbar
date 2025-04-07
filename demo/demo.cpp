#include "pgbar/pgbar.hpp"
#include <chrono>
#include <random>
#include <thread>
#include <vector>

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  auto bar = pgbar::make_multi( pgbar::config::CharBar( pgbar::option::Description( "Eating something...." ),
                                                        pgbar::option::TrueMesg( "✔ Mission Accomplished" ),
                                                        pgbar::option::TrueColor( pgbar::color::Green ),
                                                        pgbar::option::FalseMesg( "❌ Mission failed" ),
                                                        pgbar::option::FalseColor( pgbar::color::Red ),
                                                        pgbar::option::InfoColor( "#7D7" ) ),
                                pgbar::config::CharBar( pgbar::option::Description( "Picking something..." ),
                                                        pgbar::option::TrueMesg( "✔ Mission Complete" ),
                                                        pgbar::option::TrueColor( pgbar::color::Green ),
                                                        pgbar::option::FalseMesg( "❌ Mission failed" ),
                                                        pgbar::option::FalseColor( pgbar::color::Red ),
                                                        pgbar::option::InfoColor( "#7BD" ) ) );

  std::vector<std::thread> pool;
  pool.emplace_back( std::thread( [&bar]() {
    std::mt19937 rd { std::random_device {}() };
    bar.iterate<0>( 50000, [&rd]( int ) {
      std::this_thread::sleep_for(
        std::chrono::microseconds( std::uniform_int_distribution<int>( 700, 1100 )( rd ) ) );
    } );
  } ) );
  pool.emplace_back( std::thread( [&bar]() {
    std::mt19937 rd { std::random_device {}() };
    bar.config<1>().tasks( 10000 );
    const std::size_t terminate_val = 5000 + std::uniform_int_distribution<int>( 10, 1000 )( rd );
    for ( std::size_t i = 0; i < terminate_val; ++i ) {
      bar.tick<1>();
      std::this_thread::sleep_for(
        std::chrono::microseconds( std::uniform_int_distribution<int>( 1100, 3000 )( rd ) ) );
    }
    bar.reset<1>( false );
  } ) );

  for ( auto& td : pool )
    if ( td.joinable() )
      td.join();
  bar.wait();
}
