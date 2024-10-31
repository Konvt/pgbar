#pragma once

#ifndef __PGBAR_TEST
# define __PGBAR_TEST

# if defined( _MSVC_VER ) && defined( _MSVC_LANG )
#  define __PGBAR_CC_STD _MSVC_LANG
# else
#  define __PGBAR_CC_STD __cplusplus
# endif

# define CATCH_CONFIG_MAIN

# if __PGBAR_CC_STD >= 201402L
#  include "catch2/catch_test_macros.hpp"
# else
#  include "catch.hpp"
# endif

# undef __PGBAR_CC_STD

# define PGBAR_DEBUG
# include "pgbar/pgbar.hpp"

#endif
