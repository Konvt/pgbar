module;

#include "pgbar/BlockBar.hpp"

export module pgbar.BlockBar;
export import pgbar.Core;

export namespace pgbar {
  namespace config {
    using pgbar::config::Block;
  }

  using pgbar::BlockBar;
}
