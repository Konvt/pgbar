#include "pgbar/SpinBar.hpp"

int main()
{
  pgbar::SpinBar<> spibar;
  spibar.config()
    .prefix( "Working" )
    .lead( { ".", "..", "..." } )
    .tasks( 2147483647 )
    .disable()
    .entire();
  spibar.config().enable().animation().speed().elapsed();
  for ( size_t i = 0; i < 2147483647; ++i )
    spibar.tick();
}
