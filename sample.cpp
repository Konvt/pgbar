#include <vector>
#include "include/pgbar/range.hpp"
constexpr std::size_t TOTAL = 100000000;
double origin_arr[10000] { 0.0 };

int main()
{
#if defined(_WIN32) || defined(WIN32)
  system( "chcp 65001" ); // for Windows using GBK coding
#endif

  {
    std::cout << "Testing...";
    pgbar::pgbar<> bar { TOTAL, std::cerr };
    bar.set_left_bracket( " " ).set_right_bracket( "" ).set_todo_char( "\033[31m━\033[0m" ).set_done_char( "\033[32m━\033[0m" );
    bar.set_style( pgbar::style::entire ).set_step( 2 ); // Set the step.
    for ( std::size_t i = 0; i < (TOTAL / 2); ++i ) {
      bar.update(); // Normal update
      // Do anything you want here...
    }

    bar.reset().set_style( pgbar::style::percentage | pgbar::style::task_counter | pgbar::style::countdown );
    std::cout << "Task progress: "; // `range` needs a progress bar to show the progress situation
    for ( auto ele : pgbar::range( TOTAL / 2, bar ) )
      continue; // You can specify a range using `range`
    // The iterator will automatically count the number of tasks
  }
  {
    pgbar::pgbar<std::ostream, pgbar::singlethread> bar { TOTAL, std::cerr }; // single threading render
    std::vector<double> arr {};
    for ( std::size_t i = 0; i < 30000; ++i )
      arr.push_back( i );
    bar.reset().set_style( pgbar::style::entire & ~pgbar::style::bar );
    for ( auto ele : pgbar::range( arr, bar ) )
      continue; // Using a container with elements as the range
    arr.resize( 0 );

    float* pointer_arr = new float[30000] {0.0};
    bar.reset().set_style( pgbar::style::task_counter );
    std::cout << "Pointer arrays is okay: ";
    for ( auto ele : pgbar::range( pointer_arr + 50000 - 1, pointer_arr - 1, bar ) )
      continue; // Also can pass a pointer array as the range
    delete[] pointer_arr; pointer_arr = nullptr;

    bar.reset().set_style( pgbar::style::entire );
    for ( auto ele : pgbar::range( origin_arr, bar ) )
      continue;
  }
}
