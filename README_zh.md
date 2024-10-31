## 特点
- **Header-only**: 所有功能都包含在 `include/pgbar` 下的单个 `.hpp` 文件中。
- **低更新开销**: 每次更新进度条进度的开销成本极低。
- **RGB 支持**: 可通过十六进制 RGB 值定制进度条颜色。
- **Unicode 支持**：支持包括但不限于使用 Unicode 编码的字符串。
- **可选的线程安全**: 可以通过模板参数调节进度条的线程安全性。
- **类 `tqdm` 接口**: 通过模板技巧提供了友好的迭代方式。
- **Modern C++ 规范**: （基本上）遵循 Modern C++ 的最佳实践。
- **基础单元测试**: 使用 `Catch2` 框架进行基本测试。

## 样式
```
{startpoint}{done char}{todo char}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~~~~~~  Progress  ~~~~~~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~  Status  ~~~~~~~~~~~~~~~~~~~~~~~~~~~^

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

更多用例详见 [QuickStart_zh.md](QuickStart_zh.md) 及 [usage.cpp](demo/usage.cpp)。

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
正常来说没问题，只要你传递的 UTF 字符串编码正确、并且也能够被你的终端正确渲染。

从渲染机制上看，进度条只关心渲染开始前的初始终端光标位置，每次渲染都会让光标跳回这个位置再执行渲染。所以这是一个与具体字符编码无关的过程，即使你传递的字符串并非 Unicode 编码也是没问题的。

唯一的缺点是进度条要求你的终端支持解析 ANSI 转义序列，否则它无法正常工作。

> 不过我认为现在大多数终端都支持解析转义序列（只要你的终端中的字符能够被染色），所以这个问题应该不大。

你可以在 [demo/unicode.cpp](demo/unicode.cpp) 中查看具体的 Unicode 渲染示例。

## 许可
项目遵从 [MIT](LICENSE) 许可。
