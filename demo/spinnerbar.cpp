#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::SpinnerBar<> spibar;
  spibar.config()
    .description( "Working" )
    .lead( { ".", "..", "..." } )
    .tasks( 2147483647 )
    .disable()
    .entire();
  spibar.config().enable().animation().speed().elapsed();
  for ( size_t i = 0; i < 2147483647; ++i )
    spibar.tick();
}
