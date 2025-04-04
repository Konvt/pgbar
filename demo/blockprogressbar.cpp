#include "pgbar/pgbar.hpp"

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  pgbar::BlockProgressBar<> bpbar;
  bpbar.config().style( pgbar::config::BlckBar::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    bpbar.tick();
}
