#include "pgbar/DynamicBar.hpp"
#include "pgbar/ProgressBar.hpp"
#include <chrono>
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::DynamicBar<> dbar;

  auto bar1 = dbar.insert<pgbar::ProgressBar<>>();
  auto bar2 =
    dbar.insert( pgbar::config::Line( pgbar::option::Prefix( "No.2" ), pgbar::option::Tasks( 8000 ) ) );

  vector<thread> pool;
  pool.emplace_back( [&bar1]() {
    bar1->config().prefix( "No.1" ).tasks( 1919 );
    this_thread::sleep_for( chrono::seconds( 3 ) );
    do {
      bar1->tick();
      this_thread::sleep_for( chrono::milliseconds( 5 ) );
    } while ( bar1->active() );
  } );
  pool.emplace_back( [&bar2]() {
    this_thread::sleep_for( chrono::seconds( 2 ) );
    do {
      bar2->tick();
      this_thread::sleep_for( chrono::microseconds( 900 ) );
    } while ( bar2->active() );
  } );
  pool.emplace_back( [&dbar]() {
    auto bar =
      dbar.insert<pgbar::config::Line>( pgbar::option::Prefix( "No.3" ), pgbar::option::Tasks( 1000 ) );
    for ( int i = 0; i < 500; ++i ) {
      bar->tick();
      this_thread::sleep_for( chrono::milliseconds( 5 ) );
    }
    bar->reset();

    // The "No.3" bar will reappear at the bottom of the terminal.
    for ( int i = 0; i < 400; ++i ) {
      bar->tick();
      this_thread::sleep_for( chrono::milliseconds( 5 ) );
    }
    // let it be destructed.
  } );

  for ( auto& td : pool )
    td.join();
  dbar.wait();
}
