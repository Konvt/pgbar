**Contents**
- [`pgbar::ProgressBar`](#pgbarprogressbar)
  - [Construct a default object](#construct-a-default-object)
  - [Tweak the detailed configurations](#tweak-the-detailed-configurations)
  - [Customize the style of the progress bar](#customize-the-style-of-the-progress-bar)
  - [Tweak the configurations *when* constructing a object](#tweak-the-configurations-when-constructing-a-object)
  - [Reset the progress bar when you need](#reset-the-progress-bar-when-you-need)
  - [Use the method `foreach` to traverse a range](#use-the-method-foreach-to-traverse-a-range)
  - [Some exceptional situations](#some-exceptional-situations)
    - [The number of tasks is zero](#the-number-of-tasks-is-zero)
    - [Invalid iteration interval](#invalid-iteration-interval)
    - [Invalid RGB values or strings](#invalid-rgb-values-or-strings)
  - [Appoint a specific stream object](#appoint-a-specific-stream-object)
  - [Change the length of the progress bar](#change-the-length-of-the-progress-bar)
  - [Use a factory function](#use-a-factory-function)
  - [Determine the refresh rate of the progress bar](#determine-the-refresh-rate-of-the-progress-bar)
  - [The thread safety of the `pgbar::Config` and `pgbar::pgbar`](#the-thread-safety-of-the-pgbarconfig-and-pgbarpgbar)
  - [Run unit tests](#run-unit-tests)
- [`pgbar::SpinnerBar`](#pgbarspinnerbar)
  - [UI Logic](#ui-logic)
  - [Tweak the detailed configurations](#tweak-the-detailed-configurations-1)

# `pgbar::ProgressBar`
## Construct a default object
`pgbar::ProgressBar` is movable and swapable.

```cpp
pgbar::ProgressBar<> bar1;
pgbar::ProgressBar<> bar2;

bar1.swap( bar2 );          // ok
swap( bar1, bar2 );         // ok
// std::swap( bar1, bar2 ); // no!

auto bar3 = std::move( bar1 ); // ok

// auto bar4 = bar2; // no!
```

## Tweak the detailed configurations
You can use the streamlined interface `configure` with a chain of method calls to configure the specific parameters of a `pgbar::ProgressBar` object.

```cpp
pgbar::ProgressBar<> bar;

bar.configure().todo( "-" ).done( "=" ).status_color( pgbar::colors::Yellow );
```

The library provides several preset colors in `pgbar::colors`, you can disable it globally by defining a macro `PGBAR_COLORLESS` before including header.

```cpp
#define PGBAR_COLORLESS
#include "pgbar/pgbar.hpp"
// ...
```

Or you can change the value of the bar's configuration to disable colors locally.

```cpp
if ( bar.configure().colored() )
  bar.configure().colored( false );
```

Each update action of the progress bar is completed by the `tick` method.

```cpp
// Don't forget to set the task before updating.
bar.configure().tasks( 100 );
// The default tasks is 0.

for ( auto _ = 0; _ < 100; _ += 2 )
  bar.tick();
```

In addition to `tick`, there are two additional update methods.

```cpp
bar.tick( 20 ); // Step forward 20

bar.tick_to( 80 ); // Push completion to 80% immediately
```

The detailed composition of the progress bar can be found in [the next section](#customize-the-style-of-the-progress-bar).

> Available configuration parameters and their meanings are as follows:
> - `configure().styles()`:
>   (setter only) for switching different progress bar components;
> - `configure().colored()`:
>   (setter/getter) to enable or disable color effects on the progress bar;
> - `configure().bolded()`:
>   (setter/getter) to enables or disable bold text in the animation;
> - `configure().todo()`:
>   (setter only) to change the character representing the unfinished portion of the progress indicator;
> - `configure().done()`:
>   (setter only) to change the character representing the completed portion of the progress indicator;
> - `configure().startpoint()`:
>   (setter only) to change the left starting marker of the progress indicator;
> - `configure().endpoint()`:
>   (setter only) to change the right end marker of the progress indicator;
> - `configure().lstatus()`:
>   (setter only) to change the left starting marker of the status bar;
> - `configure().rstatus()`:
>   (setter only) to change the right end marker of the status bar;
> - `configure().divider()`:
>   (setter only) to change the divider between each component of the status bar;
> - `configure().bar_length()`:
>   (setter/getter) to get or change the length of the progress indicator;
> - `configure().todo_color()`:
>   (setter only) to change the color of the unfinished portion of the progress indicator, ineffective if `colored` returns `false`;
> - `configure().done_color()`:
>   (setter only) to change the color of the completed portion of the progress indicator, ineffective if `colored` returns `false`;
> - `configure().status_color()`:
>   (setter only) to change the overall color of the status bar, ineffective if `colored` returns `false`.

## Customize the style of the progress bar
```text
{startpoint}{done char}{todo char}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~~~~~~  Progress  ~~~~~~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~  Status  ~~~~~~~~~~~~~~~~~~~~~~~~~~~^

[-------                       ] [  24.00% |  48/200 |  31.00 Hz  | 00:00:01 < 00:00:04 ]
```

As shown above, the progress bar consists of several sections, which you can be toggled using bit switches from `pgbar::configs::Progress`.

```cpp
bar.configure()
   .styles( pgbar::configs::Progress::Bar | pgbar::configs::Progress::Ratio | pgbar::configs::Progress::Timer )
   .tasks( 100 );

for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

Moreover, the color setters support hex RGB value or strings.

```cpp
bar.configure()
   .todo_color( "#A52A2A" )
   .done_color( 0x0099FF )
   .status_color( "#B22" );
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

You can also output a custom string to the left of the progress bar.

> Ensure that your output strings do not push the progress bar left beyond the terminal width, as this can lead to disastrous display issues.
> \
> You can use `bar_length` and `fixed_size` in `configure` to obtain the total length of the current progress bar.

```cpp
std::cout << "Here is the progress bar: ";
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

## Tweak the configurations *when* constructing a object
You can create a `ProgressBar` by passing a list of wrapper objects defined in `pgbar::initr`.

```cpp
pgbar::ProgressBar<> bar { pgbar::initr::TodoChar( "-" ),
                           pgbar::initr::DoneChar( "=" ),
                           pgbar::initr::Styles( pgbar::configs::Progress::Rate | pgbar::configs::Progress::Timer ),
                           pgbar::initr::StatusColor( pgbar::colors::Yellow ),
                           pgbar::initr::Tasks( 100 ) };
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

This configuration method can be used not only in constructor, but also in method `configure().set()`.

```cpp
bar.configure().set( pgbar::initr::TodoChar( " " ),
                     pgbar::initr::StatusColor( pgbar::colors::Green ) );
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

This actually modifies the object `pgbar::Config` within the `ProgressBar` object, which allows you to modify the configuration in this manner if desired.

```cpp
auto cfg = pgbar::Config( pgbar::initr::TodoChar( "-" ),
                          pgbar::initr::DoneChar( "+" ),
                          pgbar::initr::Tasks( 114514 ) );
pgbar::ProgressBar<> bar2 { cfg }; // `Config` is copyable
bar.configure( std::move( cfg ) ); // also movable
```

## Reset the progress bar when you need
The method `reset()` aborts the current ticking process, and resets the progress bar to its initial state, discarding any current progress.

> The updated information refers to the cumulative number of steps, which can be accessed through the `progress` method of the `ProgressBar`.

This method is invoked automatically upon iteration completes, or can be triggered manually to abort early.

```cpp
pgbar::ProgressBar<> bar { 100, 2 };
for ( auto _ = 0; _ < 100; _ += 2 ) {
  bar.tick();
  if ( _ == 48 ) {
    bar.reset();
    // Now the iteration progress tracked by `bar` has been reset.
    std::cout << "Aborting!" << std::endl;
    break;
  }
}
```

After calling `reset`, calling `tick` again will render a new progress bar in the terminal.

## Use the method `foreach` to traverse a range
The `ProgressBar` provides a `foreach` method that **can only be called on lvalue objects**, serving as a lightweight alternative to `std::for_each`.

Unlike `std::for_each`, this method visualizes the iteration process as a progress bar displayed in the terminal while traversing a range.

This provides a convenient method for traversing a STL-like container or raw arrays, and it will automatically update the progress bar.

> STL containers are class types that provide iterator types, accessible via the `begin` and `end` methods.

There are two styles of traversal: one using a range-based for loop

```cpp
std::vector<int> vec = { 1, 2, 3, 4, 5 };
for ( auto _ : bar.foreach ( vec ) )
  // `foreach` will automatically determine the number of tasks.
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

// Or you can traverse the container using iterators.
for ( auto _ : bar.foreach ( vec.begin(), vec.end() ) )
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
```

Another style of traversing is using a lambda functor:

```cpp
int arr1[] = { 6, 7, 8, 9, 10 };
bar.foreach ( arr1, []( int _ ) {
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
} );
```

Not only that, but you can also traverse a range of values.

```cpp
std::vector<double> buffer;
// NOTE: The iteration interval is half-open,
// meaning it inclues the left endpoint and excludes the right endpoint.
bar.foreach ( 1.0, -1.0, -0.1, [&buffer]( double e ) { buffer.push_back( e ); } );

std::cout << "The elements is: ";
for ( auto e : buffer )
  std::cout << e << ' ';
std::cout << std::endl;
```

These behaviors rely on abstract range iterators defined in `pgbar::iterators`.

```cpp
// Numeric range
auto numbers = pgbar::iterators::NumericSpan<int>( 0, 20, 1 );
auto begin   = numbers.begin();

// Iterator range
// For pointer types, the start point can be greater than the endpoint (i.e., in reverse order),
// as long as the range they point to is left-closed and right-open.
int arr2[] = { 0, 1, 2 };
auto iters = pgbar::iterators::IterSpan<int*>( arr2 + 2, arr2 - 1 );
```

The `foreach()` uses a special range named `pgbar::iterators::ProxySpan`.

As its name suggests, this range takes another range and a reference to an existing `ProgressBar` object, traversing the range and updating the `ProgressBar` simultaneously.

```cpp
auto proxy =
  pgbar::iterators::ProxySpan<pgbar::iterators::NumericSpan<int>, pgbar::ProgressBar<>>(
    std::move( numbers ),
    bar );
```

In practice, we don't usually use this class directly.

## Some exceptional situations
### The number of tasks is zero
If the number of tasks is not specified or it is set to zero, the method `tick()` will throw an exception named `pgbar::exceptions::InvalidState`.

> All classes defined in `pgbar::exceptions` derive from `std::exception`.

```cpp
try {
  bar.tick();
} catch ( const pgbar::exceptions::InvalidState& e ) {
  const std::exception& base_e = e;
  std::cerr << "  Exception: \"" << base_e.what() << "\"" << std::endl;
}
```
### Invalid iteration interval
If you pass an invalid interval to `foreach()`, the object will throw an `pgbar::exceptions::InvalidArgument` exception immediately.

```cpp
try {
  // `start` is less than `end` while `step` is negative; vice versa.
  bar.foreach ( -100, 100, -1, []( double e ) {} );
} catch ( const pgbar::exceptions::InvalidArgument& e ) {
  std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
}
```
### Invalid RGB values or strings
If you give an invalid Hex RGB string, the object will throw an `pgbar::exceptions::InvalidArgument` exception immediately.

```cpp
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
```

## Appoint a specific stream object
The stream object must satisfy the `is_ostream` trait.

```cpp
using StreamType = std::ostream;
static_assert( pgbar::traits::is_ostream<StreamType>::value, "ERROR" );

pgbar::ProgressBar<StreamType> bar { std::clog };
```

The default stream object is bound to `std::cerr`.

If the program is not running in a terminal, the `Config::intty()` will return false, and the progress bar display nothing.

```cpp
std::cout << "  Current program is running in a tty? " << std::boolalpha
          << pgbar::Config::intty() << std::endl;
```

This feature is available only on Windows or Unix-like systems.

Furthermore, you can define a macro `PGBAR_INTTY` before including "pgbar.hpp", to force `pgbar` to assume it is working in a terminal.

## Change the length of the progress bar
Change the size of the progress bar via method `bar_length()`.

```cpp
std::cout << "  Default bar length: " << bar.configure().bar_length() << std::endl;
bar.configure().bar_length( 50 ); // Unit: characters
```

If you can get the terminal width, you can calculate the length to make the bar fill a line.

Normally you need to use the OS api to get the terminal width.

```cpp
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
  else
    return w.ws_col == 0 ? 120 : w.ws_col;
}();
#endif
```

You can use `configure().fixed_size()` to get the length excluding the progress indicator, then subtract that length from the terminal width.

```cpp
const auto total_length_excluding_bar = bar.configure().fixed_size();
bar.configure().bar_length( terminal_width - total_length_excluding_bar );
```

The progress bar perfectly fills one line now.

## Use a factory function
There's a simple factory function named `pgbar::make_progress`, it's just as simple as constructing a `pgbar::Config`.

```cpp
auto bar =
  pgbar::make_progress( std::clog,
                        pgbar::initr::Tasks( 1919810 ),
                        pgbar::initr::Styles( pgbar::configs::Progress::Entire & pgbar::configs::Progress::Bar ) );
```

The default stream object will be used if none is provided.

## Determine the refresh rate of the progress bar
The refresh interval is a global static object defined in `pgbar::Config`, which affects the refresh rate of all progress bars output to the terminal.

```cpp
std::cout << "  Default refresh interval: " << pgbar::Config::refresh_interval().count()
          << " ns" << std::endl;
// You can change its value by invoking a function with the same name.
pgbar::Config::refresh_interval( std::chrono::nanoseconds( 20000 ) );
std::cout << "  The new refresh interval: " << pgbar::Config::refresh_interval().count()
          << " ns" << std::endl;
```

After each progress bar is output to the terminal, it waits for the refresh interval (in `nanoseconds`) before outputting again.

Since the time consumed during each output is not always zero, the maximum refresh rate of the progress bar depends on the time required to assemble and output the progress bar.

The refresh interval merely provides the capability to extend the refresh time, or in other words, "lower the refresh rate."

## The thread safety of the `pgbar::Config` and `pgbar::pgbar`
Invoking member and static functions other than the ctor of `pgbar::Config` is thread-safe, they are protected by a read-write lock.

However, when the `pgbar::ProgressBar` is running (Use the `is_running` method for the check), invoking any non-const member function of its `pgbar::Config` is thread-unsafe, and the behavior in such cases is unspecified.

The `pgbar::ProgressBar` itself can be either thread-safe or thread-unsafe, it depends on its second template parameter.

> The second template parameter can be any type that satisfies `basic lockable` requirement.

```cpp
pgbar::ProgressBar<std::ostream, pgbar::Threadsafe> thread_safe_bar;
pgbar::ProgressBar<std::ostream, std::mutex> stl_base_bar;               // fine
pgbar::ProgressBar<std::ostream, pgbar::Threadunsafe> thread_unsafe_bar; // default
```

In this context, "thread-safe" means allowing multiple threads to invoke `tick` concurrently, but does not include moving assignment or swapping.

And as you can probably imagine, the thread-safe version doesn't perform as well.

```cpp
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
```

## Run unit tests
The project uses [Catch2](https://github.com/catchorg/Catch2) for basic unit tests and tests via the [xmake](https://github.com/xmake-io/xmake/) build system.

> Note:
> \
> When compiling test cases with different C++ standards, you need to include the appropriate version of `Catch2`;
>  For C++11, place the [Catch2 v2.x](https://github.com/catchorg/Catch2/tree/v2.x) headers directly in the [tests/](tests/) directory;
> For C++14 and later, use a package manager (or `xmake`) to bring in [Catch2 v3.x](https://github.com/catchorg/Catch2/tree/devel), or manually merge the headers and place them in the [tests/](tests/) directory.
> \
> The xmake version must be higher than 2.8.5.

To run all test cases, use the following command:

```sh
xmake test
# Add the -vD flag for verbose output
```

To run a specific test case, use:

```sh
xmake test testcase_filename/*
```

Since `xmake` tests are not bound to the terminal, some test cases may be invalid in this environment.

> Some "test cases" are performance tests and are not part of the test suite.

You can switch to the `tests/` directory and manually compile test cases with the `Makefile` as another way to run individual tests.

```sh
cd tests/ && make file=test_case_name.cpp && ./test
```
- - -

# `pgbar::SpinnerBar`
The primary usage of `pgbar::SpinnerBar` aligns with `pgbar::ProgressBar`, here we only introduce the key differences.
## UI Logic
`pgbar::SpinnerBar` is a non-progressive animation component, acting as a terminal-based animation unrelated to iteration.

Calling the `tick` method *once* initiates the animation in the terminal.

> After the first call to `tick`, subsequent calls (without a `reset` invocation) have no effect and are harmless.

To stop `pgbar::SpinnerBar`, you can allow the control flow to exit the current scope, automatically halting the animation as the object destructs.

Alternatively, you can explicitly call `reset`, which not only stops the animation but also displays an additional final frame in the terminal, set by the `true_frame()` or `false_frame()` method (indicating task success or failure, respectively).

This final frame is known as the "True" or "False" frame, determined by a `bool` parameter passed to `reset`, defaulting to the "True" frame.

## Tweak the detailed configurations
You can use the streamlined interface `configure` with a chain of method calls to configure the specific parameters of it.

Also, various parameters can be adjusted by passing parameters via wrappers in `pgbar::options`.

```cpp
pgbar::SpinnerBar<> bar;

bar.configure().suffix( "Waiting main thread..." );
```

> Available configuration parameters and their meanings are as follows:
> - `configure().colored()`:
>   (setter/getter) to enable or disable color effects on the progress bar;
> - `configure().bolded()`:
>   (setter/getter) to enables or disable bold text in the animation;
> - `configure().frames()`:
>   (setter only) to change the animation frames;
> - `configure().suffix()`:
>   (setter only) to set the suffix text displayed next to the animation, defaulting to empty;
> - `configure().true_frame()`:
>   (setter only) to set the final frame displayed on `reset()` or `reset( true )`, defaulting to empty;
> - `configure().false_frame()`:
>   (setter only) to set the final frame displayed on `reset( false )`, defaulting to empty;
>   (setter only) to change animation color, including the `suffix`;
> - `configure().true_color()`:
>   (setter only) to change the color of the True frame;
> - `configure().false_color()`:
>   (setter only) to change the color of the False frame.
