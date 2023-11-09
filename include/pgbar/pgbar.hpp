// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023 Konvt
#pragma once

#ifndef __PROGRESSBAR_HPP__
    #define __PROGRESSBAR_HPP__

#include <cmath>       // std::round()
#include <type_traits> // SFINAE
#include <utility>     // std::pair
#include <string>      // std::string
#include <chrono>      // as u know
#include <exception>   // bad_pgbar exception
#include <iostream>    // std::cout, the output stream object used.

#if defined(__GNUC__) || defined(__clang__)
    #define __PGBAR_INLINE_FUNC__ __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define __PGBAR_INLINE_FUNC__ __forceinline
#else
    #define __PGBAR_INLINE_FUNC__
#endif

#if defined(_MSVC_VER) && defined(_MSVC_LANG) // for msvc
    #define __PGBAR_CMP_V__ _MSVC_LANG
#else
    #define __PGBAR_CMP_V__ __cplusplus
#endif

#ifdef _WIN32
    #include <io.h>
    #define __PGBAR_WIN__
#elif defined(__unix__)
    #include <unistd.h>
    #define __PGBAR_UNIX__
#else
    #define __PGBAR_UNKNOW_PLATFORM__
#endif

#if __PGBAR_CMP_V__ >= 202002L
    #include <format>
    #include <concepts>
    #define __PGBAR_CXX20__
#endif // __cplusplus >= 202002L
#if __PGBAR_CMP_V__ >= 201703L
    #include <string_view>
    #define __PGBAR_CXX17__
    #define __PGBAR_IF_CONSTEXPR__ constexpr
#else
    #define __PGBAR_IF_CONSTEXPR__
#endif // __cplusplus >= 201703L
#if __PGBAR_CMP_V__ >= 201402L
    #define __PGBAR_CXX14__
#endif // __cplusplus >= 201402L

#ifndef PGBAR_NOT_COL
    /* Specify the color and font style for the status bar. */
    #define __PGBAR_COL__ "\033[1;36m"
    #define __PGBAR_DEFAULT_COL__ "\033[0m"
#else
    #define __PGBAR_COL__ ""
    #define __PGBAR_DEFAULT_COL__ ""
#endif

namespace pgbar {

    class bad_pgbar : public std::exception {
        std::string message;
    public:
        bad_pgbar(const std::string& _mes): message {_mes} {}
        virtual ~bad_pgbar() {}
        virtual const char* what() const noexcept { return message.c_str(); }
    };

    struct style_opts {
        using OptT = uint8_t;
        static constexpr OptT bar = 1 << 0;
        static constexpr OptT percentage = 1 << 1;
        static constexpr OptT task_counter = 1 << 2;
        static constexpr OptT rate = 1 << 3;
        static constexpr OptT countdown = 1 << 4;
        static constexpr OptT entire = ~0;
    };

    class pgbar {
        /* auxiliary type definition */

        using SizeT = std::size_t;
        using StrT = std::string;
#ifdef __PGBAR_CXX17__
        using ReadOnlyT = std::string_view;
#else
        using ReadOnlyT = const StrT;
#endif
        enum class txt_layut { align_left, align_right, align_center }; // text layout

        /* static variable definition */

        static constexpr char blank = ' ';
        static constexpr char reboot = '\r';
        static constexpr char backspace = '\b';
        static constexpr SizeT ratio_len = 7; // The length of `100.00%`.
        static constexpr SizeT time_len = 11; // The length of `9.9m < 9.9m`.
        static constexpr SizeT rate_len = 10; // The length of `999.99 kHz`.
        static const StrT rightward;          // ASCII Escape Sequence: moves the cursor right.
        static const StrT l_status;           // The left bracket of status bar.
        static const StrT r_status;           // The right bracket of status bar.
        static const StrT col_fmt;            // The color and font style of status bar.
        static const StrT defult_col;         // The default color and font style.

        /* private data member */

        std::ostream *stream;
        StrT todo_ch, done_ch;
        StrT l_bracket, r_bracket;
        SizeT step; // The number of steps per invoke `update()`.
        SizeT total_tsk, done_cnt;
        SizeT bar_length; // The length of the progress bar.
        style_opts::OptT option;
        bool in_terminal, is_done, is_invoked;

        /* private method member */

        /// @brief Format the `_str`.
        /// @tparam _style Format mode.
        /// @param _width Target length, do nothing if `_width` less than the length of `_str`.
        /// @param _str The string will be formatted.
        /// @return Formatted string.
        template<txt_layut _style>
        __PGBAR_INLINE_FUNC__ static StrT formatter(SizeT _width, StrT _str) {
            if (_width == 0) return {}; // `StrT` is only effective for the progress bar.
            if (_str.size() >= _width) return _str; // The other are all `StrT` type.
            // Thus the return type of `formatter` is `StrT`.
#ifdef __PGBAR_CXX20__
            if __PGBAR_IF_CONSTEXPR__ (_style == txt_layut::align_right)
                return std::format("{:>{}}", std::move(_str), _width);
            else if __PGBAR_IF_CONSTEXPR__ (_style == txt_layut::align_left)
                return std::format("{:<{}}", std::move(_str), _width);
#else
            SizeT str_size = _str.size();
            if __PGBAR_IF_CONSTEXPR__ (_style == txt_layut::align_right)
                return StrT(_width-str_size, blank) + std::move(_str);
            else if __PGBAR_IF_CONSTEXPR__ (_style == txt_layut::align_left)
                return std::move(_str) + StrT(_width-str_size, blank);
#endif
            else {
                _width -= _str.size();
                SizeT r_blank = _width/2;
                return StrT(_width-r_blank, blank) + std::move(_str) + StrT(r_blank, blank);
            }
        }
        /// @brief Copy a string mutiple times and concatenate them together.
        /// @tparam S The type of the string.
        /// @param _time Copy times.
        /// @param _src The string to be copied.
        /// @return The string copied mutiple times.
        static StrT bulk_copy(SizeT _time, ReadOnlyT _src) {
            if (_time == 0 || _src.size() == 0) return {};
            StrT ret; ret.reserve(_src.size()*_time);
            for (SizeT _ = 0; _ < _time; ++_)
                ret.append(_src, 0);
            return ret;
        }
        __PGBAR_INLINE_FUNC__ bool check_output_stream() {
            if (stream != &std::cout || stream != &std::cerr)
                return true; // Custom object, the program does not block output.
#ifdef __PGBAR_WIN__
            if (_isatty(_fileno(stdout)))
                return true;
#elif defined(__PGBAR_UNIX__)
            if (isatty(fileno(stdout)))
                return true;
#elif defined(__PGBAR_UNKNOW_PLATFORM__)
            if (true) return true;
#endif
            else return false;
        }

        __PGBAR_INLINE_FUNC__ bool skip_update(std::chrono::duration<SizeT, std::nano> interval) {
            if (!in_terminal) return true;
            static SizeT priod = 0, target_cnt = 0;
            if (!is_invoked) {
                priod = 0;
                return true;
            }
            if (done_cnt % priod != 0) return true;

            return false;
        }
        __PGBAR_INLINE_FUNC__ StrT show_bar(double percent) {
            static double lately_perc = 0;
            //if (!is_invoked); // nothing
            if (is_invoked && done_cnt != total_tsk && (percent-lately_perc)*100.0 < 1.0)
                return {};

            StrT buf {}; buf.reserve(l_bracket.size()+r_bracket.size()+bar_length+1);
            SizeT done_len = std::round(bar_length*percent);
            buf.append(
                bulk_copy(1, l_bracket) + bulk_copy(done_len, done_ch) +
                bulk_copy(bar_length-done_len, todo_ch) + bulk_copy(1, r_bracket) +
                StrT(1, blank)
            );
            return buf;
        }
        __PGBAR_INLINE_FUNC__ StrT show_proportion(double percent) {
            static double lately_perc = 0;
            if (!is_invoked){
                lately_perc = 0;
                static const StrT default_str =
                    formatter<txt_layut::align_left>(ratio_len, "0.00%");
                return default_str;
            }
            if (is_invoked && done_cnt != total_tsk && (percent-lately_perc)*100.0 < 0.08)
                return {};
            lately_perc = percent;

            StrT proportion = std::to_string(percent*100);
            proportion.resize(proportion.find('.')+3);

            return formatter<txt_layut::align_left>(
                ratio_len-proportion.size(),
                std::move(proportion) + StrT(1, '%')
            );
        }
        __PGBAR_INLINE_FUNC__ StrT show_remain_task() {
            //if (!is_invoked); // nothing
            StrT total_str = std::to_string(total_tsk);
            SizeT size = total_str.size();
            return (
                formatter<txt_layut::align_right>(size, std::to_string(done_cnt)) +
                StrT(1, '/') + std::move(total_str)
            );
        }
        __PGBAR_INLINE_FUNC__ StrT show_rate(std::chrono::duration<SizeT, std::nano> interval) {
            using namespace std::chrono;
            static std::chrono::duration<SizeT, std::nano> invoke_interval {};

            if (!is_invoked) {
                invoke_interval = {};
                static const StrT default_str =
                    formatter<txt_layut::align_center>(rate_len, "0.00 Hz");
                return default_str;
            }

            invoke_interval = (invoke_interval + interval)/2; // each invoke interval
            SizeT frequency = duration_cast<nanoseconds>(seconds(1)) / invoke_interval;

            auto splice = [](double val) -> StrT {
                StrT str = std::to_string(val);
                str.resize(str.find('.')+3); // Keep two decimal places.
                return str;
            };

            StrT rate {}; rate.reserve(rate_len);
            if (frequency < 1e3) // < 1Hz => 999.99 Hz
                rate.append(splice(frequency) + StrT(" Hz"));
            else if (frequency < 1e6) // < 1 kHz => 999.99 kHz
                rate.append(splice(frequency/1e3) + StrT(" kHz"));
            else if (frequency < 1e9) // < 1 MHz => 999.99 MHz
                rate.append(splice(frequency/1e6) + StrT(" MHz"));
            else { // < 1 GHz => 999.99 GHz
                double temp = frequency/1e9;
                if (temp > 999.99) rate.append("999.99 GHz");
                else rate.append(splice(temp) + StrT(" GHz"));
            }

            return formatter<txt_layut::align_center>(rate_len, std::move(rate));
        }
        __PGBAR_INLINE_FUNC__ StrT show_time(std::chrono::duration<SizeT, std::nano> interval) {
            using namespace std::chrono;

            if (!is_invoked) {
                static const StrT default_str =
                    formatter<txt_layut::align_center>(time_len, "0s < 99h");
                return default_str;
            }

            auto splice = [](double val) -> StrT {
                StrT str = std::to_string(val);
                str.resize(str.find('.')+2); // Keep one decimal places.
                return str;
            };
            auto to_time = [&splice](int64_t sec) -> StrT {
                if (sec < 60) // < 1 minute => 59s
                    return std::to_string(sec) + StrT(1, 's');
                else if (sec < 60*9) // < 9 minutes => 9.9m
                    return splice(static_cast<double>(sec) / 60.0) + StrT(1, 'm');
                else if (sec < 60*60) // >= 9 minutes => 59m
                    return std::to_string(sec / 60) + StrT(1, 'm');
                else if (sec < 60*60*9) // < 9 hour => 9.9h
                    return splice(static_cast<double>(sec)/(60.0*60.0)) + StrT(1, 'h');
                else { // >= 9 hour => 999h
                    if (sec > 60*60*99) return "99h";
                    else return std::to_string(sec/(60*60)) + StrT(1, 'h');
                }
            };

            return formatter<txt_layut::align_center>(
                time_len, to_time(duration_cast<seconds>(interval*done_cnt).count()) +
                StrT(" < ") + to_time(duration_cast<seconds>(interval*(total_tsk-done_cnt)).count())
            );
        }
        /// @brief Based on the value of `option` and bitwise operations,
        /// @brief determine which part of the string needs to be concatenated.
        /// @param percent The percentage of the current task execution.
        /// @return The progress bar that has been assembled but is pending output.
        __PGBAR_INLINE_FUNC__ std::pair<StrT, StrT>
        switch_feature(double percent, std::chrono::duration<SizeT, std::nano> interval) {
            StrT bar_str {};
            static const StrT division {" | "};
            StrT perc_str {}, tsk_cnt_str {}, rate_str{}, cntdwn_str {};
            SizeT divi_len = 0;

            bool disp_bar     = option & style_opts::bar,
                 disp_perc    = option & style_opts::percentage,
                 disp_tsk_cnt = option & style_opts::task_counter,
                 disp_rate    = option & style_opts::rate,
                 disp_cntdwn  = option & style_opts::countdown;
            /* Based on the bit vector indicating the switching of different sections,
             * the parts that are not turned on will be empty. */
            if (disp_bar)     bar_str = show_bar(percent);
            if (disp_perc)    perc_str = show_proportion(percent);
            if (disp_tsk_cnt) tsk_cnt_str = show_remain_task();
            if (disp_rate)    rate_str = show_rate(interval);
            if (disp_cntdwn)  cntdwn_str = show_time(std::move(interval));

            /* Using the sections that are enabled in the bit vector
             * to insert separators between different status bars. */
            bool perc_divi = false, tsk_cnt_divi = false, rate_divi = false;
            if ((disp_perc) && (disp_tsk_cnt || disp_rate || disp_cntdwn)) {
                perc_divi = true; // There are something behind the proportion.
                divi_len += division.size();
            }
            if (disp_tsk_cnt && (disp_rate || disp_cntdwn)) {
                tsk_cnt_divi = true; // There are something behind the task counter.
                divi_len += division.size();
            }
            if (disp_rate && disp_cntdwn) {
                rate_divi = true; // There are something behind the rate indicator.
                divi_len += division.size();
            }

            /* Using the sections that are enabled in the bit vector
             * to calculate the number of backspace characters. */
            static SizeT tsk_cnt_len = 0;
            if (!is_invoked) tsk_cnt_len = std::to_string(total_tsk).size()*2+1;
            SizeT backtrack_len = 0;
            if ((!disp_bar || bar_str.size() == 0) && is_invoked) {
                // It doesn't make sense to concatenate backspaces when updating `bar_str`.
                backtrack_len += disp_perc    ? ratio_len   : 0;
                backtrack_len += disp_tsk_cnt ? tsk_cnt_len : 0;
                backtrack_len += disp_rate    ? rate_len    : 0;
                backtrack_len += disp_cntdwn  ? time_len    : 0;
                backtrack_len += divi_len + l_status.size() + r_status.size();
            } else if (disp_bar && bar_str.size() != 0) // Updating `bar_str`, it will always back to the head of the line.
                bar_str = StrT(1, reboot) + std::move(bar_str);
            static StrT backtrack[2] {{}, {}}; // Provide an empty string to use when backspaces are not needed.
            static SizeT lately_bcktck_len = 0;
            if (!is_invoked) {
                backtrack[0] = {};
                lately_bcktck_len = 0;
            }
            if (backtrack_len != 0 && lately_bcktck_len != backtrack_len) {
                backtrack[0] = bulk_copy(backtrack_len, StrT(1, backspace));
                lately_bcktck_len = backtrack_len;
            }

            /* When some strings are empty,
             * use a string composed of escape sequences to move the cursor to the right. */
            static const StrT skip_perc   {bulk_copy(ratio_len, rightward)};
            static const StrT skip_rate   {bulk_copy(rate_len, rightward)};
            static const StrT skip_cntdwn {bulk_copy(time_len, rightward)};
            static StrT skip_tsk_cnt {};
            static SizeT skp_tsk_cnt_len = 0;
            if (skp_tsk_cnt_len != tsk_cnt_len) {
                skip_tsk_cnt = bulk_copy(tsk_cnt_len, rightward);
                skp_tsk_cnt_len = tsk_cnt_len;
            }
            if (disp_perc && perc_str.size() == 0) // Skip updating the proportion.
                perc_str = skip_perc;
            if (disp_tsk_cnt && tsk_cnt_str.size() == 0) // Skip updating the task counter.
                tsk_cnt_str = skip_tsk_cnt;
            if (disp_rate && rate_str.size() == 0) // Skip updating the rate indicator.
                rate_str = skip_rate;
            if (disp_cntdwn && cntdwn_str.size() == 0) // Skip updating the countdown.
                cntdwn_str = skip_cntdwn;

            /* Insert the strings that control cursor movement into the positions where they are needed. */
            if (perc_divi)    perc_str    = std::move(perc_str) + division;
            if (tsk_cnt_divi) tsk_cnt_str = std::move(tsk_cnt_str) + division;
            if (rate_divi)    rate_str    = std::move(rate_str) + division;
            /* If there is something to be displayed after the progress bar,
             * concatenate the brackets of the status bar. */
            if (disp_perc || disp_tsk_cnt || disp_rate || disp_cntdwn) {
                perc_str = col_fmt + l_status + std::move(perc_str);
                cntdwn_str = std::move(cntdwn_str) + r_status + defult_col;
            }

            SizeT bcktck_offset = disp_bar && bar_str.size() != 0 ? 1 : 0;
            return {
                std::move(bar_str),
                backtrack[bcktck_offset] +
                std::move(perc_str) + std::move(tsk_cnt_str) +
                std::move(rate_str) + std::move(cntdwn_str)
            };
        }
    public:
        pgbar(pgbar&&) = delete;
        pgbar& operator=(pgbar&&) = delete;

        pgbar(SizeT _total_tsk, std::ostream& _ostream) {
            stream    = &_ostream;
            todo_ch   = StrT(1, blank); done_ch = StrT(1, '#');
            l_bracket = StrT(1, '['); r_bracket = StrT(1, ']');
            step = 1; total_tsk = _total_tsk; done_cnt = 0;
            bar_length = 50; // default value
            option = style_opts::entire;
            in_terminal = check_output_stream();
            is_done = is_invoked = false;
        }
        pgbar(): pgbar(0, std::cerr) {} // default constructor
        pgbar(const pgbar& _other): pgbar() { // style copy
            stream      = _other.stream;
            todo_ch     = _other.todo_ch;
            done_ch     = _other.done_ch;
            l_bracket   = _other.l_bracket;
            r_bracket   = _other.r_bracket;
            in_terminal = check_output_stream();
            option = _other.option;
        }
        ~pgbar() {}

        bool check_update() const noexcept { return is_invoked; }
        bool check_full() const noexcept { return is_done; }
        /* Reset pgbar obj, EXCLUDING the total number of tasks */
        pgbar& reset() noexcept {
            done_cnt = 0;
            is_done = is_invoked = false;
            return *this;
        }
        /* Set the output stream object. */
        pgbar& set_ostream(std::ostream& _ostream) noexcept {
            if (is_invoked) return *this;
            stream = &_ostream;
            in_terminal = check_output_stream();
            return *this;
        }
        /* Set the number of steps the counter is updated each time `update()` is called. */
        pgbar& set_step(SizeT _step) noexcept {
            if (is_invoked) return *this;
            step = _step; return *this;
        }
        /* Set the number of tasks to be updated. */
        pgbar& set_task(SizeT _total_tsk) noexcept {
            if (is_invoked) return *this;
            total_tsk = _total_tsk; return *this;
        }
        /* Set the TODO characters in the progress bar. */
        pgbar& set_done_char(StrT _done_ch) {
            if (is_invoked) return *this;
            done_ch = std::move(_done_ch);
            return *this;
        }
        /* Set the DONE characters in the progress bar. */
        pgbar& set_todo_char(StrT _todo_ch) {
            if (is_invoked) return *this;
            todo_ch = std::move(_todo_ch);
            return *this;
        }
        /* Set the left bracket of the progress bar. */
        pgbar& set_left_bracket(StrT _l_bracket) {
            if (is_invoked) return *this;
            l_bracket = std::move(_l_bracket);
            return *this;
        }
        /* Set the right bracket of the progress bar. */
        pgbar& set_right_bracket(StrT _r_bracket) {
            if (is_invoked) return *this;
            r_bracket = std::move(_r_bracket);
            return *this;
        }
        /* Set the length of the progress bar. */
        pgbar& set_bar_length(SizeT _length) noexcept {
            if (is_invoked) return *this;
            bar_length = _length; return *this;
        }
        /* Select the display style by using bit operations. */
        pgbar& set_style(style_opts::OptT _selection) noexcept {
            if (is_invoked) return *this;
            option = _selection; return *this;
        }
        pgbar& operator=(const pgbar& _other) {
            if (this == &_other || is_invoked)
                return *this;
            stream      = _other.stream;
            todo_ch     = _other.todo_ch;
            done_ch     = _other.done_ch;
            l_bracket   = _other.l_bracket;
            r_bracket   = _other.r_bracket;
            in_terminal = check_output_stream();
            option = _other.option;
            return *this;
        }

        /* Update progress bar. */
        void update() {
            static std::chrono::duration<SizeT, std::nano> invoke_interval {};
            static std::chrono::system_clock::time_point first_invoked {}, lately_called {};
            static constexpr std::chrono::milliseconds refresh_rate {40}; // 25 Hz
            // TODO: empty cycle check
            if (is_done) throw bad_pgbar {"bad_pgbar: updating a full progress bar"};
            else if (total_tsk == 0) throw bad_pgbar {"bad_pgbar: the number of tasks is zero"};
            if (!is_invoked) {
                if (step == 0) throw bad_pgbar {"bad_pgbar: zero step"};
                invoke_interval = {};
                first_invoked = lately_called = std::chrono::system_clock::now();
                auto info = switch_feature(0, {});
                *stream << info.first << info.second;
                is_invoked = true;
            }
            done_cnt += step;
            //if (skip_update(invoke_interval)) return;

            auto now = std::chrono::system_clock::now();
            if (done_cnt != total_tsk && now-lately_called < refresh_rate)
                return; // The refresh rate is capped at 25 Hz.
            invoke_interval = (now - first_invoked) / done_cnt;
            lately_called = std::move(now);

            double perc = done_cnt / static_cast<double>(total_tsk);
            auto info = switch_feature(perc, std::move(invoke_interval));
            *stream << info.first << info.second;
            if (done_cnt >= total_tsk) {
                is_done = true;
                *stream << '\n';
            }
        }
    };

    const pgbar::StrT pgbar::rightward {"\033[C"};
    const pgbar::StrT pgbar::l_status {"[ "};
    const pgbar::StrT pgbar::r_status {" ]"};
    const pgbar::StrT pgbar::col_fmt {__PGBAR_COL__};
    const pgbar::StrT pgbar::defult_col {__PGBAR_DEFAULT_COL__};

} // namespace pgbar

#undef __PGBAR_CMP_V__
#undef __PGBAR_WIN__
#undef __PGBAR_UNIX__
#undef __PGBAR_UNKNOW_PLATFORM__

#undef __PGBAR_CXX14__
#undef __PGBAR_CXX17__
#undef __PGBAR_CXX20__

#undef __PGBAR_COL__
#undef __PGBAR_DEFAULT_COL__
#undef __PGBAR_IF_CONSTEXPR__
#undef __PGBAR_INLINE_FUNC__

#endif // __PROGRESSBAR_HPP__
