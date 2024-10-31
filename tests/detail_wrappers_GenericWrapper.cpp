#include "common.hpp"

// GenericWrapper is a virtual base class,
// so we test its derived classes.
TEST_CASE( "Fundamental types" )
{
  std::size_t value = 42;
  pgbar::options::Tasks wrapper { value };

  REQUIRE( wrapper.value() == 42 );

  pgbar::options::Tasks another { 114514 };
  swap( wrapper, another );

  REQUIRE( wrapper.value() == 114514 );
  REQUIRE( another.value() == 42 );
}

TEST_CASE( "Object types" )
{
  pgbar::options::TodoChar wrapper { "Hello, pgbar!" };

  REQUIRE( wrapper.value() == "Hello, pgbar!" );

  pgbar::options::TodoChar another { "Hi, pgbar!" };
  swap( wrapper, another );

  REQUIRE( wrapper.value() == "Hi, pgbar!" );
  REQUIRE( another.value() == "Hello, pgbar!" );
}
