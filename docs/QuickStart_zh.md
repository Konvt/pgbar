**Contents**
- [独立进度条](#独立进度条)
  - [`ProgressBar`](#progressbar)
    - [交互方式](#交互方式)
    - [配置选项](#配置选项)
      - [元素构成](#元素构成)
        - [可变的进度条长度](#可变的进度条长度)
      - [数据配置](#数据配置)
      - [线程安全性](#线程安全性)
      - [绑定的输出流](#绑定的输出流)
    - [与可迭代类型的交互](#与可迭代类型的交互)
  - [`BlockProgressBar`](#blockprogressbar)
    - [交互方式](#交互方式-1)
    - [配置选项](#配置选项-1)
      - [元素构成](#元素构成-1)
        - [可变的进度条长度](#可变的进度条长度-1)
      - [数据配置](#数据配置-1)
      - [线程安全性](#线程安全性-1)
      - [绑定的输出流](#绑定的输出流-1)
    - [与可迭代类型的交互](#与可迭代类型的交互-1)
  - [`ScannerBar`](#scannerbar)
    - [交互方式](#交互方式-2)
    - [配置选项](#配置选项-2)
      - [元素构成](#元素构成-2)
        - [可变的进度条长度](#可变的进度条长度-2)
      - [数据配置](#数据配置-2)
      - [线程安全性](#线程安全性-2)
      - [绑定的输出流](#绑定的输出流-2)
    - [与可迭代类型的交互](#与可迭代类型的交互-2)
  - [`SpinnerBar`](#spinnerbar)
    - [交互方式](#交互方式-3)
    - [配置选项](#配置选项-3)
      - [元素构成](#元素构成-3)
      - [数据配置](#数据配置-3)
      - [线程安全性](#线程安全性-3)
      - [绑定的输出流](#绑定的输出流-3)
    - [与可迭代类型的交互](#与可迭代类型的交互-3)
- [进度条合成器](#进度条合成器)
  - [`MultiBar`](#multibar)
    - [交互方式](#交互方式-4)
    - [辅助函数](#辅助函数)
- [全局设置](#全局设置)
  - [着色效果](#着色效果)
  - [输出流检测](#输出流检测)
  - [渲染器工作间隔](#渲染器工作间隔)
  - [断言检查](#断言检查)
- [辅助类型](#辅助类型)
  - [`NumericSpan`](#numericspan)
    - [成员方法](#成员方法)
    - [迭代器类型](#迭代器类型)
  - [`IterSpan`](#iterspan)
    - [成员方法](#成员方法-1)
    - [迭代器类型](#迭代器类型-1)
  - [`ProxySpan`](#proxyspan)
    - [成员方法](#成员方法-2)
    - [迭代器类型](#迭代器类型-2)
- [FAQ](#faq)
  - [更新计数与任务总数一致性](#更新计数与任务总数一致性)
  - [进度条对象的生命周期](#进度条对象的生命周期)
  - [Unicode 支持](#unicode-支持)
  - [渲染器设计](#渲染器设计)
  - [异常传播机制](#异常传播机制)
  - [设计架构](#设计架构)
    - [基础数据结构设计](#基础数据结构设计)
    - [进度条类型设计](#进度条类型设计)

# 独立进度条
## `ProgressBar`
![pgbar](../images/progressbar.gif)
### 交互方式
`pgbar::ProgressBar` 是一个模板类型；它需要手动配置任务数量才能开始使用，否则会抛出异常 `pgbar::exception::InvalidState`。

任务数量的配置可以通过调用 `config().tasks()` 方法并传递参数完成，也可以利用 `pgbar::option::Tasks` 包装类型传递给构造函数实现。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  {
    pgbar::ProgressBar<> bar;
    try {
      bar.tick();
    } catch ( const pgbar::exception::InvalidState& e ) {
      std::cerr << e.what() << std::endl;
    }
  }
  {
    pgbar::ProgressBar<> bar;
    bar.config().tasks( 200 );

    bar.tick( 20 );    // 前进 20 步
    bar.tick_to( 50 ); // 将进度设置为 50%

    for ( int i = 0; i < 100; ++i )
      bar.tick(); // 每次调用仅前进 1 步
  }
  {
    pgbar::ProgressBar<> bar { pbar::option::Tasks( 150 ) };
    bar.tick_to( 20 );  // 将进度设置为 20%
    bar.tick_to( 130 ); // 超出 100% 的部分会被丢弃，并将进度条进度锁定到 100%
  }
}
```

在一些特别的场景中，如果想要检查进度条运行情况，或是强行终止进度条的运行，那么可以使用 `is_running()` 和 `reset()` 方法。

```cpp
#include "pgbar/pgbar.hpp"
#include <cassert>

int main()
{
  pgbar::ProgressBar<> bar { pgbar::option::Tasks( 500 ) };

  for ( int i = 0; i < 400; ++i ) {
    if ( i > 0 ) // 要注意，只有调用一次 tick() 后进度条才开始运行
      assert( bar.is_running() );
    bar.tick();
  }

  assert( bar.progress() == 399 ); // 该方法可以获取进度条当前的迭代数
  bar.reset();
  assert( bar.is_running() == false );
}
```

`ProgressBar` 是一个 move only 且 swappable 的类型，所以你可以使用另一个对象移动构造，或者与另一个对象交换彼此的配置数据。

```cpp
{
  pgbar::ProgressBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::ProgressBar<> bar2 { std::move( bar1 ) };
}
{
  pgbar::ProgressBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::ProgressBar<> bar2;
  bar2.swap( bar1 );
  // or
  using std::swap;
  swap( bar1, bar2 );
}
```

但不允许在进度条运行过程中交换或移动对象，否则会导致不可预知的错误。

```cpp
pgbar::ProgressBar<> bar1 { pgbar::option::Tasks( 500 ) };

bar1.tick();
assert( bar1.is_running() );

// pgbar::ProgressBar<> bar2 { std::move( bar1 ) }; No!
```
### 配置选项
正如前面一节中提到的，`ProgressBar` 的所有配置操作都需要经由方法 `config()` 完成。

该方法返回的是对内部配置对象的引用，该配置对象的类型可以在 `pgbar::config` 中找到，它是 `pgbar::config::CharBar`。

`pgbar::config::CharBar` 是一个数据类型，该类型存储了所有用于描述 `ProgressBar` 元素的数据成员；它满足可复制、可移动和可交换三个性质。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::CharBar cfg1;

  auto cfg2 = cfg1;              // copy
  auto cfg3 = std::move( cfg1 ); // move
  cfg3.swap( cfg2 );             // swap
  // or
  using std::swap;
  swap( cfg2, cfg3 );
}
```
#### 元素构成
`ProgressBar` 由以下几种元素组成：

```text
{LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
```

其中可以自定义的部分有：`LeftBorder`、`Description`、`Starting`、`Filler`、`Lead`、`Remains`、`Ending`、`Speed` 和 `RightBorder`，它们的功能与名字相同。

这些元素可以直接在 `pgbar::option` 中找到对应的包装类型：

```cpp
pgbar::option::Style;   // 决定以上元素哪些需要被渲染
pgbar::option::Colored; // 开关颜色效果
pgbar::option::Bolded;  // 开关字体加粗效果

pgbar::option::LeftBorder;  // 修改整个进度条左侧的起始边框
pgbar::option::RightBorder; // 修改整个进度条右侧的终止边框

pgbar::option::Description; // 修改任务描述信息
pgbar::option::TrueMesg;    // 修改进度条结束时，用于替换 Description 部分的元素
pgbar::option::FalseMesg;   // 修改进度条结束时，用于替换 Description 部分的元素

pgbar::option::Starting;  // 修改进度条块左侧、Percent 右侧的元素
pgbar::option::Ending;    // 修改进度条块右侧、Counter 左侧的元素
pgbar::option::Filler;    // 修改已迭代部分的填充字符
pgbar::option::Lead;      // 修改可变动画部分的各个帧
pgbar::option::Remains;   // 修改未迭代部分的填充字符
pgbar::option::Shift;     // 调整动画部分（Lead）的动画速度
pgbar::option::BarLength; // 调整进度条的长度

pgbar::option::SpeedUnit; // 修改 Speed 部分的单位
pgbar::option::Magnitude; // 调整 Speed 部分的进位倍率

pgbar::option::Tasks;   // 调整任务数量
pgbar::option::Divider; // 修改位于两个元素之间的间隔符

pgbar::option::DescColor;    // 修改 Description 的颜色
pgbar::option::TrueColor;    // 修改 TrueMesg 的颜色
pgbar::option::FalseColor;   // 修改 FalseMesg 的颜色
pgbar::option::StartColor;   // 修改 Starting 的颜色
pgbar::option::EndColor;     // 修改 Ending 的颜色
pgbar::option::FillerColor;  // 修改 Filler 的颜色
pgbar::option::RemainsColor; // 修改 Remains 的颜色
pgbar::option::LeadColor;    // 修改 Lead 的颜色
pgbar::option::InfoColor;    // 修改 Divider、Percent、Counter、Speed、Elapsed 和 Countdown 的颜色
```

> `TrueMesg` 和 `FalseMesg` 可以用来显示进度条迭代的任务执行是否成功，它们可以通过向 `reset()` 方法中传递一个 `bool` 参数实现切换显示；在默认情况下，进度条自行停止、或调用无参的 `reset()` 方法会选中 `TrueMesg`。

`pgbar::option::Style` 的参数可以由 `pgbar::config::CharBar` 内的多个静态成员执行位运算得到：

```cpp
pgbar::ProgressBar<> bar { pgbar::option::Style( pgbar::config::CharBar::Sped | pgbar::config::CharBar::Per
                                                   | pgbar::config::CharBar::Elpsd
                                                   | pgbar::config::CharBar::Cntdwn ) };
```

这种方法配置会显得很繁琐，所以 `CharBar` 提供了两个辅助方法 `enable()` 和 `disable()` 用于简化以上操作。

```cpp
pgbar::ProgressBar<> bar;
bar.config().enable().speed().percent().elapsed().countdown();
// or
bar.config().enable().entire();
bar.config().disable().animation().counter();
// Animation 就是进度指示器本身
// 并且不是所有元素都可以被关闭，例如 Description 就不行
```

以上元素在 `CharBar` 都有同名方法，调用这些方法并向其中传递参数同样能做到修改数据信息。
##### 可变的进度条长度
在元素 `Starting` 和 `Ending` 中间部分的是被称作 `Animation` 的进度指示器（不包括 `Starting` 和 `Ending`），这个进度指示器的长度是可变的。

`pgbar` 本身极少探测与具体平台有关的信息，例如终端的宽度等；因此如果希望进度条能够填满一行，或者进度条太长需要缩窄，就需要使用到 `bar_length()` 方法或 `pgbar::option::BarLength` 包装器更改进度指示器的长度。

对于后者，直接使用对应的接口调整参数即可；而前者则需要一个辅助方法获取除了进度指示器之外部分的长度，才能正确计算得到恰好能让进度条占满一行的长度。

这个方法就是 `config().fixed_size()`。

```cpp
pgbar::ProgressBar<> bar;
assert( bar.config().bar_length() == 30 ); // 默认值
assert( bar.config().fixed_size() != 0 );  // 具体值取决于数据成员的内容
```
#### 数据配置
`CharBar` 有两种数据配置方法：基于包装器类型的可变参数构造，和基于链式调用的流式接口风格。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::CharBar config1 {
    pgbar::option::Tasks( 100 ),
    pgbar::option::SpeedUnit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } ),
    pgbar::option::Magnitude( 1024 ),
    pgbar::option::InfoColor( "#39C5BB" )
    // pgbar::option::InfoColor(0x39C5BB) Don't do that!
  };
  // 注意：传入相同的包装器类型两次会导致编译错误

  pgbar::config::CharBar config2;
  config2.tasks( 100 )
    .speed_unit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } )
    .magnitude( 1024 )
    .info_color( "#39C5BB" );

  auto config3 = config2; // 构造完毕后也能使用可变模板参数调整
  config3.set( pgbar::option::Description( "Do something" ), pgbar::option::DescColor( 0xFFE211 ) );
}
```

尽管配置类型可以在进度条运行过程中被修改，但这一概念并不适用于任务数量；也就是说进度条一旦开始运行，它的任务数量就不可中途改变。

```cpp
#include "pgbar/pgbar.hpp

int main()
{
  pgbar::ProgressBar<> pbar;

  pbar.config().tasks( 100 );
  for ( auto i = 0; i < 100; ++i ) {
    pbar.tick();
    if ( i == 30 ) // nothing happens
      pbar.config().tasks( 50 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  }
}
```
#### 线程安全性
配置类型 `CharBar` 的一切方法都是线程安全的，这意味着你可以在进度条运行过程中修改其中的数据。

`ProgressBar` 只有 `tick()`、`tick_to()` 和 `reset()` 方法是线程安全的；其余方法，特别是 `iterate()`，是线程不安全的。

这表示进度条允许多个线程同时调用它的 `tick()`、`tick_to()` 和 `reset()` 方法；但这不包括移动赋值、移动构造和交换两个对象，以及 `iterate()` 方法。

并且绝对不应该在进度条运行过程中尝试移动或交换当前对象。
#### 绑定的输出流
`ProgressBar` 默认向标准错误流 `stderr` 输出字符串；具体绑定的输出流由第一个模板参数定义。

```cpp
#include "pgbar/pgbar.hpp"
#include <type_traits>

int main()
{
  static_assert( std::is_same<pgbar::ProgressBar<>,
                              pgbar::ProgressBar<pgbar::Channel::Stderr>>::value,
                 "" );

  pgbar::ProgressBar<pgbar::Channel::Stdout> bar; // 绑定到 stdout 上
}
```

特别需要注意的是，绑定到相同输出流上的对象不允许同时运行，否则会抛出异常 `pgbar::exception::InvalidState`；关于这一点的详细说明见 [FAQ-渲染器设计](#渲染器设计)。

如果指向的输出流并没有被绑定到终端上，那么这个进度条并不会被渲染；这一点可以详见 [全局设置-输出流检测](#输出流检测)。
### 与可迭代类型的交互
在处理一些可迭代类型、或者是数值范围的迭代任务时，`pgbar` 提供了一个更简单地迭代手段：`iterate()` 方法。

这个方法的使用与 Python 中的 `range()` 函数类似，它可以同时在由数值指定的范围上，和由可迭代类型划定的范围内进行遍历；具体的任务数量会由 `iterate()` 自己配置。

具体的迭代手段可以是使用 Enhanced-for，也可以像 `std::for_each` 一样传递一个一元函数。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
using namespace std;

int main()
{
  pgbar::ProgressBar<> bar;

  // Iteration range: [100, 0), step: -1
  for ( auto num : bar.iterate( 100, 1, -1 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [0.0, -2.0), step: -0.01
  for ( auto fnum : bar.iterate( -2.0, -0.01 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [100, 1), step: -1
  bar.iterate( 100, []( int ) { this_thread::sleep_for( 100ms ); } );
}
```

在前两个 Enhanced-for 中，`iterate()` 方法返回的实际上是 `pgbar::scope::ProxySpan`；关于这一类型的介绍可以见[辅助类型-`ProxySpan`](#proxyspan)。

有关数值类型范围的使用，可以见[辅助类型-`NumericSpan`](#numericspan)。

除了在数值范围内工作之外，`ProgressBar` 也可以与存在迭代器的类型进行交互，例如 `std::vector` 和原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::ProgressBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1.begin(), arr1.end() ) ) {
    ele += 1; // 此处的 ele 是对 vector 内元素的引用
    this_thread::sleep_for( 300ms );
  }
  // 可以逆序遍历
  bar.iterate( arr2 + ( sizeof( arr2 ) / sizeof( int ) ) - 1, arr2 - 1, []( int& ) {
    this_thread::sleep_for( 300ms );
  } );
}
```

有关可迭代类型的范围划定，可以见[辅助类型-`IterSpan`](#iterspan)。

特别的，如果一个类型提供了返回迭代器的 `begin()` 和 `end()` 方法，那么这个类型还可以以更简单的方式进行遍历；这包括了原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::ProgressBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1 ) ) {
    ele += 1;
    this_thread::sleep_for( 300ms );
  }
  // 数组也没问题
  bar.iterate( arr2, []( int& ) { this_thread::sleep_for( 300ms ); } );
}
```

- - -

## `BlockProgressBar`
![bpbar](../images/blockprogressbar.gif)
### 交互方式
`pgbar::BlockProgressBar` 是一个模板类型；它需要手动配置任务数量才能开始使用，否则会抛出异常 `pgbar::exception::InvalidState`。

任务数量的配置可以通过调用 `config().tasks()` 方法并传递参数完成，也可以利用 `pgbar::option::Tasks` 包装类型传递给构造函数实现。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  {
    pgbar::BlockProgressBar<> bar;
    try {
      bar.tick();
    } catch ( const pgbar::exception::InvalidState& e ) {
      std::cerr << e.what() << std::endl;
    }
  }
  {
    pgbar::BlockProgressBar<> bar;
    bar.config().tasks( 200 );

    bar.tick( 20 );    // 前进 20 步
    bar.tick_to( 50 ); // 将进度设置为 50%

    for ( int i = 0; i < 100; ++i )
      bar.tick(); // 每次调用仅前进 1 步
  }
  {
    pgbar::BlockProgressBar<> bar { pbar::option::Tasks( 150 ) };
    bar.tick_to( 20 );  // 将进度设置为 20%
    bar.tick_to( 130 ); // 超出 100% 的部分会被丢弃，并将进度条进度锁定到 100%
  }
}
```

在一些特别的场景中，如果想要检查进度条运行情况，或是强行终止进度条的运行，那么可以使用 `is_running()` 和 `reset()` 方法。

```cpp
#include "pgbar/pgbar.hpp"
#include <cassert>

int main()
{
  pgbar::BlockProgressBar<> bar { pgbar::option::Tasks( 500 ) };

  for ( int i = 0; i < 400; ++i ) {
    if ( i > 0 ) // 要注意，只有调用一次 tick() 后进度条才开始运行
      assert( bar.is_running() );
    bar.tick();
  }

  assert( bar.progress() == 399 ); // 该方法可以获取进度条当前的迭代数
  bar.reset();
  assert( bar.is_running() == false );
}
```

`BlockProgressBar` 是一个 move only 且 swappable 的类型，所以你可以使用另一个对象移动构造，或者与另一个对象交换彼此的配置数据。

```cpp
{
  pgbar::BlockProgressBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::BlockProgressBar<> bar2 { std::move( bar1 ) };
}
{
  pgbar::BlockProgressBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::BlockProgressBar<> bar2;
  bar2.swap( bar1 );
  // or
  using std::swap;
  swap( bar1, bar2 );
}
```

但不允许在进度条运行过程中交换或移动对象，否则会导致不可预知的错误。

```cpp
pgbar::BlockProgressBar<> bar1 { pgbar::option::Tasks( 500 ) };

bar1.tick();
assert( bar1.is_running() );

// pgbar::BlockProgressBar<> bar2 { std::move( bar1 ) }; No!
```
### 配置选项
正如前面一节中提到的，`BlockProgressBar` 的所有配置操作都需要经由方法 `config()` 完成。

该方法返回的是对内部配置对象的引用，该配置对象的类型可以在 `pgbar::config` 中找到，它是 `pgbar::config::BlckBar`。

`pgbar::config::BlckBar` 是一个数据类型，该类型存储了所有用于描述 `BlockProgressBar` 元素的数据成员；它满足可复制、可移动和可交换三个性质。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::BlckBar cfg1;

  auto cfg2 = cfg1;              // copy
  auto cfg3 = std::move( cfg1 ); // move
  cfg3.swap( cfg2 );             // swap
  // or
  using std::swap;
  swap( cfg2, cfg3 );
}
```
#### 元素构成
`BlockProgressBar` 由以下几种元素组成：

```text
{LeftBorder}{Description}{Percent}{Starting}{BlockBar}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
```

其中可以自定义的部分有：`LeftBorder`、`Description`、`Starting`、`Ending`、`Speed` 和 `RightBorder`，它们的功能与名字相同。

这些元素可以直接在 `pgbar::option` 中找到对应的包装类型：

```cpp
pgbar::option::Style;   // 决定以上元素哪些需要被渲染
pgbar::option::Colored; // 开关颜色效果
pgbar::option::Bolded;  // 开关字体加粗效果

pgbar::option::LeftBorder;  // 修改整个进度条左侧的起始边框
pgbar::option::RightBorder; // 修改整个进度条右侧的终止边框

pgbar::option::Description; // 修改任务描述信息
pgbar::option::TrueMesg;    // 修改进度条结束时，用于替换 Description 部分的元素
pgbar::option::FalseMesg;   // 修改进度条结束时，用于替换 Description 部分的元素

pgbar::option::Starting;  // 修改进度条块左侧、Percent 右侧的元素
pgbar::option::Ending;    // 修改进度条块右侧、Counter 左侧的元素
pgbar::option::BarLength; // 调整进度条的长度

pgbar::option::SpeedUnit; // 修改 Speed 部分的单位
pgbar::option::Magnitude; // 调整 Speed 部分的进位倍率

pgbar::option::Tasks;   // 调整任务数量
pgbar::option::Divider; // 修改位于两个元素之间的间隔符

pgbar::option::DescColor;    // 修改 Description 的颜色
pgbar::option::TrueColor;    // 修改 TrueMesg 的颜色
pgbar::option::FalseColor;   // 修改 FalseMesg 的颜色
pgbar::option::StartColor;   // 修改 Starting 的颜色
pgbar::option::EndColor;     // 修改 Ending 的颜色
pgbar::option::InfoColor;    // 修改 Divider、Percent、Counter、Speed、Elapsed 和 Countdown 的颜色
```

> `TrueMesg` 和 `FalseMesg` 可以用来显示进度条迭代的任务执行是否成功，它们可以通过向 `reset()` 方法中传递一个 `bool` 参数实现切换显示；在默认情况下，进度条自行停止、或调用无参的 `reset()` 方法会选中 `TrueMesg`。

`pgbar::option::Style` 的参数可以由 `pgbar::config::BlckBar` 内的多个静态成员执行位运算得到：

```cpp
pgbar::BlockProgressBar<> bar { pgbar::option::Style(
  pgbar::config::BlckBar::Sped | pgbar::config::BlckBar::Per | pgbar::config::BlckBar::Elpsd
  | pgbar::config::BlckBar::Cntdwn ) };
```

这种方法配置会显得很繁琐，所以 `BlckBar` 提供了两个辅助方法 `enable()` 和 `disable()` 用于简化以上操作。

```cpp
pgbar::BlockProgressBar<> bar;
bar.config().enable().speed().percent().elapsed().countdown();
// or
bar.config().enable().entire();
bar.config().disable().animation().counter();
// Animation 就是指 BlockBar
// 并且不是所有元素都可以被关闭，例如 Description 就不行
```

以上元素在 `BlckBar` 都有同名方法，调用这些方法并向其中传递参数同样能做到修改数据信息。
##### 可变的进度条长度
`BlockProgressBar` 的元素 `BlockBar` 也被称作 `Animation`，它是一个使用 Unicode 方块字符实现的进度指示器，这个进度指示器的长度是可变的。

`pgbar` 本身极少探测与具体平台有关的信息，例如终端的宽度等；因此如果希望进度条能够填满一行，或者进度条太长需要缩窄，就需要使用到 `bar_length()` 方法或 `pgbar::option::BarLength` 包装器更改进度指示器的长度。

对于后者，直接使用对应的接口调整参数即可；而前者则需要一个辅助方法获取除了进度指示器之外部分的长度，才能正确计算得到恰好能让进度条占满一行的长度。

这个方法就是 `config().fixed_size()`。

```cpp
pgbar::BlockProgressBar<> bar;
assert( bar.config().bar_length() == 30 ); // 默认值
assert( bar.config().fixed_size() != 0 );  // 具体值取决于数据成员的内容
```
#### 数据配置
`BlckBar` 有两种数据配置方法：基于包装器类型的可变参数构造，和基于链式调用的流式接口风格。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::BlckBar config1 {
    pgbar::option::Tasks( 100 ),
    pgbar::option::SpeedUnit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } ),
    pgbar::option::Magnitude( 1024 ),
    pgbar::option::InfoColor( "#39C5BB" )
    // pgbar::option::InfoColor(0x39C5BB) Don't do that!
  };
  // 注意：传入相同的包装器类型两次会导致编译错误

  pgbar::config::BlckBar config2;
  config2.tasks( 100 )
    .speed_unit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } )
    .magnitude( 1024 )
    .info_color( "#39C5BB" );

  auto config3 = config2; // 构造完毕后也能使用可变模板参数调整
  config3.set( pgbar::option::Description( "Do something" ), pgbar::option::DescColor( 0xFFE211 ) );
}
```

尽管配置类型可以在进度条运行过程中被修改，但这一概念并不适用于任务数量；也就是说进度条一旦开始运行，它的任务数量就不可中途改变。

```cpp
#include "pgbar/pgbar.hpp

int main()
{
  pgbar::BlockProgressBar<> pbar;

  pbar.config().tasks( 100 );
  for ( auto i = 0; i < 100; ++i ) {
    pbar.tick();
    if ( i == 30 ) // nothing happens
      pbar.config().tasks( 50 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  }
}
```
#### 线程安全性
配置类型 `BlckBar` 的一切方法都是线程安全的，这意味着你可以在进度条运行过程中修改其中的数据。

`BlockProgressBar` 只有 `tick()`、`tick_to()` 和 `reset()` 方法是线程安全的；其余方法，特别是 `iterate()` 是线程不安全的。

这表示进度条允许多个线程同时调用它的 `tick()`、`tick_to()` 和 `reset()` 方法；但这不包括移动赋值、移动构造和交换两个对象，以及 `iterate()` 方法。

并且绝对不应该在进度条运行过程中尝试移动或交换当前对象。
#### 绑定的输出流
`BlockProgressBar` 默认向标准错误流 `stderr` 输出字符串；具体绑定的输出流由第一个模板参数定义。

```cpp
#include "pgbar/pgbar.hpp"
#include <type_traits>

int main()
{
  static_assert( std::is_same<pgbar::BlockProgressBar<>,
                              pgbar::BlockProgressBar<pgbar::Channel::Stderr>>::value,
                 "" );

  pgbar::BlockProgressBar<pgbar::Channel::Stdout> bar; // 绑定到 stdout 上
}
```

特别需要注意的是，绑定到相同输出流上的对象不允许同时运行，否则会抛出异常 `pgbar::exception::InvalidState`；关于这一点的详细说明见 [FAQ-渲染器设计](#渲染器设计)。

如果指向的输出流并没有被绑定到终端上，那么这个进度条并不会被渲染；这一点可以详见 [全局设置-输出流检测](#输出流检测)。
### 与可迭代类型的交互
在处理一些可迭代类型、或者是数值范围的迭代任务时，`pgbar` 提供了一个更简单地迭代手段：`iterate()` 方法。

这个方法的使用与 Python 中的 `range()` 函数类似，它可以同时在由数值指定的范围上，和由可迭代类型划定的范围内进行遍历；具体的任务数量会由 `iterate()` 自己配置。

具体的迭代手段可以是使用 Enhanced-for，也可以像 `std::for_each` 一样传递一个一元函数。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
using namespace std;

int main()
{
  pgbar::BlockProgressBar<> bar;

  // Iteration range: [100, 0), step: -1
  for ( auto num : bar.iterate( 100, 1, -1 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [0.0, -2.0), step: -0.01
  for ( auto fnum : bar.iterate( -2.0, -0.01 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [100, 1), step: -1
  bar.iterate( 100, []( int ) { this_thread::sleep_for( 100ms ); } );
}
```

在前两个 Enhanced-for 中，`iterate()` 方法返回的实际上是 `pgbar::scope::ProxySpan`；关于这一类型的介绍可以见[辅助类型-`ProxySpan`](#proxyspan)。

有关数值类型范围的使用，可以见[辅助类型-`NumericSpan`](#numericspan)。

除了在数值范围内工作之外，`BlockProgressBar` 也可以与存在迭代器的类型进行交互，例如 `std::vector` 和原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::BlockProgressBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1.begin(), arr1.end() ) ) {
    ele += 1; // 此处的 ele 是对 vector 内元素的引用
    this_thread::sleep_for( 300ms );
  }
  // 可以逆序遍历
  bar.iterate( arr2 + ( sizeof( arr2 ) / sizeof( int ) ) - 1, arr2 - 1, []( int& ) {
    this_thread::sleep_for( 300ms );
  } );
}
```

有关可迭代类型的范围划定，可以见[辅助类型-`IterSpan`](#iterspan)。

特别的，如果一个类型提供了返回迭代器的 `begin()` 和 `end()` 方法，那么这个类型还可以以更简单的方式进行遍历；这包括了原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::BlockProgressBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1 ) ) {
    ele += 1;
    this_thread::sleep_for( 300ms );
  }
  // 数组也没问题
  bar.iterate( arr2, []( int& ) { this_thread::sleep_for( 300ms ); } );
}
```

- - -

## `ScannerBar`
![scbar](../images/scannerbar.gif)
### 交互方式
`pgbar::ScannerBar` 是一个模板类型；它不关心具体的任务数量，因此不需要为它配置任务数量也能够使用。

如果需要显示具体的任务数量，也可以通过调用 `config().tasks()` 方法并传递参数完成，利用 `pgbar::option::Tasks` 包装类型传递给构造函数同样也行。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  {
    pgbar::ScannerBar<> bar;
    bar.tick(); // no problem
  }
  {
    pgbar::ScannerBar<> bar;
    bar.config().tasks( 200 );

    bar.tick( 20 );    // 前进 20 步
    bar.tick_to( 50 ); // 将进度设置为 50%

    for ( int i = 0; i < 100; ++i )
      bar.tick(); // 每次调用仅前进 1 步
  }
  {
    pgbar::ScannerBar<> bar { pbar::option::Tasks( 150 ) };
    bar.tick_to( 20 );  // 将进度设置为 20%
    bar.tick_to( 130 ); // 超出 100% 的部分会被丢弃，并将进度条进度锁定到 100%
  }
}
```

在一些特别的场景中，如果想要检查进度条运行情况，或是强行终止进度条的运行，那么可以使用 `is_running()` 和 `reset()` 方法。

注意：因为 `ScannerBar` 允许在任务数量为零的情况下启动，因此这种情况下 `ScannerBar` 不会知道它应该在什么时候自动停止。

这也意味着如果需要停止一个无任务数量的 `ScannerBar`，必须手动调用 `reset()` 方法。

> 也可以让 `ScannerBar` 因超出作用域、被析构而停止运行，但并不建议这样做。

```cpp
#include "pgbar/pgbar.hpp"
#include <cassert>

int main()
{
  pgbar::ScannerBar<> bar;

  bar.tick();
  // 要注意，只有调用一次 tick() 后进度条才开始运行
  assert( bar.is_running() );

  assert( bar.progress() == 0 ); // 该方法可以获取进度条当前的迭代数
  bar.reset();
  assert( bar.is_running() == false );
}
```

`ScannerBar` 是一个 move only 且 swappable 的类型，所以你可以使用另一个对象移动构造，或者与另一个对象交换彼此的配置数据。

```cpp
{
  pgbar::ScannerBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::ScannerBar<> bar2 { std::move( bar1 ) };
}
{
  pgbar::ScannerBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::ScannerBar<> bar2;
  bar2.swap( bar1 );
  // or
  using std::swap;
  swap( bar1, bar2 );
}
```

但不允许在进度条运行过程中交换或移动对象，否则会导致不可预知的错误。

```cpp
pgbar::ScannerBar<> bar1;

bar1.tick();
assert( bar1.is_running() );

// pgbar::ScannerBar<> bar2 { std::move( bar1 ) }; No!
```
### 配置选项
正如前面一节中提到的，`ScannerBar` 的所有配置操作都需要经由方法 `config()` 完成。

该方法返回的是对内部配置对象的引用，该配置对象的类型可以在 `pgbar::config` 中找到，它是 `pgbar::config::ScanBar`。

`pgbar::config::ScanBar` 是一个数据类型，该类型存储了所有用于描述 `ScannerBar` 元素的数据成员；它满足可复制、可移动和可交换三个性质。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::ScanBar cfg1;

  auto cfg2 = cfg1;              // copy
  auto cfg3 = std::move( cfg1 ); // move
  cfg3.swap( cfg2 );             // swap
  // or
  using std::swap;
  swap( cfg2, cfg3 );
}
```
#### 元素构成
`ScannerBar` 由以下几种元素组成：

```text
{LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
```

其中可以自定义的部分有：`LeftBorder`、`Description`、`Starting`、`Filler`、`Lead`、`Ending`、`Speed` 和 `RightBorder`，它们的功能与名字相同。

这些元素可以直接在 `pgbar::option` 中找到对应的包装类型：

```cpp
pgbar::option::Style;   // 决定以上元素哪些需要被渲染
pgbar::option::Colored; // 开关颜色效果
pgbar::option::Bolded;  // 开关字体加粗效果

pgbar::option::LeftBorder;  // 修改整个进度条左侧的起始边框
pgbar::option::RightBorder; // 修改整个进度条右侧的终止边框

pgbar::option::Description; // 修改任务描述信息
pgbar::option::TrueMesg;    // 修改进度条结束时，用于替换 Description 部分的元素
pgbar::option::FalseMesg;   // 修改进度条结束时，用于替换 Description 部分的元素

pgbar::option::Starting;  // 修改进度条块左侧、Percent 右侧的元素
pgbar::option::Ending;    // 修改进度条块右侧、Counter 左侧的元素
pgbar::option::Filler;    // 修改进度条背景的填充字符
pgbar::option::Lead;      // 修改可变动画部分的各个帧
pgbar::option::Shift;     // 调整动画部分（Lead）的动画速度
pgbar::option::BarLength; // 调整进度条的长度

pgbar::option::SpeedUnit; // 修改 Speed 部分的单位
pgbar::option::Magnitude; // 调整 Speed 部分的进位倍率

pgbar::option::Tasks;   // 调整任务数量
pgbar::option::Divider; // 修改位于两个元素之间的间隔符

pgbar::option::DescColor;    // 修改 Description 的颜色
pgbar::option::TrueColor;    // 修改 TrueMesg 的颜色
pgbar::option::FalseColor;   // 修改 FalseMesg 的颜色
pgbar::option::StartColor;   // 修改 Starting 的颜色
pgbar::option::EndColor;     // 修改 Ending 的颜色
pgbar::option::FillerColor;  // 修改 Filler 的颜色
pgbar::option::LeadColor;    // 修改 Lead 的颜色
pgbar::option::InfoColor;    // 修改 Divider、Percent、Counter、Speed、Elapsed 和 Countdown 的颜色
```

> `TrueMesg` 和 `FalseMesg` 可以用来显示进度条迭代的任务执行是否成功，它们可以通过向 `reset()` 方法中传递一个 `bool` 参数实现切换显示；在默认情况下，进度条自行停止、或调用无参的 `reset()` 方法会选中 `TrueMesg`。

`pgbar::option::Style` 的参数可以由 `pgbar::config::ScanBar` 内的多个静态成员执行位运算得到：

```cpp
pgbar::ScannerBar<> bar { pgbar::option::Style( pgbar::config::ScanBar::Sped | pgbar::config::ScanBar::Per
                                                | pgbar::config::ScanBar::Elpsd
                                                | pgbar::config::ScanBar::Cntdwn ) };
```

这种方法配置会显得很繁琐，所以 `ScanBar` 提供了两个辅助方法 `enable()` 和 `disable()` 用于简化以上操作。

```cpp
pgbar::ScannerBar<> bar;
bar.config().enable().speed().percent().elapsed().countdown();
// or
bar.config().enable().entire();
bar.config().disable().animation().counter();
// Animation 指的是来回扫描的进度条
// 并且不是所有元素都可以被关闭，例如 Description 就不行
```

以上元素在 `ScanBar` 都有同名方法，调用这些方法并向其中传递参数同样能做到修改数据信息。
##### 可变的进度条长度
在元素 `Starting` 和 `Ending` 中间部分的是被称作 `Animation` 的扫描进度条（不包括 `Starting` 和 `Ending`），这个扫描进度条的长度是可变的。

`pgbar` 本身极少探测与具体平台有关的信息，例如终端的宽度等；因此如果希望进度条能够填满一行，或者进度条太长需要缩窄，就需要使用到 `bar_length()` 方法或 `pgbar::option::BarLength` 包装器更改扫描进度条的长度。

对于后者，直接使用对应的接口调整参数即可；而前者则需要一个辅助方法获取除了扫描进度条之外部分的长度，才能正确计算得到恰好能让进度条占满一行的长度。

这个方法就是 `config().fixed_size()`。

```cpp
pgbar::ScannerBar<> bar;
assert( bar.config().bar_length() == 30 ); // 默认值
assert( bar.config().fixed_size() != 0 );  // 具体值取决于数据成员的内容
```
#### 数据配置
`ScanBar` 有两种数据配置方法：基于包装器类型的可变参数构造，和基于链式调用的流式接口风格。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::ScanBar config1 {
    pgbar::option::SpeedUnit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } ),
    pgbar::option::Magnitude( 1024 ),
    pgbar::option::InfoColor( "#39C5BB" )
    // pgbar::option::InfoColor(0x39C5BB) Don't do that!
  };
  // 注意：传入相同的包装器类型两次会导致编译错误

  pgbar::config::ScanBar config2;
  config2.speed_unit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } )
    .magnitude( 1024 )
    .info_color( "#39C5BB" );

  auto config3 = config2; // 构造完毕后也能使用可变模板参数调整
  config3.set( pgbar::option::Description( "Do something" ), pgbar::option::DescColor( 0xFFE211 ) );
}
```

尽管配置类型可以在进度条运行过程中被修改，但这一概念并不适用于任务数量；也就是说进度条一旦开始运行，它的任务数量就不可中途改变。

```cpp
#include "pgbar/pgbar.hpp

int main()
{
  pgbar::ScannerBar<> pbar;

  pbar.config().tasks( 100 );
  for ( auto i = 0; i < 100; ++i ) {
    pbar.tick();
    if ( i == 30 ) // nothing happens
      pbar.config().tasks( 50 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  }
}
```
#### 线程安全性
配置类型 `ScanBar` 的一切方法都是线程安全的，这意味着你可以在进度条运行过程中修改其中的数据。

`ScannerBar` 只有 `tick()`、`tick_to()` 和 `reset()` 方法是线程安全的；其余方法，特别是 `iterate()` 是线程不安全的。

这表示进度条允许多个线程同时调用它的 `tick()`、`tick_to()` 和 `reset()` 方法；但这不包括移动赋值、移动构造和交换两个对象，以及 `iterate()` 方法。

并且绝对不应该在进度条运行过程中尝试移动或交换当前对象。
#### 绑定的输出流
`ScannerBar` 默认向标准错误流 `stderr` 输出字符串；具体绑定的输出流由第一个模板参数定义。

```cpp
#include "pgbar/pgbar.hpp"
#include <type_traits>

int main()
{
  static_assert( std::is_same<pgbar::ScannerBar<>,
                              pgbar::ScannerBar<pgbar::Channel::Stderr>>::value,
                 "" );

  pgbar::ScannerBar<pgbar::Channel::Stdout> bar; // 绑定到 stdout 上
}
```

特别需要注意的是，绑定到相同输出流上的对象不允许同时运行，否则会抛出异常 `pgbar::exception::InvalidState`；关于这一点的详细说明见 [FAQ-渲染器设计](#渲染器设计)。

如果指向的输出流并没有被绑定到终端上，那么这个进度条并不会被渲染；这一点可以详见 [全局设置-输出流检测](#输出流检测)。
### 与可迭代类型的交互
在处理一些可迭代类型、或者是数值范围的迭代任务时，`pgbar` 提供了一个更简单地迭代手段：`iterate()` 方法。

这个方法的使用与 Python 中的 `range()` 函数类似，它可以同时在由数值指定的范围上，和由可迭代类型划定的范围内进行遍历；具体的任务数量会由 `iterate()` 自己配置。

具体的迭代手段可以是使用 Enhanced-for，也可以像 `std::for_each` 一样传递一个一元函数。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
using namespace std;

int main()
{
  pgbar::ScannerBar<> bar;

  // Iteration range: [100, 0), step: -1
  for ( auto num : bar.iterate( 100, 1, -1 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [0.0, -2.0), step: -0.01
  for ( auto fnum : bar.iterate( -2.0, -0.01 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [100, 1), step: -1
  bar.iterate( 100, []( int ) { this_thread::sleep_for( 100ms ); } );
}
```

在前两个 Enhanced-for 中，`iterate()` 方法返回的实际上是 `pgbar::scope::ProxySpan`；关于这一类型的介绍可以见[辅助类型-`ProxySpan`](#proxyspan)。

有关数值类型范围的使用，可以见[辅助类型-`NumericSpan`](#numericspan)。

除了在数值范围内工作之外，`ScannerBar` 也可以与存在迭代器的类型进行交互，例如 `std::vector` 和原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::ScannerBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1.begin(), arr1.end() ) ) {
    ele += 1; // 此处的 ele 是对 vector 内元素的引用
    this_thread::sleep_for( 300ms );
  }
  // 可以逆序遍历
  bar.iterate( arr2 + ( sizeof( arr2 ) / sizeof( int ) ) - 1, arr2 - 1, []( int& ) {
    this_thread::sleep_for( 300ms );
  } );
}
```

有关可迭代类型的范围划定，可以见[辅助类型-`IterSpan`](#iterspan)。

特别的，如果一个类型提供了返回迭代器的 `begin()` 和 `end()` 方法，那么这个类型还可以以更简单的方式进行遍历；这包括了原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::ScannerBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1 ) ) {
    ele += 1;
    this_thread::sleep_for( 300ms );
  }
  // 数组也没问题
  bar.iterate( arr2, []( int& ) { this_thread::sleep_for( 300ms ); } );
}
```

- - -

## `SpinnerBar`
![spbar](../images/spinnerbar.gif)
### 交互方式
`pgbar::SpinnerBar` 是一个模板类型；它不关心具体的任务数量，因此不需要为它配置任务数量也能够使用。

如果需要显示具体的任务数量，也可以通过调用 `config().tasks()` 方法并传递参数完成，利用 `pgbar::option::Tasks` 包装类型传递给构造函数同样也行。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  {
    pgbar::SpinnerBar<> bar;
    bar.tick(); // no problem
  }
  {
    pgbar::SpinnerBar<> bar;
    bar.config().tasks( 200 );

    bar.tick( 20 );    // 前进 20 步
    bar.tick_to( 50 ); // 将进度设置为 50%

    for ( int i = 0; i < 100; ++i )
      bar.tick(); // 每次调用仅前进 1 步
  }
  {
    pgbar::SpinnerBar<> bar { pbar::option::Tasks( 150 ) };
    bar.tick_to( 20 );  // 将进度设置为 20%
    bar.tick_to( 130 ); // 超出 100% 的部分会被丢弃，并将进度条进度锁定到 100%
  }
}
```

在一些特别的场景中，如果想要检查进度条运行情况，或是强行终止进度条的运行，那么可以使用 `is_running()` 和 `reset()` 方法。

注意：因为 `SpinnerBar` 允许在任务数量为零的情况下启动，因此这种情况下 `SpinnerBar` 不会知道它应该在什么时候自动停止。

这也意味着如果需要停止一个无任务数量的 `SpinnerBar`，必须手动调用 `reset()` 方法。

> 也可以让 `SpinnerBar` 因超出作用域、被析构而停止运行，但并不建议这样做。

```cpp
#include "pgbar/pgbar.hpp"
#include <cassert>

int main()
{
  pgbar::SpinnerBar<> bar;

  bar.tick();
  // 要注意，只有调用一次 tick() 后进度条才开始运行
  assert( bar.is_running() );

  assert( bar.progress() == 0 ); // 该方法可以获取进度条当前的迭代数
  bar.reset();
  assert( bar.is_running() == false );
}
```

`SpinnerBar` 是一个 move only 且 swappable 的类型，所以你可以使用另一个对象移动构造，或者与另一个对象交换彼此的配置数据。

```cpp
{
  pgbar::SpinnerBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::SpinnerBar<> bar2 { std::move( bar1 ) };
}
{
  pgbar::SpinnerBar<> bar1 { /* 传递一些复杂的配置数据 */ };
  pgbar::SpinnerBar<> bar2;
  bar2.swap( bar1 );
  // or
  using std::swap;
  swap( bar1, bar2 );
}
```

但不允许在进度条运行过程中交换或移动对象，否则会导致不可预知的错误。

```cpp
pgbar::SpinnerBar<> bar1;

bar1.tick();
assert( bar1.is_running() );

// pgbar::SpinnerBar<> bar2 { std::move( bar1 ) }; No!
```
### 配置选项
正如前面一节中提到的，`SpinnerBar` 的所有配置操作都需要经由方法 `config()` 完成。

该方法返回的是对内部配置对象的引用，该配置对象的类型可以在 `pgbar::config` 中找到，它是 `pgbar::config::SpinBar`。

`pgbar::config::SpinBar` 是一个数据类型，该类型存储了所有用于描述 `SpinnerBar` 元素的数据成员；它满足可复制、可移动和可交换三个性质。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::SpinBar cfg1;

  auto cfg2 = cfg1;              // copy
  auto cfg3 = std::move( cfg1 ); // move
  cfg3.swap( cfg2 );             // swap
  // or
  using std::swap;
  swap( cfg2, cfg3 );
}
```
#### 元素构成
`SpinnerBar` 由以下几种元素组成：

```text
{LeftBorder}{Lead}{Description}{Percent}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
```

其中可以自定义的部分有：`LeftBorder`、`Lead`、`Description`、`Speed` 和 `RightBorder`，它们的功能与名字相同。

这些元素可以直接在 `pgbar::option` 中找到对应的包装类型：

```cpp
pgbar::option::Style;   // 决定以上元素哪些需要被渲染
pgbar::option::Colored; // 开关颜色效果
pgbar::option::Bolded;  // 开关字体加粗效果

pgbar::option::LeftBorder;  // 修改整个进度条左侧的起始边框
pgbar::option::RightBorder; // 修改整个进度条右侧的终止边框

pgbar::option::Description; // 修改任务描述信息
pgbar::option::TrueMesg;    // 修改进度条结束时，用于替换 Description 部分的元素
pgbar::option::FalseMesg;   // 修改进度条结束时，用于替换 Description 部分的元素

pgbar::option::Lead;      // 修改可变动画部分的各个帧
pgbar::option::Shift;     // 调整动画部分（Lead）的动画速度

pgbar::option::SpeedUnit; // 修改 Speed 部分的单位
pgbar::option::Magnitude; // 调整 Speed 部分的进位倍率

pgbar::option::Tasks;   // 调整任务数量
pgbar::option::Divider; // 修改位于两个元素之间的间隔符

pgbar::option::DescColor;    // 修改 Description 的颜色
pgbar::option::TrueColor;    // 修改 TrueMesg 的颜色
pgbar::option::FalseColor;   // 修改 FalseMesg 的颜色
pgbar::option::LeadColor;    // 修改 Lead 的颜色
pgbar::option::InfoColor;    // 修改 Divider、Percent、Counter、Speed、Elapsed 和 Countdown 的颜色
```

> `TrueMesg` 和 `FalseMesg` 可以用来显示进度条迭代的任务执行是否成功，它们可以通过向 `reset()` 方法中传递一个 `bool` 参数实现切换显示；在默认情况下，进度条自行停止、或调用无参的 `reset()` 方法会选中 `TrueMesg`。

`pgbar::option::Style` 的参数可以由 `pgbar::config::SpinBar` 内的多个静态成员执行位运算得到：

```cpp
pgbar::SpinnerBar<> bar { pgbar::option::Style( pgbar::config::SpinBar::Sped | pgbar::config::SpinBar::Per
                                                | pgbar::config::SpinBar::Elpsd
                                                | pgbar::config::SpinBar::Cntdwn ) };
```

这种方法配置会显得很繁琐，所以 `SpinBar` 提供了两个辅助方法 `enable()` 和 `disable()` 用于简化以上操作。

```cpp
pgbar::SpinnerBar<> bar;
bar.config().enable().speed().percent().elapsed().countdown();
// or
bar.config().enable().entire();
bar.config().disable().animation().counter();
// Animation 指的是左侧的动画组件 Lead
// 并且不是所有元素都可以被关闭，例如 Description 就不行
```

以上元素在 `SpinBar` 都有同名方法，调用这些方法并向其中传递参数同样能做到修改数据信息。
#### 数据配置
`SpinBar` 有两种数据配置方法：基于包装器类型的可变参数构造，和基于链式调用的流式接口风格。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::config::SpinBar config1 {
    pgbar::option::SpeedUnit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } ),
    pgbar::option::Magnitude( 1024 ),
    pgbar::option::InfoColor( "#39C5BB" )
    // pgbar::option::InfoColor(0x39C5BB) Don't do that!
  };
  // 注意：传入相同的包装器类型两次会导致编译错误

  pgbar::config::SpinBar config2;
  config2.speed_unit( { "B/s", "kiB/s", "MiB/s", "GiB/s" } )
    .magnitude( 1024 )
    .info_color( "#39C5BB" );

  auto config3 = config2; // 构造完毕后也能使用可变模板参数调整
  config3.set( pgbar::option::Description( "Do something" ), pgbar::option::DescColor( 0xFFE211 ) );
}
```

尽管配置类型可以在进度条运行过程中被修改，但这一概念并不适用于任务数量；也就是说进度条一旦开始运行，它的任务数量就不可中途改变。

```cpp
#include "pgbar/pgbar.hpp

int main()
{
  pgbar::SpinnerBar<> pbar;

  pbar.config().tasks( 100 );
  for ( auto i = 0; i < 100; ++i ) {
    pbar.tick();
    if ( i == 30 ) // nothing happens
      pbar.config().tasks( 50 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  }
}
```
#### 线程安全性
配置类型 `SpinBar` 的一切方法都是线程安全的，这意味着你可以在进度条运行过程中修改其中的数据。

`SpinnerBar` 只有 `tick()`、`tick_to()` 和 `reset()` 方法是线程安全的；其余方法，特别是 `iterate()` 是线程不安全的。

这表示进度条允许多个线程同时调用它的 `tick()`、`tick_to()` 和 `reset()` 方法；但这不包括移动赋值、移动构造和交换两个对象，以及 `iterate()` 方法。

并且绝对不应该在进度条运行过程中尝试移动或交换当前对象。
#### 绑定的输出流
`SpinnerBar` 默认向标准错误流 `stderr` 输出字符串；具体绑定的输出流由第一个模板参数定义。

```cpp
#include "pgbar/pgbar.hpp"
#include <type_traits>

int main()
{
  static_assert( std::is_same<pgbar::SpinnerBar<>,
                              pgbar::SpinnerBar<pgbar::Channel::Stderr>>::value,
                 "" );

  pgbar::SpinnerBar<pgbar::Channel::Stdout> bar; // 绑定到 stdout 上
}
```

特别需要注意的是，绑定到相同输出流上的对象不允许同时运行，否则会抛出异常 `pgbar::exception::InvalidState`；关于这一点的详细说明见 [FAQ-渲染器设计](#渲染器设计)。

如果指向的输出流并没有被绑定到终端上，那么这个进度条并不会被渲染；这一点可以详见 [全局设置-输出流检测](#输出流检测)。
### 与可迭代类型的交互
在处理一些可迭代类型、或者是数值范围的迭代任务时，`pgbar` 提供了一个更简单地迭代手段：`iterate()` 方法。

这个方法的使用与 Python 中的 `range()` 函数类似，它可以同时在由数值指定的范围上，和由可迭代类型划定的范围内进行遍历；具体的任务数量会由 `iterate()` 自己配置。

具体的迭代手段可以是使用 Enhanced-for，也可以像 `std::for_each` 一样传递一个一元函数。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
using namespace std;

int main()
{
  pgbar::SpinnerBar<> bar;

  // Iteration range: [100, 0), step: -1
  for ( auto num : bar.iterate( 100, 1, -1 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [0.0, -2.0), step: -0.01
  for ( auto fnum : bar.iterate( -2.0, -0.01 ) ) {
    this_thread::sleep_for( 100ms );
  }
  // Iteration range: [100, 1), step: -1
  bar.iterate( 100, []( int ) { this_thread::sleep_for( 100ms ); } );
}
```

在前两个 Enhanced-for 中，`iterate()` 方法返回的实际上是 `pgbar::scope::ProxySpan`；关于这一类型的介绍可以见[辅助类型-`ProxySpan`](#proxyspan)。

有关数值类型范围的使用，可以见[辅助类型-`NumericSpan`](#numericspan)。

除了在数值范围内工作之外，`SpinnerBar` 也可以与存在迭代器的类型进行交互，例如 `std::vector` 和原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::SpinnerBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1.begin(), arr1.end() ) ) {
    ele += 1; // 此处的 ele 是对 vector 内元素的引用
    this_thread::sleep_for( 300ms );
  }
  // 可以逆序遍历
  bar.iterate( arr2 + ( sizeof( arr2 ) / sizeof( int ) ) - 1, arr2 - 1, []( int& ) {
    this_thread::sleep_for( 300ms );
  } );
}
```

有关可迭代类型的范围划定，可以见[辅助类型-`IterSpan`](#iterspan)。

特别的，如果一个类型提供了返回迭代器的 `begin()` 和 `end()` 方法，那么这个类型还可以以更简单的方式进行遍历；这包括了原始数组。

```cpp
#include "pgbar/pgbar.hpp"
#include <thread>
#include <vector>
using namespace std;

int main()
{
  pgbar::SpinnerBar<> bar;

  vector<int> arr1 {
    0, 1, 2, 3, 4, 5, 6,
  };
  int arr2[] { 100, 99, 98, 97, 96, 95, 94, 93, 92, 91 };

  for ( auto& ele : bar.iterate( arr1 ) ) {
    ele += 1;
    this_thread::sleep_for( 300ms );
  }
  // 数组也没问题
  bar.iterate( arr2, []( int& ) { this_thread::sleep_for( 300ms ); } );
}
```

- - -

# 进度条合成器
## `MultiBar`
![mtbar](../images/multibar.gif)
### 交互方式
`pgbar::MultiBar` 是一个 tuple-like 类型，它可以接收多个独立进度条类型，并将它们组合起来，实现多进度条的输出。

`MultiBar` 要求它持有的所有对象都必须指向同一个输出流。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::MultiBar<pgbar::ProgressBar<>, pgbar::ProgressBar<>, pgbar::BlockProgressBar<>> bar1;
  // or
  pgbar::MultiBar<pgbar::ProgressBar<pgbar::Channel::Stdout>,
                  pgbar::ProgressBar<pgbar::Channel::Stdout>,
                  pgbar::BlockProgressBar<pgbar::Channel::Stdout>>
    bar2;

  mbar.config<0>().tasks( 100 );
  mbar.config<1>().tasks( 200 );
  mbar.config<2>().tasks( 300 );

  // 不带模板参数的方法表示访问 MultiBar 对象本身
  assert( mbar.is_running() );

  // do tasks...
}
```

`MultiBar` 的构造函数可以接受独立进度条对象，也可以接受这些进度条对象的配置类型。

如果使用的 C++ 标准高于 C++17，`pgbar` 还为 `MultiBar` 添加了一个类型模板推导指引。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  // 得益于数据对象和进度条类型分离的设计，所以你完全可以传递一个输出流指向不同的进度条对象
  // MultiBar 只要求它的模板参数列表中的所有类型，必须具有相同输出流属性
  pgbar::ProgressBar<> bar1;
  pgbar::BlockProgressBar<pgbar::Channel::Stdout> bar2, bar3;

  // 因为 MultiBar 只会访问进度条的配置数据类型，所以不对进度条对象使用 std::move 也是没问题的
  auto mbar1 =
    pgbar::MultiBar<pgbar::ProgressBar<>, pgbar::BlockProgressBar<>, pgbar::ProgressBar<>>( std::move( bar1 ),
                                                                                            std::move( bar2 ),
                                                                                            bar3 );
  auto mbar2 = pgbar::MultiBar<pgbar::ProgressBar<>, pgbar::BlockProgressBar<>, pgbar::ProgressBar<>>(
    pgbar::config::CharBar(),
    pgbar::config::BlckBar(),
    pgbar::config::CharBar() );

#if __cplusplus >= 201703L
  // 如果在 C++17 之后，以下语句将是合法的
  auto mbar3 =
    pgbar::MultiBar( pgbar::config::CharBar(), pgbar::config::BlckBar(), pgbar::config::CharBar() );
  // 这个对象的类型将会是指向 pgbar::Channel::Stderr 的 MultiBar

  static_assert( std::is_same<decltype( mbar3 ), decltype( mbar2 )>::value, "" );
#endif
}
```

独立进度条的所有方法都可以在 `MultiBar` 中以模板函数的方式访问；某种程度上来说，`MultiBar` 更像是一个容器而非进度条类型。

与独立进度条类型相同，`MultiBar` 也是一个 movable 且 swappable 的类型；并且也同样不应该在 `MultiBar` 运行过程中移动或交换它。

特别需要注意的是，如果调用 `MultiBar` 自己的 `reset()` 方法，那么所有归属于该 `MultiBar` 对象的独立进度条都会立刻终止运行，这种终止运行的效果等价于析构这些独立进度条，但这里不会真的析构它们。

析构导致的进度条终止效果可以见[FAQ-进度条对象的生命周期](#进度条对象的生命周期)。
### 辅助函数
`pgbar` 提供了多个名为 `make_multi` 的重载函数，以简化构造 `MultiBar` 的类型构造操作。

这些函数及其作用分别是：

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  // 创建与参数数量相同大小的 MultiBar
  auto bar1 = pgbar::make_multi<pgbar::Channel::Stdout>( pgbar::config::CharBar(), pgbar::config::BlckBar() );
  auto bar2 =
    pgbar::make_multi<>( pgbar::ProgressBar<pgbar::Channel::Stdout>(), pgbar::BlockProgressBar<>() );

  // 创建一个固定长度、所有进度条类型都相同的 MultiBar，并使用参数提供的配置对象初始化内部所有进度条对象
  auto bar3 = pgbar::make_multi<6, pgbar::Channel::Stdout>( pgbar::config::SpinBar() );
  auto bar4 = pgbar::make_multi<6>( pgbar::SpinnerBar<pgbar::Channel::Stdout>() );
  // bar3 和 bar4 内部的所有进度条的配置数据都是相同的

  // 创建一个固定长度、所有进度条类型都相同的 MultiBar，提供的参数会按顺序作用在内部的进度条对象上
  auto bar5 = pgbar::make_multi<3, pgbar::config::ScanBar>( pgbar::config::ScanBar() );
  auto bar6 = pgbar::make_multi<3, pgbar::ScannerBar<pgbar::Channel::Stdout>>( pgbar::ScannerBar<>() );
  // bar5 和 bar6 只有第一个进度条对象被初始化为参数指定的内容，其他两个进度条均被默认初始化
}
```

- - -

# 全局设置
## 着色效果
`pgbar` 依赖于 ANSI 转义代码实现终端光标的位置控制、字符颜色着色和字体加粗效果；而因为不存在一种通用的、检测本地终端环境是否支持这项功能的方法，所以这需要由用户（也就是你）负责检查并保证使用的终端能够正常处理 ANSI 转义序列。

`pgbar` 支持使用 RGB 值定义渲染的进度条颜色。这里的 RGB 值可以是十六进制颜色代码字符串，如 `#123456` 和 `#7D1`；也可以是直接写成字面量形式的十六进制整数值，如 `0x123456`。

尝试传递错误的十六进制颜色代码字符串会导致异常 `pgbar::exception::InvalidArgument` 抛出。

对于一些不支持着色效果的终端，`pgbar` 允许使用宏开关 `PGBAR_COLORLESS` 关闭全局的 RGB 支持；但这不会影响传入错误的 RGB 字符串时，抛出异常 `pgbar::exception::InvalidArgument` 的行为。

除此之外，每个独立进度条的配置类型都允许使用方法 `colored()` 单独关闭该对象的着色效果。

## 输出流检测
考虑到 `stdout`、`stderr` 会被重定向到其他文件中，因此 `pgbar` 还会在开始渲染进度条之前检查当前程序的输出流是否绑定到终端上。

`pgbar` 支持检查 Windows 和 unix-like（实际上是支持 POSIX 标准的）平台的输出流绑定；对于非 Windows 和 unix-like 平台，`pgbar` 将无法识别输出流是否绑定在终端上。

当 `pgbar` 发现某个输出流并不指向终端时，所有指向该输出流的进度条都不会对外输出任何信息，但是内部的异常检查工作是正常进行的。

你也可以调用在名称空间 `pgbar::config` 中的 `intty()` 方法检查某个输出流是否指向终端。

```cpp
#include "pgbar/pgbar.hpp"
#include <iostream>

int main()
{
  if ( pgbar::config::intty( pgbar::Channel::Stdout ) )
    std::cout << "Standard output is bound to a terminal." << std::endl;
  else
    std::cout << "Standard output is not bound to a terminal." << std::endl;
}
```

## 渲染器工作间隔
为了提升性能表现，后台渲染器会停顿一个固定的时间间隔后再执行渲染任务，这个时间间隔的大小可以经由 `pgbar::config::refresh_interval()` 方法获取和修改。

`pgbar::config::refresh_interval()` 使用的时间单位被定义在 `pgbar::config::TimeUnit` 中；一般来说，它是类型 `std::chrono::nanoseconds` 的别名。

调用该方法是完全线程安全的，因此可以在任意进度条运行过程中调整渲染器的停顿间隔。

因为在不同的输出流上都有一个单独的渲染器，因此 `pgbar::config::refresh_interval()` 被定义为一个模板函数，你需要传入具体的 `pgbar::Channel` 值决定修改哪个输出流上渲染器的工作间隔。

```cpp
pgbar::config::refresh_interval<pgbar::Channel::Stderr>(
  std::chrono::milliseconds( 20 ) ); // Increase the refresh rate from the default 25 Hz to 60 Hz
```

由于单次渲染工作的执行时间不可能为零，因此若将工作间隔下调为零，则表示渲染器永不停顿；这会带来更高的视觉流畅度以及处理器负载。

若仅需修改动画组件的视觉流畅度，可以使用 `pgbar::option::Shift` 类型和配置类型中的 `shift()` 方法进行调整。

## 断言检查
`pgbar` 使用 `<cassert>` 中的 `assert` 在代码中插入了多处断言检查，这些断言仅会在定义宏 `PGBAR_DEBUG`、且开启标准库断言时生效。

绝大多数的断言都是为了在内部组件中确认某些参数的有效性，仅有少数断言会被放置在诸如构造函数和赋值运算符等位置，这些断言用于检查当前对象状态是否符合预期。

例如，`pgbar` 不允许任何进度条对象在其方法 `is_running()` 返回 `true` 时调用 `operator=()` 或 `swap()` 函数，因此这些位置的断言有助于检查程序中是否存在这样的非法情况。

自赋值操作同样会被断言检查并拒绝。

- - -

# 辅助类型
## `NumericSpan`
`pgbar::scope::NumericSpan` 是一个模板类型，它被用于表达一个数值范围的起始点、终止点和步长；该数值范围在数学上被表示为：`[start, end)`。

在构造过程、或者更改成员数值过程中，以下情况会导致异常 `pgbar::exception::InvalidArgument` 抛出：
1. 起点大于终点，而步长是正数；
2. 起点小于终点，而步长是负数；
3. 步长为零。

### 成员方法
`NumericSpan` 有以下几个方法：

```cpp
iterator begin() const noexcept;          // 返回一个指向数值范围起点的迭代器
iterator end() const noexcept;            // 返回一个指向数值范围终点的迭代器
NumericSpan& step( N step );              // 更改当前步长，会对传入参数进行检查
NumericSpan& start_value( N startpoint ); // 更改起始点数值，会对传入参数进行检查
NumericSpan& end_value( N endpoint );     // 更改终止点数值，会对传入参数进行检查 

N start_value() const noexcept;     // 返回当前起始点数值
N end_value() const noexcept;       // 返回当前终止点数值
N step() const noexcept;            // 返回当前步长
/* size_t */ size() const noexcept; // 返回当前数值范围的步数

void swap( NumericSpan& ) noexcept; // 交换两个数值范围
```
### 迭代器类型
`NumericSpan::iterator` 属于前向迭代器，重载了包括但不限于 `operator++()`、`operator++( int )`、`operator+=()`、`operator*()` 和判等运算符在内的运算符函数。

迭代器的有效迭代次数与 `NumericSpan` 的方法 `size()` 返回的值相同；特别的，如果步长大于数值范围，那么迭代器在前进一步后的值将会超出数值范围的终止点。

## `IterSpan`
`pgbar::scope::IterSpan` 是一个模板类型，它被用于表达两个迭代器所划定的抽象范围；可以被视作是 `std::views::ref_view` 的极度简化版本。

`IterSpan` 要求传入的迭代器类型必须可以复制构造或移动构造，并且必须能够计算两个迭代器对象之间的距离，否则会导致编译失败。

`IterSpan` 为指针类型提供了单独的特化版本，与主模板相比，指针类型允许直接倒置传入起始点和终止点，以表示逆序范围；而主模板只能接受逆序迭代器才能实现。

```cpp
#include "pgbar/pgbar.hpp"
#include <vector>

int main()
{
  int arr1[50] = {};
  std::vector<int> arr2;

  auto reverse_span1 = pgbar::scope::IterSpan<int*>( arr1 + 49, arr1 - 1 );
  auto reverse_span2 =
    pgbar::scope::IterSpan<std::reverse_iterator<std::vector<int>::iterator>>( arr2.rbegin(), arr2.rend() );
}
```

在指针类型的特化版本中，如果传入的起止点的任意一个为空指针，那么会导致异常 `pgbar::exception::InvalidArgument` 抛出。
### 成员方法
`IterSpan` 有以下几个方法：

```cpp
iterator begin() const noexcept;       // 返回一个指向抽象范围起点的迭代器
iterator end() const noexcept;         // 返回一个指向抽象范围终点的迭代器
IterSpan& start_value( I startpoint ); // 更改起点迭代器
IterSpan& end_value( I endpoint );     // 更改终点迭代器

/* I&, const I&, I&& */ start_iter() /* &, const&, && */ noexcept; // 根据当前对象的值语义返回内部的起点迭代器
/* I&, const I&, I&& */ end_iter() /* &, const&, && */ noexcept;   // 根据当前对象的值语义返回内部的终点迭代器
/* size_t */ step() const noexcept;                                // 返回当前步长，通常是编译期常数 1
/* size_t */ size() const noexcept;                                // 返回当前抽象范围的大小

void swap( IterSpan& ) noexcept; // 交换两个抽象范围
```
### 迭代器类型
`IterSpan::iterator` 属于前向迭代器，重载了包括但不限于 `operator++()`、`operator++( int )`、`operator+=()`、`operator*()` 和判等运算符在内的运算符函数。

由于是前向迭代器且没有提供自减运算符，因此所有逆序操作都依赖于迭代器类型实现；对于特化版本，则依赖于构造过程中传递的指针顺序。

## `ProxySpan`
`pgbar::scope::ProxySpan` 是一个可空的模板类型，它用于表达某个独立进度条的迭代范围。

`ProxySpan` 只能接受 `NumericSpan` 或 `IterSpan` 类型，和一个独立进度条对象；它的作用是为了简化进度条实例与 Enhanced-for 等需要使用迭代器的场景的交互。

这是一个 move-only 的特殊类型，它只应该被工厂函数，如进度条的 `iterate()` 方法，构造并返回，而不应该手动构造。

调用 `ProxySpan` 的 `begin()` 方法会导致 `ProxySpan` 对象尝试根据内部的抽象范围大小，对其持有的进度条实例设置任务数量。
### 成员方法
`ProxySpan` 有以下几个方法：

```cpp
iterator begin() & noexcept;   // 为内部的进度条实例赋值，并返回起始迭代器
iterator end() const noexcept; // 返回终止迭代器
bool empty() const noexcept;   // 检查当前对象是否指向了一个有效的进度条实例
explicit operator bool();      // 根据求值语境转换为 bool 值

void swap( ProxySpan& ) noexcept; // 交换两个代理范围
```
### 迭代器类型
`ProxySpan::iterator` 属于前向迭代器，该迭代器的自增运算符会尝试调用与之绑定的进度条实例的 `tick()` 方法，因此会在意料之外的场景触发副作用。

- - -

# FAQ
## 更新计数与任务总数一致性
`pgbar` 中的进度条类型会在第一次调用任意一个 `tick()` 方法时启动，在调用 `tick()` 所产生的已完成任务数量恰好达到预定任务数量时，进度条类型就会自动停止。

因此，`pgbar` 需要由用户自己确保：调用任意 `tick()` 所产生的累计任务数量严格等于预定任务数量。

如果调用次数过多，那么有可能会导致进度条对象的意外重启；过少则可能会导致进度条没有正确停止。

> 即使进度条因为更新计数与任务总数不一致而持续运行，它也会因为超出生命周期被析构而强制终止。

## 进度条对象的生命周期
每个进度条对象的生命周期都服从于 C++ 标准的对象生命周期机制：
- 在局部作用域创建的对象，会在控制流离开该作用域时被析构；
- 被动态创建的对象，它的生命周期从 `new` 开始，到 `delete` 终止。

这里之所以提及生命周期问题，是因为进度条在被析构过程中，会无视当前迭代进度立即终止运行。

这种强制性终止与调用 `reset()` 方法停止不同：`reset()` 方法允许进度条在停止时，根据传递的参数，使用预先定义好的 `TrueMesg` 或 `FalseMesg` 替换元素 `Description` 所在位置的内容；而析构导致的终止则不会执行这个过程，而是立即关闭与之关联的全局渲染器并清理资源。

因析构而停止的进度条不会向终端再追加任何信息，因此这可能会导致一定程度上的终端渲染混乱。

## Unicode 支持
`pgbar` 默认所有传入的字符串都以 UTF-8 格式编码；使用任何不以 UTF-8 编码的字符串都会有以下四种结果：
1. 被认为是不完整的 UTF-8 字符串，并抛出 `pgbar::exception::InvalidArgument` 异常；
2. 被认为是部分字节存在错误的破损 UTF-8 字符串，同样抛出 `pgbar::exception::InvalidArgument` 异常；
3. 被认为是非标准的 UTF-8 字符串，行为同上；
4. 被错误认为是 UTF-8 字符串，无异常抛出。

`pgbar` 仅处理有关字符类型的 Unicode 编码，并不会在运行时主动更改终端编码环境；用户（也就是你）必须自行确保当前终端的字符集编码使用的是 UTF-8。

如果使用的 C++ 标准在 C++20 以上，那么 `pgbar` 也能接受标准库的 `std::u8string` 和 `std::u8string_view` 等类型；但是不会在 C++20 之外的标准接受字面量 UTF-8 字符串，也就是这种类型：`u8"This is a UTF-8 literal string"`.

## 渲染器设计
`pgbar` 采用了多线程协作模式设计，因此渲染器实际上是一个在后台工作的子线程；并且 `pgbar` 的渲染器采用了单例模式设计。

具体来说，每个独立进度条的 `tick()` 和 `tick_to()` 等方法都会被视作是一次状态更新，并将状态变迁作用到进度条类型内部的原子量上；在每个进度条实例的全局第一次 `tick()` 调用时，都会向 `pgbar` 的全局单例渲染器派发一个任务，并在迭代结束后清空这个任务。

派发任务后，进度条实例会启动渲染器，在这期间调用 `tick()` 方法的线程会循环等待后台渲染线程启动；同理，当进度条实例关闭渲染器时也会等待后台渲染线程挂起。

进度条实例可以工作在不同的输出流上，所以全局单例的渲染器也被分为了指向 `stdout` 和 `stderr` 的两个单独实例；它们之间互不影响且不存在依赖关系。

在全局范围内，`pgbar` 要求同一时刻，指向同一个输出流的情况下，只能有一个进度条实例向全局渲染器派发任务。

如果在同一作用域内创建了多个进度条对象并先后发起任务，则最先派发任务的进度条会成功工作，而后续尝试派发任务的进度条会因全局渲染器已被占用而在其任务调用处抛出 `pgbar::exception::InvalidState` 异常。

在多线程环境下，哪个线程是“最先派发任务”的线程要取决于具体的线程调度策略。

```cpp
#include "pgbar/pgbar.hpp"
#include <iostream>

int main()
{
  {
    pgbar::ProgressBar<> bar1;
    pgbar::ScannerBar<> bar2;
    pgbar::SpinnerBar<pgbar::Channel::Stdout> bar3;

    bar1.config().tasks( 100 );
    bar1.tick();

    try {
      bar2.tick(); // Oops!
    } catch ( const pgbar::exception::InvalidState& e ) {
      std::cerr << std::endl << e.what() << std::endl;
    }

    bar3.tick(); // Ok!
  }

  pgbar::ProgressBar<> bar;
  bar.config().tasks( 100 );

  bar.tick(); // Ok!
}
```

> 在这段代码中，首先创建了三个不同类型的进度条对象：
>
> `bar1` 通过 `bar1.config().tasks( 100 )` 成功配置任务数量，并调用 `bar1.tick()` 向全局渲染器派发任务。
> 
> 接着，`bar2` 调用 `bar2.tick()` 时，由于全局渲染器已被 `bar1` 占用，因而触发了异常。
> 
> 而 `bar3` 则能正常调用 `bar3.tick()`，这是因为它所使用的输出流与前两个进度条不同，不会与已被占用的全局渲染器产生冲突。
> 
> 此外，当代码块结束后，之前占用全局渲染器的进度条对象被销毁，随后在全局范围内新建的 `ProgressBar` 对象再次能够正常派发任务；也就是说，全局渲染器在前一个进度条生命周期结束后恢复为可用状态。

如果有多进度条输出需求，请使用 [`pgbar::MultiBar`](#multibar)。

## 异常传播机制
`pgbar` 内涉及了非常多的动态内存分配申请，因此在大部分复制拷贝/构造和默认初始化过程中，标准库的异常都有可能被抛出。

`pgbar` 会在内部自行处理 IO 过程，所以在不同平台下也会有一些不同的异常检查机制。

如果在 Windows 平台下，`pgbar` 无法获取到当前进程的标准输出流 Handle，那么会抛出一个本地系统错误异常 `pgbar::exception::SystemError`。

若后台渲染线程接收到了一个抛出的异常，它会将这个异常存储在内部的异常类型容器中，并终止当前渲染工作；等待下一次前台线程尝试启动或者挂起渲染线程时，这个被捕获异常会在调用点重新被抛出。

如果渲染线程的异常容器已经存在了一个未处理的异常，此时线程内部又再次抛出了一个异常，那么渲染线程将会进入凋亡（`dead`）状态；在这个状态下，新的异常不会被捕获，而是任其传播直至渲染线程终止。

在凋亡状态下，重新尝试启动后台渲染线程将会尝试创建一个新的渲染线程对象；这个过程中，上一次未被处理的异常将会在新的渲染线程创建完毕、且开始工作之前抛出。

在不与渲染线程发生交互的上下文中，异常遵循 C++ 标准机制抛出并传递。

## 设计架构
### 基础数据结构设计
从性能角度出发，`pgbar` 在内部编写了许多针对性优化的数据结构，包括但不限于模板元编程组件、简化版的 `std::move_only_function`、根据 C++ 标准提供不同实现的数值格式化函数、绕过标准库缓冲区的 IO 函数等。

这些组件只适用于 `pgbar` 自身，`pgbar` 不对外做任何可用性和兼容性保证。
### 进度条类型设计
受到[这篇文章](https://zhuanlan.zhihu.com/p/106672814)的启发，`pgbar` 的独立进度条使用了 Mixin 模式组合继承自内部的多个不同模板基类，这些基类都采用 CRTP 设计，因此在配置类型中可以以链式调用的形式顺序访问不同的配置方法。

为了避免 Mixin 组合继承时引入多继承导致基类构造顺序不确定、且 C++ 虚继承情况下不允许使用 CRTP 设计的问题，`pgbar` 使用了一个编译期拓扑排序算法生成最终的组合继承结构。

这个拓扑排序算法类似于 Python 中的 C3 线性化算法，但与之不同的是：C3 线性化算法的目的是在类继承结构中寻找到一个最合适的类方法，而这里的拓扑排序算法则是直接线性化整个继承结构，将一个复杂的多继承结构在编译期内线性化为一条继承链。

但它们有一点是相似的：在最派生类处的方法解析满足基类继承顺序的局部优先原则；也就是说在继承过程中，位置在继承列表中靠左的基类会优先被放到更靠近派生类的位置上，以确保这类基类的方法不会被更靠右的基类所覆盖。

此外，该拓扑排序算法还有一个特点：对于存在非虚拟继承关系的两个类，在排序后会被尽可能的放在最靠近彼此的位置上；这一点是为了保证在非虚拟继承中，派生类可以直接引用基类而无需进行复杂的模板嵌套工作。

具体工作原理可以参照前文提及的文章，我只是在该文章提出的原理的基础上自行实现了一个能够同时解析虚拟继承和非虚拟继承关系，并且满足一部分 C3 线性化算法特征的拓扑排序算法。
