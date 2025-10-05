#ifndef __PGBAR_TUPLESLOT
#define __PGBAR_TUPLESLOT

#include "../types/Types.hpp"

namespace pgbar {
  namespace __details {
    namespace assets {
      template<typename Base, types::Size>
      struct TupleSlot : public Base {
        using Base::Base;
        __PGBAR_CXX23_CNSTXPR TupleSlot( TupleSlot&& )              = default;
        __PGBAR_CXX14_CNSTXPR TupleSlot& operator=( TupleSlot&& ) & = default;
        constexpr TupleSlot( Base&& rhs ) noexcept : Base( std::move( rhs ) ) {}
        __PGBAR_CXX14_CNSTXPR TupleSlot& operator=( Base&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( rhs ) );
          Base::operator=( std::move( rhs ) );
          return *this;
        }
      };
    } // namespace assets
  } // namespace __details
} // namespace pgbar

#endif
