#include "pgbar/pgbar.hpp"
#include <numeric>
#include <vector>
#ifdef __unix__
# include <sys/ioctl.h>
#elif defined( _WIN32 )
# include <windows.h>
#endif

int main()
{
  {
    std::cout << "Set every configurations to default." << std::endl;
    // There are two kinds of "Progress Bar".
    pgbar::ProgressBar<> progress; // movable and swapable.
    pgbar::SpinnerBar<> spinner;   // ditto

    // In face, they are alias for template class `pgbar::Indicator`.
    static_assert(
      std::is_same<pgbar::ProgressBar<>, pgbar::Indicator<pgbar::configs::Progress>>::value,
      "" );
    static_assert(
      std::is_same<pgbar::SpinnerBar<>, pgbar::Indicator<pgbar::configs::Spinner>>::value,
      "" );
  }

  {
    std::cout << "Tweak the detailed configurations." << std::endl;
    {
      pgbar::ProgressBar<> bar;

      // Set them by chain call
      bar.configure().todo( "-" ).done( "=" ).status_color( pgbar::colors::Yellow );
      // The library provides several preset colors in `pgbar::colors`,
      // you can disable it globally by defining a macro `PGBAR_COLORLESS` before including header.

      // Or you can change the value of the bar's configuration to disable colors locally.
      if ( bar.configure().colored() )
        bar.configure().colored( false );

      // Don't forget to set the task before updating.
      bar.configure().tasks( 100 );
      // The default tasks is 0.

      for ( auto _ = 0; _ < 100; ++_ )
        bar.tick();
    }
    {
      pgbar::SpinnerBar<> bar;
      bar.configure().suffix( "Waiting main thread..." );

      bar.tick();

      std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

      bar.reset();
    }
  }

  {
    std::cout << "Customize the style of the progress bar." << std::endl;
    pgbar::ProgressBar<> bar;

    // As mentioned in the README, the progress bar consists of several sections,
    // which you can be toggled using bit switches from `pgbar::configs::Progress`.
    bar.configure()
      .styles( pgbar::configs::Progress::Bar | pgbar::configs::Progress::Ratio
               | pgbar::configs::Progress::Timer )
      .tasks( 100 );
    for ( auto _ = 0; _ < 100; ++_ )
      bar.tick();

    // Moreover, the color setters support hex RGB value or strings.
    bar.configure().todo_color( "#A52A2A" ).done_color( 0x0099FF ).status_color( "#B22" );
    for ( auto _ = 0; _ < 100; ++_ )
      bar.tick();

    // You can also output a custom string to the left of the progress bar.
    std::cout << "Here is the progress bar: ";
    for ( auto _ = 0; _ < 100; ++_ )
      bar.tick();
  }

  {
    std::cout << "Tweak the configurations *when* constructing a object." << std::endl;
    {
      // You can create a `ProgressBar` by passing a list of wrapper objects defined in
      // `pgbar::options`.
      pgbar::ProgressBar<> bar { pgbar::options::Bolded( false ),
                                 pgbar::options::Styles( pgbar::configs::Progress::Rate
                                                         | pgbar::configs::Progress::Timer ),
                                 pgbar::options::StatusColor( pgbar::colors::Yellow ),
                                 pgbar::options::Tasks( 100 ) };
      for ( auto _ = 0; _ < 100; ++_ )
        bar.tick();

      // This configuration method can be used not only in constructor,
      // but also in method `configure().set()`.
      bar.configure().set( pgbar::options::StatusColor( pgbar::colors::Green ) );
      for ( auto _ = 0; _ < 100; ++_ )
        bar.tick();

      // This actually modifies the object `pgbar::configs::Progress` within the `ProgressBar`
      // object, which allows you to modify the configuration in this manner if desired.
      auto cfg = pgbar::configs::Progress( pgbar::options::TodoChar( "-" ),
                                           pgbar::options::DoneChar( "+" ),
                                           pgbar::options::Tasks( 114514 ) );
      pgbar::ProgressBar<> bar2 { cfg }; // copyable
      bar.configure( std::move( cfg ) ); // also movable
    }
    { // It's same for `SpinnerBar`.
      pgbar::SpinnerBar<> bar {
        pgbar::options::Frames(
          { ">", ">", ">", ">", ">>", ">>", ">>", ">>>", ">>>", ">>>", ">>>", ">>>" } ),
        pgbar::options::FramesColor( pgbar::colors::Magenta ),
        pgbar::options::Suffix( "En attendant Godot" ),
        pgbar::options::FalseFrame( "Oui, allons-y." ),
        pgbar::options::FalseColor( "#A1A1A1" )
      };
      bar.tick();

      std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

      bar.reset( false );
    }
  }

  {
    std::cout << "Reset the progress bar when you need." << std::endl;
    // The method `reset()` aborts the current ticking process,
    // and resets the progress bar to its initial state,
    // discarding any current progress.

    // This method is invoked automatically upon iteration completes,
    // or can be triggered manually to abort early.
    pgbar::ProgressBar<> bar { 100 };
    for ( auto _ = 0; _ < 100; ++_ ) {
      bar.tick();
      if ( _ == 49 ) {
        bar.reset();
        // Now the iteration progress tracked by `bar` has been reset.
        std::cout << "Aborting!" << std::endl;
        break;
      }
    }
  }

  {
    std::cout << "Use the method `foreach()` to traverse a range." << std::endl;
    pgbar::ProgressBar<> bar;
    // This provides a convenient method for traversing a STL-like container or raw arrays,
    // and it will automatically update the progress bar.

    // There are two styles of traversal: one using a range-based for loop,
    std::vector<int> vec = { 1, 2, 3, 4, 5 }; // STL container
    for ( auto _ : bar.foreach ( vec ) )
      // `foreach` will automatically determine the number of tasks.
      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    // Or you can traverse the container using iterators.
    for ( auto _ : bar.foreach ( vec.begin(), vec.end() ) )
      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    // Another style of traversing is using a lambda functor:
    int arr1[] = { 6, 7, 8, 9, 10 }; // raw array
    bar.foreach ( arr1, []( int _ ) {
      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    } );

    // Not only that, but you can also traverse a range of values.
    std::vector<double> buffer;
    // NOTE: The iteration interval is half-open,
    // meaning it inclues the left endpoint and excludes the right endpoint.
    bar.foreach ( 1.0, -1.0, -0.1, [&buffer]( double e ) { buffer.push_back( e ); } );

    std::cout << "The elements is: ";
    for ( auto e : buffer )
      std::cout << e << ' ';
    std::cout << std::endl;

    // These behaviors rely on abstract range iterators defined in `pgbar::iterators`.
    auto numbers = pgbar::iterators::NumericSpan<int>( 0, 20, 1 );
    auto begin   = numbers.begin();

    int arr2[] = { 0, 1, 2 };
    // For pointer types, the start point can be greater than the endpoint (i.e., in reverse
    // order), as long as the range they point to is left-closed and right-open.
    auto iters = pgbar::iterators::IterSpan<int*>( arr2 + 2, arr2 - 1 );

    // The `foreach()` uses a special range named `pgbar::iterators::ProxySpan`.
    // As its name suggests,
    // this range takes another range and a reference to an existing `ProgressBar` object,
    // traversing the range and updating the `ProgressBar` simultaneously.
    auto proxy =
      pgbar::iterators::ProxySpan<pgbar::iterators::NumericSpan<int>, pgbar::ProgressBar<>>(
        std::move( numbers ),
        bar );
    // In practice, we don't usually use this class directly.
  }

  {
    std::cout << "Some exceptional situations." << std::endl;
    pgbar::ProgressBar<> bar;

    // If the number of tasks is not specified or it is set to zero,
    // the method `tick()` will throw an exception named `pgbar::exceptions::InvalidState`.
    try {
      bar.tick();
    } catch ( const pgbar::exceptions::InvalidState& e ) {
      // All classes defined in `pgbar::exceptions` derive from `std::exception`.
      const std::exception& base_e = e;
      std::cerr << "  Exception: \"" << base_e.what() << "\"" << std::endl;
    }

    // If you pass an invalid interval to `foreach()`,
    // the object will throw an `pgbar::exceptions::InvalidArgument` exception immediately.
    try {
      // `start` is less than `end` while `step` is negative; vice versa.
      bar.foreach ( -100, 100, -1, []( double e ) {} );
    } catch ( const pgbar::exceptions::InvalidArgument& e ) {
      std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
    }

    // If you give an invalid Hex RGB string,
    // the object will throw an `pgbar::exceptions::InvalidArgument` exception immediately.
    try {
      bar.configure().status_color( "#F" );
    } catch ( const pgbar::exceptions::InvalidArgument& e ) {
      std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
    }
    try {
      bar.configure().status_color( "#1234567" );
    } catch ( const pgbar::exceptions::InvalidArgument& e ) {
      std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
    }
    try {
      bar.configure().status_color( "#NON" );
    } catch ( const pgbar::exceptions::InvalidArgument& e ) {
      std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
    }
    try {
      bar.configure().status_color( "FFFFFF" );
    } catch ( const pgbar::exceptions::InvalidArgument& e ) {
      std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
    }

    pgbar::SpinnerBar<> spbar;
    // If you pass an empty vector to `frames()`,
    // the object will throw an `pgbar::exceptions::InvalidArgument` exception immediately.
    try {
      spbar.configure().frames( {} );
    } catch ( const pgbar::exceptions::InvalidArgument& e ) {
      std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
    }
  }

  {
    std::cout << "Appoint a specific stream object." << std::endl;

    using StreamType = std::ostream;
    // The stream object must satisfy the `is_ostream` trait.
    static_assert( pgbar::traits::is_ostream<StreamType>::value, "ERROR" );

    pgbar::ProgressBar<StreamType> bar { std::clog };
    pgbar::SpinnerBar<StreamType> spbar { std::clog };
    // The default stream object is bound to `std::cerr`.

    // If the program is not running in a terminal,
    // the `Config::intty()` will return false,
    // and the progress bar display nothing.
    std::cout << "  Current program is running in a tty? " << std::boolalpha
              << pgbar::configs::Global::intty() << std::endl;
    // This feature is available only on Windows or Unix-like systems.

    // Furthermore, you can define a macro `PGBAR_INTTY` before including "pgbar.hpp",
    // to force `pgbar` to assume it is working in a terminal.
  }

  {
    std::cout << "Change the length of the progress bar." << std::endl;
    pgbar::ProgressBar<> bar { pgbar::options::Styles( pgbar::configs::Progress::Entire ) };
    // Change the size of the progress bar via method `bar_length()`.

    std::cout << "  Default bar length: " << bar.configure().bar_length() << std::endl;
    bar.configure().bar_length( 50 ); // Unit: characters

    bar.configure().tasks( 20 );
    for ( auto _ = 0; _ < 20; ++_ ) {
      bar.tick();
      std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    // If you can get the terminal width,
    // you can calculate the length to make the bar fill a line.

    // We need to use the OS api to get the terminal width.
#ifdef _WIN32
    const auto terminal_width = []() {
      HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      if ( GetConsoleScreenBufferInfo( hConsole, &csbi ) ) {
        const auto width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        return width == 0 ? 120 : width;
      } else
        return 120;
    }();
#elif defined( __unix__ )
    const auto terminal_width = []() {
      struct winsize w;
      if ( ioctl( STDOUT_FILENO, TIOCGWINSZ, &w ) == -1 )
        return 120;
      return w.ws_col == 0 ? 120 : w.ws_col;
    }();
#else
    constexpr auto terminal_width = 120; // Assuming terminal width is 120.
#endif

    const auto total_length_excluding_bar = bar.configure().fixed_size();
    bar.configure().bar_length( terminal_width - total_length_excluding_bar );

    bar.configure().tasks( 40 );
    for ( auto _ = 0; _ < 40; ++_ ) {
      bar.tick();
      std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
  }

  {
    std::cout << "Determine the refresh rate of the progress bar." << std::endl;
    // The refresh interval is a global static object defined in `pgbar::configs::Global`,
    // which affects the refresh rate of all progress bars output to the terminal.

    std::cout << "  Default refresh interval: "
              << pgbar::configs::Global::refresh_interval().count() << " ns" << std::endl;
    // You can change its value by invoking a function with the same name.
    pgbar::configs::Global::refresh_interval( std::chrono::nanoseconds( 20000 ) );
    std::cout << "  The new refresh interval: "
              << pgbar::configs::Global::refresh_interval().count() << " ns" << std::endl;

    // After each progress bar is output to the terminal,
    // it waits for the refresh interval (in nanoseconds) before outputting again.

    // Since the time consumed during each output is not always zero,
    // the maximum refresh rate of the progress bar depends on the time
    // required to assemble and output the progress bar.

    // The refresh interval merely provides the capability to extend the refresh time,
    // or in other words, "lower the refresh rate."
  }

  {
    std::cout << "The thread safety." << std::endl;
    // Invoking member and static functions other than the ctor of the objects from `pgbar::configs`
    // is thread-safe, they are protected by a read-write lock.

    // However, for `pgbar::ProgressBar`, when it is running (Use the `is_running` method for the
    // check), invoking any non-const member function of its `pgbar::configs::Progress` is
    // thread-unsafe, and the behavior in such cases is unspecified.

    // The `pgbar::ProgressBar` itself can be either thread-safe or thread-unsafe (same for
    // `pgbar::SpinnerBar`), it depends on its second template parameter.
    // The second template parameter can be any type that satisfies "basic lockable" requirement.
    pgbar::ProgressBar<std::ostream, pgbar::Threadsafe> thread_safe_bar;
    pgbar::ProgressBar<std::ostream, std::mutex> stl_base_bar;               // fine
    pgbar::ProgressBar<std::ostream, pgbar::Threadunsafe> thread_unsafe_bar; // default

    // In this context,
    // "thread-safe" means allowing multiple threads to invoke `tick` concurrently,
    // but does not include moving assignment or swapping.

    // And as you can probably imagine, the thread-safe version doesn't perform as well.

    constexpr auto iteration   = 10000000;
    constexpr auto num_threads = 10;
    thread_safe_bar.configure().tasks( iteration );

    std::vector<std::thread> threads;
    for ( auto _ = 0; _ < num_threads; ++_ ) {
      threads.emplace_back( [&]() {
        for ( auto _ = 0; _ < iteration / num_threads; ++_ ) {
          thread_safe_bar.tick();
          std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
        }
      } );
    }

    for ( auto& td : threads ) {
      if ( td.joinable() )
        td.join();
    }
  }
}
