[中文文档见此](docs/README_zh.md)。

## Features
- **Header-only design**: All functionality contained within a single `.hpp` file in `include/pgbar`.
- **Low-overhead updates**: Microsecond level cost per iteration.
- **C++11 & later compatible**: Supports all standard revisions from C++11 through C++23.
- **Unicode support**: Parse each string in UTF-8 encoding.
- **RGB color support**: Customizable progress bar colors.
- **Thread-safe design**: Can be safely used in multi-threaded environments.
- **`tqdm`-like interface**: Chainable methods powered by template metaprogramming.
- **Modern C++ core**: Leverages `constexpr`, RAII, and type traits for zero-cost abstractions.

## Styles
### ProgressBar
```
{LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Remains}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
 30.87% | [=========>                    ] |  662933732/2147483647 |  11.92 MHz | 00:00:55 < 00:02:03
```
![progressbar](images/progressbar.gif)

### BlockBar
```
{LeftBorder}{Description}{Percent}{Starting}{BlockBar}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
 35.22% | ██████████▋                    |  47275560/134217727 |  16.80 MHz | 00:00:02 < 00:00:05
```
![BlockBar](images/blockbar.gif)

### SpinBar
```
{LeftBorder}{Description}{Lead}{Percent}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
\ |  48.64% |  65288807/134217727 |  17.84 MHz | 00:00:03 < 00:00:03
```
![spinbar](images/spinbar.gif)

### SweepBar
```
{LeftBorder}{Description}{Percent}{Starting}{Filler}{Lead}{Filler}{Ending}{Counter}{Speed}{Elapsed}{Countdown}{RightBorder}
 39.82% | [---------------------<==>----] |  53458698/134217727 |  17.89 MHz | 00:00:02 < 00:00:04
```
![sweepbar](images/sweepbar.gif)

### MultiBar
![multibar](images/multibar.gif)

### DynamicBar
![dynamicbar](images/dynamicbar.gif)

### Usage
```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  pgbar::ProgressBar<> bar { pgbar::option::Remains( "-" ),
                             pgbar::option::Filler( "=" ),
                             pgbar::option::Styles( pgbar::config::Line::Entire ),
                             pgbar::option::TodoColor( "#A52A2A" ),
                             pgbar::option::DoneColor( 0x0099FF ),
                             pgbar::option::StatusColor( pgbar::color::Yellow ),
                             pgbar::option::Tasks( 100 ) };

  for ( auto _ = 0; _ < 100; ++_ )
    bar.tick();
}
```

For more examples, see [QuickStart.md](docs/QuickStart.md) and [demo/](demo/).

## FAQ
### Does updating the progress bar slow down the program?
No, as mentioned in the [Features](#features) section, updating the progress bar has *essentially* zero overhead.

With `O2/O3` optimizations enabled, the performance overhead of the second iteration in the code below will *approach* that of the first iteration[^1].

[^1]: In practice, the overhead of updating the progress bar only adds the overload of calling jumps and branches a few more times.

```cpp
#include "pgbar/pgbar.hpp"

int main()
{
  std::size_t count = 0;
  for ( std::size_t _ = 0; _ < 2147483647; ++_ )
    ++count;

  pgbar::ProgressBar<> bar { pgbar::option::Tasks( 2147483647 ) };
  for ( std::size_t _ = 0; _ < 2147483647; ++_ )
    bar.tick();
}
```

However, the smoothness of the display will depend on the single-core performance of your processor.
### Is it compatible with Windows/Linux?
Absolutely. I designed this library to work seamlessly on both systems, providing a unified visualization of iteration progress.

Btw, it should be noted that if it is on the Windows platform, then `pgbar` will depend on the `Windows.h` header file; Moreover, `NOMINMAX` will be defined to disable the `min` and `max` macros.
### Does it support Unicode?
As pointed out at the beginning, there is no problem.

Although only UTF-8 encoded strings are currently supported, using any non-UTF-8 encoded string will result in an exception.

If you are using the C++20, `pgbar`'s functions also support `u8string`.

![unicode](images/unicode.gif)

## License
This project is licensed under the [MIT](LICENSE) license.
