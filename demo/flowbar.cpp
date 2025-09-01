#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::FlowBar<> flwbar;
  flwbar.config().style( pgbar::config::Flow::Entire ).tasks( 2147483647 );
  for ( size_t i = 0; i < 2147483647; ++i )
    flwbar.tick();
}
