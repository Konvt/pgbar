#include "../include/pgbar/pgbar.hpp"
#include <iomanip>
#include <limits>

int main()
{
  using IterationType      = std::uint32_t;
  constexpr auto iteration = std::numeric_limits<IterationType>::max();
  {
    pgbar::Indicator<pgbar::configs::Progress> bar { iteration };

    auto now = std::chrono::system_clock::now();
    for ( IterationType _ = 0; _ < iteration; ++_ )
      bar.tick();
    const auto total_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now() - now )
        .count();
    const auto average_ns = static_cast<double>( total_ns ) / iteration;

    std::cout << "The average 'tick' takes " << std::fixed << std::setprecision( 2 ) << average_ns
              << " ns\n"
              << std::endl;
  }

  {
    auto now = std::chrono::system_clock::now();
    pgbar::Indicator<pgbar::configs::Progress> bar { iteration };

    const auto ctor_us = std::chrono::duration_cast<std::chrono::microseconds>(
                           std::chrono::system_clock::now() - now )
                           .count();

    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

    now = std::chrono::system_clock::now();
    bar.tick();
    const auto firstly_tick_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                   std::chrono::system_clock::now() - now )
                                   .count();

    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

    now = std::chrono::system_clock::now();
    bar.tick();
    const auto normally_tick_us =
      std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now() - now )
        .count();

    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

    bar.tick( iteration - 3 );

    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

    now = std::chrono::system_clock::now();
    bar.tick();
    const auto finally_tick_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                   std::chrono::system_clock::now() - now )
                                   .count();

    std::cout << "The default constructor takes " << ctor_us << " us" << std::endl
              << "Firstly 'tick' takes " << firstly_tick_us << " us" << std::endl
              << "Normally 'tick' takes " << normally_tick_us << " ns" << std::endl
              << "Finally 'tick' takes " << finally_tick_us << " us" << std::endl;
  }
}
