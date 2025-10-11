module;

#include "pgbar/pgbar.hpp"

export module pgbar;

export namespace pgbar {
  namespace exception {
    using pgbar::exception::Error;
    using pgbar::exception::InvalidArgument;
    using pgbar::exception::InvalidState;
    using pgbar::exception::SystemError;
  }

  namespace option {
    using pgbar::option::BarLength;
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
    using pgbar::config::disable_styling;
    using pgbar::config::hide_completed;
    using pgbar::config::intty;
    using pgbar::config::refresh_interval;
    using pgbar::config::terminal_width;
    using pgbar::config::TimeUnit;

    using pgbar::config::Block;
    using pgbar::config::Flow;
    using pgbar::config::Line;
    using pgbar::config::Spin;
    using pgbar::config::Sweep;
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

  using pgbar::BlockBar;
  using pgbar::FlowBar;
  using pgbar::ProgressBar;
  using pgbar::SpinBar;
  using pgbar::SweepBar;

  using pgbar::make_multi;
  using pgbar::MultiBar;

  using pgbar::DynamicBar;
  using pgbar::make_dynamic;
} // namespace pgbar
