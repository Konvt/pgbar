#include "pgbar/pgbar.hpp"

#ifdef _WIN32
# include <windows.h>
#endif

int main()
{
#ifdef _WIN32
  SetConsoleOutputCP( 65001 ); // Set UTF-8 output
#endif

  {
    constexpr auto iteraton = 1000;
    pgbar::ProgressBar<> bar {
      pgbar::options::TodoChar( "━" ),          pgbar::options::DoneChar( "━" ),
      pgbar::options::StartPoint( "🔥 " ),      pgbar::options::EndPoint( "" ),
      pgbar::options::LeftStatus( "➔ " ),       pgbar::options::Tasks( iteraton ),
      pgbar::options::TodoColor( "#E20044" ),   pgbar::options::DoneColor( "#24F246" ),
      pgbar::options::StatusColor( "#6540ED" ), pgbar::options::Divider( " " )
    };

    for ( auto _ = 0; _ < iteraton; ++_ ) {
      bar.tick();
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
  }

  {
    pgbar::SpinnerBar<> bar { pgbar::options::Frames( { "◜", "◜", "◝", "◝", "◞", "◞", "◟", "◟" } ),
                              pgbar::options::FramesColor( "#ABC" ),
                              pgbar::options::Suffix( "Check the configuration" ),
                              pgbar::options::TrueFrame( "✔ Mission Complete!" ),
                              pgbar::options::TrueColor( pgbar::colors::Green ),
                              pgbar::options::FalseFrame( "✖ Execution Failure!" ),
                              pgbar::options::FalseColor( pgbar::colors::Red ) };

    bar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
    bar.reset();

    bar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
    bar.reset( false );
  }
}
