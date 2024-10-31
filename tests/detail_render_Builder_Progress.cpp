#define PGBAR_COLORLESS
#include "common.hpp"

TEST_CASE( "Construct from existing config object" )
{
  pgbar::configs::Progress cfg { pgbar::options::Tasks( 20 ) };
  pgbar::__detail::render::Builder<pgbar::configs::Progress> builder { std::move( cfg ) };

  REQUIRE( builder.tasks() == 20 );
}

TEST_CASE( "Rendering default string" )
{
  pgbar::__detail::render::Builder<pgbar::configs::Progress> builder { pgbar::configs::Progress(
    pgbar::options::StartPoint( "" ),
    pgbar::options::EndPoint( "" ),
    pgbar::options::LeftStatus( "" ),
    pgbar::options::RightStatus( "" ),
    pgbar::options::Tasks( 100 ) ) };
  pgbar::__detail::StringBuffer buffer;

  builder.styles( pgbar::configs::Progress::Bar );
  builder.build( buffer, 0, 0, {} );

  REQUIRE( buffer.data().size() == builder.bar_length() + 1 );

  buffer.clear();
  std::cout << "Rendering a progress bar string:\n"
            << builder.build( buffer, 0.42, 42, std::chrono::nanoseconds( 3000000000 ) )
            << std::endl;

  REQUIRE( buffer.empty() );

  builder.styles( pgbar::configs::Progress::Rate );
  builder.build( buffer, 0, 0, {} );

  REQUIRE( buffer.data() == "   inf Hz " );

  buffer.clear();
  std::cout << "Rendering a rate string:\n"
            << builder.build( buffer, 0.42, 42, std::chrono::nanoseconds( 3000000000 ) )
            << std::endl;

  REQUIRE( buffer.empty() );

  builder.styles( pgbar::configs::Progress::Ratio );
  builder.build( buffer, 0, 0, {} );

  REQUIRE( buffer.data() == " 0.00% " );

  buffer.clear();
  std::cout << "Rendering a ratio string:\n"
            << builder.build( buffer, 0.42, 42, std::chrono::nanoseconds( 3000000000 ) )
            << std::endl;

  REQUIRE( buffer.empty() );

  builder.styles( pgbar::configs::Progress::Timer );
  builder.build( buffer, 0, 0, {} );

  REQUIRE( buffer.data() == "00:00:00 < --:--:--" );

  buffer.clear();
  std::cout << "Rendering a timer string:\n"
            << builder.build( buffer, 0.42, 42, std::chrono::nanoseconds( 3000000000 ) )
            << std::endl;

  REQUIRE( buffer.empty() );

  builder.styles( pgbar::configs::Progress::Entire );
  builder.build( buffer, 0, 0, {} );

  REQUIRE( buffer.data().size() - builder.bar_length() == builder.fixed_size() - 1 );

  buffer.clear();
  std::cout << "Rendering a full progress bar string:\n"
            << builder.build( buffer, 0.42, 42, std::chrono::nanoseconds( 3000000000 ) )
            << std::endl;

  REQUIRE( buffer.empty() );
}
