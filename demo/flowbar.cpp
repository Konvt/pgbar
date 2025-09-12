#include "pgbar/FlowBar.hpp"
#include <chrono>
#include <thread>

int main()
{
  pgbar::FlowBar<> flwbar;
  flwbar.config().reverse( true );
  flwbar.tick();
  std::this_thread::sleep_for( std::chrono::seconds( 20 ) );
  flwbar.reset();
}
