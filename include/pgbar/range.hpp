// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __PGBAR_RANGE_HPP__
# define __PGBAR_RANGE_HPP__

#include "pgbar.hpp" // Other required header files have been included in pgbar.hpp.
#include <algorithm> // std::distance

#if defined(__GNUC__) || defined(__clang__)
# define __PGBAR_NODISCARD__ __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
# define __PGBAR_NODISCARD__ _Check_return_
#else
# define __PGBAR_NODISCARD__
#endif

#if defined(_MSVC_VER) && defined(_MSVC_LANG) // for msvc
# define __PGBAR_CMP_V__ _MSVC_LANG
#else
# define __PGBAR_CMP_V__ __cplusplus
#endif

#if __PGBAR_CMP_V__ >= 202002L
# include <ranges>   // std::ranges::begin、std::ranges::end
# define __PGBAR_CXX20__ 1
#else
# define __PGBAR_CXX20__ 0
#endif // __cplusplus >= 202002L

namespace pgbar {
  namespace __detail {
    template<typename EleT, typename BarT>
    class range_iterator final { // for number
      static_assert(
        is_pgbar<BarT>::value,
        "pgbar::__detail::range_iterator: Only available for `pgbar::pgbar`"
      );

      numeric_iterator<EleT> iter;
      BarT* bar;

    public:
      using iterator_category = std::output_iterator_tag;
      using value_type = EleT;
      using difference_type = void;
      using pointer = void;
      using reference = value_type;

      explicit range_iterator( EleT _startpoint, EleT _endpoint, EleT _step, BarT& _bar )
        : iter { _startpoint, _endpoint, _step } {
        if ( _endpoint < _startpoint && _step > 0 )
          throw bad_pgbar { "pgbar::__detail::range_iterator: invalid iteration range" };
        bar = &_bar;
        bar->reset().set_task( iter.extent() ).set_step( 1 );
      }
      __PGBAR_NODISCARD__ range_iterator begin() const noexcept {
        return *this;
      } // invokes copy constructor
      __PGBAR_NODISCARD__ range_iterator end() const noexcept {
        range_iterator ed_pnt = *this;
        ed_pnt.iter = ed_pnt.iter.end();
        return ed_pnt;
      }
      __PGBAR_NODISCARD__ reference operator*() const noexcept {
        return *iter;
      }
      __PGBAR_NODISCARD__ bool operator==( const range_iterator& lhs ) const noexcept {
        return iter == lhs.iter;
      }
      __PGBAR_NODISCARD__ bool operator!=( const range_iterator& lhs ) const noexcept {
        return !(operator==( lhs ));
      }
      range_iterator& operator++() {
        ++iter; bar->update(); return *this;
      }
      range_iterator operator++( int ) {
        range_iterator before = *this; operator++(); return before;
      }
    };

    template<typename IterT, typename BarT>
    class container_iterator final { // for container
      static_assert( // `IterT` means iterator type
        !std::is_arithmetic<IterT>::value &&
        is_pgbar<BarT>::value,
        "pgbar::__detail::container_iterator: Only available for container types"
      );

      using SizeT = size_t;
      using EleT = typename std::iterator_traits<IterT>::value_type;

      BarT* bar;
      IterT start, terminus, current;
      SizeT extent;
      bool is_reversed;

    public:
      using iterator_category = std::output_iterator_tag;
      using value_type = EleT;
      using difference_type = void;
      using pointer = EleT*;
      using reference = EleT&;

      explicit container_iterator( IterT _begin, IterT _endpoint, BarT& _bar ) {
        static_assert(
          !std::is_same<typename std::iterator_traits<IterT>::difference_type, void>::value,
          "pgbar::__detail::container_iterator: the difference_type of the iterator shouldn't be 'void'"
        );
        auto dist = std::distance( _begin, _endpoint );
        if ( dist > 0 ) {
          is_reversed = false;
          extent = static_cast<SizeT>(dist);
        } else {
          is_reversed = true;
          extent = static_cast<SizeT>(-dist);
        }
        start = _begin; terminus = _endpoint;
        current = start; bar = &_bar;
        bar->set_task( extent ).set_step( 1 );
      }
      container_iterator( const container_iterator& _other ) {
        bar = _other.bar;
        start = _other.start; terminus = _other.terminus;
        current = start; extent = _other.extent;
        is_reversed = _other.is_reversed;
      }
      container_iterator( container_iterator&& _rhs ) {
        bar = _rhs.bar; // same as copy constructor
        start = std::move( _rhs.start ); terminus = std::move( _rhs.terminus );
        current = std::move( _rhs.current ); extent = _rhs.extent;
        is_reversed = _rhs.is_reversed;

        _rhs.bar = nullptr; // clear up
        _rhs.start = {}; _rhs.terminus = {};
        _rhs.terminus = {}; _rhs.extent = 0;
        _rhs.is_reversed = false;
      }
      ~container_iterator() {
        bar = nullptr;
      }
      __PGBAR_NODISCARD__ container_iterator begin() const {
        return container_iterator( *this );
      } // invokes copy constructor
      __PGBAR_NODISCARD__ container_iterator end() const {
        auto ed_pnt = container_iterator( *this );
        ed_pnt.start = terminus;
        ed_pnt.current = terminus;
        return ed_pnt;
      }
      __PGBAR_NODISCARD__ reference operator*() noexcept {
        return *current;
      }
      __PGBAR_NODISCARD__ pointer operator->() noexcept {
        return std::addressof( current );
      }
      __PGBAR_NODISCARD__ bool operator==( const container_iterator& _other ) const noexcept {
        return current == _other.current;
      }
      __PGBAR_NODISCARD__ bool operator!=( const container_iterator& _other ) const noexcept {
        return !(operator==( _other ));
      }
      container_iterator& operator++() {
        if ( is_reversed ) --current; else ++current; bar->update(); return *this;
      }
      container_iterator operator++( int ) {
        container_iterator before = *this; operator++(); return before;
      }
    };

#if __PGBAR_CXX20__
    template<typename T>
    concept ArithmeticType =
      std::is_arithmetic_v<std::decay_t<T>>;
#endif // __PGBAR_CXX20__
  } // namespace __detail

  /// @brief Update the progress bar based on the range specified by the parameters.
  /// @tparam EleT The type of generated elements.
  /// @tparam BarT The type of the progress bar.
  /// @param _bar The progress bar that will be updated.
  /// @return Return an iterator that moves unidirectionally within the range `[_startpoint, _endpoint-1]`.
  template<
#if __PGBAR_CXX20__
    __detail::ArithmeticType _EleT, __detail::PgbarType BarT
    , typename EleT = std::decay_t<_EleT>
  > __PGBAR_NODISCARD__ __detail::range_iterator<EleT, BarT> // return type
#else
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > __PGBAR_NODISCARD__ typename std::enable_if<
    std::is_arithmetic<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::range_iterator<EleT, BarT>>::type
#endif // __PGBAR_CXX20__
    inline range( _EleT&& _startpoint, _EleT&& _endpoint, _EleT&& _step, BarT& _bar ) {
    return __detail::range_iterator<EleT, BarT>( _startpoint, _endpoint, _step, _bar );
  }

#if __PGBAR_CXX20__
  template<typename _EleT, __detail::PgbarType BarT
    , typename EleT = typename std::decay_t<_EleT>
  > requires std::is_arithmetic_v<EleT>
  __PGBAR_NODISCARD__ __detail::range_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > __PGBAR_NODISCARD__ typename std::enable_if<
    std::is_arithmetic<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::range_iterator<EleT, BarT>
  >::type
#endif // __PGBAR_CXX20__
  inline range( _EleT&& _endpoint, _EleT&& _step, BarT& _bar ) {
    return __detail::range_iterator<EleT, BarT>( {}, _endpoint, _step, _bar );
  }

#if __PGBAR_CXX20__
  template<typename _EleT, __detail::PgbarType BarT
    , typename EleT = typename std::decay_t<_EleT>
  > requires std::integral<EleT>
  __PGBAR_NODISCARD__ __detail::range_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > __PGBAR_NODISCARD__ typename std::enable_if<
    std::is_integral<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::range_iterator<EleT, BarT>
  >::type
#endif // __PGBAR_CXX20__
    inline range( _EleT&& _startpoint, _EleT&& _endpoint, BarT& _bar ) {
    return __detail::range_iterator<EleT, BarT>( _startpoint, _endpoint, 1, _bar );
  }

#if __PGBAR_CXX20__
  template<typename _EleT, __detail::PgbarType BarT
    , typename EleT = typename std::decay_t<_EleT>
  > requires std::integral<EleT>
  __PGBAR_NODISCARD__ __detail::range_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > __PGBAR_NODISCARD__ typename std::enable_if<
    std::is_integral<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::range_iterator<EleT, BarT>
  >::type
#endif // __PGBAR_CXX20__
  inline range( _EleT&& _endpoint, BarT& _bar ) {
    return __detail::range_iterator<EleT, BarT>( {}, _endpoint, 1, _bar );
  }

  /// @brief Accepts the beginning and end iterators,
  /// @brief and updates the passed `_bar` based on the range defined by these two iterators.
  /// @tparam IterT The type of the iterators.
  template<
    typename _BeginT, typename _endpointT, typename BarT
    , typename IterT = typename std::decay<_BeginT>::type
#if __PGBAR_CXX20__
  > requires std::conjunction_v<
      std::negation<std::is_arithmetic<IterT>>,
      std::is_same<IterT, std::decay_t<_endpointT>>,
      is_pgbar<BarT>
  > __PGBAR_NODISCARD__ __detail::container_iterator<IterT, BarT>
#else
  > __PGBAR_NODISCARD__ typename std::enable_if<
    !std::is_arithmetic<IterT>::value &&
    std::is_same<IterT, typename std::decay<_endpointT>::type>::value &&
    is_pgbar<BarT>::value
    , __detail::container_iterator<IterT, BarT>
  >::type
#endif // __PGBAR_CXX20__
  inline range( _BeginT&& _startpoint, _endpointT&& _endpoint, BarT& _bar ) {
    return __detail::container_iterator<IterT, BarT>( _startpoint, _endpoint, _bar );
  }

  /// @brief Accepts a iterable container,
  /// @brief and updates the passed `_bar` based on the elements in the container.
  /// @param container A lvalue container whose iterator type name is iterator.
  /// @param _bar A pgbar object.
  /// @tparam ConT The type of the container.
  template<typename ConT, typename BarT>
#if __PGBAR_CXX20__
  requires std::conjunction_v<
    std::is_lvalue_reference<ConT>,
    std::negation<std::is_array<std::remove_reference_t<ConT>>>,
    is_pgbar<BarT>
  > __PGBAR_NODISCARD__ __detail::container_iterator<typename std::decay_t<ConT>::iterator, BarT>
#else
  __PGBAR_NODISCARD__ typename std::enable_if<
    std::is_lvalue_reference<ConT>::value &&
    !std::is_array<typename std::remove_reference<ConT>>::value &&
    is_pgbar<BarT>::value
    , __detail::container_iterator<typename std::decay<ConT>::type::iterator, BarT>
  >::type
#endif // __PGBAR_CXX20__
  inline range( ConT&& container, BarT& _bar ) {
#if __PGBAR_CXX20__
    return range( std::ranges::begin( std::forward<ConT>( container ) ), std::ranges::end( std::forward<ConT>( container ) ), _bar );
#else
    using std::begin; using std::end; // ADL
    return range( begin( std::forward<ConT>( container ) ), end( std::forward<ConT>( container ) ), _bar );
#endif // __PGBAR_CXX20__
  }

  /// @brief Accepts a original array,
  /// @brief and updates the passed `_bar` based on the elements in the container.
  /// @param container An original array.
  /// @param _bar A pgbar object.
  /// @tparam ArrT The type of the array.
  template<typename ArrT, typename BarT>
#if __PGBAR_CXX20__
    requires std::conjunction_v<
      std::is_lvalue_reference<ArrT>,
      std::is_array<std::remove_reference_t<ArrT>>,
      is_pgbar<BarT>
    > __PGBAR_NODISCARD__ __detail::container_iterator<std::decay_t<ArrT>, BarT> // here's the difference between array type and container type
#else
  __PGBAR_NODISCARD__ typename std::enable_if<
    std::is_lvalue_reference<ArrT>::value &&
    std::is_array<typename std::remove_reference<ArrT>::type>::value &&
    is_pgbar<BarT>::value
    , __detail::container_iterator<typename std::decay<ArrT>::type, BarT>
  >::type
#endif // __PGBAR_CXX20__
  inline range( ArrT&& container, BarT& _bar ) {// for original arrays
#if __PGBAR_CXX20__
    return range( std::ranges::begin( std::forward<ArrT>( container ) ), std::ranges::end( std::forward<ArrT>( container ) ), _bar );
#else
    using std::begin; using std::end; // ADL
    return range( begin( std::forward<ArrT>( container ) ), end( std::forward<ArrT>( container ) ), _bar );
#endif // __PGBAR_CXX20__
  }

} // namespace pgbar

#undef __PGBAR_CMP_V__
#undef __PGBAR_CXX20__

#endif
