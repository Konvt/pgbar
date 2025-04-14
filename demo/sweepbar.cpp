#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::SweepBar<> swpbar;
  swpbar.config().style( pgbar::config::Sweep::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    swpbar.tick();
}
