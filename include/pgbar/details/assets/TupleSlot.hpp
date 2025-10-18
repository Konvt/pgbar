#ifndef PGBAR__TUPLESLOT
#define PGBAR__TUPLESLOT

#include "../types/Types.hpp"

namespace pgbar {
  namespace _details {
    namespace assets {
      template<typename Base, types::Size>
      struct TupleSlot : public Base {
        using Base::Base;
        PGBAR__CXX23_CNSTXPR TupleSlot( TupleSlot&& )              = default;
        PGBAR__CXX14_CNSTXPR TupleSlot& operator=( TupleSlot&& ) & = default;
        constexpr TupleSlot( Base&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        PGBAR__CXX14_CNSTXPR TupleSlot& operator=( Base&& rhs ) & noexcept
        {
          PGBAR__ASSERT( this != std::addressof( rhs ) );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
      };
    } // namespace assets
  } // namespace _details
} // namespace pgbar

#endif
