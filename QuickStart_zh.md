**Contents**
- [`pgbar::ProgressBar`](#pgbarprogressbar)
  - [构造默认对象](#构造默认对象)
  - [调整详细配置](#调整详细配置)
  - [自定义进度条的样式](#自定义进度条的样式)
  - [构造对象时传入配置参数](#构造对象时传入配置参数)
  - [在*需要的时候*重置进度条](#在需要的时候重置进度条)
  - [使用 `foreach` 方法遍历一个范围](#使用-foreach-方法遍历一个范围)
  - [异常场景](#异常场景)
    - [任务数为零](#任务数为零)
    - [错误的迭代范围](#错误的迭代范围)
    - [错误的 RGB 数值](#错误的-rgb-数值)
  - [指派特定的流对象](#指派特定的流对象)
  - [更改进度条的长度](#更改进度条的长度)
  - [工厂函数](#工厂函数)
  - [更改全局刷新率](#更改全局刷新率)
  - [线程安全性](#线程安全性)
  - [运行单元测试](#运行单元测试)
- [`pgbar::SpinnerBar`](#pgbarspinnerbar)
  - [UI 逻辑](#ui-逻辑)
  - [调整详细配置](#调整详细配置-1)

# `pgbar::ProgressBar`
## 构造默认对象
`pgbar::ProgressBar` 是一个可默认构造、可交换、可移动，但不可复制的对象类型。

```cpp
pgbar::ProgressBar<> bar1;
pgbar::ProgressBar<> bar2;

bar1.swap( bar2 );          // ok
swap( bar1, bar2 );         // ok
// std::swap( bar1, bar2 ); // no!

auto bar3 = std::move( bar1 ); // ok

// auto bar4 = bar2; // no!
```

## 调整详细配置
可以使用流式接口 `configure` 的链式调用来配置 `pgbar::ProgressBar` 对象的具体参数。

```cpp
pgbar::ProgressBar<> bar;

bar.configure().todo( "-" ).done( "=" ).status_color( pgbar::colors::Yellow );
```

代码在 `pgbar::colors` 中提供了多个预设的颜色值，

你可以通过在引入头文件前，定义一个 `PGBAR_COLORLESS` 宏，以关闭所有颜色效果。

```cpp
#define PGBAR_COLORLESS
#include "pgbar/pgbar.hpp"
// ...
```

或者你也可以更改某个进度条对象的配置选项，局部性地关闭颜色效果。

```cpp
if ( bar.configure().colored() )
  bar.configure().colored( false );
```

进度条的每个更新动作都由 `tick` 方法完成。

```cpp
// 别忘了设置任务的数量。
bar.configure().tasks( 100 );
// 它的默认值为 0。

for ( auto _ = 0; _ < 100; _ += 2 )
  bar.tick();
```

除了 `tick`，还有以下两个额外的更新方法。

```cpp
bar.tick( 20 ); // 向前步进 20

bar.tick_to( 80 ); // 将完成度立即推进到 80%
```

进度条的详细构成可以见[下一节](#自定义进度条的样式)。

> 所有可用的配置参数及其含义如下：
> - `configure().styles()`：
>  （仅 setter）切换不同的进度条组件；
> - `configure().colored()`：
>  （setter/getter）用于设置是否开启着色效果；
> - `configure().bolded()`：
>  （setter/getter）用于设置是否加粗状态栏的字体；
> - `configure().todo()`：
>  （仅 setter）更改进度指示器中标识未完成部分的填充字符；
> - `configure().done()`：
>  （仅 setter）更改进度指示器中标识已完成部分的填充字符；
> - `configure().startpoint()`：
>  （仅 setter）更改进度指示器的左侧起点标识；
> - `configure().endpoint()`：
>  （仅 setter）更改进度指示器的右侧终点标识；
> - `configure().lstatus()`：
>  （仅 setter）更改状态栏的左侧起点标识；
> - `configure().rstatus()`：
>  （仅 setter）更改状态栏的左侧终点标识；
> - `configure().divider()`：
>  （仅 setter）更改分隔每个状态栏组件的分隔栏；
> - `configure().bar_length()`：
>  （setter/getter）获取或更改进度指示器的长度；
> - `configure().todo_color()`：
>  （仅 setter）更改进度指示器中未完成部分的颜色，在 `colored` 返回 `false` 时无效果；
> - `configure().done_color()`：
>  （仅 setter）更改进度指示器中已完成部分的颜色，在 `colored` 返回 `false` 时无效果；
> - `configure().status_color()`：
>  （仅 setter）更改状态栏的整体颜色，在 `colored` 返回 `false` 时无效果。

## 自定义进度条的样式
```text
{startpoint}{done char}{todo char}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~~~~~~ Indicator  ~~~~~~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~  Status  ~~~~~~~~~~~~~~~~~~~~~~~~~~~^

[-------                       ] [  24.00% |  48/200 |  31.00 Hz  | 00:00:01 < 00:00:04 ]
```

如上所示，进度条是由多个部分组成的。你可以使用 `pgbar::configs::Progress` 中的位开关随意组合它们。

```cpp
bar.configure()
   .styles( pgbar::configs::Progress::Bar | pgbar::configs::Progress::Ratio | pgbar::configs::Progress::Timer )
   .tasks( 100 );

for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

此外，更改颜色的方法支持读取一个十六进制 RGB 数值，或者是 RGB 字符串。

```cpp
bar.configure()
   .todo_color( "#A52A2A" )
   .done_color( 0x0099FF )
   .status_color( "#B22" );
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

最后，你可以在进度条的左侧随便输出字符串。

> 请确保你输出的字符串不会将进度条向左推动至超过终端宽度；一旦超过会导致灾难性的显示效果。
> \
> 你可以使用 `configure` 中的 `bar_length` 和 `fixed_size` 获取当前进度条的总长度。

```cpp
std::cout << "Here is the progress bar: ";
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

## 构造对象时传入配置参数
你可以在构造对象时，传入一个由 `pgbar::options` 中定义的包装器类型所构成的参数列表。

```cpp
pgbar::ProgressBar<> bar { pgbar::options::TodoChar( "-" ),
                           pgbar::options::DoneChar( "=" ),
                           pgbar::options::Styles( pgbar::configs::Progress::Rate | pgbar::configs::Progress::Timer ),
                           pgbar::options::StatusColor( pgbar::colors::Yellow ),
                           pgbar::options::Tasks( 100 ) };
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

这种配置方法也不仅局限在构造函数中，对于 `configure().set()` 方法也是可用的。

```cpp
bar.configure().set( pgbar::options::TodoChar( " " ),
                     pgbar::options::StatusColor( pgbar::colors::Green ) );
for ( auto _ = 0; _ < 100; ++_ )
  bar.tick();
```

实际上，以上的所有操作都是在修改 `ProgressBar` 内部的 `pgbar::Config` 对象。所以你也可以单独定义一个 `pgbar::Config` 类型，然后将它传递给 `ProgressBar`。

```cpp
auto cfg = pgbar::Config( pgbar::options::TodoChar( "-" ),
                          pgbar::options::DoneChar( "+" ),
                          pgbar::options::Tasks( 114514 ) );
pgbar::ProgressBar<> bar2 { cfg }; // `Config` 是可复制的
bar.configure( std::move( cfg ) ); // 也是可移动的
```

## 在*需要的时候*重置进度条
`ProgressBar` 中有一个 `reset` 方法，调用该方法可以立即终止当前迭代过程，丢弃所有已更新信息，并重置进度条状态到初值。

> 已更新信息是指累计更新的步数，可以通过 `ProgressBar` 的 `progress` 获取。

这个方法在每次迭代任务完成时都会被自动调用，不过你也可以提前调用它。

```cpp
pgbar::ProgressBar<> bar { 100, 2 };
for ( auto _ = 0; _ < 100; _ += 2 ) {
  bar.tick();
  if ( _ == 48 ) {
    bar.reset();
    // 此时 bar 追踪的迭代信息已经被丢弃
    std::cout << "Aborting!" << std::endl;
    break;
  }
}
```

在调用 `reset` 后再次调用 `tick` 会在终端中重新渲染一个新的进度条。

## 使用 `foreach` 方法遍历一个范围
`ProgressBar` 提供了一个**仅供左值对象调用的** `foreach` 方法，你可以当它是一种低配版的 `std::for_each`。

但与 `std::for_each` 的不同之处在于，这个方法可以在遍历某个范围的同时，将迭代过程可视化为终端上显示的一个进度条。

该方法支持接收类 STL 容器，或原始数组。

> STL 容器是指有迭代器类型，并且迭代器可以通过 `begin` 和 `end` 方法获得的类类型。

这个方法有两种遍历一个范围的方式，其中一种是使用 `range-base for`。

```cpp
std::vector<int> vec = { 1, 2, 3, 4, 5 };
for ( auto _ : bar.foreach ( vec ) )
  // `foreach` 会自动计算任务数量
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

// 你也可以只传递两个迭代器
for ( auto _ : bar.foreach ( vec.begin(), vec.end() ) )
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
```

另一种方式是像 `std::for_each` 一样使用一个一元 `lambda` 函子。

```cpp
int arr1[] = { 6, 7, 8, 9, 10 };
bar.foreach ( arr1, []( int _ ) {
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
} );
```

而且不仅仅只有容器类型，`foreach` 还能在一个数值范围上进行迭代。

```cpp
std::vector<double> buffer;
// 注意：这个数值范围是一个左闭右开的半区间
bar.foreach ( 1.0, -1.0, -0.1, [&buffer]( double e ) { buffer.push_back( e ); } );

std::cout << "The elements is: ";
for ( auto e : buffer )
  std::cout << e << ' ';
std::cout << std::endl;
```

所有这些行为都依赖于定义于 `pgbar::iterators` 中的抽象范围迭代器。

```cpp
// 数值范围
auto numbers = pgbar::iterators::NumericSpan<int>( 0, 20, 1 );
auto begin   = numbers.begin();

// 迭代器范围
// 对于指针类型，起点可以大于终点（即逆序），只要它们指向的范围满足左闭右开即可
int arr2[] = { 0, 1, 2 };
auto iters = pgbar::iterators::IterSpan<int*>( arr2 + 2, arr2 - 1 );
```

而 `foreach` 方法则使用了名为 `pgbar::iterators::ProxySpan` 的一个特殊范围类型。

字面意思，这个类型会获取（你也可以叫做“代理”）一个范围类型的所有权，以及一个对已存在的 `ProgressBar` 对象的引用，然后在一边遍历这个范围的同时更新这个 `ProgressBar` 对象。

```cpp
auto proxy =
  pgbar::iterators::ProxySpan<pgbar::iterators::NumericSpan<int>, pgbar::ProgressBar<>>(
    std::move( numbers ),
    bar );
```

实际上一般都不会直接使用上述的几个类型。

## 异常场景
### 任务数为零
如果没有设定任务数量，或者被设置为了零，那么调用任意一个 `tick` 方法时都会抛出一个 `pgbar::exceptions::InvalidState` 异常。

> 所有的异常类型都派生自 `std::exception`。

```cpp
try {
  bar.tick();
} catch ( const pgbar::exceptions::InvalidState& e ) {
  const std::exception& base_e = e;
  std::cerr << "  Exception: \"" << base_e.what() << "\"" << std::endl;
}
```
### 错误的迭代范围
如果向 `foreach` 中传递了一个数学上（或是类型上，这种情况常见于迭代器范围）显然非法的范围，同样会立即抛出一个 `pgbar::exceptions::InvalidArgument` 异常。

```cpp
try {
  // 起点小于终点，但步长却为负值；反之亦然
  bar.foreach ( -100, 100, -1, []( double e ) {} );
} catch ( const pgbar::exceptions::InvalidArgument& e ) {
  std::cerr << "  Exception: \"" << e.what() << "\"" << std::endl;
}
```
### 错误的 RGB 数值
传递非法的 RGB 数值或字符串都会抛出 `pgbar::exceptions::InvalidArgument`。

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

## 指派特定的流对象
你可以向 `ProgressBar` 中传递一个满足类型谓词 `pgbar::traits::is_ostream` 的流对象类型。

```cpp
using StreamType = std::ostream;
static_assert( pgbar::traits::is_ostream<StreamType>::value, "ERROR" );

pgbar::ProgressBar<StreamType> bar { std::clog };
```

默认使用的流对象会被绑定到 `std::cerr` 上。

如果程序并没有运行在终端中，`Config::intty()` 方法还会返回 `false`，并且进度条将不会显示任何东西。

```cpp
std::cout << "  Current program is running in a tty? " << std::boolalpha
          << pgbar::Config::intty() << std::endl;
```

不过这个功能仅限于 Windows 或类 Unix 系统。

另外，你可以在引入头文件前定义一个 `PGBAR_INTTY` 宏，这样进度条就会始终相信它运行在终端中。

## 更改进度条的长度
可以使用方法 `bar_length` 获取并修改进度条的进度指示部分。

```cpp
std::cout << "  Default bar length: " << bar.configure().bar_length() << std::endl;
bar.configure().bar_length( 50 ); // 单位：字符
```

如果你能获取终端的宽度，那么你可以自己计算能令进度条填满一行的 `bar_length` 数值。

不过这里需要使用具体的操作系统 API。

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

你可以使用 `configure().fixed_size()` 获取除了进度指示部分的长度，然后使用终端宽度减去这部分的长度。

```cpp
const auto total_length_excluding_bar = bar.configure().fixed_size();
bar.configure().bar_length( terminal_width - total_length_excluding_bar );
```

此时的进度条恰能填满一行。

## 工厂函数
库中提供了一个名为 `pgbar::make_progress` 的工厂函数。

```cpp
auto bar =
  pgbar::make_progress( std::clog,
                        pgbar::options::Tasks( 1919810 ),
                        pgbar::options::Styles( pgbar::configs::Progress::Entire & pgbar::configs::Progress::Bar ) );
```

同样的，缺省流对象时会默认绑定到默认的上。

## 更改全局刷新率
刷新率（或者叫刷新间隔）是一个 `pgbar::Config` 的静态成员，它会影响到全局的进度条显示效果。

它的单位是 `std::chrono::nanoseconds`。

```cpp
std::cout << "  Default refresh interval: " << pgbar::Config::refresh_interval().count()
          << " ns" << std::endl;
// 可以通过调用同名方法更改它的值
pgbar::Config::refresh_interval( std::chrono::nanoseconds( 20000 ) );
std::cout << "  The new refresh interval: " << pgbar::Config::refresh_interval().count()
          << " ns" << std::endl;
```

进度条对象在每次向终端输出字符后，都会在下一次输出前等待这么长的时间。

由于输出字符串的时间开销并非零，因此终端的进度条的最大刷新率仅取决于进度条对象每次拼装字符串并输出到终端的速度。

这里的刷新间隔仅提供了一个延长每次刷新时间的手段，或者换句话说，用来降低刷新率。

## 线程安全性
调用 `pgbar::Config` 除了构造函数之外的成员方法以及静态方法是线程安全的，它们都受到读写锁的保护。

但是如果 `pgbar::ProgressBar` 在运行中（使用 `is_running` 方法判断），那么调用任何非 `const` 修饰的非静态成员方法必然线程不安全，并且调用的行为是不确定的。

`pgbar::ProgressBar` 本身既可以是线程安全，也可以是线程不安全的，这取决于它的第二模板类型参数。

> 它的第二模板类型参数可以是任何满足 `basic lockable` 的锁类型。

```cpp
pgbar::ProgressBar<std::ostream, pgbar::Threadsafe> thread_safe_bar;
pgbar::ProgressBar<std::ostream, std::mutex> stl_base_bar;               // fine
pgbar::ProgressBar<std::ostream, pgbar::Threadunsafe> thread_unsafe_bar; // default
```

在这里的语境中，“线程安全”指的是允许多个线程同时调用 `tick` 方法，并不包括尝试交换或移动对象本身。

而且正如你所想的，线程安全的版本需要支付代价（性能开销）。

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

## 运行单元测试
项目使用了 [Catch2](https://github.com/catchorg/Catch2) 编写了一些基础的单元测试，并基于 [xmake](https://github.com/xmake-io/xmake/) 提供的编译工具链测试。

> 注意：
> \
> 使用不同的 C++ 标准编译测试用例时，你需要引入不同版本的 `Catch2`；
> 对于 C++11，请将 [Catch2 v2.x](https://github.com/catchorg/Catch2/tree/v2.x) 的头文件直接放在 [tests/](tests/) 目录下；
> 对于 C++14 及更高版本，请使用包管理工具（你也可以使用 `xmake`）引入 [Catch2 v3.x](https://github.com/catchorg/Catch2/tree/devel)，或者自行将头文件合并、放在 [tests/](tests/) 目录下。
> \
> 使用的 `xmake` 版本必须高于 `2.8.5`.

如果想同时运行所有测试用例，可以使用以下命令。

```sh
xmake test
# 带上 -vD 参数可以查看详细输出信息
```

单独运行某个测试，可以使用以下命令。

```sh
xmake test testcase_filename/*
```

由于 `xmake` 的测试程序不会绑定到终端，所以部分测试用例在这种测试环境下是无效测试。

> 部分“测试用例”属于性能测试文件，不会归入测试集中。

这时候可以切入 `tests/` 目录使用 `Makefile` 手动编译测试用例；这也是一种单独运行某个测试的方法。

```sh
cd tests/ && make file=test_case_name.cpp && ./test
```
- - -

# `pgbar::SpinnerBar`
`pgbar::SpinnerBar` 的主要使用方式与 `pgbar::ProgressBar` 大体相同，这里仅介绍差异部分。
## UI 逻辑
`pgbar::SpinnerBar` 是一个与迭代过程无关的进度条；实际上就是一个运行在终端的动画组件。

只需要*调用一次* `pgbar::SpinnerBar` 的 `tick` 就会在终端输出对应的加载动画。

> 第一次调用 `tick` 后，在未调用 `reset` 前，再次调用 `tick` 是无害且无效的。

如果想要停止 `pgbar::SpinnerBar`，可以任由控制流离开当前对象的作用域，这样进度条对象会在析构时自动终止渲染过程。

当然也可以显式调用 `reset` 方法，这样除了可以停止渲染，还可以利用传递给 `true_frame()` 和 `false_frame()` 方法的字符串向终端输出额外信息（任务成功 or 失败）。

这种输出的最后一帧被称为“True”帧和“False”帧；具体输出哪个则由调用 `reset` 方法时传递的 `bool` 类型参数的值有关。

默认情况下输出“True”帧。

## 调整详细配置
使用流式接口 `configure` 的链式调用以配置不同参数；并且也可以使用 `pgbar::options` 中的包装器传递参数。

```cpp
pgbar::SpinnerBar<> bar;

bar.configure().suffix( "Waiting main thread..." );
```

> 对应的所有详细配置方法见下。
> - `configure().colored()`：
>  （setter/getter）用于设置是否开启着色效果；
> - `configure().bolded()`：
>  （setter/getter）用于设置是否加粗加载动画中的所有字体；
> - `configure().frames()`：
>  （仅 setter）更改加载动画；
> - `configure().suffix()`：
>  （仅 setter）更改在加载动画右侧显示的字符串，默认为空；
> - `configure().true_frame()`：
>  （仅 setter）更改调用 `reset()` 及 `reset( true )` 后输出的最后一帧，默认为空；
> - `configure().false_frame()`：
>  （仅 setter）更改调用 `reset( false )` 后输出的最后一帧，默认为空；
> - `configure().frames_color()`：
>  （仅 setter）更改加载动画的颜色，会一并修改作用在 `suffix` 上；
> - `configure().true_color()`：
>  （仅 setter）更改 True 帧的颜色；
> - `configure().false_color()`：
>  （仅 setter）更改 False 帧的颜色。
