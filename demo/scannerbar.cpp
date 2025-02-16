#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::ScannerBar<> scnbar;
  scnbar.config().style( pgbar::config::ScanBar::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    scnbar.tick();
}
