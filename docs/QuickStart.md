**Contents**
- [Basic user guide.](#basic-user-guide)
  - [Construct, swap and base class reference](#construct-swap-and-base-class-reference)
  - [Let it work](#let-it-work)
  - [Helper function](#helper-function)
- [Customize your own progress bar object](#customize-your-own-progress-bar-object)
  - [By constructor](#by-constructor)
  - [By member method](#by-member-method)
- [Insight into the configuration types](#insight-into-the-configuration-types)
  - [Basic configuration types](#basic-configuration-types)
  - [The base class of configuration types](#the-base-class-of-configuration-types)
- [Insight into the progress bar types](#insight-into-the-progress-bar-types)
- [Variable progress bar length](#variable-progress-bar-length)
- [Variable animation rate](#variable-animation-rate)
- [Thread safety](#thread-safety)
  - [Cross-thread call](#cross-thread-call)
  - [Lock type](#lock-type)
- [Switching output stream](#switching-output-stream)
- [Design principle](#design-principle)
  - [Basic architecture](#basic-architecture)
  - [About exception passing](#about-exception-passing)
  - [Implementation principle of progress bar type and configuration](#implementation-principle-of-progress-bar-type-and-configuration)

# Basic user guide.
## Construct, swap and base class reference
All progress bar objects can be constructed by default without parameters.

```cpp
pgbar::ProgressBar<> pbar;
pgbar::BlockProgressBar<> bpbar;
pgbar::SpinnerBar<> spibar;
pgbar::ScannerBar<> scnbar;
```

And these objects are movable, swappble but not copyable.

```cpp
pgbar::ProgressBar<> pbar;
pgbar::ProgressBar<> pbar2;

pbar = std::move( pbar2 ); // movable
swap( pgbar, pbar2 );      // swapable
// or
pbar.swap( pbar2 );
```

The classes are derived from `pgbar::Indicator`.

```cpp
pgbar::ProgressBar<> pbar;
pgbar::Indicator& base_ref = pbar;
```

## Let it work
The `ProgressBar` and `BlockProgressBar` constructed by default cannot be used directly, it must be used only after passing a number of tasks to the method `config().tasks()`. Otherwise it will throw an exception `pgbar::exception::InvalidState`.

> All exceptions in the library are derived from `std::exception`, and they all have a common base class, `pgbar::exception::Error`.

```cpp
pgbar::ProgressBar<> pbar;

try {
  pbar.tick();
} catch ( const pgbar::exception::InvalidState& e ) {
  std::cerr << "Oops! An exception occurs here: \"" << e.what() << "\"" << std::endl;
}
```

During iteration, you can use the `is_running()` and `progress()` methods to check whether the current progress bar is running and to get the number of times that `tick()` has been called so far.

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().tasks( 100 );
for ( auto i = 0; i < 100; ++i ) {
  pbar.tick();
  if ( i == 99 ) // The last call of `tick()` causes the progress bar to stop automatically
    assert( !pbar.is_running() ); // So the assert is not going to be true at that moment
  else
    assert( pbar.is_running() );
  assert( pbar.progress() != 0 );
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

As soon as' `tick()` is called as many times as the predetermined value of `config().tasks()`, the progress bar will stop running on its own. However, you can also call the `reset()` method of the progress bar in advance to actively stop the progress bar; If the progress bar object is destructed, it is equivalent to calling `reset()`.

Unlike the two progress bar types mentioned above, `SpinnerBar` and `ScannerBar` do not require the number of tasks to be specified and can be used directly.

```cpp
pgbar::SpinnerBar<> spibar;

spibar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
spibar.reset();
```

However, because there is no limit on the number of tasks, the work of the progress bar will not stop by itself, and the `reset()` method must be actively called at this time to stop rendering

## Helper function
Instead of manually specifying the number of tasks, you can use the `iterate()` method to make the progress bar work on an "abstract range", where the progress bar object will count the number of tasks itself.

`iterate()` is used in a similar way to the `range()` function in Python.

```cpp
pgbar::BlockProgressBar<> bpbar;

std::cout << "Iterate over a range of values with BlockProgressBar: ";
for ( auto&& _ : bpbar.iterate( 50 ) ) {
  // Iterate over the numeric interval [0, 50) with step size 1
  std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );
}
```

The `iterate()` method can also work on a data container, such as an array.

```cpp
pgbar::BlockProgressBar<> bpbar;

std::cout << "Iterate over a raw array with BlockProgressBar: ";
int arr[100] { 0 };
for ( auto&& e : bpbar.iterate( arr ) ) {
  e = 114514; // access the elements directly within a loop
}
const auto baseline = std::vector<int>( 100, 114514 );
std::cout << "Are the values in these two ranges equal? " << std::boolalpha
          << std::equal( arr, arr + 100, baseline.cbegin() ) << std::endl;
```

You can also pass two iterators to `iterate()` for the iterated range; If it is an iterator of pointer type, it can be accessed in reverse order by inverting the starting and ending points.

```cpp
pgbar::SpinnerBar<> spibar;

std::cout << "Reverse iterate over a raw array with SpinnerBar: ";
for ( auto&& _ : spibar.iterate( arr + 100, arr ) ) {
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

The data container here also contains the generic container in the STL, such as `std::vector`.

```cpp
pgbar::ScannerBar<> scnbar;
// The baseline here appears above
std::cout << "Iterate over an object of std::vector with ScannerBar: ";
for ( auto&& _ : scnbar.iterate( baseline ) ) {
  // do something here...
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

`iterate()` can also accept a unary function and act on the element in the range, as can `std::for_each`.

```cpp
pgbar::ScannerBar<> scnbar;

std::cout << "Iterate over an object of std::vector with an unary function: ";
std::vector<char> alphabet( 26 );
std::iota( alphabet.begin(), alphabet.end(), 'a' );
scnbar.iterate( alphabet, []( char ) { /*...*/ } );
```

# Customize your own progress bar object
## By constructor
All types of progress bars can be constructed as modified objects from the default configuration by passing in no less than one `pgbar::option` wrapper type.

```cpp
pgbar::ProgressBar<> pbar { pgbar::option::Style( // Select a different progress bar information section
                              pgbar::config::CharBar::Per | pgbar::config::CharBar::Elpsd ),
                            pgbar::option::InfoColor( pgbar::color::Red ), // Change the color of the information section
                            pgbar::option::Tasks( 100 ) };
for ( auto _ = 0; _ < 100; ++_ ) {
  pbar.tick();
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

It is not allowed to pass a duplicate `pgbar::option` type, otherwise it will cause compilation error.

```cpp
pgbar::SpinnerBar<> spibar { pgbar::option::Lead( { "◜", "◝", "◞", "◟" } ),
                             pgbar::option::Description( "Loading..." ),
                             pgbar::option::DescColor( "#FFF" ),
                             pgbar::option::DescColor( "#39C5BB" ) };
// Compilation Error!
```

## By member method
In addition to the fact that the constructor can use multiple variable `pgbar::option` wrappers, the member method `config().set()` can also accept arguments of this form.

```cpp
pgbar::SpinnerBar<> spibar;

spibar.config().set( pgbar::option::InfoColor( "#FFA500" ),
                     pgbar::option::Style( pgbar::config::CharBar::Sped | pgbar::config::CharBar::Per
                                            | pgbar::config::CharBar::Elpsd
                                            | pgbar::config::CharBar::Cntdwn ),
                     pgbar::option::SpeedUnit( { "B/s", "kB/s", "MB/s", "GB/s" } ),
                     pgbar::option::Tasks( 70 ) );
for ( auto i = 0; i < 70; ++i ) {
  spibar.tick();
  std::this_thread::sleep_for( std::chrono::milliseconds( 70 - i ) );
}
```

All types/methods used to configure colors can receive valid RGB color codes as strings, or RGB color values given directly as hexadecimal integers.

And color effects can be forcibly turned off by defining a `PGBAR_COLORLESS` macro.

In addition, `config()` itself provides a streaming interface style, which can also configure different parts of the progress bar style one by one.

```cpp
pgbar::SpinnerBar<> spibar;

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
```

# Insight into the configuration types
## Basic configuration types
In fact, the return value of the `config()` method is a reference to a configuration object held inside the progress bar object, whose type can be found in `pgbar::config`; Each progress bar type has only one configuration type corresponding to it.

```cpp
pgbar::config::CharBar pbar_cfg;   // config type of ProgressBar
pgbar::config::BlckBar bpbar_cfg;  // config type of BlockProgressBar
pgbar::config::SpinBar spibar_cfg; // config type of SpinnerBar
pgbar::config::ScanBar scnbar_cfg; // config type of ScannerBar
```

All these configuration types are copyable, movable and swappble.

```cpp
pgbar::config::CharBar pbar_cfg;
pgbar::config::CharBar pbar_cfg2;

pbar_cfg = pbar_cfg2;              // copyable
pbar_cfg = std::move( pbar_cfg2 ); // movable
swap( pbar_cfg, pbar_cfg2 );       // swapable
// or
pbar_cfg.swap( pbar_cfg2 );
```

You can pass a configuration object directly to the constructor of the progress bar object to create a new progress bar based on existing configuration information.

```cpp
pgbar::ProgressBar<> pbar { pbar_cfg };
// Or reassign the internal configuration type using the config() method
pbar.config() = pbar_cfg;

// The constructor of the configuration object itself also supports variable argument lists
// so you can also change the configuration object in this way
pbar.config() = { pgbar::option::InfoColor( pgbar::color::Red ), pgbar::option::Tasks( 100 ) };
// This is equivalent to pbar.config().swap( another_config )
```

## The base class of configuration types
These types are derived from `pgbar::config::Core`.

```cpp
pgbar::config::CharBar pbar_cfg;
pgbar::config::Core& base_ref = pbar_cfg;
```

This base class also stores some configuration data related to the underlying settings.

For example, the refresh rate of progress bar objects in the terminal interface is controlled by the return value of `pgbar::config::Core::refresh_interval()`, and `pgbar::config::Core` allows you to modify the basic configuration data at run time.

```cpp
// Increase the refresh rate from the default 25 Hz to 60 Hz
pgbar::config::Core::refresh_interval( std::chrono::milliseconds( 20 ) );
```

This method actually requires the progress bar object to sleep for 20 milliseconds after each output to the console; Shorter refresh intervals mean smoother animations, but also a higher single-core CPU load.

However, since the time of each rendering output cannot be 0, this method is essentially controlling the minimum interval of rendering output can not be lower than the given value.

You can also use `pgbar::config::Core::intty()` method to learn the standard output stream of the current process is binding on the terminal, it need to combine the enumeration `pgbar::Channel` values to determine which is you need to check the output stream; For example, check the standard output stream `stdout`:

```cpp
if ( pgbar::config::Core::intty( pgbar::Channel::Stdout ) )
  std::cout << "Standard output is bound to a terminal." << std::endl;
else
  std::cout << "Standard output is not bound to a terminal." << std::endl;
```

You can define a `PGBAR_INTTY` macro before the include file to force the `pgbar::config::Core::intty()` method to always return `true`.

# Insight into the progress bar types
As mentioned earlier, all progress bar types are highly similar, with the only differences being the behavior when the `tick()` method is called, the arguments the constructor can accept, and the kinds of methods that can be chain-called when the `config()` method is called.

In addition to `is_running()`, `progress()`, and `iterate()` mentioned earlier, the progress bar object has several useful methods; Take `ProgressBar` as an example.

```cpp
pgbar::ProgressBar<> pbar { pgbar::option::Tasks( 100 ),
                            pgbar::option::Description( "Doing sth..." ),
                            pgbar::option::TrueMesg( "✔ Mission Complete!" ),
                            pgbar::option::TrueColor( pgbar::color::Green ),
                            pgbar::option::FalseMesg( "✖ Execution Failure!" ),
                            pgbar::option::FalseColor( pgbar::color::Red ) };
pbar.tick();         // Make the progress bar advance one step
pbar.reset();        // Reset the progress bar and immediately terminate rendering while displaying TrueMesg's contents
pbar.tick_to( 50 );  // Make the progress bar progress to 50%
pbar.tick( 3 );      // Make the progress bar advance three steps
pbar.reset( false ); // Reset the progress bar and immediately terminate rendering while displaying FalseMesg's contents
```

> `TrueMesg` and `FalseMesg` are part of the `Description` element and appear only before the progress bar is about to stop working and are used to replace the predefined description with a custom one.
> \
> Which information is displayed depends on the `bool` parameter passed when the `reset()` method is called; `TrueMesg` is selected by default.

Note: If the progress bar object is destructed, `TrueMesg` or `FalseMesg` are not rendered, even if they have been written to the configuration object.

> In the destructor scenario, stopping work immediately and releasing all resources is considered a top priority.

```cpp
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
```

In addition to configuring all the information for the progress bar before the task starts, you can also dynamically modify the information in the progress bar while it is running.

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().tasks( 5 );
for ( auto i = 0; i < 5; ++i ) {
  pbar.tick();
  pbar.config().description( "Working process (" + std::to_string( i + 1 ) + "/5)" );
  std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
}
```

But changing the number of tasks is **not effective**.

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().tasks( 100 );
for ( auto i = 0; i < 100; ++i ) {
  pbar.tick();
  if ( i == 30 )
    pbar.config().tasks( 50 );
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

If you wish, you can also "gradient" the color of the progress bar like this.

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().description( "Rainbow" ).tasks( 0xFFFFFF + 1 );
for ( size_t i = 0; i < 0xFFFFFF + 1; ++i ) {
  pbar.tick();
  pbar.config().info_color( i );
  std::this_thread::sleep_for( std::chrono::microseconds( 30 ) );
}
```

# Variable progress bar length
In particular, the length of the bar indicator can be changed for `ProgressBar`, `BlockProgressBar` and `ScannerBar`; Take `ScannerBar` as an example.

```cpp
pgbar::ScannerBar<> scnbar;

// The counting unit of config().bar_length() is "character"
// That is, the value you pass in represents the number of characters that the bar indicator occupies in the terminal
scnbar.config().bar_length( 20 );
scnbar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
scnbar.reset();
```

If you can get the horizontal width of the current terminal window, then you can also calculate a length that allows the progress bar to "fill" a row.

You can get the length of the progress bar section except for the bar indicator by calling `config().fixed_size()` and calculate it as follows.

```cpp
#ifdef _WIN32 // Windows
const auto terminal_width = []() {
  HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if ( GetConsoleScreenBufferInfo( hConsole, &csbi ) ) {
    const auto width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return width == 0 ? 120 : width;
  } else
    return 120;
}();
#elif defined( __unix__ ) // Linux
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
```

# Variable animation rate
For `ProgressBar`, `SpinnerBar` and `ScannerBar`, the motion rate of their component `Lead` part is variable.

This variable rate can be adjusted by the type wrapper `pgbar::option::Shift` and the method `config().shift()`.

```cpp
pgbar::ScannerBar<> scnbar;
scnbar.config().shift( 2 ); // Switch to 2x speed
scnbar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
scnbar.reset();
```

`Shift` is between -128 and 127; If the value k is negative, it means that the rate is adjusted to 1/k of the normal case, otherwise it is k times the normal case.

In fact, the effect is equivalent to change `pgbar::config::Core::refresh_interval()`; However, the effect of `Shift` only applies to local progress bar objects.

# Thread safety
## Cross-thread call
First, any type of method in `pgbar::config` is thread-safe, including replacing the configuration object itself with the `config()` method; This means that you can configure different parameters for another thread's progress bar object in another thread.

Second, the progress bar object itself is "optional" thread-safe, and this optionality is reflected in the template parameters of the progress bar object. For all progress bar objects that use the default construction, they are equivalent to the following declaration statement:

```cpp
pgbar::ProgressBar<pgbar::Threadunsafe> unsafe_bar;
```

For thread-unsafe progress bar objects, calling any method other than `config()` is thread-unsafe.

Using the `pgbar::Threadsafe` parameter, you can create a thread-safe progress bar object.

```cpp
pgbar::ProgressBar<pgbar::Threadsafe> safe_bar;
```

For the thread-safe version, you can call its `tick()` and `reset()` methods from as many threads as you want; However, it is still thread-unsafe for `swap()`, `iterate()`, and `operator=()`.

```cpp
safe_bar = pgbar::ProgressBar<pgbar::Threadsafe>(); // Thread Unsafe!
```

## Lock type
`pgbar::Threadsafe` is just a lock class that satisfies the Basic Locakable requirement. You can use another lock type instead of this parameter type. For example `std::mutex`.

> In contrast, the thread-safe version of the progress bar has an additional overhead than the less secure version, an overhead that cannot be resolved by introducing a more lightweight lock type.

```cpp
pgbar::ProgressBar<std::mutex> safe_bar;

constexpr auto iteration   = 2147483648;
constexpr auto num_threads = 4;
safe_bar.config().tasks( iteration );

std::vector<std::thread> threads;
for ( size_t _ = 0; _ < num_threads; ++_ ) {
  threads.emplace_back( [&]() {
    for ( size_t _ = 0; _ < iteration / num_threads; ++_ ) {
      safe_bar.tick();
    }
  } );
}

for ( auto& td : threads ) {
  if ( td.joinable() )
    td.join();
}

safe_bar.wait();
```

Notice that a `wait()` method is called at the end of the previous code; This is because in a multithreaded environment, if the thread holding the progress bar object leaves the scope of the progress bar, the progress bar rendering will immediately stop because of the destructor.

So the progress bar object provides `wait()` and `wait_for()` methods to block the current thread until the progress bar is stopped.

But the blocking effect only takes effect after the first `tick()` method is called; So in a multithreaded environment, the optimal solution is to wait for all child threads to finish before calling the `wait()` or `wait_for()` method.

# Switching output stream
By default, the progress bar object outputs a string to the current process's standard error stream `stderr`; The destination of the output stream can be changed by the template type parameter passed to the progress bar when the progress bar object is created.

For example, create a progress bar object that outputs to `stdout`:

```cpp
pgbar::ScannerBar<pgbar::Threadunsafe, pgbar::Channel::Stdout> scnbar;
```

The progress bar itself does not monopolize a standard output stream of the current process at any point in time, so output information to the standard output stream bound to the progress bar during progress bar work will cause the string rendered by the terminal to be distorted.

If an output stream is not bound to the terminal, the progress bar does not write any data to the output stream file, but the exception check and task iteration count proceed normally.

> To check whether an output stream is bound to a terminal, you can use the `pgbar::config::Core::intty()` method mentioned earlier.

# Design principle
## Basic architecture
The progress bar consists of two main parts: the notification thread and the rendering thread. The notification thread is the thread responsible for calling the `tick()` method each time, while the render thread is a thread object managed by the thread manager `pgbar::__details::render::Renderer`.

Each notification thread's first call to `tick()` wakes up the rendering thread; Since thread managers are designed as lazy initialization, the first call of each progress bar object at the global scope also attempts to initialize the thread manager, i.e., create a child thread.

Each time the `reset()` method is called, the progress bar object suspends the rendering thread through the thread manager's member methods.

In order to ensure that the working state of the rendering thread is always valid, only the first `tick()` and the last `tick()`, or when the `reset()` is called at run, the notification thread will use a spin lock to wait for the rendering thread to transfer to the specified state; That is to say, only in the above three time points, the notification thread will block for an indefinite length.

## About exception passing
Since the rendering thread needs to repeatedly concatenate strings and write data to the standard output stream, it is possible to throw exceptions throughout the process.

If in Windows platform, the program can't get to the current process flow standard output Handle, the render thread throws a local system error exception `pgbar::exception::SystemError`; In a more general case, if the current machine is running low on resources (e.g., out of memory), a library exception such as `std::bad_alloc` will be thrown by the standard library at any memory request point.

The rest of the case, if the rendering thread received a thrown exception, it will be the exception is stored in the internal `pgbar::__details::concurrent::ExceptionBox` container, and stop the current render job; Wait until the next time the notification thread calls `activate()` or `suspend()` the caught exception is re-thrown in the notification thread.

> `activate()` and `suspend()` are called only on the first and last `tick()` and `reset()` methods of the progress bar.

If the rendering thread already has an unhandled exception in its exception container, and another exception is thrown inside the thread, the rendering thread will enter a `dead` state. In this state, new exceptions are not caught, but are allowed to propagate until the rendering thread terminates.

In the `dead` state, recalling the thread manager's `activate()` method (i.e. reactivating the progress bar object) will attempt to pull up a new rendering thread; During this process, the last unhandled exception will be thrown before a new rendering thread is created and can start working.

## Implementation principle of progress bar type and configuration
As mentioned earlier, the functionality of the different types of progress bars is highly similar, except that their semantic expression and rendering styles differ at runtime.

In fact, all progress bar types are aliases of the template class `pgbar::BasicBar`; Similarly, all configuration type is `pgbar::config::BasicConfig` alias.

By [this article](https://zhuanlan.zhihu.com/p/106672814), the progress bar follow a Mixin pattern combination inherited from `pgbar::__details::assets` the different template base class; All template classes in `pgbar::__details::assets` are designed in CRTP mode, so a whole bunch of methods for configuring data can be called through the `config()` method chain for final use.

The Mixin pattern here owes much to a compile-time topological sorting algorithm called `pgbar::__details::traits::TopoSort`.

This topological sorting algorithm is similar to the C3 linearization algorithm in Python, but unlike it: The purpose of C3 linearization algorithm is to find the most suitable class method in the class inheritance structure, while the topological sorting algorithm here linearizes the entire inheritance structure directly, linearizing a complex multi-inheritance structure into an inheritance chain at compile time.

But they have one thing in common: the method resolution at the most derived class satisfies the local priority principle of the inheritance order of the base class; In other words, during the inheritance process, the base class located to the left of the inheritance list is preferentially placed closer to the derived class to ensure that the methods of this base class are not overwritten by the base class further to the right.

In addition, the topological sorting algorithm has another feature: for the two classes with non-virtual inheritance relationship, they will be placed as close to each other as possible after sorting; This is to ensure that in non-virtual inheritance, derived classes can refer directly to base classes without complex template nesting.

The specific working principle can be referred to the article mentioned above. Based on the principle proposed in the article, I have implemented a topological sorting algorithm that can analyze the relationship between virtual inheritance and non-virtual inheritance at the same time and meet some characteristics of C3 linearization algorithm.
