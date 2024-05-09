# pgbar - ProgressBar for C++11

**Contents**  
- [pgbar - ProgressBar for C++11](#pgbar---progressbar-for-c11)
  - [Styles](#styles)
  - [How to use](#how-to-use)
    - [Construct](#construct)
    - [Use](#use)
  - [Public Functions](#public-functions)
    - [pgbar/pgbar.hpp](#pgbarpgbarhpp)
    - [pgbar/range.hpp](#pgbarrangehpp)
  - [Instructions for Use](#instructions-for-use)
  - [FAQ](#faq)
    - [Will it slow down the program?](#will-it-slow-down-the-program)
    - [Can it be used on Windows/Linux?](#can-it-be-used-on-windowslinux)
    - [What kind of C++ version is required?](#what-kind-of-c-version-is-required)
    - [What distinguishes it from other C++ progress bars on GitHub?](#what-distinguishes-it-from-other-c-progress-bars-on-github)
- [pgbar - ProgressBar for C++11 - zh\_cn](#pgbar---progressbar-for-c11---zh_cn)
  - [风格样式](#风格样式)
  - [使用例](#使用例)
    - [构造](#构造)
    - [使用](#使用)
  - [公有方法](#公有方法)
    - [pgbar/pgbar.hpp](#pgbarpgbarhpp-1)
    - [pgbar/range.hpp](#pgbarrangehpp-1)
  - [使用须知](#使用须知)
  - [FAQ](#faq-1)
    - [会拖慢程序吗？](#会拖慢程序吗)
    - [能在 Windows/Linux 上使用吗？](#能在-windowslinux-上使用吗)
    - [需要什么样的 C++ 版本？](#需要什么样的-c-版本)
    - [与 Github 上的其他 C++ 进度条有什么区别？](#与-github-上的其他-c-进度条有什么区别)

A simple tqdm-like, header-only progress bar for C++11 and higher standards.

No third party dependencies required.

## Styles
```
{startpoint}{done char}{todo char}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~~~~~ Progress bar ~~~~~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Status bar ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^

[-------                       ] [  24.00% |  48/200 |  31.00 Hz  | 00:00:01 < 00:00:04 ]
```
![example-color](images/example_color.gif)
> The "Hz" indicates how many tasks are performed per second.

The status bar is colored using escape sequences by default.

If your terminal doesn't support this or don't want the coloring effect, define a macro at the beginning of the program.

You can also use multiple constants from pgbar::dye to customize the color of the status bar.
```cpp
#define PGBAR_NOT_COL // This will cancel the character coloring of the status bar.
#include "pgbar/pgbar.hpp"
// ...
```
![example-text](images/example_text.gif)

## How to use
### Construct

```cpp
#include "pgbar/pgbar.hpp"
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
      std::cerr, // The stream object must be provided.
      pgbar::initr::option( pgbar::style::percentage ),
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
      pgbar::initr::option( pgbar::style::percentage ),
      pgbar::initr::todo_char( "-" ),
      pgbar::initr::done_char( "=" ),
      pgbar::initr::left_status( "" ),
      pgbar::initr::right_status( "" ) /// and so on...
    );
  }
}
```
For more infomation, see `demo/how-to-use.cpp`.
### Use
```cpp
#include "pgbar/range.hpp" // it will automatically include pgbar/pgbar.hpp
int main()
{
  { // Using the pgbar object by calling method `update()`
    pgbar::pgbar<> bar;
    bar.set_task( 500 ).set_step( 2 );
    // Modify the progress bar presentation using predefined values in pgbar::style
    // It is a typically bit vector.
    bar.set_style( pgbar::style::percentage | pgbar::style::rate );
    for ( auto _ = 0; _ < 250; ++_ ) {
      bar.update();
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
    bar.reset(); // If you want to use the pgbar object later again, you must invoke the method `reset()`.
  }
  { // Using the pgbar object by calling function `range()` which is in pgbar/range.hpp
    static int arr[200] {}; // Supports raw arrays, or containers with `begin()` and `end()` methods.
    pgbar::pgbar<> bar;
    for ( auto ele : pgbar::range( arr, bar ) )
      continue; // `range()` will return references to elements within the container.
    bar.reset(); // ditto
  }
  { // Using numeric types to define iteration intervals with `range()`.
    pgbar::pgbar<> bar;
    for ( auto ele : pgbar::range( 100, bar ) ) // The progress bar iterates within the range [0, 100).
      continue; // `range()` will return the numerical value of the current iteration progress.
    bar.reset(); // ditto
  }
}
```
For more infomation, see `demo/how-to-use.cpp` and `demo/sample.cpp`.

The progress bar information switches provided in `pgbar::style` are as follows:
```cpp
style {
  bar;
  percentage;
  task_counter;
  rate;
  timer;
  entire;
};
```
Additionally, you can use the given constants in `pgbar::dye` along with the member methods `set_todo_col()`, `set_done_col()`, and `set_status_col()` to set the color effects of the progress bar characters and the status information bar.

The color options provided in `pgbar::dye` are as follows:
```cpp
dye {
  none;
  black;
  red;
  green;
  yellow;
  blue;
  magenta;
  cyan;
  white;
};
```

## Public Functions
### pgbar/pgbar.hpp
The header file provides several methods related to the progress bar object `pgbar`, including template type predicates and member methods of the object itself.
```cpp
#include "pgbar/pgbar.hpp"
using namespace pgbar; // Ignore namespace prefix

/* Template type predicate, checks if the given type is a pgbar object */
template<typename B>
struct is_pgbar { constexpr bool value; };

/* Template type predicate, checks if the given type is a renderer object accepted by pgbar */
template<typename R>
struct is_renderer { constexpr bool value; };

/* Template type predicate, checks if the given type is acceptable by a pgbar object as an output stream object */
template<typename S>
struct is_stream { constexpr bool value; };

/* Notify the progress bar that it needs updating, return value is the current iteration progress. */
void pgbar::update();

/* Requests the progress bar to advance its iteration progress by next_step steps,
 * steps beyond the total number of tasks will be ignored; return value ditto. */
void pgbar::update(size_t next_step);

/* Set the iteration progress of the progress bar to the specified percentage, with parameter range [0, 100];
 * values outside this range will be treated as 100%; return value ditto. */
size_t update_to( size_t percentage );

/* Get the total number of tasks. */
size_t get_tasks() const noexcept;

/* Get the number of steps the iteration advances. */
size_t get_step() const noexcept;

/* Get the number of tasks that have been updated. */
size_t get_current();

/* Check if the progress bar object has been started. */
bool pgbar::is_updated() const noexcept;

/* Check if the progress bar object has finished updating. */
bool pgbar::is_done() const noexcept;

/* Set the number of tasks to be updated each time `update()` is called. */
pgbar& pgbar::set_step(size_t _step) noexcept;

/* Set the total number of tasks the progress bar needs to handle. */
pgbar& pgbar::set_task(size_t _total_tsk) noexcept;

/* Set the character for the unfilled part of the progress bar. */
pgbar& pgbar::set_done(std::string _done_ch);

/* Set the character used to fill the progress bar. */
pgbar& pgbar::set_todo(std::string _todo_ch);

/* Set the style of the left bracket of the progress bar, can be passed an empty string. */
pgbar& pgbar::set_startpoint(std::string _startpoint);

/* Set the style of the right side of the progress bar. */
pgbar& pgbar::set_endpoint(std::string _endpoint);

/* Set the style of the left parenthesis in the status bar, can be passd an empty string. */
pgbar& pgbar::set_lstatus(std::string _lstatus);

/* Set the style of the right side in the status bar. */
pgbar& pgbar::set_rstatus(std::string _rstatus);

/* Set the length of the progress bar, indicating how many characters the progress bar occupies after output. */
pgbar& pgbar::set_bar_length(size_t _length) noexcept;

/* Set the color of unfilled characters in the progress bar, using the given constant value from pgbar::dye.
 * This function has no effect when the macro PGBAR_NOT_COL is activated. */
pgbar& pgbar::set_todo_col(/* literal type */ _dye) noexcept;

/* Similarly, set the color of filled characters in the progress bar. */
pgbar& pgbar::set_done_col(/* literal type */ _dye) noexcept;

/* Also, set the color of the status information bar. */
pgbar& pgbar::set_status_col(/* literal type */ _dye) noexcept;

/* Use bitwise operations to set the information to be displayed based on multiple predefined options. */
pgbar& pgbar::set_style(pgbar::style::Type _selection) noexcept;

/* Use the types in pgbar::initr to configure different options. */
pgbar& pgbar::set_style( Args&&... args );
```
### pgbar/range.hpp
```cpp
#include "pgbar/range.hpp"
using namespace pgbar; // Ignore namespace prefix
// The following function signatures will hide unnecessary template parameters

/* Only accept integer and floating-point types */
range(_startpoint, _endpoint, _step, BarT& _bar)
range(_endpoint, _step, BarT& _bar)
/* The following two functions only accept integer types */
range(_startpoint, _endpoint, BarT& _bar)
range(_endpoint, BarT& _bar)

/* Functions accepting iterator types as range */
range(_startpoint, _endpoint, BarT& _bar) // Only supports iterators that move using increment/decrement operators

/* Accepts an iterable container type (including raw arrays) as a range,
 * requiring the container to use `iterator` as the iterator name */
range(container, BarT& _bar) // Raw arrays do not have iterator name requirements
```

## Instructions for Use
If `update()` is called again (directly or indirectly) without executing `reset()` after the progress bar has finished updating, it will throw the exception `bad_pgbar`.

Calling `update()` without setting the total number of tasks will also throw the exception `bad_pgbar`.

Attempting to update tasks with an incremental step size of 0 when calling `update()` will also throw the exception `bad_pgbar`.

When using `range`, specifying an incorrect range (such as an endpoint smaller than the starting point) will also throw the exception `bad_pgbar`.

The progress bar object considers the output stream object type and renderer type as part of the type parameters, with the default output stream object bound to `std::cerr`.

By default, the progress bar uses multithreaded rendering for terminal output, and you can switch rendering modes using `pgbar::singlethread` and `pgbar::multithread`.

After completing tasks, if you need to reset the progress bar style, you must execute `reset()` first, otherwise the change will not take effect.

Attempting to call non-`const` member methods while the progress bar is running is invalid but will not throw an exception.

## FAQ
### Will it slow down the program?
If using a multithreaded renderer, the answer is no. In the case of multithreaded rendering, the program consists of a notification thread (i.e., the main thread) and a rendering thread.

In that case, each update is handled by the main thread, which changes the internal iteration count of the progress bar (i.e., each call to `update()`).

The actual terminal display is handled by a background rendering thread.

The refresh rate of the rendering thread is designed to be approximately 25Hz.

By splitting threads, the updates of the progress bar itself do not affect the operation of the main thread.

Only at specific time points will the synchronization between the rendering thread and the main thread block the main thread (using spin locks), but this typically does not take up much time.

However, a single-threaded renderer will skip some refreshes at a fixed refresh rate (25Hz).

Each update of the progress bar will be executed by the main thread, so the performance will be much lower than multithreading, but the display effect will be smoother.
### Can it be used on Windows/Linux?
Yes, it can.  Since the code implementation only uses standard library features, it has good cross-platform compatibility.

Some components will enable the concepts feature when compiling with C++20, so make sure your compiler supports it.

The code also checks if its standard output is bound to a terminal, and if not, it will not output strings externally.

**However, if using an output stream object other than `std::ostream`, the program will not attempt to determine whether it is running in a terminal.**
### What kind of C++ version is required?
It requires C++11 and later versions of the C++ standard.
### What distinguishes it from other C++ progress bars on GitHub?
There's not much difference, except for the ability to customize progress bar characters more freely.

There's already [a better progress bar library](https://github.com/p-ranav/indicators) on GitHub, and if you want a more powerful progress bar, I recommend that one.

I wrote this because [tqdm.cpp](https://github.com/tqdm/tqdm.cpp) couldn't run on my machine, [progressbar](https://github.com/gipert/progressbar) was too slow, and [cpptqdm](https://github.com/aminnj/cpptqdm) couldn't run on Windows.

Just for practice, that's all.

- - -

# pgbar - ProgressBar for C++11 - zh_cn
一个简单的，适用于 C++11 及更高标准的 tqdm-like、header-only 进度条.

不需要第三方依赖.

## 风格样式
```
{startpoint}{done char}{todo char}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~~~~~ Progress bar ~~~~~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Status bar ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^

[-------                       ] [  24.00% |  48/200 |  31.00 Hz  | 00:00:01 < 00:00:04 ]
```
![example-color](images/example_color.gif)
> 这里的 'Hz' 表示每秒执行了多少次更新.

默认情况下会使用 ASCII 转义序列对状态栏进行染色，如果你的终端不支持、或者不希望有着色效果，可以在程序最开头定义一个宏.

也可以使用 `pgbar::dye` 中的多个常量自定义状态栏的颜色.
```cpp
#define PGBAR_NOT_COL // 这会取消状态栏染色.
#include "pgbar/pgbar.hpp"
// ...
```
![example-text](images/example_text.gif)

## 使用例
### 构造
```cpp
#include "pgbar/pgbar.hpp"
int main()
{
  { // 将包括任务数和步长在内的所有内容设置为默认值.
    pgbar::pgbar<> bar;
  }
  { // 使用一个满足谓词 `pgbar::is_stream` 约束的流对象构造对象.
    using Stream = std::ostream; // 默认情况下的流对象会被绑定在 `std::cerr` 上.
    static_assert(pgbar::is_stream<Stream>::value == true, "");
    pgbar::pgbar<Stream> bar { std::clog };
  }
  { // 使用一个满足谓词 `pgbar::is_renderer` 约束的渲染器类型构造对象.
    using Renderer = pgbar::singlethread; // 默认情况使用 `pgbar::multithread` 多线程渲染器
    static_assert(pgbar::is_renderer<Renderer>::value == true, "");
    pgbar::pgbar<std::ostream, Renderer> bar;
  }
  { // 构造时指定进度条的任务数和每次迭代的步长.
    constexpr size_t num_tasks = 0x7fffffff;
    pgbar::pgbar<> bar { num_tasks, 2 }; // 默认情况下步长是 1.
  }
  { // **构造后**再修改进度条的各项参数.
    pgbar::pgbar<> bar;
    bar.set_style( pgbar::style::bar )
       .set_todo( "-" )
       .set_done( "=" )
       .set_status_col( pgbar::dye::yellow ); // and so on...
  }
  { // **构造时**修改进度条的各项参数.
    pgbar::pgbar<> bar {
      std::cerr, // 必须指定流对象
      pgbar::initr::option( pgbar::style::percentage ),
      pgbar::initr::todo_char( "-" ),
      pgbar::initr::done_char( "=" ),
      pgbar::initr::left_status( "" ),
      pgbar::initr::right_status( "" ) /// and so on...
    };
    // 成员方法 `set_style` 也支持这种方式.
    bar.set_style(
      pgbar::initr::total_tasks( 300 ),
      pgbar::initr::status_color( pgbar::dye::green )
    );
  }
  { // 使用一个工厂函数构造进度条对象.
    auto bar = pgbar::make_pgbar<pgbar::singlethread>(
      std::cerr, // 必须指定流对象
      pgbar::initr::option( pgbar::style::percentage ),
      pgbar::initr::todo_char( "-" ),
      pgbar::initr::done_char( "=" ),
      pgbar::initr::left_status( "" ),
      pgbar::initr::right_status( "" ) /// and so on...
    );
  }
}
```
详细可见 `demo/how-to-use.cpp`.
### 使用
```cpp
#include "pgbar/range.hpp" // 该头文件会自动包含 pgbar/pgbar.hpp
int main()
{
  { // 构造对象并正确设置好参数后，调用方法 `update()` 开始推动进度条迭代
    pgbar::pgbar<> bar;
    bar.set_task( 500 ).set_step( 2 );
    // 可以使用 pgbar::style 中的预定值修改进度条呈现的信息，开关方式可以视作是使用一个位向量
    bar.set_style( pgbar::style::percentage | pgbar::style::rate );
    for ( auto _ = 0; _ < 250; ++_ ) {
      bar.update();
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
    bar.reset(); // 如果对应的进度条对象在后续还需要复用，则必须调用 `reset()` 方法重置.
  }
  { // 配合 pgbar/range.hpp 中的 `range()` 函数输出进度条信息.
    static int arr[200] {}; // 支持原始数组或含有 `begin()`、`end()` 方法的容器
    pgbar::pgbar<> bar;
    for ( auto ele : pgbar::range( arr, bar ) )
      continue; // `range()` 函数会返回容器内的元素引用
    bar.reset(); // 此处同上
  }
  { // `range()` 函数也支持使用数值类型在某一区间内迭代.
    pgbar::pgbar<> bar;
    for ( auto ele : pgbar::range( 100, bar ) ) // 进度条会在区间 [0, 100) 内迭代
      continue; // `range()` 函数会返回当前迭代进度的数值
    bar.reset(); // 此处同上
  }
}
```
详细可见 `demo/how-to-use.cpp` 及 `demo/sample.cpp`.

`pgbar::style` 中给定的进度条信息开关如下：
```cpp
style {
  bar;
  percentage;
  task_counter;
  rate;
  timer;
  entire;
};
```
此外，可以使用 `pgbar::dye` 中的给定常量与成员方法 `set_todo_col`, `set_done_col` 和 `set_status_col` 设置进度条的字符以及状态信息栏的颜色效果.

`pgbar::dye` 中给定的颜色选项如下：
```cpp
dye {
  none;
  black;
  red;
  green;
  yellow;
  blue;
  magenta;
  cyan;
  white;
};
```

## 公有方法
### pgbar/pgbar.hpp
头文件中提供了多个与进度条对象 `pgbar` 有关的方法，包括模板类型谓词及对象本身的成员方法.
```cpp
#include "pgbar/pgbar.hpp"
using namespace pgbar; // 忽略名称空间前缀

/* 模板类型谓词，检查给定类型是否是 pgbar 对象 */
template<typename B>
struct is_pgbar { constexpr bool value; };

/* 模板类型谓词，检查给定类型是否是 pgbar 能接受的渲染器对象 */
template<typename R>
struct is_renderer { constexpr bool value; };

/* 模板类型谓词，检查给定类型是否是渲染器能接受的输出流对象 */
template<typename S>
struct is_stream { constexpr bool value; };

/* 通知进度条该更新了，返回值为当前迭代进度，下同. */
size_t pgbar::update();

/* 要求进度条的迭代进度一次性前进 next_step 步，超出任务总数的步数会被忽略. */
size_t pgbar::update(size_t next_step);

/* 将进度条的迭代进度设置到指定的百分比，参数范围为 [0, 100]；超出范围的数值会被视作 100%. */
size_t update_to( size_t percentage );

/* 获取当前进度条对象的任务总数. */
size_t get_tasks() const noexcept;

/* 获取当前进度条对象的迭代步长. */
size_t get_step() const noexcept;

/* 获取当前进度条对象已被迭代的任务数. */
size_t get_current();

/* 检查进度条对象是否已经启动. */
bool pgbar::is_updated() const noexcept;

/* 检查进度条对象是否更新完毕. */
bool pgbar::is_done() const noexcept;

/* 设置每次调用 `update()` 时更新的任务数. */
pgbar& pgbar::set_step(size_t _step) noexcept;

/* 设置进度条需要处理的总任务数. */
pgbar& pgbar::set_task(size_t _total_tsk) noexcept;

/* 设置进度条中未被填充部分的字符. */
pgbar& pgbar::set_done(std::string _done_ch);

/* 设置用于填充进度条的字符. */
pgbar& pgbar::set_todo(std::string _todo_ch);

/* 设置进度条左侧的括号样式，可以传入一个空字符串. */
pgbar& pgbar::set_startpoint(std::string _startpoint);

/* 设置进度条右侧样式. */
pgbar& pgbar::set_endpoint(std::string _endpoint);

/* 设置状态栏左侧的括号样式，可以传入一个空字符串. */
pgbar& pgbar::set_lstatus(std::string _lstatus);

/* 设置状态栏右侧样式. */
pgbar& pgbar::set_rstatus(std::string _rstatus);

/* 设置进度条的长度，表示进度条在输出后占多少字符长. */
pgbar& pgbar::set_bar_length(size_t _length) noexcept;

/* 设置进度条中未填充字符的颜色，使用 pgbar::dye 中的给定常量值来设置.
 * 在宏 PGBAR_NOT_COL 激活时该函数没有实际效果. */
pgbar& pgbar::set_todo_col(/* literal type */ _dye) noexcept;

/* 同上，设置进度条中已填充字符的颜色. */
pgbar& pgbar::set_done_col(/* literal type */ _dye) noexcept;

/* 同上，设置状态信息栏的颜色. */
pgbar& pgbar::set_status_col(/* literal type */ _dye) noexcept;

/* 根据多个预定选项，使用位操作设定需要显示的信息. */
pgbar& pgbar::set_style(pgbar::style::Type _selection) noexcept;

/* 使用 pgbar::initr 中的类型设置不同选项 */
pgbar& pgbar::set_style( Args&&... args );
```
### pgbar/range.hpp
```cpp
#include "pgbar/range.hpp"
using namespace pgbar; // 忽略名称空间前缀
// 以下函数签名会隐藏不必要的模板参数

/* 只接受整型和浮点型 */
range(_startpoint, _endpoint, _step, BarT& _bar)
range(_endpoint, _step, BarT& _bar)
/* 以下两个函数只接受整数类型 */
range(_startpoint, _endpoint, BarT& _bar)
range(_endpoint, BarT& _bar)

/* 接受迭代器类型作为范围的函数 */
range(_startpoint, _endpoint, BarT& _bar) // 仅支持使用自增/自减运算符移动的迭代器

/* 接受一个可迭代的容器类型（包括原始数组）作为范围，要求容器使用 `iterator` 作为迭代器名称 */
range(container, BarT& _bar) // 原始数组没有 iterator 名称要求
```

## 使用须知
如果在进度条更新完成后，没有执行 `reset()` 的情况下（直接或间接地）再次调用 `update()`，会导致异常 `bad_pgbar` 抛出.

如果没有设置任务数就执行 `update()`，同样会抛出异常 `bad_pgbar`.

如果递进步数 `step` 为 0，调用 `update()` 尝试更新任务时也会抛出异常 `bad_pgbar`.

使用 `range` 时，指定了错误的范围（如结尾数值小于开头），同样会抛出异常 `bad_pgbar`.

进度条对象会将输出流对象类型、渲染器类型视作是类型参数的一部分，同时默认情况下的流对象被绑定在 `std::cerr` 上.

默认情况下，进度条会使用多线程渲染终端输出，可以使用 `pgbar::singlethread` 和 `pgbar::multithread` 切换渲染方式.

任务完成后，如果需要重设进度条风格，必须先执行 `reset()`，否则更改不会发生.

在进度条正在运行时，尝试调用非 `const` 成员方法是无效的，但不会抛出异常.

## FAQ
### 会拖慢程序吗？
如果使用的是多线程渲染器，答案是不会. 在多线程渲染的情况下，程序分别由通知线程（即主线程）和一个渲染线程组成.

每次更新时都由主线程负责改变进度条的内部迭代量（即每次调用 `update()` 的操作），实际的终端效果呈现是由后台的渲染线程负责的.

设计时渲染线程的刷新速率被设定为约 25Hz. 通过拆分线程，进度条的更新本身不会影响主线程的运行.

只有到了特定时间点，渲染线程和主线程之间的同步确认才会阻塞主线程（使用自旋锁），不过这通常不会占用很长时间.

但单线程渲染器只会按照固定刷新速率（25Hz）跳过部分刷新，每次进度条更新都会一定会由主线程执行，故性能会比多线程低很多，但显示效果会更流畅.
### 能在 Windows/Linux 上使用吗？
可以的，由于代码实现只使用了标准库功能，因此具有很好的跨平台兼容性.

部分组件在开启 C++20 编译标准时会启用 `concepts` 功能，请确保你的编译器支持.

代码还会判断自己的标准输出是否绑定在终端上，如果输出不在终端中显示则不会对外输出字符串.

**但是如果使用除了 `std::ostream` 以外的输出流对象，程序将不会尝试判断自己是否运行在终端中.**
### 需要什么样的 C++ 版本？
如题，支持 C++11 及以后的 C++ 标准.
### 与 Github 上的其他 C++ 进度条有什么区别？
嗯... 没有太大区别，除了支持更自由的进度条字符设定操作. Github 上已经有一个[更好的进度条库](https://github.com/p-ranav/indicators)了，想用功能更强大的进度条的话，我更推荐那个.

写这个是因为 [tqdm.cpp](https://github.com/tqdm/tqdm.cpp) 没法在我自己的机器上运行，还有 [progressbar](https://github.com/gipert/progressbar) 太慢，以及 [cpptqdm](https://github.com/aminnj/cpptqdm) 没法在 windows 上运行.

纯粹练手，就是这样.
