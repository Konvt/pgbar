module;

#include "pgbar/Indicator.hpp"
#include "pgbar/color/Color.hpp"
#include "pgbar/exception/Error.hpp"
#include "pgbar/option/Option.hpp"
#include "pgbar/slice/BoundedSpan.hpp"
#include "pgbar/slice/IteratorSpan.hpp"
#include "pgbar/slice/NumericSpan.hpp"
#include "pgbar/slice/TrackedSpan.hpp"

export module pgbar.Core;

export namespace pgbar {
  namespace exception {
    using pgbar::exception::Error;
    using pgbar::exception::InvalidArgument;
    using pgbar::exception::InvalidState;
    using pgbar::exception::SystemError;
  }

  namespace option {
    using pgbar::option::BarWidth;
    using pgbar::option::Bolded;
    using pgbar::option::Colored;
    using pgbar::option::Divider;
    using pgbar::option::Ending;
    using pgbar::option::Filler;
    using pgbar::option::Lead;
    using pgbar::option::LeftBorder;
    using pgbar::option::Magnitude;
    using pgbar::option::Postfix;
    using pgbar::option::Prefix;
    using pgbar::option::Remains;
    using pgbar::option::Reversed;
    using pgbar::option::RightBorder;
    using pgbar::option::Shift;
    using pgbar::option::SpeedUnit;
    using pgbar::option::Starting;
    using pgbar::option::Style;
    using pgbar::option::Tasks;

    using pgbar::option::EndColor;
    using pgbar::option::FillerColor;
    using pgbar::option::InfoColor;
    using pgbar::option::LeadColor;
    using pgbar::option::PostfixColor;
    using pgbar::option::PrefixColor;
    using pgbar::option::RemainsColor;
    using pgbar::option::StartColor;
  } // namespace option

  namespace config {
    using pgbar::config::auto_style_off;
    using pgbar::config::hide_completed;
    using pgbar::config::intty;
    using pgbar::config::refresh_interval;
    using pgbar::config::terminal_width;
    using pgbar::config::TimeUnit;
  } // namespace config

  namespace color {
    using pgbar::color::Black;
    using pgbar::color::Blue;
    using pgbar::color::Cyan;
    using pgbar::color::Green;
    using pgbar::color::Magenta;
    using pgbar::color::None;
    using pgbar::color::Red;
    using pgbar::color::White;
    using pgbar::color::Yellow;
  } // namespace color

  namespace slice {
    using pgbar::slice::BoundedSpan;
    using pgbar::slice::IteratorSpan;
    using pgbar::slice::NumericSpan;
    using pgbar::slice::TrackedSpan;
  }

  using pgbar::Channel;
  using pgbar::Policy;
  using pgbar::Region;

  using pgbar::Indicator;
  using pgbar::iterate;
} // namespace pgbar
