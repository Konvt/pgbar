#include "common.hpp"

TEST_CASE( "Default constructor" )
{
  pgbar::iterators::NumericSpan<int> span;

  REQUIRE( span.start_value() == 0 );
  REQUIRE( span.end_value() == 0 );
  REQUIRE( span.step() == 1 );
  REQUIRE( span.size() == 0 );

  REQUIRE( *span.begin() == 0 );
  REQUIRE( *span.end() == 0 );
}

TEST_CASE( "Parameter constructor" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 101, 2 };

  REQUIRE( span.start_value() == 1 );
  REQUIRE( span.end_value() == 101 );
  REQUIRE( span.step() == 2 );
  REQUIRE( span.size() == ( 101 - 1 ) / 2 );

  REQUIRE( *span.begin() == 1 );
  REQUIRE( *span.end() == 101 );
}

TEST_CASE( "Copy constructor" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 101, 2 };
  auto copy = span;

  REQUIRE( copy.start_value() == 1 );
  REQUIRE( copy.end_value() == 101 );
  REQUIRE( copy.step() == 2 );
  REQUIRE( copy.size() == ( 101 - 1 ) / 2 );

  REQUIRE( *copy.begin() == 1 );
  REQUIRE( *copy.end() == 101 );
}

TEST_CASE( "Move constructor" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 101, 2 };
  auto copy = std::move( span );

  REQUIRE( copy.start_value() == 1 );
  REQUIRE( copy.end_value() == 101 );
  REQUIRE( copy.step() == 2 );
  REQUIRE( copy.size() == ( 101 - 1 ) / 2 );

  REQUIRE( *copy.begin() == 1 );
  REQUIRE( *copy.end() == 101 );
}

TEST_CASE( "Normal iteration" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 101, 2 };
  auto itr = span.begin();

  REQUIRE( *itr == 1 );
  REQUIRE( itr != span.end() );

  ++itr;

  REQUIRE( *itr == 3 );
  REQUIRE( itr != span.end() );

  auto before = itr++;

  REQUIRE( *before == 3 );
  REQUIRE( *itr == 5 );
  REQUIRE( itr != span.end() );

  itr += 2;

  REQUIRE( *itr == 7 );

  while ( ++itr != span.end() ) {}
  REQUIRE( *itr == 101 );
  REQUIRE( itr == span.end() );
}

TEST_CASE( "Boundary step size iteration" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 101, 2 };
  auto itr = span.begin();
  itr += 4;

  REQUIRE( *itr == 5 );

  itr += 1; // increment is less than step

  REQUIRE( *itr == 5 );

  itr += 3;

  REQUIRE( *itr == 7 );
}

TEST_CASE( "Valid interval value" )
{
  pgbar::iterators::NumericSpan<int> span { 0, 100, 1 };
  REQUIRE( span.start_value() == 0 );
  REQUIRE( span.end_value() == 100 );
  REQUIRE( span.step() == 1 );

  span.start_value( 20 );

  REQUIRE( span.start_value() == 20 );

  span.end_value( 40 );

  REQUIRE( span.end_value() == 40 );
  REQUIRE( span.size() == 20 );

  span.step( 2 );

  REQUIRE( span.step() == 2 );
  REQUIRE( span.size() == 10 );

  span.step( 3 );

  REQUIRE( span.step() == 3 );
  REQUIRE( span.size() == 7 );

  span.start_value( 0 );

  REQUIRE( span.start_value() == 0 );

  span.end_value( 0 );

  REQUIRE( span.end_value() == 0 );
  REQUIRE( span.size() == 0 );
}

TEST_CASE( "Step greater than range" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 10, 20 };

  REQUIRE( span.size() == 1 );
  REQUIRE( span.step() > std::max( span.start_value(), span.end_value() ) );

  auto itr = span.begin();

  REQUIRE( *itr == 1 );
  REQUIRE( itr != span.end() );

  ++itr;

  REQUIRE( itr == span.end() );
  REQUIRE( *itr >= *span.end() );
}

TEST_CASE( "Start equals end value" )
{
  pgbar::iterators::NumericSpan<int> span { 10, 10, 1 };

  REQUIRE( span.begin() == span.end() );
  REQUIRE( span.size() == 0 );

  span.start_value( 0 );

  REQUIRE( span.start_value() == 0 );

  span.start_value( 10 );
  span.step( -1 );

  REQUIRE( span.step() == -1 );
}

TEST_CASE( "Coprime step and endpoint" )
{
  pgbar::iterators::NumericSpan<int> span { 1, 100, 3 };
  auto itr = span.begin();
  while ( ++itr != span.end() ) {}

  REQUIRE( *itr >= 100 );
  REQUIRE( itr == span.end() );
}

TEST_CASE( "Floating point iteration using integer steps" )
{
  pgbar::iterators::NumericSpan<double> span { 1.0, 10.0, 0.3 };
  auto itr = span.begin();

  constexpr double epsilon = 1e-6;
  std::size_t total_steps  = static_cast<std::size_t>( std::ceil( ( 10.0 - 1.0 ) / 0.3 ) );

  REQUIRE( span.size() == total_steps );

  for ( std::size_t i = 0; i < total_steps; ++i ) {
    double expected_value = 1.0 + i * 0.3;

    REQUIRE( std::fabs( *itr - expected_value ) < epsilon );

    ++itr;
  }

  REQUIRE( *itr >= 10.0 - epsilon );
  REQUIRE( itr == span.end() );
}

TEST_CASE( "Swap two objects" )
{
  pgbar::iterators::NumericSpan<int> span1 { 0, 10, 2 };
  pgbar::iterators::NumericSpan<int> span2 { 10, 0, -1 };

  REQUIRE( span1.start_value() == 0 );
  REQUIRE( span1.end_value() == 10 );
  REQUIRE( span1.step() == 2 );
  REQUIRE( span2.start_value() == 10 );
  REQUIRE( span2.end_value() == 0 );
  REQUIRE( span2.step() == -1 );

  span1.swap( span2 );

  REQUIRE( span1.start_value() == 10 );
  REQUIRE( span1.end_value() == 0 );
  REQUIRE( span1.step() == -1 );
  REQUIRE( span2.start_value() == 0 );
  REQUIRE( span2.end_value() == 10 );
  REQUIRE( span2.step() == 2 );
}

TEST_CASE( "Invalid initialization" )
{
  // deascending order but with a positive `step`
  REQUIRE_THROWS_AS( pgbar::iterators::NumericSpan<int>( 42, 0, 2 ),
                     pgbar::exceptions::InvalidArgument );
  // ascending order but with a negative `step`
  REQUIRE_THROWS_AS( pgbar::iterators::NumericSpan<int>( 42, -2 ),
                     pgbar::exceptions::InvalidArgument );
  // step is zero
  REQUIRE_THROWS_AS( pgbar::iterators::NumericSpan<int>( 42, 0 ),
                     pgbar::exceptions::InvalidArgument );
}

TEST_CASE( "Invalid setting" )
{
  pgbar::iterators::NumericSpan<int> span1 { 0, 10, 2 };

  REQUIRE_THROWS_AS( span1.start_value( 20 ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( span1.end_value( -10 ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( span1.step( 0 ), pgbar::exceptions::InvalidArgument );

  pgbar::iterators::NumericSpan<int> span2 { 10, 0, -2 };

  REQUIRE_THROWS_AS( span2.start_value( -10 ), pgbar::exceptions::InvalidArgument );
  REQUIRE_THROWS_AS( span2.end_value( 20 ), pgbar::exceptions::InvalidArgument );
}
