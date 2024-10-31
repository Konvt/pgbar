#include "common.hpp"

TEST_CASE( "RenderFnWrapper Tests" )
{
  bool executed = false;
  auto fn       = [&]() { executed = true; };

  pgbar::__detail::wrappers::RenderFnWrapper<decltype( fn )> wrapper { fn };
  wrapper.run();

  REQUIRE( executed );
}
