## 特点
- **Header-only**: 所有功能都包含在 `include/pgbar` 下的单个 `.hpp` 文件中。
- **低更新开销**: 每次更新进度条进度的开销成本极低。
- **RGB 支持**: 可通过十六进制 RGB 值定制进度条颜色。
- **可选的线程安全**: 可以通过模板参数调节进度条的线程安全性。
- **类 `tqdm` 接口**: 通过模板技巧提供了友好的迭代方式。
- **Modern C++ 规范**: （基本上）遵循 Modern C++ 的最佳实践。
- **基础单元测试**: 使用 `Catch2` 框架进行基本测试。

[有关 Unicode 的支持见此](#支持-unicode-吗)。

## 样式
```
{startpoint}{done}{todo}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~  Progress  ~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~  Status  ~~~~~~~~~~~~~~~~~~~~~~~~~~~^

[-------                       ] [  24.00% |  48/200 |  31.00 Hz  | 00:00:01 < 00:00:04 ]
```
![example-color](images/example_color.gif)

> 图中的 "Hz" 表示每秒更新进度条的频次。

可以利用 `pgbar::colors` 中预定义的多个常量，或者自己传递一个十六进制的 RGB 色值更改进度条的颜色。

![example-rgb](images/example_rgb.gif)
### 用例
```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::ProgressBar<> bar { pgbar::options::TodoChar( "-" ),
                             pgbar::options::DoneChar( "=" ),
                             pgbar::options::Styles( pgbar::configs::Progress::Entire ),
                             pgbar::options::TodoColor( "#A52A2A" ),
                             pgbar::options::DoneColor( 0x0099FF ),
                             pgbar::options::StatusColor( pgbar::colors::Yellow ),
                             pgbar::options::Tasks( 100 ) };

  for ( auto _ = 0; _ < 100; ++_ )
    bar.tick();
}
```

![example-usage](images/example_usage.gif)

更多用例详见 [QuickStart_zh.md](QuickStart_zh.md) 及 [demo.cpp](demo/demo.cpp)。

## FAQ
### 进度条的更新工作会拖慢程序本身吗？
不，正如[特点](#特点)中指出的，进度条的更新是*基本上*零开销。

在开启 `O2/O3` 优化的情况下，下面代码中的第二个迭代的性能开销会*趋近于*上面迭代的开销[^1]。

[^1]: 实际上进度条更新的代码会仅多出一次函数调用跳转、以及条件分支的开销。

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  std::size_t count = 0;
  for ( std::size_t _ = 0; _ < 2147483647; ++_ )
    ++count;

  pgbar::ProgressBar<> bar { pgbar::options::Tasks( 2147483647 ) };
  for ( std::size_t _ = 0; _ < 2147483647; ++_ )
    bar.tick();
}
```

不过显示效果流畅与否，就取决于使用的处理器的单核性能了。
### 可以运行在 Windows/Linux 上吗？
没问题，我写这个库的初衷就是想在两个系统上同时使用一个可视化迭代进度的库。
### 支持 Unicode 吗？
答案有些微妙：可以既是肯定的，也是否定的。

**肯定的方面：** 从进度条的渲染机制上来看，进度条不知道它输出的字符串的字符编码是什么，它只知道在某个终端行内不断输出字符串以刷新画面。所以如果你提交了一个 UTF 字符串、甚至是 emoji 表情，这完全没问题，在正常的情况下这工作得很好。

**否定的方面：** 因为我根本没有编写字符串的字符编解码方案，所以当然，进度条是以字节为单位识别传递给它的字符串的。这就会导致：同时使用多字节渲染的 UTF 字符（如 CJK 字符和 emoji）和 ASCII 字符时，终端的渲染效果可能会不尽如人意；在这种情况下往往需要你手动将 ASCII 字符与 UTF 的字符宽度对齐。

举例来说，如果进度条的 “todo” 是一个双字节宽度的 emoji “🎈”，你所提供的 “undo” 必须也是一个双字节宽度的 ASCII 字符，例如两个空格。

你可以在 [unicode.cpp](demo/unicode.cpp) 中查看具体的 Unicode 渲染示例。

## 许可
项目遵从 [MIT](LICENSE) 许可。
