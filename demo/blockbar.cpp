#include "pgbar/pgbar.hpp"

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  pgbar::BlockBar<> blckbar;
  blckbar.config().style( pgbar::config::Block::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    blckbar.tick();
}
