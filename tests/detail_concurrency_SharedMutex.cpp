#include "common.hpp"

TEST_CASE( "Exclusive lock and unlock" )
{
  pgbar::__detail::concurrency::SharedMutex mtx;
  REQUIRE_NOTHROW( mtx.lock() );

  REQUIRE_FALSE( mtx.try_lock_shared() );
  REQUIRE_FALSE( mtx.try_lock() );

  REQUIRE_NOTHROW( mtx.unlock() );
}

TEST_CASE( "Shared lock and unlock" )
{
  pgbar::__detail::concurrency::SharedMutex mtx;
  mtx.lock_shared();

  REQUIRE_NOTHROW( mtx.lock_shared() );
  REQUIRE_NOTHROW( mtx.unlock_shared() );

  mtx.unlock_shared();
}

TEST_CASE( "Exclusive lock works correctly" )
{
  pgbar::__detail::concurrency::SharedMutex mtx;

  std::thread writer( [&mtx]() {
    mtx.lock();
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    mtx.unlock();
  } );

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  REQUIRE_FALSE( mtx.try_lock_shared() );

  writer.join();
}

TEST_CASE( "Shared lock works correctly" )
{
  pgbar::__detail::concurrency::SharedMutex mtx;

  constexpr std::size_t num_readers = 10;
  std::atomic<std::size_t> active_readers { 0 };
  std::vector<std::thread> readers;

  for ( std::size_t i = 0; i < num_readers; ++i ) {
    readers.emplace_back( [&mtx, &active_readers]() {
      mtx.lock_shared();
      ++active_readers;
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      --active_readers;
      mtx.unlock_shared();
    } );
  }
  std::for_each( readers.begin(), readers.end(), []( std::thread& td ) { td.join(); } );

  REQUIRE( active_readers == 0 );
}

TEST_CASE( "Reader and writer" )
{
  pgbar::__detail::concurrency::SharedMutex mtx;

  int shared_value = 0;
  std::atomic<std::size_t> write_count { 0 };

  std::vector<std::thread> readers;
  for ( std::size_t i = 0; i < 5; ++i )
    readers.emplace_back( [&]() {
      while ( true ) {
        mtx.lock_shared();
        auto value = shared_value;
        mtx.unlock_shared();

        if ( value == 4 )
          break;
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
      }
    } );

  std::thread writer( [&]() {
    for ( std::size_t i = 0; i < 5; ++i ) {
      mtx.lock();
      shared_value = i;
      ++write_count;
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      mtx.unlock();
    }
  } );

  writer.join();
  std::for_each( readers.begin(), readers.end(), []( std::thread& td ) { td.join(); } );

  REQUIRE( write_count == 5 );
  REQUIRE( shared_value == 4 );
}
