// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023 Konvt
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
    #define __PGBAR_CXX20__
#endif // __cplusplus >= 202002L
#if __PGBAR_CMP_V__ >= 201703L
    #define __PGBAR_CXX17__
    #define __PGBAR_IF_CONSTEXPR__ constexpr
#else
    #define __PGBAR_IF_CONSTEXPR__
#endif // __cplusplus >= 201703L
#if __PGBAR_CMP_V__ >= 201402L
    #define __PGBAR_CXX14__
#endif // __cplusplus >= 201402L

namespace pgbar {

    template< // Numeric type iterator.
        typename EleT, typename BarT
#ifdef __PGBAR_CXX20__
    > requires std::is_same_v<BarT, pgbar> && std::is_arithmetic_v<EleT>
#elif defined(__PGBAR_CXX14__)
        , typename = std::enable_if_t<std::is_same<BarT, pgbar>::value && std::is_arithmetic<EleT>::value>>
#else
        , typename = typename std::enable_if<std::is_same<BarT, pgbar>::value && std::is_arithmetic<EleT>::value>::type>
#endif
    class range_iterator_arith {
        using SizeT = std::size_t;

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

        explicit range_iterator_arith(EleT _start, EleT _end, EleT _step, BarT& _bar) {
            if (_start > _end) {
                throw bad_pgbar {
                    std::string("bad_pgbar: invalid iteration range, '_start = ") +
                    std::to_string(_end) + std::string("' while '_end = ") +
                    std::to_string(_start) + std::string(1, '\'')
                };
            }
            bar = &_bar;
            EleT diff = _end-_start > 0 ? _end-_start : _start-_end;
            EleT denom = _step > 0 ? _step : -_step;
            cnt = 0, extent = static_cast<size_t>(diff/denom);
            start_point = _start; end_point = _end;
            step = _step; current = _start;
            bar->set_task(extent).set_step(step); // Only constructor with arguments will invoke.
        }
        explicit range_iterator_arith(EleT _start, EleT _end, BarT& _bar)
            :range_iterator_arith(_start, _end, 1, _bar) {}
        explicit range_iterator_arith(EleT _end, BarT& _bar)
            :range_iterator_arith({}, _end, 1, _bar) {}
        range_iterator_arith(const range_iterator_arith& _other) {
            bar = _other.bar; // The task number will not be set in copy constructor.
            cnt = _other.cnt; extent = _other.extent;
            start_point = _other.start_point;
            end_point = _other.end_point;
            step = _other.step; current = _other.current;
        }
        range_iterator_arith(range_iterator_arith&& _rhs) {
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
        ~range_iterator_arith() { bar = nullptr; }
        range_iterator_arith begin() const { return *this; } // invokes copy constructor
        range_iterator_arith end() const { // invokes copy constructor and move constructor
            range_iterator_arith ed_pnt = *this;
            ed_pnt.current = ed_pnt.start_point = ed_pnt.end_point;
            ed_pnt.cnt = extent;
            return ed_pnt;
        }
        EleT& operator*() noexcept { return current; }
        bool operator==(const range_iterator_arith& _other) const noexcept { return cnt == _other.cnt; }
        bool operator!=(const range_iterator_arith& _other) const noexcept { return !(operator==(_other)); }
        range_iterator_arith& operator++() { ++cnt; current += step; bar->update(); return *this; }
        range_iterator_arith operator++(int) { range_iterator_arith before = *this; operator++(); return before; }
    };

    template< // Iterator type iterator.
        typename IterT, typename BarT // `IterT` means iterator type
#ifdef __PGBAR_CXX20__
    > requires std::is_same_v<BarT, pgbar>
#elif defined(__PGBAR_CXX14__)
        , typename = std::enable_if_t<std::is_same<BarT, pgbar>::value && !std::is_arithmetic<IterT>::value>>
#else
        , typename = typename std::enable_if<std::is_same<BarT, pgbar>::value && !std::is_arithmetic<IterT>::value>::type>
#endif
    class range_iterator_iter {
        using SizeT = std::size_t;
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

        explicit range_iterator_iter(IterT _begin, IterT _end, BarT& _bar) {
            static_assert(
                !std::is_same<typename std::iterator_traits<IterT>::difference_type, void>::value,
                "range_iterator_iter error: the difference_type of the iterator shouldn't be 'void'"
            );
            auto dist = std::distance(_begin, _end);
            if (dist > 0) {
                is_reversed = false;
                extent = static_cast<SizeT>(dist);
                start = _begin; terminus = _end;
                current = start;
            } else {
                is_reversed = true;
                extent = static_cast<SizeT>(-dist);;
                start = _end; terminus = _begin;
                current = start;
            }
            bar = &_bar;
            bar->set_task(extent).set_step(1);
        }
        range_iterator_iter(const range_iterator_iter& _other) {
            bar = _other.bar;
            start = _other.start; terminus = _other.terminus;
            current = start; extent = _other.extent;
        }
        range_iterator_iter(range_iterator_iter&& _rhs) {
            bar = _rhs.bar; // same as copy constructor
            start = std::move(_rhs.start); terminus = std::move(_rhs.terminus);
            current = std::move(_rhs.current); extent = _rhs.extent;
            
            _rhs.bar = nullptr; // clear up
            _rhs.start = {}; _rhs.terminus = {};
            _rhs.terminus = {}; _rhs.extent = 0;
        }
        ~range_iterator_iter() { bar = nullptr; }
        range_iterator_iter begin() const { return *this; } // invokes copy constructor
        range_iterator_iter end() const { // invokes copy constructor and move constructor
            range_iterator_iter ed_pnt = *this;
            ed_pnt.start = terminus;
            ed_pnt.current = terminus;
            return ed_pnt;
        }
        EleT& operator*() noexcept { return *current; }
        bool operator==(const range_iterator_iter& _other) const noexcept { return current == _other.current; }
        bool operator!=(const range_iterator_iter& _other) const noexcept { return !(operator==(_other)); }
        range_iterator_iter& operator++() { if (is_reversed) --current; else ++current; bar->update(); return *this; }
        range_iterator_iter operator++(int) { range_iterator_iter before = *this; operator++(); return before; }
    };

#ifdef __PGBAR_CXX20__
    template<typename EleT>
    concept ValidEleT = 
        std::is_arithmetic_v<std::decay_t<EleT>>;
#endif

    /// @brief Update the progress bar based on the range specified by the parameters.
    /// @tparam EleT The type of generated elements.
    /// @tparam BarT The type of the progress bar.
    /// @param _bar The progress bar that will be updated.
    /// @return Return an iterator that moves unidirectionally within the range `[_start, _end-1]`.
    #ifdef __PGBAR_CXX20__
    template<
        ValidEleT _EleT, typename BarT
        , typename EleT = std::decay_t<_EleT>
    > range_iterator_arith<EleT, BarT> // return type
#else
    template<
        typename _EleT, typename BarT
#ifdef __PGBAR_CXX14__
        , typename EleT = std::decay_t<_EleT>
    > std::enable_if_t<
        std::is_arithmetic<EleT>::value
        , range_iterator_arith<EleT, BarT>>
#else
        , typename EleT = typename std::decay<_EleT>::type
    > typename std::enable_if<
        std::is_arithmetic<EleT>::value
        , range_iterator_arith<EleT, BarT>>::type
#endif
#endif
    inline range(_EleT&& _start, _EleT&& _end, _EleT&& _step, BarT& _bar)
        { return range_iterator_arith<EleT, BarT>(_start, _end, _step, _bar); }

#ifdef __PGBAR_CXX20__
    template<
        ValidEleT _EleT, typename BarT
        , typename EleT = std::decay_t<_EleT>
    > range_iterator_arith<EleT, BarT> // return type
#else
    template<
        typename _EleT, typename BarT
#ifdef __PGBAR_CXX14__
        , typename EleT = std::decay_t<_EleT>
    > std::enable_if_t<
        std::is_arithmetic<EleT>::value
        , range_iterator_arith<EleT, BarT>>
#else
        , typename EleT = typename std::decay<_EleT>::type
    > typename std::enable_if<
        std::is_arithmetic<EleT>::value
        , range_iterator_arith<EleT, BarT>>::type
#endif
#endif
    inline range(_EleT&& _start, _EleT&& _end, BarT& _bar)
        { return range_iterator_arith<EleT, BarT>(_start, _end, _bar); }

#ifdef __PGBAR_CXX20__
    template<
        ValidEleT _EleT, typename BarT
        , typename EleT = std::decay_t<_EleT>
    > range_iterator_arith<EleT, BarT> // return type
#else
    template<
        typename _EleT, typename BarT
#ifdef __PGBAR_CXX14__
        , typename EleT = std::decay_t<_EleT>
    > std::enable_if_t<
        std::is_arithmetic<EleT>::value
        , range_iterator_arith<EleT, BarT>>
#else
        , typename EleT = typename std::decay<_EleT>::type
    > typename std::enable_if<
        std::is_arithmetic<EleT>::value
        , range_iterator_arith<EleT, BarT>>::type
#endif
#endif
    inline range(_EleT&& _end, BarT& _bar)
        { return range_iterator_arith<EleT, BarT>(_end, _bar); }

    /// @brief Accepts the beginning and end iterators,
    /// @brief and updates the passed `_bar` based on the range defined by these two iterators.
    /// @tparam IterT The type of the iterators.
    template<
        typename _BeginT, typename _EndT, typename BarT
#ifdef __PGBAR_CXX14__
        , typename IterT = std::decay_t<_BeginT>
#ifdef __PGBAR_CXX20__
    > requires std::is_same_v<IterT, std::decay_t<_EndT>>
    range_iterator_iter<IterT, BarT> // return type
#else
    > std::enable_if_t<
        std::is_same<IterT, std::decay_t<_EndT>>::value
        , range_iterator_iter<IterT, BarT>>
#endif
#else
        , typename IterT = typename std::decay<_BeginT>::type
    > typename std::enable_if<
        !std::is_arithmetic<IterT>::value &&
        std::is_same<IterT, typename std::decay<_EndT>::type>::value
        , range_iterator_iter<IterT, BarT>>::type
#endif
    inline range(_BeginT&& _start, _EndT&& _end, BarT& _bar)
        { return range_iterator_iter<IterT, BarT>(_start, _end, _bar); }

} // namespace pgbar

#undef __PGBAR_CMP_V__
#undef __PGBAR_CXX14__
#undef __PGBAR_CXX17__
#undef __PGBAR_CXX20__

#endif
