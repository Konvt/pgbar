#ifndef __PGBAR_BUILDER
#define __PGBAR_BUILDER

namespace pgbar {
  namespace __details {
    namespace render {
      // Extension points.
      template<typename Config>
      struct Builder;
    } // namespace render
  }
} // namespace pgbar

#endif
