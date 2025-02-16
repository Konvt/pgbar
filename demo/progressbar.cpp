#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::ProgressBar<> pbar;
  pbar.config().style( pgbar::config::CharBar::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    pbar.tick();
}
