#include "common.hpp"

TEST_CASE( "Parameter constructor" )
{
  int arr1[] = { 1, 2, 3, 4 };
  std::vector<int> arr2 { 1, 2, 3, 4 };

  pgbar::iterators::IterSpan<int*> span1 { arr1, arr1 + 4 };
  pgbar::iterators::IterSpan<std::vector<int>::iterator> span2 { arr2.begin(), arr2.end() };

  REQUIRE( span1.start_iter() == arr1 );
  REQUIRE( span1.end_iter() == arr1 + 4 );
  REQUIRE( span1.step() == 1 );
  REQUIRE( span2.start_iter() == arr2.begin() );
  REQUIRE( span2.end_iter() == arr2.end() );
  REQUIRE( span2.step() == 1 );
}

TEST_CASE( "Copy constructor" )
{
  int arr1[] = { 1, 2, 3, 4 };
  std::vector<int> arr2 { 1, 2, 3, 4 };

  pgbar::iterators::IterSpan<int*> span1 { arr1, arr1 + 4 };
  pgbar::iterators::IterSpan<std::vector<int>::iterator> span2 { arr2.begin(), arr2.end() };

  auto copy1 = span1;
  auto copy2 = span2;

  REQUIRE( copy1.start_iter() == arr1 );
  REQUIRE( copy1.end_iter() == arr1 + 4 );
  REQUIRE( copy1.step() == 1 );
  REQUIRE( copy1.start_iter() == span1.start_iter() );
  REQUIRE( copy1.end_iter() == span1.end_iter() );
  REQUIRE( copy1.step() == span1.step() );

  REQUIRE( copy2.start_iter() == arr2.begin() );
  REQUIRE( copy2.end_iter() == arr2.end() );
  REQUIRE( copy2.step() == 1 );
  REQUIRE( copy2.start_iter() == span2.start_iter() );
  REQUIRE( copy2.end_iter() == span2.end_iter() );
  REQUIRE( copy2.step() == span2.step() );
}

TEST_CASE( "Move constructor" )
{
  int arr1[] = { 1, 2, 3, 4 };
  std::vector<int> arr2 { 1, 2, 3, 4 };

  pgbar::iterators::IterSpan<int*> span1 { arr1, arr1 + 4 };
  pgbar::iterators::IterSpan<std::vector<int>::iterator> span2 { arr2.begin(), arr2.end() };

  auto moved1 = span1;
  auto moved2 = span2;

  REQUIRE( moved1.start_iter() == arr1 );
  REQUIRE( moved1.end_iter() == arr1 + 4 );
  REQUIRE( moved1.step() == 1 );

  REQUIRE( moved2.start_iter() == arr2.begin() );
  REQUIRE( moved2.end_iter() == arr2.end() );
  REQUIRE( moved2.step() == 1 );
}

TEST_CASE( "Valid setting" )
{
  int arr[]               = { 1, 2, 3, 4 };
  constexpr auto arr_size = sizeof( arr ) / sizeof( int );
  pgbar::iterators::IterSpan<int*> span { arr, arr + arr_size };

  span.start_iter( arr + 1 );

  REQUIRE( span.size() == arr_size - 1 );
  REQUIRE( span.start_iter() == arr + 1 );

  span.end_iter( arr + 2 );
  REQUIRE( span.end_iter() == arr + 2 );
}

TEST_CASE( "Iterators iteration" )
{
  const std::vector<int> arr { 1, 2, 3, 4, 5, 6, 7, 8 };
  pgbar::iterators::IterSpan<std::vector<int>::const_iterator> span { arr.begin(), arr.end() };

  REQUIRE( span.begin() == arr.begin() );
  REQUIRE( *span.begin() == arr.front() );
  REQUIRE( span.end() == arr.end() );
  REQUIRE( span.size() == arr.size() );

  std::vector<int> copy;
  for ( auto itr = span.begin(); itr != span.end(); ++itr )
    copy.push_back( *itr );

  REQUIRE( arr.size() == copy.size() );
  REQUIRE( arr == copy );
}

TEST_CASE( "Pointers iteration" )
{
  const int arr[]         = { 1, 2, 3, 4, 5, 6, 7, 8 };
  constexpr auto arr_size = sizeof( arr ) / sizeof( int );
  pgbar::iterators::IterSpan<const int*> span { arr, arr + arr_size };

  REQUIRE( span.begin() == arr );
  REQUIRE( *span.begin() == arr[0] );
  REQUIRE( span.end() == arr + arr_size );
  REQUIRE( span.size() == arr_size );

  std::vector<int> copy;
  for ( auto itr = span.begin(); itr != span.end(); ++itr )
    copy.push_back( *itr );

  REQUIRE( arr_size == copy.size() );

  for ( std::size_t i = 0; i < arr_size; ++i )
    REQUIRE( arr[i] == copy[i] );
}

TEST_CASE( "Start equals end" )
{
  int arr[] = { 1 };
  pgbar::iterators::IterSpan<const int*> span { arr, arr };

  REQUIRE( span.start_iter() == span.end_iter() );
  REQUIRE( span.begin() == arr );
  REQUIRE( span.end() == arr );
  REQUIRE( span.size() == 0 );
}

TEST_CASE( "Reverse iterators" )
{
  const std::vector<int> arr { 1, 2, 3, 4, 5, 6, 7, 8 };
  pgbar::iterators::IterSpan<std::vector<int>::const_reverse_iterator> span { arr.rbegin(),
                                                                              arr.rend() };

  REQUIRE( span.begin() == arr.rbegin() );
  REQUIRE( span.end() == arr.rend() );
  REQUIRE( span.size() == arr.size() );

  std::vector<int> copy;
  for ( auto itr = span.begin(); itr != span.end(); ++itr )
    copy.insert( copy.begin(), *itr );

  REQUIRE( arr.size() == copy.size() );
  REQUIRE( arr == copy );
}

TEST_CASE( "Reverse pointers" )
{
  const int arr[]         = { 1, 2, 3, 4, 5, 6, 7, 8 };
  constexpr auto arr_size = sizeof( arr ) / sizeof( int );
  pgbar::iterators::IterSpan<const int*> span { arr + arr_size - 1, arr - 1 };

  REQUIRE( span.begin() == arr + arr_size - 1 );
  REQUIRE( *span.begin() == arr[arr_size - 1] );
  REQUIRE( span.end() == arr - 1 );
  REQUIRE( span.size() == arr_size );

  std::vector<int> copy;
  for ( auto itr = span.begin(); itr != span.end(); ++itr )
    copy.insert( copy.begin(), *itr );

  REQUIRE( arr_size == copy.size() );

  for ( std::size_t i = 0; i < arr_size; ++i )
    REQUIRE( arr[i] == copy[i] );
}

TEST_CASE( "Swap two objects" )
{
  int arr1[]               = { 1, 2, 3, 4 };
  int arr2[]               = { 5, 6, 7, 8, 9, 10 };
  constexpr auto arr1_size = sizeof( arr1 ) / sizeof( int );
  constexpr auto arr2_size = sizeof( arr2 ) / sizeof( int );
  pgbar::iterators::IterSpan<int*> span1 { arr1, arr1 + arr1_size };
  pgbar::iterators::IterSpan<int*> span2 { arr2, arr2 + arr2_size };

  REQUIRE( arr1_size != arr2_size );

  REQUIRE( span1.size() == arr1_size );
  REQUIRE( span1.begin() == arr1 );
  REQUIRE( span1.end() == arr1 + arr1_size );
  REQUIRE( span2.size() == arr2_size );
  REQUIRE( span2.begin() == arr2 );
  REQUIRE( span2.end() == arr2 + arr2_size );

  span1.swap( span2 );

  REQUIRE( span1.size() == arr2_size );
  REQUIRE( span1.begin() == arr2 );
  REQUIRE( span1.end() == arr2 + arr2_size );
  REQUIRE( span2.size() == arr1_size );
  REQUIRE( span2.begin() == arr1 );
  REQUIRE( span2.end() == arr1 + arr1_size );
}

TEST_CASE( "Invalid setting" )
{
  REQUIRE_THROWS_AS( pgbar::iterators::IterSpan<int*>( nullptr, nullptr ),
                     pgbar::exceptions::InvalidArgument );

  int arr[] = { 0, 1, 2, 3 };

  REQUIRE_THROWS_AS( pgbar::iterators::IterSpan<int*>( arr, nullptr ),
                     pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( pgbar::iterators::IterSpan<int*>( nullptr, arr ),
                     pgbar::exceptions::InvalidArgument );
}
