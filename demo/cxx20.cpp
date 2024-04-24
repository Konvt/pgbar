#include <vector>
#include "pgbar/range.hpp"
constexpr size_t TOTAL = INT32_MAX; // mersenne prime
double origin_arr[10000] { 0.0 };

int main()
{
#if defined(_WIN32) || defined(WIN32)
  system( "chcp 65001" ); // For Windows platform that uses GBK coding
#endif
  {
    std::cout << "Multithreaded rendering...\n";
    // If you are using the C++20 standard
    // You can initialize objects using pgbar::style and designated initializer
    pgbar::pgbar bar { pgbar::style {
      .todo_char = "\033[31m━\033[0m",
      .done_char = "\033[32m━\033[0m",
      .left_bracket = " ",
      .right_bracket = "",
      .total_tasks = TOTAL,
      .each_setp = 2,
      .option = pgbar::style::entire
    } };
    for ( size_t i = 0; i < (TOTAL / 2); ++i ) {
      bar.update(); // Normal update
      // Do anything you want here...
    }

    bar.reset().set_style( pgbar::style::percentage | pgbar::style::task_counter | pgbar::style::countdown );
    std::cout << "Task progress: "; // `range` needs a progress bar to show the progress situation
    for ( auto ele : pgbar::range( TOTAL / 2, bar ) )
      continue; // You can specify a range using `range`
    // The total number of tasks will be automatically set by function `range`
  }

  {
    std::cout << "Single threaded rendering...\n";
    pgbar::pgbar<std::ostream, pgbar::singlethread> bar; // single threading render
    // `pgbar::set_style` also supports the use of a designated initializer
    bar.set_style( pgbar::style {
      .left_bracket = " ",
      .right_bracket = "",
      .bar_length = 20
    } );
    std::vector<double> arr {};
    for ( size_t i = 0; i < 30000; ++i )
      arr.push_back( i );
    bar.set_style( pgbar::style::entire & ~pgbar::style::bar );
    // The total number of tasks will be automatically set by function `range`
    for ( auto ele : pgbar::range( arr, bar ) )
      continue; // Using a container with elements as the range
    arr.resize( 0 );

    float* pointer_arr = new float[30000] {0.0};
    bar.reset().set_style( pgbar::style::percentage );
    std::cout << "Pointer arrays is okay: ";
    // Also can pass a pointer array as the range
    for ( auto ele : pgbar::range( pointer_arr + 30000 - 1, pointer_arr - 1, bar ) )
      continue;
    delete[] pointer_arr; pointer_arr = nullptr;

    bar.reset().set_style( pgbar::style::entire );
    for ( auto ele : pgbar::range( origin_arr, bar ) )
      continue;
  }
}
