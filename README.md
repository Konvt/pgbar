[中文文档见此](README_zh.md)。

## Features
- Header-only: All functionality contained within a single `.hpp` file in `include/pgbar`.
- Low update overhead: Minimal cost for each progress update.
- RGB color support: Customizable progress bar colors.
- Unicode support: Supports strings in various encodings, including but not limited to Unicode.
- Optional thread safety: Switchable via template parameters.
- `tqdm`-like interface: Access provided via template metaprogramming.
- Modern C++ compliance: Adheres to best practices in modern C++.
- Basic unit tests: Utilizes `Catch2` framework for fundamental testing.

## Styles
```
{startpoint}{done char}{todo char}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~~~~~~  Progress  ~~~~~~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~  Status  ~~~~~~~~~~~~~~~~~~~~~~~~~~~^

[-------                       ] [  24.00% |  48/200 |  31.00 Hz  | 00:00:01 < 00:00:04 ]
```
![example-color](images/example_color.gif)

> The "Hz" indicates how many tasks are performed per second.

You can use multiple constants from `pgbar::colors` or pass custom hexadecimal RGB values to customize the color of the progress bar.

![example-rgb](images/example_rgb.gif)

### Usage
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

For more examples, see [QuickStart.md](QuickStart.md) and [usage.cpp](demo/usage.cpp).

## FAQ
### Does updating the progress bar slow down the program?
No, as mentioned in the [Features](#features) section, updating the progress bar has *essentially* zero overhead.

With `O2/O3` optimizations enabled, the performance overhead of the second iteration in the code below will *approach* that of the first iteration[^1].

[^1]: In practice, the overhead of updating the progress bar only adds a single function call and some branching cost.

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

However, the smoothness of the display will depend on the single-core performance of your processor.
### Is it compatible with Windows/Linux?
Absolutely. I designed this library to work seamlessly on both systems, providing a unified visualization of iteration progress.
### Does it support Unicode?
As long as the UTF string is correctly encoded and your terminal can render it properly, there should be no issues.

From a rendering perspective, the progress bar only cares about the cursor position before rendering starts. Each render resets the cursor to that position, making it independent of the specific character encoding. Even if the string isn't Unicode-encoded, it should work fine.

The only caveat is that the progress bar requires a terminal that supports ANSI escape sequences; otherwise, it won’t function correctly.

> Most modern terminals support escape sequences, so this shouldn't be a concern as long as your terminal can render colored characters.

For a specific example of Unicode rendering, see [demo/unicode.cpp].

## License
This project is licensed under the [MIT](LICENSE) license.
