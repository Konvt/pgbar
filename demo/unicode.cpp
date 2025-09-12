#include "pgbar/ProgressBar.hpp"

int main()
{
  pgbar::ProgressBar<> pbar;
  pbar.config().set( pgbar::option::Starting( "🔥 " ),
                     pgbar::option::Ending( " ✅" ),
                     pgbar::option::Lead( "🚀" ),
                     pgbar::option::Filler( "急" ),
                     pgbar::option::Tasks( 214748364 ) );
  for ( size_t i = 0; i < 214748364; ++i )
    pbar.tick();
}
