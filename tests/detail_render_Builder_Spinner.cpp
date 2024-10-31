#define PGBAR_COLORLESS
#include "common.hpp"

TEST_CASE( "Construct from existing config object" )
{
  pgbar::configs::Spinner cfg { pgbar::options::Colored( false ) };
  pgbar::__detail::render::Builder<pgbar::configs::Spinner> builder { std::move( cfg ) };

  REQUIRE_FALSE( builder.colored() );
}

TEST_CASE( "Rendering default string" )
{
  const std::vector<pgbar::__detail::types::String> frames = { ".", "..", "...", "...." };
  const pgbar::__detail::types::String true_frame = "True", false_frame = "";
  pgbar::__detail::render::Builder<pgbar::configs::Spinner> builder { pgbar::configs::Spinner(
    pgbar::options::Colored( false ),
    pgbar::options::Bolded( false ),
    pgbar::options::Frames( frames ),
    pgbar::options::TrueFrame( true_frame ),
    pgbar::options::FalseFrame( false_frame ) ) };
  pgbar::__detail::StringBuffer buffer;

  builder.build( buffer, 0, frames.back().size() );

  REQUIRE( buffer.data().size() == frames.back().size() + 2 );

  buffer.clear();
  std::cout << "Rendering the first frame:\n"
            << builder.build( buffer, 0, frames.back().size() ) << std::endl;

  REQUIRE( buffer.empty() );

  const pgbar::__detail::types::String suffix = "Suffix";
  builder.suffix( suffix );

  builder.build( buffer, 0, frames.back().size() );

  REQUIRE( buffer.data().size() == frames.back().size() + suffix.size() + 2 );

  buffer.clear();
  std::cout << "Rendering the first frame with suffix:\n"
            << builder.build( buffer, 0, frames.back().size() ) << std::endl;

  REQUIRE( buffer.empty() );

  builder.build( buffer, true );

  REQUIRE( buffer.data().size() == true_frame.size() );

  buffer.clear();
  std::cout << "Rendering the final true frame:\n"
            << builder.build( buffer, true ) << std::endl;
  REQUIRE( buffer.empty() );

  builder.build( buffer, false );

  REQUIRE( buffer.data().empty() );

  buffer.clear();
  std::cout << "Rendering the final empty false frame:\n"
            << builder.build( buffer, false ) << std::endl;

}
