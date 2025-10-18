#ifndef PGBAR__BUILDER
#define PGBAR__BUILDER

namespace pgbar {
  namespace _details {
    namespace render {
      // Extension points.
      template<typename Config>
      struct Builder;
    } // namespace render
  }
} // namespace pgbar

#endif
