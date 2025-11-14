// This file is only used to trigger CMake to generate compile_commands.json.
#include "pgbar/pgbar.hpp"
#include <random>
#include <thread>
using namespace std;

int main()
{
  mt19937 rd { random_device {}() };
  {
    pgbar::iterate<pgbar::config::Block>(
      10000,
      [&]( int ) {
        this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<int>( 1, 1200 )( rd ) ) );
      },
      []( pgbar::BlockBar<>& self ) { self.config().filler_color( pgbar::color::Green ); },
      pgbar::option::Lead( { " ", "▁", "▂", "▃", "▄", "▅", "▆", "▇" } ),
      pgbar::option::InfoColor( "#FFD200" ) );
  }
  {
    for ( auto _ : pgbar::iterate<pgbar::config::Block>(
            10000,
            []( pgbar::BlockBar<>& self ) { self.config().filler_color( pgbar::color::Green ); },
            pgbar::option::Lead( { " ", " ", "░", "░", "▒", "▒", "▓", "▓" } ),
            pgbar::option::InfoColor( "#00BBFF" ) ) ) {
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<int>( 1, 1200 )( rd ) ) );
    }
  }
}
