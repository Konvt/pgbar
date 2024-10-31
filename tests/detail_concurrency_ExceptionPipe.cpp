#include "common.hpp"

TEST_CASE( "Default constructor" )
{
  pgbar::__detail::concurrency::ExceptionPipe pipe;

  REQUIRE( pipe.empty() );
  REQUIRE( pipe.size() == 0 );
}

TEST_CASE( "Basic operations" )
{
  pgbar::__detail::concurrency::ExceptionPipe pipe;
  {
    auto ex = std::make_exception_ptr( std::runtime_error( "Test exception" ) );
    pipe.push( ex );
  }
  REQUIRE_FALSE( pipe.empty() );
  REQUIRE( pipe.size() == 1 );

  auto front_ex = pipe.front();
  pipe.pop();

  REQUIRE( pipe.empty() );
  REQUIRE( pipe.size() == 0 );

  REQUIRE( front_ex );
  REQUIRE_THROWS_AS( std::rethrow_exception( front_ex ), std::runtime_error );
}

TEST_CASE( "Pop and throw exception" )
{
  pgbar::__detail::concurrency::ExceptionPipe pipe;
  {
    auto ex1 = std::make_exception_ptr( std::runtime_error( "Test exception" ) );
    pipe.push( ex1 );
  }
  REQUIRE_FALSE( pipe.empty() );
  REQUIRE( pipe.size() == 1 );
  REQUIRE_THROWS_AS( pipe.pop_throw(), std::runtime_error );
  REQUIRE( pipe.empty() );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::__detail::concurrency::ExceptionPipe pipe1;
  pgbar::__detail::concurrency::ExceptionPipe pipe2;
  {
    auto ex = std::make_exception_ptr( std::runtime_error( "Test exception" ) );
    pipe1.push( ex );
  }
  REQUIRE_FALSE( pipe1.empty() );
  REQUIRE( pipe2.empty() );

  pipe1.swap( pipe2 );

  REQUIRE( pipe1.empty() );
  REQUIRE_FALSE( pipe2.empty() );
  REQUIRE_THROWS_AS( pipe2.pop_throw(), std::runtime_error );
}

TEST_CASE( "Multi-threaded operations" )
{
  pgbar::__detail::concurrency::ExceptionPipe pipe;
  std::thread t1( [&pipe]() {
    pipe.push( std::make_exception_ptr( std::runtime_error( "Exception from thread 1" ) ) );
  } );

  std::thread t2( [&pipe]() {
    try {
      throw std::runtime_error( "Exception from thread 2" );
    } catch ( ... ) {
      pipe.push( std::current_exception() );
    }
  } );

  t1.join();
  t2.join();

  REQUIRE_FALSE( pipe.empty() );
  REQUIRE( pipe.size() == 2 );

  REQUIRE_THROWS_AS( pipe.pop_throw(), std::runtime_error );
  REQUIRE( pipe.size() == 1 );
  REQUIRE_THROWS_AS( pipe.pop_throw(), std::runtime_error );
  REQUIRE( pipe.empty() );
}
