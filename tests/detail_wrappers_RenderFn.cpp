#include "common.hpp"

TEST_CASE( "RenderFn Tests" )
{
  bool executed = false;
  auto fn       = [&]() { executed = true; };

  pgbar::__detail::wrappers::RenderFn* wrapper =
    new pgbar::__detail::wrappers::RenderFnWrapper<decltype( fn )>( fn );
  wrapper->run();

  REQUIRE( executed );
}
