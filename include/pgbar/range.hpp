// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __PGBAR_RANGE_HPP__
  #define __PGBAR_RANGE_HPP__

#include "pgbar.hpp" // Other required header files have been included in pgbar.hpp.
#include <iterator>  // marks iterator tags
#include <algorithm> // std::distance

#if defined(_MSVC_VER) && defined(_MSVC_LANG) // for msvc
  #define __PGBAR_CMP_V__ _MSVC_LANG
#else
  #define __PGBAR_CMP_V__ __cplusplus
#endif

#if __PGBAR_CMP_V__ >= 202002L
  #include <concepts>
  #define __PGBAR_CXX20__
#endif // __cplusplus >= 202002L
#if __PGBAR_CMP_V__ >= 201402L
  #define __PGBAR_CXX14__
#endif // __cplusplus >= 201402L

namespace pgbar {
  namespace __detail {
    template<typename EleT, typename BarT>
    class numeric_iterator { // for number
      static_assert(std::is_same<BarT, pgbar<>>::value
        && std::is_arithmetic<EleT>::value);

      using SizeT = size_t;

      BarT *bar;
      SizeT cnt, extent; // type-independent iteration
      EleT start_point, end_point, step;
      EleT current;

    public:
      using iterator_category = std::output_iterator_tag;
      using value_type = EleT;
      using difference_type = void;
      using pointer = EleT*;
      using reference = EleT;

      explicit numeric_iterator(EleT _start, EleT _end, EleT _step, BarT& _bar) {
        if (_start > _end) {
          throw bad_pgbar {
            std::string("bad_pgbar: invalid iteration range, '_start = ") +
            std::to_string(_end) + std::string("' while '_end = ") +
            std::to_string(_start) + std::string(1, '\'')
          };
        }
        bar = &_bar;
        EleT diff = _end > _start ? _end - _start : _start - _end;
        EleT denom = _step > 0 ? _step : -_step;
        cnt = 0, extent = static_cast<size_t>(diff / denom);
        start_point = _start; end_point = _end;
        step = _step; current = _start;
        bar->set_task(extent).set_step(step); // Only constructor with arguments will invoke these func.
      }
      explicit numeric_iterator(EleT _start, EleT _end, BarT& _bar)
        :numeric_iterator(_start, _end, 1, _bar) {}
      explicit numeric_iterator(EleT _end, BarT& _bar)
        :numeric_iterator({}, _end, 1, _bar) {}
      numeric_iterator(const numeric_iterator& _other) {
        bar = _other.bar; // The task number will not be set in copy constructor.
        cnt = _other.cnt; extent = _other.extent;
        start_point = _other.start_point;
        end_point = _other.end_point;
        step = _other.step; current = _other.current;
      }
      numeric_iterator(numeric_iterator&& _rhs) {
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
        return std::addressof(current);
      }
      bool operator==(const numeric_iterator& _other) const noexcept {
        return cnt == _other.cnt;
      }
      bool operator!=(const numeric_iterator& _other) const noexcept {
        return !(operator==(_other));
      }
      numeric_iterator& operator++() {
        ++cnt; current += step; bar->update(); return *this;
      }
      numeric_iterator operator++(int) {
        numeric_iterator before = *this; operator++(); return before;
      }
    };

    template<typename IterT, typename BarT>
    class container_iterator { // for container
      static_assert(std::is_same<BarT, pgbar<>>::value
        && !std::is_arithmetic<IterT>::value); // `IterT` means iterator type

      using SizeT = size_t;
      using EleT = typename std::iterator_traits<IterT>::value_type;

      BarT *bar;
      IterT start, terminus, current;
      SizeT extent;
      bool is_reversed;

    public:
      using iterator_category = std::output_iterator_tag;
      using value_type = EleT;
      using difference_type = void;
      using pointer = EleT*;
      using reference = EleT&;

      explicit container_iterator(IterT _begin, IterT _end, BarT& _bar) {
        static_assert(
          !std::is_same<typename std::iterator_traits<IterT>::difference_type, void>::value,
          "container_iterator error: the difference_type of the iterator shouldn't be 'void'"
        );
        auto dist = std::distance(_begin, _end);
        if (dist > 0) {
          is_reversed = false;
          extent = static_cast<SizeT>(dist);
        } else {
          is_reversed = true;
          extent = static_cast<SizeT>(-dist);
        }
        start = _begin; terminus = _end;
        current = start; bar = &_bar;
        bar->set_task(extent).set_step(1);
      }
      container_iterator(const container_iterator& _other) {
        bar = _other.bar;
        start = _other.start; terminus = _other.terminus;
        current = start; extent = _other.extent;
        is_reversed = _other.is_reversed;
      }
      container_iterator(container_iterator&& _rhs) {
        bar = _rhs.bar; // same as copy constructor
        start = std::move(_rhs.start); terminus = std::move(_rhs.terminus);
        current = std::move(_rhs.current); extent = _rhs.extent;
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
        return container_iterator(*this);
      } // invokes copy constructor
      container_iterator end() const {
        auto ed_pnt = container_iterator(*this);
        ed_pnt.start = terminus;
        ed_pnt.current = terminus;
        return ed_pnt;
      }
      reference operator*() noexcept {
        return *current;
      }
      pointer operator->() noexcept {
        return std::addressof(current);
      }
      bool operator==(const container_iterator& _other) const noexcept {
        return current == _other.current;
      }
      bool operator!=(const container_iterator& _other) const noexcept {
        return !(operator==(_other));
      }
      container_iterator& operator++() {
        if (is_reversed) --current; else ++current; bar->update(); return *this;
      }
      container_iterator operator++(int) {
        container_iterator before = *this; operator++(); return before;
      }
    };

#ifdef __PGBAR_CXX20__
    template<typename T>
    concept ArithmeticT =
      std::is_arithmetic_v<std::decay_t<T>>;
#endif
  } // namespace __detail

  /// @brief Update the progress bar based on the range specified by the parameters.
  /// @tparam EleT The type of generated elements.
  /// @tparam BarT The type of the progress bar.
  /// @param _bar The progress bar that will be updated.
  /// @return Return an iterator that moves unidirectionally within the range `[_start, _end-1]`.
#ifdef __PGBAR_CXX20__
  template<
    __detail::ArithmeticT _EleT, typename BarT
    , typename EleT = std::decay_t<_EleT>
    > __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_arithmetic<EleT>::value
    , __detail::numeric_iterator<EleT, BarT>>::type
#endif
    inline range(_EleT&& _start, _EleT&& _end, _EleT&& _step, BarT& _bar) {
    return __detail::numeric_iterator<EleT, BarT>(_start, _end, _step, _bar);
  }

#ifdef __PGBAR_CXX20__
  template<
    __detail::ArithmeticT _EleT, typename BarT
    , typename EleT = std::decay_t<_EleT>
  > __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_arithmetic<EleT>::value
    , __detail::numeric_iterator<EleT, BarT>
  >::type
#endif
    inline range(_EleT&& _start, _EleT&& _end, BarT& _bar) {
    return __detail::numeric_iterator<EleT, BarT>(_start, _end, _bar);
  }

#ifdef __PGBAR_CXX20__
  template<
    __detail::ArithmeticT _EleT, typename BarT
    , typename EleT = std::decay_t<_EleT>
  > __detail::numeric_iterator<EleT, BarT> // return type
#else
  template<
    typename _EleT, typename BarT
    , typename EleT = typename std::decay<_EleT>::type
  > typename std::enable_if<
    std::is_arithmetic<EleT>::value
    , __detail::numeric_iterator<EleT, BarT>
  >::type
#endif
    inline range(_EleT&& _end, BarT& _bar) {
    return __detail::numeric_iterator<EleT, BarT>(_end, _bar);
  }

  /// @brief Accepts the beginning and end iterators,
  /// @brief and updates the passed `_bar` based on the range defined by these two iterators.
  /// @tparam IterT The type of the iterators.
  template<
    typename _BeginT, typename _EndT, typename BarT
    , typename IterT = typename std::decay<_BeginT>::type
#ifdef __PGBAR_CXX20__
  > requires (
      !std::is_arithmetic<IterT>::value &&
      std::same_as<IterT, std::decay_t<_EndT>>
  ) __detail::container_iterator<IterT, BarT>
#else
  > typename std::enable_if<
    !std::is_arithmetic<IterT>::value &&
    std::is_same<IterT, typename std::decay<_EndT>::type>::value
    , __detail::container_iterator<IterT, BarT>
  >::type
#endif
    inline range(_BeginT&& _start, _EndT&& _end, BarT& _bar) {
    return __detail::container_iterator<IterT, BarT>(_start, _end, _bar);
  }

  /// @brief Accepts a iterable container,
  /// @brief and updates the passed `_bar` based on the elements in the container.
  /// @tparam _ConT The type of the container.
  template<
    typename _ConT, typename BarT
    , typename ConT = typename std::decay<_ConT>::type
  >
#ifdef __PGBAR_CXX20__
  requires (
    std::is_lvalue_reference_v<std::remove_const_t<_ConT>> &&
    !std::is_array_v<std::remove_reference_t<_ConT>>
  ) __detail::container_iterator<typename ConT::iterator, BarT>
#else
  typename std::enable_if<
    std::is_lvalue_reference<typename std::remove_const<_ConT>::type>::value &&
    !std::is_array<typename std::remove_reference<_ConT>>::value
    , __detail::container_iterator<typename ConT::iterator, BarT>
  >::type
#endif
    inline range(_ConT&& container, BarT& _bar) {
    using std::begin; using std::end; // ADL
    return range(begin(std::forward<_ConT>(container)), end(std::forward<_ConT>(container)), _bar);
  }

  /// @brief Accepts a original array,
  /// @brief and updates the passed `_bar` based on the elements in the container.
  /// @tparam _ArrT The type of the array.
  template<
    typename _ArrT, typename BarT
    , typename ArrT = typename std::decay<_ArrT>::type
  >
#ifdef __PGBAR_CXX20__
  requires (
    std::is_lvalue_reference_v<std::remove_const_t<_ArrT>> &&
    std::is_array_v<std::remove_reference_t<_ArrT>>
  ) __detail::container_iterator<ArrT, BarT> // here's the difference between array type and container type
#else
  typename std::enable_if<
    std::is_lvalue_reference<typename std::remove_const<_ArrT>::type>::value &&
    std::is_array<typename std::remove_reference<_ArrT>::type>::value
    , __detail::container_iterator<ArrT, BarT>
  >::type
#endif
    inline range(_ArrT&& container, BarT& _bar) {// for original arrays
    using std::begin; using std::end; // ADL
    return range(begin(std::forward<_ArrT>(container)), end(std::forward<_ArrT>(container)), _bar);
  }

} // namespace pgbar

#undef __PGBAR_CMP_V__
#undef __PGBAR_CXX20__

#endif
