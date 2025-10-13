module;

#include "pgbar/SpinBar.hpp"

export module pgbar.SpinBar;
export import pgbar.Core;

export namespace pgbar {
  namespace config {
    using pgbar::config::Spin;
  }

  using pgbar::SpinBar;
}
