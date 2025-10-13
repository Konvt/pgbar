module;

#include "pgbar/ProgressBar.hpp"

export module pgbar.ProgressBar;
export import pgbar.Core;

export namespace pgbar {
  namespace config {
    using pgbar::config::Line;
  }

  using pgbar::ProgressBar;
}
