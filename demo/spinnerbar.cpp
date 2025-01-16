#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::SpinnerBar<> spibar;
  spibar.config().style( pgbar::config::SpinBar::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    spibar.tick();
}
