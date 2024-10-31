#include "common.hpp"

TEST_CASE( "Default constructor" )
{
  pgbar::__detail::render::Renderer thread;

  REQUIRE_FALSE( thread.valid() );
}

TEST_CASE( "Parameter constructor" )
{
  pgbar::__detail::render::Renderer thread { []() {} };

  REQUIRE( thread.valid() );
}

TEST_CASE( "Reset with a new task" )
{
  pgbar::__detail::render::Renderer thread;

  REQUIRE_FALSE( thread.valid() );

  thread.reset( []() {} );

  REQUIRE( thread.valid() );

  thread.reset();

  REQUIRE_FALSE( thread.valid() );
}

TEST_CASE( "Activate and execute task" )
{
  std::atomic<std::size_t> count { 0 };
  pgbar::__detail::render::Renderer thread { [&count]() { ++count; } };

  thread.activate();
  std::this_thread::sleep_for( std::chrono::nanoseconds( 50 ) );

  REQUIRE( count > 0 );
}

TEST_CASE( "Suspend correctly" )
{
  std::atomic<std::size_t> count { 0 };
  pgbar::__detail::render::Renderer thread { [&count]() { ++count; } };

  thread.activate();

  REQUIRE( count > 0 );

  thread.suspend();
  const auto count_after_suspend = count.load();

  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

  REQUIRE( count_after_suspend == count );
}
