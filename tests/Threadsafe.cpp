#include "common.hpp"

TEST_CASE( "Lock and unlock" )
{
  pgbar::Threadsafe mtx;
  mtx.lock();
  REQUIRE_NOTHROW( mtx.unlock() );
}

TEST_CASE( "Try lock" )
{
  pgbar::Threadsafe mtx;
  REQUIRE( mtx.try_lock() );
  mtx.unlock();
}

TEST_CASE( "Try lock when already locked" )
{
  pgbar::Threadsafe mtx;
  mtx.lock();
  REQUIRE_FALSE( mtx.try_lock() );
  mtx.unlock();
}

TEST_CASE( "Multiple threads writing to vector in order" )
{
  pgbar::Threadsafe mtx;
  std::vector<std::size_t> ids;
  std::vector<std::thread> threads;

  constexpr std::size_t num_threads = 100;
  const auto timeout_s              = std::chrono::seconds( 10 );

  for ( std::size_t i = 0; i < num_threads; ++i ) {
    threads.emplace_back( [&mtx, &ids, &timeout_s, i]() {
      auto start_time = std::chrono::steady_clock::now();
      bool inserted   = false;

      while ( std::chrono::steady_clock::now() - start_time < timeout_s ) {
        mtx.lock();

        if ( ( ids.empty() && i != 0 ) || ( !ids.empty() && ids.back() + 1 != i ) ) {
          mtx.unlock();
          std::this_thread::yield();
        } else {
          ids.push_back( i );
          inserted = true;
          mtx.unlock();
          break;
        }
      }

      if ( !inserted )
        throw std::runtime_error( "Thread timed out while trying to lock" );
    } );
  }

  for ( auto& t : threads )
    t.join();

  REQUIRE( ids.size() == num_threads );
  REQUIRE( std::is_sorted( ids.cbegin(), ids.cend() ) );
}
