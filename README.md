# pgbar - ProgressBar for C++11

**Contents**  
- [pgbar - ProgressBar for C++11](#pgbar---progressbar-for-c11)
  - [Styles](#styles)
  - [Example](#example)
  - [Public API](#public-api)
    - [pgbar/pgbar.hpp](#pgbarpgbarhpp)
    - [pgbar/range.hpp](#pgbarrangehpp)
  - [NOTICE:](#notice)
  - [FAQ](#faq)
    - [Will it slow down my program?](#will-it-slow-down-my-program)
    - [Can it be used on Windows/Linux?](#can-it-be-used-on-windowslinux)
    - [What C++ version is required?](#what-c-version-is-required)
    - [What's the difference from other C++ progress bars on Github?](#whats-the-difference-from-other-c-progress-bars-on-github)
- [pgbar - ProgressBar for C++11 - zh\_cn](#pgbar---progressbar-for-c11---zh_cn)
  - [样式](#样式)
  - [使用例](#使用例)
  - [开放接口](#开放接口)
    - [pgbar/pgbar.hpp](#pgbarpgbarhpp-1)
    - [pgbar/range.hpp](#pgbarrangehpp-1)
  - [注意事项](#注意事项)
  - [FAQ](#faq-1)
    - [会拖慢程序吗？](#会拖慢程序吗)
    - [能在 Windows/Linux 上使用吗？](#能在-windowslinux-上使用吗)
    - [需要什么样的 C++ 版本？](#需要什么样的-c-版本)
    - [与 Github 上的其他 C++ 进度条有什么区别？](#与-github-上的其他-c-进度条有什么区别)

A simple tqdm-like, header-only progress bar for C++11 and higher standards.

No third party dependencies required.

## Styles
```
[###############                         ] [ 36.63%  |  36638968/100000000 |  21.27 MHz |   1s < 3s   ]
```
![example-color](images/example_color.gif)
> The "Hz" indicates how many tasks are performed per second.

The status bar is colored using escape sequences by default.

If your terminal doesn't support this or don't want the coloring effect, define a macro at the beginning of the program.
```cpp
#define PGBAR_NOT_COL // This will cancel the character coloring of the status bar.
#include "pgbar/pgbar.hpp"
// ...
```
![example-text](images/example_text.gif)

## Example
```cpp
#include "pgbar/pgbar.hpp"
#include "pgbar/range.hpp"

int main()
{
    std::size_t TOTAL = 10000;

    pgbar::pgbar bar;
    bar.set_task(TOTAL).set_step(2); // You can change the number of steps advanced with each `update()` call.
    for (std::size_t i = 0; i<(TOTAL/2); ++i) {
        bar.update(); // Insert an `update()` within the loop or within other task execution points.
        // Do anything you want here...
    }

    bar.reset(); // Reset the progress bar status.
    for (auto iter : pgbar::range(TOTAL, bar))
        continue; // When you don't want to call `update()`, you can use the `range from 'pgbar/range.hpp'.
    // Then hand over the update operations and the number of tasks to the iterator.

    // For more usage examples, please refer to sample.cpp.

    return 0;
}
```

## Public API
### pgbar/pgbar.hpp
```cpp
#include "pgbar/pgbar.hpp"
/* Notify the progress bar that it's time to update. */
void update()

/* The iterative progress of the progress bar will advance `next_step` steps at a time,
 * and steps that exceed the total number of tasks will be ignored. */
void update(std::size_t next_step)

/* Check whether the progress bar object has been started. */
bool check_update() const noexcept

/* Check whether the progress bar object has been updated. */
bool check_full() const noexcept

/* Setting the stream object for progress bar output.
 * Does not guarantee visibility when output to a file. */
pgbar& set_ostream(std::ostream& _ostream) noexcept

/* Setting the number of tasks to be updated each time `update()` is called. */
pgbar& set_step(std::size_t _step) noexcept

/* Setting the total number of tasks that the progress bar needs to handle. */
pgbar& set_task(std::size_t _total_tsk) noexcept

/* Setting the character for the unfilled portion of the progress bar. */
pgbar& set_done_char(std::string _done_ch)

/* Setting the character used to fill the progress bar. */
pgbar& set_todo_char(std::string _todo_ch)

/* Setting the style of the left side of the progress bar. Empty string is okay. */
pgbar& set_left_bracket(std::string _l_bracket)

/* Setting the style on the right side of the progress bar. Same as above. */
pgbar& set_right_bracket(std::string _r_bracket)

/* Setting the length of the progress bar. */
pgbar& set_bar_length(std::size_t _length) noexcept

/* Use bitwise operations to set the information to be displayed based on multiple predefined options. */
pgbar& set_style(pgbar::style_opts _selection) noexcept
```
### pgbar/range.hpp
```cpp
#include "pgbar/range.hpp"
// The following function signature will hide unnecessary template parameters.

/* Accept a numerical type as the range for `range`, non-numeric parameters will cause a function mismatch. */
range(_start, _end, _step, BarT& _bar)
range(_start, _end, BarT& _bar)
range(_end, BarT& _bar)

/* Accept an iterator type as the range for `range`. */
range(_start, _end, BarT& _bar) // Only iterators moved by increment/decrement operators are supported

/* Accepts an iterable container type (including original arrays) as a range,
 * requiring the container to use `iterator` as the iterator name. */
range(container, BarT& _bar) // Original arrays have no iterator name requirement.
```

## NOTICE:
1. If the progress bar is called again without executing `reset()` after the update is completed, it will result in throwing the `bad_pgbar` exception.
2. Executing `update()` without setting the number of tasks will also throw the `bad_pgbar` exception.
3. If the increment step `step` is set to 0, attempting to update the task with `update()` will also throw the `bad_pgbar` exception.
4. When using `range`, specifying an incorrect range (e.g., the ending value is less than the starting value) will also throw the `bad_pgbar` exception.
5. The progress bar uses `std::cerr` as the output stream object by default.
6. After the task is completed, if you need to reset the progress bar style, or the output stream object, you must first execute `reset()`, otherwise the change will not occur.

## FAQ
### Will it slow down my program?
No. The program structure consists of a notification thread (main thread) and a rendering thread.

During each update, the main thread handles the modification of the progress bar's internal iteration count (i.e., each call to `update()`), while the actual terminal display is managed by the background rendering thread.

The rendering thread's refresh rate is set to approximately 25Hz to ensure that progress bar updates do not impact the main thread's execution.

Synchronization between the rendering thread and the main thread only blocks the main thread briefly at specific time points, usually not consuming a significant amount of time.
### Can it be used on Windows/Linux?
The code implementation maintains excellent cross-platform compatibility as it exclusively relies on standard library functionalities.

Certain components utilize C++20 compilation standards to enable `concepts`; please ensure your compiler supports this feature.

The code also checks whether its standard output is bound to a terminal, refraining from external string output if not displayed in the terminal.

**If a `std::ostream` output stream object is used other than `std::cout` or `std::cerr`, the program will not attempt to determine whether it is running in a terminal.**
### What C++ version is required?
As mentioned, it supports C++11 and later C++ standards.

However, when using the range.hpp part, it may lead to longer compilation times due to extensive template matching operations.
### What's the difference from other C++ progress bars on Github?
There doesn't seem to be much difference, except for the support for more customizable progress bar character settings.

Github already has a [better progress bar library](https://github.com/p-ranav/indicators), if you want to use a more powerful progress bar, I'd recommend that one.

I wrote this for the reason that [tqdm.cpp](https://github.com/tqdm/tqdm.cpp) can't run on my machine, [progressbar](https://github.com/gipert/progressbar) is too slow, and [cpptqdm](https://github.com/aminnj/cpptqdm) can't run on Windows.

Purely for practice, that's how it is.
- - -

# pgbar - ProgressBar for C++11 - zh_cn
一个简单的，适用于 C++11 及更高标准的 tqdm-like、header-only 进度条.

不需要第三方依赖.

## 样式
```
[###############                         ] [ 36.63%  |  36638968/100000000 |  21.27 MHz |   1s < 3s   ]
```
![example-color](images/example_color.gif)
> 这里的 'Hz' 表示每秒执行了多少任务.

默认情况下会使用转义序列对状态栏进行染色，如果你的终端不支持、或者不想要着色效果，可以在程序最开头定义一个宏.
```cpp
#define PGBAR_NOT_COL // 这会取消状态栏染色.
#include "pgbar/pgbar.hpp"
// ...
```
![example-text](images/example_text.gif)

## 使用例
```cpp
#include "pgbar/pgbar.hpp"
#include "pgbar/range.hpp"

int main()
{
    std::size_t TOTAL = 10000;

    pgbar::pgbar bar;
    bar.set_task(TOTAL).set_step(2); // 可以更改每次调用 `update()` 时前进的步数
    for (std::size_t i = 0; i<(TOTAL/2); ++i) {
        bar.update(); // 在循环或者是其他任务执行的地方插入一个 `update()`
        // Do anything you want here...
    }

    bar.reset(); // 重置进度条状态
    for (auto iter : pgbar::range(TOTAL, bar)) // for (std::size_t i = 0; i < TOTAL; ++i) ...
        continue; // 不想调用 `update()` 时，可以使用 'pgbar/range.hpp' 中的 `range`
    // 把更新操作和任务数设置交给迭代器进行

    // 更多使用例详见 sample.cpp

    return 0;
}
```

## 开放接口
### pgbar/pgbar.hpp
```cpp
#include "pgbar/pgbar.hpp"
/* 通知进度条该更新了. */
void update()

/* 要求进度条的迭代进度一次性前进 next_step 步，超出任务总数的步数会被忽略. */
void update(std::size_t next_step)

/* 确认进度条对象是否已经启动. */
bool check_update() const noexcept

/* 检查进度条对象是否更新完毕. */
bool check_full() const noexcept

/* 设置进度条输出的流对象，不保证输出到文件中依然有可视性. */
pgbar& set_ostream(std::ostream& _ostream) noexcept

/* 设置每次调用 `update()` 时更新的任务数. */
pgbar& set_step(std::size_t _step) noexcept

/* 设置进度条需要处理的总任务数. */
pgbar& set_task(std::size_t _total_tsk) noexcept

/* 设置进度条中未填充部分的字符. */
pgbar& set_done_char(std::string _done_ch)

/* 设置用于填充进度条的字符. */
pgbar& set_todo_char(std::string _todo_ch)

/* 设置进度条左侧的括号样式，可以传入一个空字符串. */
pgbar& set_left_bracket(std::string _l_bracket)

/* 设置进度条右侧样式. */
pgbar& set_right_bracket(std::string _r_bracket)

/* 设置进度条的长度. */
pgbar& set_bar_length(std::size_t _length) noexcept

/* 根据多个预定选项，使用位操作设定需要显示的信息. */
pgbar& set_style(pgbar::style_opts _selection) noexcept
```
### pgbar/range.hpp
```cpp
#include "pgbar/range.hpp"
// 以下函数签名会隐藏不必要的模板参数

/* 接受数值类型作为范围的 `range`，非数值参数会导致函数匹配失败 */
range(_start, _end, _step, BarT& _bar)
range(_start, _end, BarT& _bar)
range(_end, BarT& _bar)

/* 接受迭代器类型作为范围的 `range` */
range(_start, _end, BarT& _bar) // 仅支持使用自增/自减运算符移动的迭代器

/* 接受一个可迭代的容器类型（包括原始数组）作为范围，要求容器使用 `iterator` 作为迭代器名称 */
range(container, BarT& _bar) // 原始数组没有 iterator 名称要求
```

## 注意事项
1. 如果在进度条更新完成后，没有执行 `reset()` 的情况下再次调用进度条，会导致异常 `bad_pgbar` 抛出.
2. 如果没有设置任务数就执行 `update()`，同样会抛出异常 `bad_pgbar`.
3. 如果递进步数 `step` 为 0，调用 `update()` 尝试更新任务时也会抛出异常 `bad_pgbar`.
4. 使用 `range` 时，指定了错误的范围（如结尾数值小于开头），同样会抛出异常 `bad_pgbar`.
5. 进度条默认使用 `std::cerr` 作为输出流对象.
6. 任务完成后，如果需要重设进度条风格、或是输出流对象，必须先执行 `reset()`，否则更改不会发生.

## FAQ
### 会拖慢程序吗？
不会. 程序结构分别由通知线程（即主线程）和一个渲染线程组成.

每次更新时都由主线程负责改变进度条的内部迭代量（即每次调用 `update()` 的操作），实际的终端效果呈现是由后台的渲染线程负责的.

设计时渲染线程的刷新速率被设定为约 25Hz. 通过拆分线程，进度条的更新本身不会影响主线程的运行.

只有到了特定时间点，渲染线程和主线程之间的同步确认才会阻塞主线程，不过这通常不会占用很长时间.
### 能在 Windows/Linux 上使用吗？
可以的，由于代码实现只使用了标准库功能，因此具有很好的跨平台兼容性.

部分组件在开启 C++20 编译标准时会启用 `concepts` 功能，请确保你的编译器支持.

代码还会判断自己的标准输出是否绑定在终端上，如果输出不在终端中显示则不会对外输出字符串.

**但是如果使用除了 `std::cout` 或 `std::cerr` 之外的 `std::ostream` 输出流对象，程序将不会尝试判断自己是否运行在终端中.**
### 需要什么样的 C++ 版本？
如题，支持 C++11 及以后的 C++ 标准.
### 与 Github 上的其他 C++ 进度条有什么区别？
嗯... 没有太大区别，除了支持更自由的进度条字符设定操作. Github 上已经有一个[更好的进度条库](https://github.com/p-ranav/indicators)了，想用功能更强大的进度条的话，我更推荐那个.

写这个是因为 [tqdm.cpp](https://github.com/tqdm/tqdm.cpp) 没法在我自己的机器上运行，还有 [progressbar](https://github.com/gipert/progressbar) 太慢，以及 [cpptqdm](https://github.com/aminnj/cpptqdm) 没法在 windows 上运行.

纯粹练手，就是这样.
