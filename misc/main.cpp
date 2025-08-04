// This file is only used to trigger CMake to generate compile_commands.json.
#include "pgbar/pgbar.hpp"
#include <random>
#include <thread>
using namespace std;

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  pgbar::iterate<pgbar::config::Block>(
    10000,
    []( int ) {
      mt19937 rd { random_device {}() };
      this_thread::sleep_for( chrono::microseconds( uniform_int_distribution<int>( 700, 1100 )( rd ) ) );
    },
    pgbar::option::Lead( { " ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█" } ),
    pgbar::option::InfoColor( "#FFD200" ) );
}
