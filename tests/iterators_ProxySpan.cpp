#include "common.hpp"

TEST_CASE( "Lazy evaluation in method begin" )
{
  pgbar::ProgressBar<> bar;

  REQUIRE( bar.configure().tasks() == 0 );

  pgbar::iterators::NumericSpan<int> span { 0, 100 };

  REQUIRE( span.size() == 100 );

  pgbar::iterators::ProxySpan<decltype( span ), decltype( bar )> pspan { span, bar };

  REQUIRE( bar.configure().tasks() == 0 );
  REQUIRE( bar.configure().tasks() != span.size() );

  auto itr = pspan.begin();

  REQUIRE( bar.configure().tasks() == 100 );
  REQUIRE( bar.configure().tasks() == span.size() );
  REQUIRE( itr == span.begin() );
}

TEST_CASE( "Method end" )
{
  pgbar::ProgressBar<> bar;

  REQUIRE( bar.configure().tasks() == 0 );

  pgbar::iterators::NumericSpan<int> span { 0, 100 };

  REQUIRE( span.size() == 100 );

  pgbar::iterators::ProxySpan<decltype( span ), decltype( bar )> pspan { span, bar };

  REQUIRE( bar.configure().tasks() == 0 );
  REQUIRE( bar.configure().tasks() != span.size() );

  auto itr = pspan.end();

  REQUIRE( bar.configure().tasks() == 0 );
  REQUIRE( bar.configure().tasks() != span.size() );
}
