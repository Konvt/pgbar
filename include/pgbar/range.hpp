// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __PGBAR_RANGE_HPP__
# define __PGBAR_RANGE_HPP__

#include "pgbar.hpp" // Other required header files have been included in pgbar.hpp.
#include <algorithm> // std::distance

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

#ifndef PGBAR_NOT_COL
# define __PGBAR_ASSERT_FAILURE__ "\033[1;31m"
# define __PGBAR_DEFAULT_COL__ "\033[0m"
#else
# define __PGBAR_ASSERT_FAILURE__ ""
# define __PGBAR_DEFAULT_COL__ ""
#endif // PGBAR_NOT_COL

namespace pgbar {
  namespace __detail {
    template<typename EleT, typename BarT>
    class numeric_iterator { // for number
      static_assert(
        std::is_arithmetic<EleT>::value &&
        is_pgbar<BarT>::value,
        __PGBAR_ASSERT_FAILURE__
        "numeric_iterator: Only available for numeric types"
        __PGBAR_DEFAULT_COL__
      );

      using SizeT = size_t;

      BarT* bar;
      SizeT cnt, extent; // type-independent iteration
      EleT start_point, end_point, step;
      EleT current;

    public:
      using iterator_category = std::output_iterator_tag;
      using value_type = EleT;
      using difference_type = void;
      using pointer = EleT*;
      using reference = EleT;

      explicit numeric_iterator( EleT _startpoint, EleT _endpoint, EleT _step, BarT& _bar ) {
        if ( _startpoint > _endpoint ) {
          throw bad_pgbar {
            std::string( "numeric_iterator: invalid iteration range, '_startpoint = " ) +
            std::to_string( _endpoint ) + std::string( "' while '_endpoint = " ) +
            std::to_string( _startpoint ) + std::string( 1, '\'' )
          };
        }
        bar = &_bar;
        EleT diff = _endpoint > _startpoint ? _endpoint - _startpoint : _startpoint - _endpoint;
        EleT denom = _step > 0 ? _step : -_step;
        cnt = 0, extent = static_cast<size_t>(diff / denom);
        start_point = _startpoint; end_point = _endpoint;
        step = _step; current = _startpoint;
        bar->set_task( extent ).set_step( 1 ); // Only constructor with arguments will invoke these func.
      }
      numeric_iterator( const numeric_iterator& _other ) {
        bar = _other.bar; // The task number will not be set in copy constructor.
        cnt = _other.cnt; extent = _other.extent;
        start_point = _other.start_point;
        end_point = _other.end_point;
        step = _other.step; current = _other.current;
      }
      numeric_iterator( numeric_iterator&& _rhs ) {
        bar = _rhs.bar; // same as copy constructor
        cnt = _rhs.cnt; extent = _rhs.extent;
        start_point = _rhs.start_point;
        end_point = _rhs.end_point;
        step = _rhs.step; current = _rhs.current;

        _rhs.bar = nullptr; // clear up
        _rhs.cnt = _rhs.extent = 0;
        _rhs.start_point = {}; _rhs.end_point = {};
        _rhs.step = {}; _rhs.current = {};
      }
      ~numeric_iterator() {
        bar = nullptr;
      }
      numeric_iterator begin() const {
        return *this;
      } // invokes copy constructor
      numeric_iterator end() const { // invokes copy constructor and move constructor
        numeric_iterator ed_pnt = *this;
        ed_pnt.current = ed_pnt.start_point = ed_pnt.end_point;
        ed_pnt.cnt = extent;
        return ed_pnt;
      }
      reference operator*() noexcept {
        return current;
      }
      pointer operator->() noexcept {
        return std::addressof( current );
      }
      bool operator==( const numeric_iterator& _other ) const noexcept {
        return cnt == _other.cnt;
      }
      bool operator!=( const numeric_iterator& _other ) const noexcept {
        return !(operator==( _other ));
      }
      numeric_iterator& operator++() {
        ++cnt; current += step; bar->update(); return *this;
      }
      numeric_iterator operator++( int ) {
        numeric_iterator before = *this; operator++(); return before;
      }
    };

    template<typename IterT, typename BarT>
    class container_iterator { // for container
      static_assert( // `IterT` means iterator type
        !std::is_arithmetic<IterT>::value &&
        is_pgbar<BarT>::value,
        __PGBAR_ASSERT_FAILURE__
        "container_iterator: Only available for container types"
        __PGBAR_DEFAULT_COL__
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
          __PGBAR_ASSERT_FAILURE__
          "container_iterator: the difference_type of the iterator shouldn't be 'void'"
          __PGBAR_DEFAULT_COL__
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
      container_iterator begin() const {
        return container_iterator( *this );
      } // invokes copy constructor
      container_iterator end() const {
        auto ed_pnt = container_iterator( *this );
        ed_pnt.start = terminus;
        ed_pnt.current = terminus;
        return ed_pnt;
      }
      reference operator*() noexcept {
        return *current;
      }
      pointer operator->() noexcept {
        return std::addressof( current );
      }
      bool operator==( const container_iterator& _other ) const noexcept {
        return current == _other.current;
      }
      bool operator!=( const container_iterator& _other ) const noexcept {
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
#if __PGBAR_CXX20__
  template<__detail::ArithmeticType _EleT, __detail::PgbarType BarT
    , typename EleT = std::decay_t<_EleT>
  > __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_arithmetic<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::numeric_iterator<EleT, BarT>>::type
#endif // __PGBAR_CXX20__
    inline range( _EleT&& _startpoint, _EleT&& _endpoint, _EleT&& _step, BarT& _bar ) {
    return __detail::numeric_iterator<EleT, BarT>( _startpoint, _endpoint, _step, _bar );
  }

#if __PGBAR_CXX20__
  template<typename _EleT, __detail::PgbarType BarT
    , typename EleT = typename std::decay_t<_EleT>
  > requires std::is_arithmetic_v<EleT>
  __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_arithmetic<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::numeric_iterator<EleT, BarT>
  >::type
#endif // __PGBAR_CXX20__
    inline range( _EleT&& _endpoint, _EleT&& _step, BarT& _bar ) {
    return __detail::numeric_iterator<EleT, BarT>( {}, _endpoint, _step, _bar );
  }

#if __PGBAR_CXX20__
  template<typename _EleT, __detail::PgbarType BarT
    , typename EleT = typename std::decay_t<_EleT>
  > requires std::integral<EleT>
  __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_integral<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::numeric_iterator<EleT, BarT>
  >::type
#endif // __PGBAR_CXX20__
    inline range( _EleT&& _startpoint, _EleT&& _endpoint, BarT& _bar ) {
    return __detail::numeric_iterator<EleT, BarT>( _startpoint, _endpoint, 1, _bar );
  }

#if __PGBAR_CXX20__
  template<typename _EleT, __detail::PgbarType BarT
    , typename EleT = typename std::decay_t<_EleT>
  > requires std::integral<EleT>
  __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_integral<EleT>::value &&
    is_pgbar<BarT>::value
    , __detail::numeric_iterator<EleT, BarT>
  >::type
#endif // __PGBAR_CXX20__
    inline range( _EleT&& _endpoint, BarT& _bar ) {
    return __detail::numeric_iterator<EleT, BarT>( {}, _endpoint, 1, _bar );
  }

  /// @brief Accepts the beginning and end iterators,
  /// @brief and updates the passed `_bar` based on the range defined by these two iterators.
  /// @tparam IterT The type of the iterators.
  template<
    typename _BeginT, typename _endpointT, typename BarT
    , typename IterT = typename std::decay<_BeginT>::type
#if __PGBAR_CXX20__
  > requires (
      !std::is_arithmetic_v<IterT> &&
      std::same_as<IterT, std::decay_t<_endpointT>> &&
      __detail::PgbarType<BarT>
  ) __detail::container_iterator<IterT, BarT>
#else
  > typename std::enable_if<
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
  requires (
    std::is_lvalue_reference_v<ConT> &&
    !std::is_array_v<std::remove_reference_t<ConT>> &&
    __detail::PgbarType<BarT>
  ) __detail::container_iterator<typename std::decay_t<ConT>::iterator, BarT>
#else
  typename std::enable_if<
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
  requires (
    std::is_lvalue_reference_v<ArrT> &&
    std::is_array_v<std::remove_reference_t<ArrT>> &&
    __detail::PgbarType<BarT>
  ) __detail::container_iterator<std::decay_t<ArrT>, BarT> // here's the difference between array type and container type
#else
  typename std::enable_if<
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

#undef __PGBAR_ASSERT_FAILURE__
#undef __PGBAR_DEFAULT_COL__

#undef __PGBAR_CMP_V__
#undef __PGBAR_CXX20__

#endif
