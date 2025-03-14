#include "pgbar/pgbar.hpp"
#include <cassert>
#include <iostream>
#include <numeric>
#ifdef __unix__
# include <sys/ioctl.h>
#endif

int main()
{
#if _WIN32
  system( "chcp 65001" );
#endif

  {
    std::cout << "Basic user guide." << std::endl;
    // All progress bar objects can be constructed by default without parameters
    // And these objects are movable, swappble but not copyable
    pgbar::ProgressBar<> pbar;
    pgbar::BlockProgressBar<> bpbar;
    pgbar::SpinnerBar<> spibar;
    pgbar::ScannerBar<> scnbar;

    // The classes are derived from pgbar::Indicator
    pgbar::Indicator& base_ref = bpbar;

    // The ProgressBar and BlockProgressBar constructed by default cannot be used directly
    // it must be used only after passing a number of tasks to the method config().tasks()
    // Otherwise it will throw an exception pgbar::exception::InvalidState
    try {
      pbar.tick();
    } catch ( const pgbar::exception::InvalidState& e ) {
      std::cerr << "Oops! An exception occurs here: \"" << e.what() << "\"" << std::endl;
    }
    pbar.config().tasks( 100 );
    for ( auto i = 0; i < 100; ++i ) {
      pbar.tick();
      // During iteration, you can use the is_running() and progress() methods
      // to check whether the current progress bar is running
      // and to get the number of times that tick() has been called so far
      if ( i == 99 )                  // The last call of tick() causes the progress bar to stop automatically
        assert( !pbar.is_running() ); // So the assert is not going to be true at that moment
      else
        assert( pbar.is_running() );
      assert( pbar.progress() != 0 );
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    // As soon as' tick() is called as many times as the predetermined value of config().tasks()
    // the progress bar will stop running on its own
    // However, you can also call the reset() method of the progress bar in advance to actively stop the
    // progress bar If the progress bar object is destructed, it is equivalent to calling reset()

    // SpinnerBar and ScannerBar do not require the number of tasks to be specified and can be used directly
    spibar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    // However, because there is no limit on the number of tasks
    // the work of the progress bar will not stop by itself
    // and the reset() method must be actively called at this time to stop rendering
    spibar.reset();

    // Instead of manually specifying the number of tasks
    // you can use the iterate() method to make the progress bar work on an "abstract range"
    // where the progress bar object will count the number of tasks itself
    // iterate() is used in a similar way to the range() function in Python
    std::cout << "Iterate over a range of values with BlockProgressBar: " << std::flush;
    for ( auto&& _ : bpbar.iterate( 50 ) ) {
      // Iterate over the numeric interval [0, 50) with step size 1
      std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );
    }

    // The iterate() method can also work on a data container, such as an array
    std::cout << "Iterate over a raw array with BlockProgressBar: " << std::flush;
    int arr[100] { 0 };
    for ( auto&& e : bpbar.iterate( arr ) ) {
      e = 114514; // access the elements directly within a loop
    }
    bpbar.wait();
    const auto baseline = std::vector<int>( 100, 114514 );
    std::cout << "Are the values in these two ranges equal? " << std::boolalpha
              << std::equal( arr, arr + 100, baseline.cbegin() ) << std::endl;

    // You can also pass two iterators to iterate() for the iterated range
    // If it is an iterator of pointer type
    // it can be accessed in reverse order by inverting the starting and ending points
    std::cout << "Reverse iterate over a raw array with SpinnerBar: " << std::flush;
    for ( auto&& _ : spibar.iterate( arr + 100, arr ) ) {
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }

    // The data container here also contains the generic container in the STL, such as std::vector
    std::cout << "Iterate over an object of std::vector with ScannerBar: " << std::flush;
    for ( auto&& _ : scnbar.iterate( baseline ) ) {
      // do something here...
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }

    // iterate() can also accept a unary function and act on the element in the range, as can std::for_each
    std::cout << "Iterate over an object of std::vector with an unary function: " << std::flush;
    std::vector<char> alphabet( 26 );
    std::iota( alphabet.begin(), alphabet.end(), 'a' );
    scnbar.iterate( alphabet, []( char ) { /*...*/ } );
  }
  {
    std::cout << "Customize your own progress bar object." << std::endl;
    // All types of progress bars can be constructed as modified objects from the default configuration
    // by passing in no less than one pgbar::option wrapper type.
    pgbar::ProgressBar<> pbar { pgbar::option::Style( // Select a different progress bar information section
                                  pgbar::config::CharBar::Per | pgbar::config::CharBar::Elpsd ),
                                // Change the color of the information section
                                pgbar::option::InfoColor( "#7D7" ),
                                pgbar::option::Tasks( 100 ) };
    for ( auto _ = 0; _ < 100; ++_ ) {
      pbar.tick();
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }

    pgbar::SpinnerBar<> spibar { pgbar::option::Lead( { "◜", "◝", "◞", "◟" } ),
                                 pgbar::option::Description( "Loading..." ),
                                 pgbar::option::DescColor( "#39C5BB" ) };
    // You cannot pass the same option twice.
    spibar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    spibar.reset();

    // In addition to the fact that the constructor can use multiple variable pgbar::option wrappers
    // the member method config().set() can also accept arguments of this form
    spibar.config().set(
      pgbar::option::InfoColor( "#FFA500" ),
      // All types/methods used to configure colors can receive valid RGB color codes as strings
      // or RGB color values given directly as hexadecimal integers
      // And color effects can be forcibly turned off by defining a PGBAR_COLORLESS macro
      pgbar::option::Style( pgbar::config::CharBar::Sped | pgbar::config::CharBar::Per
                            | pgbar::config::CharBar::Elpsd | pgbar::config::CharBar::Cntdwn ),
      pgbar::option::SpeedUnit( { "B/s", "kB/s", "MB/s", "GB/s" } ),
      pgbar::option::Tasks( 70 ) );
    for ( auto i = 0; i < 70; ++i ) {
      spibar.tick();
      std::this_thread::sleep_for( std::chrono::milliseconds( 70 - i ) );
    }

    // In addition, config() itself provides a streaming interface style
    // which can also configure different parts of the progress bar style one by one
    spibar.config()
      .bolded( false )
      .info_color( 0xFFE211 )
      .speed_unit( { "Actually", "you can write", "anything", "here" } )
      .divider( " >< " )
      .left_border( "< " )
      .right_border( " >" )
      .tasks( 1000 );
    for ( auto i = 0; i < 1000; ++i ) {
      spibar.tick();
      if ( i == 499 )
        std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    }
  }
  {
    std::cout << "Insight into the configuration types." << std::endl;
    // In fact, the return value of the config() method is a reference
    // to a configuration object held inside the progress bar object
    // whose type can be found in pgbar::config
    // Each progress bar type has only one configuration type corresponding to it
    pgbar::config::CharBar pbar_cfg;   // config type of ProgressBar
    pgbar::config::BlckBar bpbar_cfg;  // config type of BlockProgressBar
    pgbar::config::SpinBar spibar_cfg; // config type of SpinnerBar
    pgbar::config::ScanBar scnbar_cfg; // config type of ScannerBar
    // All these configuration types are copyable, movable and swappble

    // You can pass a configuration object directly to the constructor of the progress bar object
    // to create a new progress bar based on existing configuration information
    pgbar::ProgressBar<> pbar { pbar_cfg };
    // Or reassign the internal configuration type using the config() method
    pbar.config() = pbar_cfg;

    // The constructor of the configuration object itself also supports variable argument lists
    // so you can also change the configuration object in this way
    pbar.config() = { pgbar::option::InfoColor( pgbar::color::Red ), pgbar::option::Tasks( 100 ) };
    // This is equivalent to pbar.config().swap( another_config )

    // These types are derived from pgbar::config::Core
    pgbar::config::Core& base_ref = pbar_cfg;
    // This base class also stores some configuration data related to the underlying settings
    // For example, the refresh rate of progress bar objects in the terminal interface is controlled
    // by the return value of pgbar::config::Core::refresh_interval()
    // and pgbar::config::Core allows you to modify the basic configuration data at run time
    pgbar::config::Core::refresh_interval(
      std::chrono::milliseconds( 20 ) ); // Increase the refresh rate from the default 25 Hz to 60 Hz
    // This method actually requires the progress bar object to sleep for 20 milliseconds after
    // each output to the console
    // Shorter refresh intervals mean smoother animations, but also a higher single-core CPU load
    // However, since the time of each rendering output cannot be 0
    // this method is essentially controlling the minimum interval of rendering output
    // can not be lower than the given value

    // You can also use pgbar::config::Core::intty() method
    // to learn the standard output stream of the current process is binding on the terminal
    // it need to combine the enumeration pgbar::Channel values to determine
    // which is you need to check the output stream
    // For example, check the standard output stream `stdout`
    if ( pgbar::config::Core::intty( pgbar::Channel::Stdout ) )
      std::cout << "Standard output is bound to a terminal." << std::endl;
    else
      std::cout << "Standard output is not bound to a terminal." << std::endl;
    // You can define a PGBAR_INTTY macro before the include file
    // to force the pgbar::config::Core::intty() method to always return true
  }
  {
    std::cout << "Insight into the progress bar types." << std::endl;
    // As mentioned earlier, all progress bar types are highly similar
    // with the only differences being the behavior when the tick() method is called
    // the arguments the constructor can accept
    // and the kinds of methods that can be chain-called when the config() method is called

    // In addition to is_running(), progress(), and iterate() mentioned earlier
    // the progress bar object has several useful methods
    // Take ProgressBar as an example
    pgbar::ProgressBar<> pbar { pgbar::option::Tasks( 100 ),
                                pgbar::option::Description( "Doing sth..." ),
                                pgbar::option::TrueMesg( "✔ Mission Complete!" ),
                                pgbar::option::TrueColor( pgbar::color::Green ),
                                pgbar::option::FalseMesg( "✖ Execution Failure!" ),
                                pgbar::option::FalseColor( pgbar::color::Red ) };
    pbar.tick();  // Make the progress bar advance one step
    pbar.reset(); // Reset the progress bar and immediately terminate rendering while displaying TrueMesg's
                  // contents
    pbar.tick_to( 50 );  // Make the progress bar progress to 50%
    pbar.tick( 3 );      // Make the progress bar advance three steps
    pbar.reset( false ); // Reset the progress bar and immediately terminate rendering while displaying
                         // FalseMesg's contents

    // Note: If the progress bar object is destructed, `TrueMesg` or `FalseMesg` are not rendered
    // even if they have been written to the configuration object
    {
      std::cout << "Be deconstructed while rendering." << std::endl;
      pgbar::SpinnerBar<> spibar { pgbar::option::Description( "???" ),
                                   pgbar::option::TrueMesg( "!!!" ),
                                   pgbar::option::FalseMesg( "///" ) };
      spibar.tick();
      std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    } // spibar is destructed here and rendering stops immediately
    // so we have to add a newline
    std::cout << std::endl;

    // In addition to configuring all the information for the progress bar before the task starts
    // you can also dynamically modify the information in the progress bar while it is running
    pbar.config().tasks( 5 );
    for ( auto i = 0; i < 5; ++i ) {
      pbar.tick();
      pbar.config().description( "Working process (" + std::to_string( i + 1 ) + "/5)" );
      std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
    }

    // But changing the number of tasks is **not effective**
    pbar.config().tasks( 100 );
    for ( auto i = 0; i < 100; ++i ) {
      pbar.tick();
      if ( i == 30 )
        pbar.config().tasks( 50 );
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }

    // If you wish, you can also "gradient" the color of the progress bar like this
    pbar.config().description( "Rainbow" ).tasks( 0xFFFFFF + 1 );
    for ( size_t i = 0; i < 0xFFFFFF + 1; ++i ) {
      pbar.tick();
      pbar.config().info_color( i );
    }
  }
  {
    std::cout << "Variable progress bar length." << std::endl;
    // In particular, the length of the bar indicator can be changed for ProgressBar,
    // BlockProgressBar and ScannerBar
    // Take ScannerBar as an example
    pgbar::ScannerBar<> scnbar;

    // The counting unit of config().bar_length() is "character"
    // That is, the value you pass in represents the number of characters that the bar indicator occupies in
    // the terminal
    scnbar.config().bar_length( 20 );
    scnbar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    scnbar.reset();

    // If you can get the horizontal width of the current terminal window
    // then you can also calculate a length that allows the progress bar to "fill" a row
    // You can get the length of the progress bar section except for the bar indicator
    // by calling config().fixed_size() and calculate it as follows
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
    constexpr auto terminal_width = 120; // Assuming terminal width is 120
#endif
    const auto total_length_excluding_bar = scnbar.config().fixed_size();
    scnbar.config().bar_length( terminal_width - total_length_excluding_bar );
    // config().bar_length() itself also returns the length of the bar indicator for the current progress bar

    scnbar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
    scnbar.reset();
  }
  {
    std::cout << "Variable animation rate." << std::endl;
    // For ProgressBar, SpinnerBar and ScannerBar
    // the motion rate of their component Lead part is variable

    pgbar::ScannerBar<> scnbar;
    scnbar.config().shift( 2 ); // Switch to 2x speed
    // Shift is between -128 and 127
    // If the value k is negative, it means that the rate is adjusted to 1/k of the normal case
    // otherwise it is k times the normal case.

    scnbar.tick();
    std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
    scnbar.reset();
    // In fact, the effect is equivalent to change pgbar::config::Core::refresh_interval()
    // However, the effect of Shift only applies to local progress bar objects
  }
  {
    std::cout << "Thread safety." << std::endl;
    // First, any type of method in pgbar::config is thread-safe
    // including replacing the configuration object itself with the config() method
    // This means that you can configure different parameters for another thread's progress bar object in
    // another thread

    // Second, the progress bar object itself is "optional" thread-safe,
    // and this optionality is reflected in the template parameters of the progress bar object
    // For all progress bar objects that use the default construction
    // they are equivalent to the following declaration statement:
    pgbar::ProgressBar<pgbar::Threadunsafe> unsafe_bar;
    // For thread-unsafe progress bar objects, calling any method other than config() is thread-unsafe

    // Using the pgbar::Threadsafe parameter, you can create a thread-safe progress bar object
    pgbar::ProgressBar<pgbar::Threadsafe> safe_bar1;

    // For the thread-safe version
    // you can call its tick() and reset() methods from as many threads as you want
    // However, it is still thread-unsafe for swap(), iterate(), and operator=()
    safe_bar1 = pgbar::ProgressBar<pgbar::Threadsafe>(); // Thread Unsafe!

    // pgbar::Threadsafe is just a lock class that satisfies the Basic Locakable requirement
    // You can use another lock type instead of this parameter type
    // For example std::mutex
    pgbar::ProgressBar<std::mutex> safe_bar2;

    // In contrast
    // the thread-safe version of the progress bar has an additional overhead than the less secure version
    // an overhead that cannot be resolved by introducing a more lightweight lock type
    constexpr auto iteration   = 65536;
    constexpr auto num_threads = 8;
    safe_bar2.config().tasks( iteration );

    std::vector<std::thread> threads;
    for ( size_t _ = 0; _ < num_threads; ++_ ) {
      threads.emplace_back( [&]() {
        for ( size_t _ = 0; _ < iteration / num_threads; ++_ ) {
          safe_bar2.tick();
        }
      } );
    }

    for ( auto& td : threads ) {
      if ( td.joinable() )
        td.join();
    }
    // Notice that a wait() method is called at the end of the previous code
    // This is because in a multithreaded environment
    // if the thread holding the progress bar object leaves the scope of the progress bar
    // the progress bar rendering will immediately stop because of the destructor
    // So the progress bar object provides wait() and wait_for() methods
    // to block the current thread until the progress bar is stopped
    safe_bar2.wait();
    // But the blocking effect only takes effect after the first tick() method is called
    // So in a multithreaded environment
    // the optimal solution is to wait for all child threads to finish before calling the wait() or wait_for()
    // method
  }
  {
    std::cout << "Switching output stream." << std::endl;
    // By default, the progress bar object outputs a string
    // to the current process's standard error stream stderr
    // The destination of the output stream can be changed by the template type parameter passed to
    // the progress bar when the progress bar object is created
    pgbar::ScannerBar<pgbar::Threadunsafe, pgbar::Channel::Stdout> scnbar;

    // The progress bar itself does not monopolize a standard output stream of the current process
    // at any point in time
    // so output information to the standard output stream bound to the progress bar during progress bar work
    // will cause the string rendered by the terminal to be distorted

    // If an output stream is not bound to the terminal
    // the progress bar does not write any data to the output stream file
    // but the exception check and task iteration count proceed normally
  }
}
