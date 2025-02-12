**Contents**
- [基本使用指南](#基本使用指南)
  - [构造、交换与基类引用](#构造交换与基类引用)
  - [让其工作](#让其工作)
  - [辅助函数](#辅助函数)
- [定制自己的进度条对象](#定制自己的进度条对象)
  - [构造函数定制](#构造函数定制)
  - [成员方法定制](#成员方法定制)
- [深入了解配置类型](#深入了解配置类型)
  - [基本配置类型](#基本配置类型)
  - [配置类型基类](#配置类型基类)
- [深入了解进度条类型](#深入了解进度条类型)
- [可更改的进度条长度](#可更改的进度条长度)
- [可变的动画速率](#可变的动画速率)
- [线程安全性](#线程安全性)
  - [跨线程调用](#跨线程调用)
  - [锁类型](#锁类型)
- [切换输出流](#切换输出流)
- [设计原理](#设计原理)
  - [基本架构](#基本架构)
  - [关于异常传播](#关于异常传播)
  - [进度条类型与配置的实现原理](#进度条类型与配置的实现原理)

# 基本使用指南
## 构造、交换与基类引用
所有的进度条对象均可以无参数默认构造。

```cpp
pgbar::ProgressBar<> pbar;
pgbar::BlockProgressBar<> bpbar;
pgbar::SpinnerBar<> spibar;
pgbar::ScannerBar<> scnbar;
```

同时这些进度条对象均满足可移动、可交换但不可复制。

```cpp
pgbar::ProgressBar<> pbar;
pgbar::ProgressBar<> pbar2;

pbar = std::move( pbar2 ); // movable
swap( pgbar, pbar2 );      // swapable
// or
pbar.swap( pbar2 );
```

所有这些进度条对象都是 `pgbar::Indicator` 的派生类。

```cpp
pgbar::ProgressBar<> pbar;
pgbar::Indicator& base_ref = pbar;
```

## 让其工作
无参数构造的 `ProgressBar` 和 `BlockProgressBar` 并不能直接使用，必须向它们的方法 `config()` 的子方法 `tasks()` 传递一个任务数量后才能够使用；否则会抛出异常 `pgbar::exception::InvalidState`。

> 库中所有异常均派生自 `std::exception`，它们都有一个共同基类 `pgbar::exception::Error`。

```cpp
pgbar::ProgressBar<> pbar;

try {
  pbar.tick();
} catch ( const pgbar::exception::InvalidState& e ) {
  std::cerr << "Oops! An exception occurs here: \"" << e.what() << "\"" << std::endl;
}
```

在迭代过程中，可以使用 `is_running()` 和 `progress()` 方法，检查当前进度条是否正在运行，以及获取当前已调用 `tick()` 的次数。

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().tasks( 100 );
for ( auto i = 0; i < 100; ++i ) {
  pbar.tick();
  if ( i == 99 ) // 最后一次 `tick()` 的调用会使得进度条自动停止
    assert( !pbar.is_running() ); // 所以这个断言会在该时刻不成立
  else
    assert( pbar.is_running() );
  assert( pbar.progress() != 0 );
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

只要调用 `tick()` 的次数等于 `config().tasks()` 的预定值时，进度条就会自行停止运行。但也可以提前调用进度条的 `reset()` 方法主动停止进度条运行；如果进度条对象被析构，则等价于调用 `reset()`。

与上述的两个进度条类型不同，`SpinnerBar` 和 `ScannerBar` 并不需要指定任务数量也可以直接使用。

```cpp
pgbar::SpinnerBar<> spibar;

spibar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
spibar.reset();
```

但因为不存在任务数限制，所以进度条的工作不会自行停止，此时必须主动调用 `reset()` 方法停止渲染

## 辅助函数
除了手动指定任务数量，还可以利用 `iterate()` 方法让进度条自己在某个“抽象范围”上工作，这个时候进度条对象会自行计算任务数量。

`iterate()` 的使用方法与 Python 中的 `range()` 函数类似。

```cpp
pgbar::BlockProgressBar<> bpbar;

std::cout << "Iterate over a range of values with BlockProgressBar: ";
for ( auto&& _ : bpbar.iterate( 50 ) ) {
  // 在数值区间 [0, 50) 上迭代，步长为 1
  std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );
}
```

`iterate()` 方法也能在某个数据容器上进行工作，例如数组。

```cpp
pgbar::BlockProgressBar<> bpbar;

std::cout << "Iterate over a raw array with BlockProgressBar: ";
int arr[100] { 0 };
for ( auto&& e : bpbar.iterate( arr ) ) {
  e = 114514; // 可以在循环内直接访问被迭代容器的元素
}
const auto baseline = std::vector<int>( 100, 114514 );
std::cout << "Are the values in these two ranges equal? " << std::boolalpha
          << std::equal( arr, arr + 100, baseline.cbegin() ) << std::endl;
```

也可以向 `iterate()` 传入两个迭代器划定的被迭代范围；如果是指针类型的迭代器，可以通过倒置起点和终点实现逆序访问。

```cpp
pgbar::SpinnerBar<> spibar;

std::cout << "Reverse iterate over a raw array with SpinnerBar: ";
for ( auto&& _ : spibar.iterate( arr + 100, arr ) ) {
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

这里的数据容器也包含了 STL 中的泛型容器，如 `std::vector`。

```cpp
pgbar::ScannerBar<> scnbar;
// 此处的 baseline 出现在上文中
std::cout << "Iterate over an object of std::vector with ScannerBar: ";
for ( auto&& _ : scnbar.iterate( baseline ) ) {
  // do something here...
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

`iterate()` 也能和 `std::for_each` 一样，接受一个一元函数并作用在范围内的元素上。

```cpp
pgbar::ScannerBar<> scnbar;

std::cout << "Iterate over an object of std::vector with an unary function: ";
std::vector<char> alphabet( 26 );
std::iota( alphabet.begin(), alphabet.end(), 'a' );
scnbar.iterate( alphabet, []( char ) { /*...*/ } );
```

# 定制自己的进度条对象
## 构造函数定制
所有类型的进度条都可以通过传入不少于一个的 `pgbar::option` 包装器类型，实现在默认配置的基础上构造一个经过修改的对象。

```cpp
pgbar::ProgressBar<> pbar { pgbar::option::Style( // 选择不同的进度条信息部分
                              pgbar::config::CharBar::Per | pgbar::config::CharBar::Elpsd ),
                            pgbar::option::InfoColor( pgbar::color::Red ), // 更改信息部分的颜色
                            pgbar::option::Tasks( 100 ) };
for ( auto _ = 0; _ < 100; ++_ ) {
  pbar.tick();
  std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
}
```

不允许传递重复的 `pgbar::option` 类型，否则会导致编译错误。

```cpp
pgbar::SpinnerBar<> spibar { pgbar::option::Lead( { "◜", "◝", "◞", "◟" } ),
                             pgbar::option::Description( "Loading..." ),
                             pgbar::option::DescColor( "#FFF" ),
                             pgbar::option::DescColor( "#39C5BB" ) };
// 编译错误！
```

## 成员方法定制
除了构造函数可以使用多个可变的 `pgbar::option` 包装，成员方法 `config().set()` 同样可以接收这样形式的参数。

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

其中，所有用于配置颜色的类型/方法都能接收字符串形式的合法 RGB 颜色代码，或者是直接以十六进制整数形式给出的 RGB 颜色数值。

并且颜色效果能够通过定义一个 `PGBAR_COLORLESS` 宏强制关闭。

此外，`config()` 本身提供了一种流式接口风格，同样能够逐一配置不同部分的进度条样式。

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

# 深入了解配置类型
## 基本配置类型
实际上，`config()` 方法的返回值是进度条对象内部持有的一个配置对象的引用，这个配置对象所属类型可以在 `pgbar::config` 中找到；每个进度条类型有且仅有一个与之对应的配置类型。

```cpp
pgbar::config::CharBar pbar_cfg;   // ProgressBar 的配置类型
pgbar::config::BlckBar bpbar_cfg;  // BlockProgressBar 的配置类型
pgbar::config::SpinBar spibar_cfg; // SpinnerBar 的配置类型
pgbar::config::ScanBar scnbar_cfg; // ScannerBar 的配置类型
```

所有配置类型均满足可复制、可移动且可交换。

```cpp
pgbar::config::CharBar pbar_cfg;
pgbar::config::CharBar pbar_cfg2;

pbar_cfg = pbar_cfg2;              // copyable
pbar_cfg = std::move( pbar_cfg2 ); // movable
swap( pbar_cfg, pbar_cfg2 );       // swapable
// or
pbar_cfg.swap( pbar_cfg2 );
```

你可以直接向进度条对象的构造函数传递一个配置对象，以根据已存在的配置信息创建新的进度条。

```cpp
pgbar::ProgressBar<> pbar { pbar_cfg };
// 或者通过 config() 方法重新对内部的配置类型进行再赋值
pbar.config() = pbar_cfg;

// 配置对象的构造函数本身也支持可变参数列表，所以你也可以这样更改配置对象
pbar.config() = { pgbar::option::InfoColor( pgbar::color::Red ), pgbar::option::Tasks( 100 ) };
// 这等价于 pbar.config().swap( another_config )
```

## 配置类型基类
这些类型都有一个统一的基类：`pgbar::config::Core`。

```cpp
pgbar::config::CharBar pbar_cfg;
pgbar::config::Core& base_ref = pbar_cfg;
```

这个基类也存储了一些与底层设置有关的配置数据。例如进度条对象在终端界面上的刷新率就由 `pgbar::config::Core::refresh_interval()` 的返回值控制，并且 `pgbar::config::Core` 还可以让你在运行时修改这些基础配置数据。

```cpp
// 将刷新率从默认的 25 Hz 提升为 60 Hz
pgbar::config::Core::refresh_interval( std::chrono::milliseconds( 20 ) );
```

这个方法实际上是要求进度条对象在每次向控制台输出后休眠 20 毫秒；更短的刷新间隔意味着更流畅的动画效果，但也会带来更高的单核 CPU 负载。

但由于每次渲染输出的时间不可能为 0，所以这个方法实质上是在控制渲染输出的间隔最短不能低于给定值。

同时你也能通过 `pgbar::config::Core::intty()` 方法获知当前进程的标准输出流是否绑定在终端上，这需要结合枚举量 `pgbar::StreamChannel` 的值确定你需要检查的是哪个输出流；例如检查标准输出流 `stdout`：

```cpp
if ( pgbar::config::Core::intty( pgbar::StreamChannel::Stdout ) )
  std::cout << "Standard output is bound to a terminal." << std::endl;
else
  std::cout << "Standard output is not bound to a terminal." << std::endl;
```

你可以在 include 文件之前定义一个 `PGBAR_INTTY` 宏，进而强制让 `pgbar::config::Core::intty()` 方法始终返回 `true`。

# 深入了解进度条类型
正如前面所介绍的，所有进度条类型都是高度相似的，它们唯一的区别仅在于调用 `tick()` 方法时的行为、构造函数能够接收的参数，以及调用 `config()` 方法时能够链式调用的方法种类不同。

除了前文提到的 `is_running()`、`progress()` 和 `iterate()` 外，进度条对象还有几个有用的方法；这里以 `ProgressBar` 为例。

```cpp
pgbar::ProgressBar<> pbar { pgbar::option::Tasks( 100 ),
                            pgbar::option::Description( "Doing sth..." ),
                            pgbar::option::TrueMesg( "✔ Mission Complete!" ),
                            pgbar::option::TrueColor( pgbar::color::Green ),
                            pgbar::option::FalseMesg( "✖ Execution Failure!" ),
                            pgbar::option::FalseColor( pgbar::color::Red ) };
pbar.tick();         // 让进度条前进一步
pbar.reset();        // 重置进度条，并立即终止渲染，同时显示 TrueMesg 的内容
pbar.tick_to( 50 );  // 让进度条前进到 50%
pbar.tick( 3 );      // 让进度条前进三步
pbar.reset( false ); // 重置进度条，除了立即终止渲染，还显示 FalseMesg 的内容
```

> `TrueMesg` 和 `FalseMesg` 是 `Description` 元素的一部分，它们仅出现在进度条即将停止工作之前，用于将预先设定好的描述信息替换为自定义的信息。
> \
> 具体显示哪个信息则由调用 `reset()` 方法时传入的 `bool` 参数而定；默认情况下选择 `TrueMesg`。

要注意：如果进度条对象被析构，则不会呈现 `TrueMesg` 或 `FalseMesg`，即使它们已经被写入配置对象中。

> 在析构场景下，立即停止工作并释放所有资源会被视为最优先的决策。

```cpp
{
  std::cout << "Be deconstructed while rendering." << std::endl;
  pgbar::SpinnerBar<> spibar { pgbar::option::Description( "???" ),
                               pgbar::option::TrueMesg( "!!!" ),
                               pgbar::option::FalseMesg( "///" ) };
  spibar.tick();
  std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
} // spibar 在这里被析构，渲染立即停止
// 所以这里得补上一个换行符
std::cout << std::endl;
```

除了在任务开始前就为进度条配置好所有信息，你也可以在进度条运行时动态修改其中的信息。

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().tasks( 5 );
for ( auto i = 0; i < 5; ++i ) {
  pbar.tick();
  pbar.config().description( "Working process (" + std::to_string( i + 1 ) + "/5)" );
  std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
}
```

但是对于任务数量的修改是**不会起效**的。

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

如果你愿意，你还可以像这样“渐变”进度条的颜色。

```cpp
pgbar::ProgressBar<> pbar;

pbar.config().description( "Rainbow" ).tasks( 0xFFFFFF + 1 );
for ( size_t i = 0; i < 0xFFFFFF + 1; ++i ) {
  pbar.tick();
  pbar.config().info_color( i );
  std::this_thread::sleep_for( std::chrono::microseconds( 30 ) );
}
```

# 可更改的进度条长度
特别的，对于 ProgressBar、BlockProgressBar 和 ScannerBar 来说，它们的条状指示器的长度是可以改变的；这里以 ScannerBar 为例。

```cpp
pgbar::ScannerBar<> scnbar;

// config().bar_length() 的计数单位是“字符”
// 也就是说你传入的值就表示条状指示器在终端所占的字符数
scnbar.config().bar_length( 20 );
scnbar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
scnbar.reset();
```

如果能获得到当前终端窗口的横向宽度，那么你还能自己计算得到一个能让进度条“填满”一行的长度。

你可以通过调用 `config().fixed_size()` 获得除了条状指示器之外的进度条部分的长度，然后像下面这样计算。

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
constexpr auto terminal_width = 120; // Assuming terminal width is 120.
#endif
const auto total_length_excluding_bar = scnbar.config().fixed_size();
scnbar.config().bar_length( terminal_width - total_length_excluding_bar );
// config().bar_length() 本身也能返回当前进度条的条状指示器长度

scnbar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
scnbar.reset();
```

# 可变的动画速率
对于 `ProgressBar`、`SpinnerBar` 和 `ScannerBar` 而言，它们的组件 `Lead` 部分的运动速率是可变的。

这一可变速率可以通过类型包装器 `pgbar::option::Shift` 及方法 `config().shift()` 调节。

```cpp
pgbar::ScannerBar<> scnbar;
scnbar.config().shift( 2 ); // 切换到 2 倍速
scnbar.tick();
std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
scnbar.reset();
```

`Shift` 的取值在 -128 到 127 之间；如果取值 k 为负数，则表示将速率调整至正常情况下的 1/k，否则则是正常情况下的 k 倍。

实际上，切换动画速率的效果与更改 `pgbar::config::Core::refresh_interval()` 等效；但 `Shift` 的效果仅会作用在局部进度条对象上。

# 线程安全性
## 跨线程调用
首先，`pgbar::config` 中任何类型的方法都是线程安全的，这包括使用 `config()` 方法替换配置对象本身；也就是说你可以在别的线程中为另一个线程的进度条对象配置不同的参数。

其次，进度条对象本身是“可选”线程安全的，这一可选性体现在进度条对象的模板参数中。对于所有使用默认构造的进度条对象，它们等价于以下声明语句：

```cpp
pgbar::ProgressBar<pgbar::Threadunsafe> unsafe_bar;
```

对于线程不安全的进度条对象来说，调用除了 `config()` 外的任何方法都是线程不安全的。

而使用 pgbar::Threadsafe 参数则可以创建一个线程安全的进度条对象。

```cpp
pgbar::ProgressBar<pgbar::Threadsafe> safe_bar;
```

对于线程安全的版本，你可以在任意多个线程中同时调用它的 `tick()` 和 `reset()` 方法；但是对于 `swap()`、`iterate()` 以及 `operator=()` 依然是线程不安全的。

```cpp
safe_bar = pgbar::ProgressBar<pgbar::Threadsafe>(); // Thread Unsafe!
```

## 锁类型
实际上，`pgbar::Threadsafe` 只是一个满足 `Basic Locakable` 要求的锁类，你完全可以使用别的锁类型代替这个参数类型；例如 `std::mutex`。

> 相比之下，线程安全版本的进度条会比不安全的版本有额外的开销，这一开销并不能通过引入更轻量级的锁类型解决。

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

可以注意到上段代码的末尾调用了一个 `wait()` 方法；这是因为在多线程环境下，如果持有进度条对象的线程离开了进度条的作用域，就会因为析构对象而导致进度条渲染工作立即停止。

所以进度条对象提供了 `wait()` 和 `wait_for()` 方法，用于阻塞当前线程直到进度条更新完毕。

但阻塞效果仅在第一次 `tick()` 方法被调用后生效；所以在多线程环境下，最优解是等待所有子线程都结束后再调用 `wait()` 或 `wait_for()` 方法。

# 切换输出流
在默认情况下，进度条对象会向当前进程的标准错误流 `stderr` 输出字符串；输出流的目的地可以经由创建进度条对象时，传递给进度条的模板类型参数改变。

例如创建一个向 `stdout` 输出的进度条对象：

```cpp
pgbar::ScannerBar<pgbar::Threadunsafe, pgbar::StreamChannel::Stdout> scnbar;
```

进度条本身没有实现在某一时刻独占当前进程的某个标准输出流，因此在进度条工作过程中向该进度条所绑定的标准输出流中输出信息时，会导致终端渲染的字符串出现错乱。

如果某个输出流并没有被绑定到终端上，那么进度条不会向该输出流文件写入任何数据，但是异常检查和任务迭代计数正常进行。

> 检查某个输出流是否绑定在终端上，可以使用前文提及的 `pgbar::config::Core::intty()` 方法判断。

# 设计原理
## 基本架构
进度条由两个主要部分组成：通知线程和渲染线程。通知线程就是每次负责调用 `tick()` 方法的线程，而渲染线程则是一个由线程管理器 `pgbar::__detail::render::Renderer` 负责管理的线程对象。

每次通知线程的首次调用 `tick()` 都会唤醒渲染线程；由于线程管理器被设计为惰性初始化，因此每个进度条对象在全局范围内的第一次调用还会尝试初始化线程管理器，即创建一个子线程。

每次调用 `reset()` 方法时，进度条对象会通过线程管理器的成员方法将渲染线程挂起。

为了保证渲染线程的工作状态始终有效，仅在第一次 `tick()` 和最后一次 `tick()`，或者在运行中调用 `reset()` 时，通知线程会使用自旋锁等待渲染线程转移到指定状态；也就是说仅在上述三个时间点中，通知线程会有一次不定长的阻塞。

## 关于异常传播
由于渲染线程需要重复拼接字符串并向标准输出流写入数据，因此这整个过程都是有可能抛出异常的。

如果在 Windows 平台下，程序无法获取到当前进程的标准输出流 Handle，那么渲染线程中会抛出一个本地系统错误异常 `pgbar::exception::SystemError`；在更一般的情况下，如果当前机器的资源不足（例如内存爆了），则会由标准库在任意内存申请点位抛出 `std::bad_alloc` 等标准库异常。

其余情况下，如果渲染线程接收到了一个抛出的异常，它会将这个异常存储在内部的 `pgbar::__detail::concurrent::ExceptionBox` 容器中，并终止当前渲染工作；等待下一次通知线程调用 `activate()` 或 `suspend()` 时该被捕获异常会在通知线程中重新被抛出。

> `activate()` 和 `suspend()` 仅会在进度条的第一次和最后一次 `tick()` 及 `reset()` 方法中被调用。

如果渲染线程的异常容器已经存在了一个未处理的异常，此时线程内部再次抛出了一个异常，那么渲染线程将会进入凋亡（`dead`）状态；在这个状态下，新的异常不会被捕获，而是任其传播直至渲染线程终止。

在凋亡状态下，重新调用线程管理器的 `activate()` 方法（即让进度条对象重新开始工作）将会尝试拉起一个新的渲染线程；这个过程中，上一次未被处理的异常将会在新的渲染线程创建完毕、且开始工作之前抛出。

## 进度条类型与配置的实现原理
如前文所说，不同类型的进度条的功能是高度相似的，只是它们在运行时的语义表达和渲染样式不同。

实际上，所有进度条类型都是模板类 `pgbar::BasicBar` 的别名；同理，所有的配置类型都是 `pgbar::config::BasicConfig` 的别名。

受到[这篇文章](https://zhuanlan.zhihu.com/p/106672814)的启发，进度条遵循 Mixin 模式组合继承自 `pgbar::__detail::assets` 中的不同模板基类；而 `pgbar::__detail::assets` 内的所有模板类都按照 CRTP 模式设计，因此在最终使用时可以通过 `config()` 方法链式调用一大堆用于配置数据的方法。

这里的 Mixin 模式主要归功于一个编译期拓扑排序算法 `pgbar::__detail::traits::TopoSort`。

这个拓扑排序算法类似于 Python 中的 C3 线性化算法，但与之不同的是：C3 线性化算法的目的是在类继承结构中寻找到一个最合适的类方法，而这里的拓扑排序算法则是直接线性化整个继承结构，将一个复杂的多继承结构在编译期内线性化为一条继承链。

但它们有一点是相似的：在最派生类处的方法解析满足基类继承顺序的局部优先原则；也就是说在继承过程中，位置在继承列表中靠左的基类会优先被放到更靠近派生类的位置上，以确保这类基类的方法不会被更靠右的基类所覆盖。

此外，该拓扑排序算法还有一个特点：对于存在非虚拟继承关系的两个类，在排序后会被尽可能的放在最靠近彼此的位置上；这一点是为了保证在非虚拟继承中，派生类可以直接引用基类而无需进行复杂的模板嵌套工作。

具体工作原理可以参照前文提及的文章，我只是在该文章提出的原理的基础上自行实现了一个能够同时解析虚拟继承和非虚拟继承关系，并且满足一部分 C3 线性化算法特征的拓扑排序算法。
