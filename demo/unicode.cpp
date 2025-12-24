#include "pgbar/ProgressBar.hpp"
#include <chrono>
#include <iostream>
using namespace std;

int main()
{
  constexpr auto iteration = 2147483647;

  pgbar::ProgressBar<> pbar;
  pbar.config().with( pgbar::option::Prefix( u8"にほんご" ),
                      pgbar::option::Starting( u8"🔥 " ),
                      pgbar::option::Ending( u8" ✅" ),
                      pgbar::option::Lead( u8"🚀" ),
                      pgbar::option::Filler( u8"急" ),
                      pgbar::option::Postfix( u8"한국어" ),
                      pgbar::option::Tasks( iteration ) );

  auto start = chrono::high_resolution_clock::now();
  for ( size_t i = 0; i < iteration; ++i )
    pbar.tick();
  auto duration = chrono::duration_cast<chrono::microseconds>( chrono::high_resolution_clock::now() - start );
  cout << "Average time per iteration: " << ( duration.count() / static_cast<double>( iteration ) ) << " us\n"
       << flush;
}
