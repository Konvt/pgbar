[中文文档见此](README_zh.md)。

## Features
- Header-only: All functionality contained within a single `.hpp` file in `include/pgbar`.
- Low update overhead: Minimal cost for each progress update.
- RGB color support: Customizable progress bar colors.
- Optional thread safety: Switchable via template parameters.
- `tqdm`-like interface: Access provided via template metaprogramming.
- Modern C++ compliance: Adheres to best practices in modern C++.
- Basic unit tests: Utilizes `Catch2` framework for fundamental testing.

[See here for Unicode support](#does-it-support-unicode).

## Styles
```
{startpoint}{done}{todo}{endpoint} {left status}{percentage}{task counter}{rate}{timer}{right status}
^~~~~~~~~~~  Progress  ~~~~~~~~~~^ ^~~~~~~~~~~~~~~~~~~~~~~~~~~~  Status  ~~~~~~~~~~~~~~~~~~~~~~~~~~~^

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

For more examples, see [QuickStart.md](QuickStart.md) and [demo.cpp](demo/demo.cpp).

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
Well..., the answer is somewhat nuanced; it can be both affirmative and negative.

**Affirmative Aspect:** From the perspective of the progress bar's rendering mechanism, it doesn't recognize the character encoding of the strings it outputs; it only knows that it must continually output strings on a given terminal line to refresh the display.

Thus, if you supply a UTF string, or even emoji characters, it works just fine under normal conditions.

**Negative Aspect:** However, because I didn’t implement specific character encoding and decoding logic, the progress bar interprets the strings passed to it at the byte level. This can lead to rendering inconsistencies when mixing multi-byte UTF characters (such as CJK characters and emojis) with ASCII characters.

In such cases, you may need to manually align the width of ASCII and UTF characters. For instance, if the progress bar’s "todo" element is a double-width emoji like "🎈", the corresponding "done" element should also be double-width, such as two spaces.

For a specific example of Unicode rendering, see [unicode.cpp](demo/unicode.cpp).

## License
This project is licensed under the [MIT](LICENSE) license.
