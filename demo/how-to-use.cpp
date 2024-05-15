#include "pgbar/pgbar.hpp"
#include "pgbar/range.hpp"
#include <numeric>
#include <vector>

int main()
{
  { // Set everything to default, including the number of tasks and each step.
    pgbar::pgbar<> bar;
  }

  { // Using another stream object which satisfies the predicate `pgbar::is_stream`.
    using Stream = std::ostream; // The default stream type is `std::ostream`.
    static_assert(pgbar::is_stream<Stream>::value == true, "");
    pgbar::pgbar<Stream> bar { std::clog };
  }

  { // Using another rendering mode which satisfies the predicate `pgbar::is_renderer`.
    using Renderer = pgbar::singlethread; // The default renderer is `pgbar::multithread`.
    static_assert(pgbar::is_renderer<Renderer>::value == true, "");
    pgbar::pgbar<std::ostream, Renderer> bar;
  }

  { // Set the number of tasks and step while creating a pgbar object.
    constexpr size_t num_tasks = 0x7fffffff;
    pgbar::pgbar<> bar { num_tasks, 2 }; // The default step is 1.
  }

  { // Set the style **after** creating a pgbar object.
    pgbar::pgbar<> bar;
    bar.set_style( pgbar::style::bar )
       .set_todo( "-" )
       .set_done( "=" )
       .set_status_col( pgbar::dye::yellow ); // and so on...
  }

  { // Set the style **while** creating a pgbar object.
    pgbar::pgbar<> bar {
      std::cerr, // The stream object must be provided
      pgbar::initr::option( pgbar::style::ratio ),
      pgbar::initr::todo_char( "-" ),
      pgbar::initr::done_char( "=" ),
      pgbar::initr::left_status( "" ),
      pgbar::initr::right_status( "" ) /// and so on...
    };
    // Member method `set_style` also supports this.
    bar.set_style(
      pgbar::initr::total_tasks( 300 ),
      pgbar::initr::status_color( pgbar::dye::green )
    );
  }

  { // Using the factory function to create a pgbar object.
    auto bar = pgbar::make_pgbar<pgbar::singlethread>(
      std::cerr, // ditto
      pgbar::initr::option( pgbar::style::ratio ),
      pgbar::initr::todo_char( "-" ),
      pgbar::initr::done_char( "=" ),
      pgbar::initr::left_status( "" ),
      pgbar::initr::right_status( "" ) // and so on...
    );
  }

  { // Using the pgbar object by calling method `update()`
    pgbar::pgbar<> bar { 500 };
    for ( auto _ = 0; _ < 500; ++_ ) {
      bar.update();
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
    bar.reset(); // If you want to use the pgbar object later again, you must invoke the method `reset()`.
  }

  { // Using the pgbar object by calling function `range()` which is in pgbar/range.hpp
    static int arr[200] {};
    std::iota( arr, arr + 200, 0 );
    pgbar::pgbar<> bar;
    for ( auto ele : pgbar::range( arr, bar ) )
      std::this_thread::sleep_for( std::chrono::milliseconds( ele ) ); // do something you want here...
    bar.reset(); // ditto...
  }

  { // Using numeric types to define iteration intervals with `range()`.
    pgbar::pgbar<> bar;
    std::vector<int> interval;
    for ( auto ele : pgbar::range( 10, bar ) ) { // in interval [0, 10)
      interval.push_back( ele );
      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
    bar.reset();
    std::cout << "The iteration intervals: ";
    for ( auto ele : interval )
      std::cout << ele << ' ';
    std::cout << std::endl;
  }

  { // Using the member method `update_to()` to update the pgbar object.
    pgbar::pgbar<> bar {
      std::clog,
      pgbar::initr::total_tasks( 233 )
    };
    for ( auto _ = 0; _ < 20; ++_ ) {
      bar.update( 2 ); // completes 2 tasks in each cycle iteration
      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
    bar.update_to( 80 ); // then completes 80% of the tasks at once
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    bar.update_to( 100 ); // finally completes 100% of the tasks, and quit.
  }
}
