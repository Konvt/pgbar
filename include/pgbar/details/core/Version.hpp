#ifndef PGBAR_VERSION

# define PGBAR_MAJOR 1
# define PGBAR_MINOR 0
# define PGBAR_PATCH 0
# define PGBAR_STAGE "alpha.4"

# define PGBAR__STR( x ) #x
# define PGBAR__GEN_VER( major, minor, patch ) \
   PGBAR__STR( major ) "." PGBAR__STR( minor ) "." PGBAR__STR( patch )

# define PGBAR_VERSION PGBAR__GEN_VER( PGBAR_MAJOR, PGBAR_MINOR, PGBAR_PATCH )

#endif
