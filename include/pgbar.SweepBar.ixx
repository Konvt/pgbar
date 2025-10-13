module;

#include "pgbar/SweepBar.hpp"

export module pgbar.SweepBar;
export import pgbar.Core;

export namespace pgbar {
  namespace config {
    using pgbar::config::Sweep;
  }

  using pgbar::SweepBar;
}
