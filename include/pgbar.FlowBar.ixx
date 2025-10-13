module;

#include "pgbar/FlowBar.hpp"

export module pgbar.FlowBar;
export import pgbar.Core;

export namespace pgbar {
  namespace config {
    using pgbar::config::Flow;
  }

  using pgbar::FlowBar;
}
