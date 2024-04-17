// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __PROGRESSBAR_HPP__
# define __PROGRESSBAR_HPP__

# include <cstdint>
# include <cmath>       // std::round()
# include <type_traits> // SFINAE
# include <utility>     // std::pair
# include <functional>  // std::function
# include <string>      // std::string
# include <chrono>      // as u know
# include <exception>   // bad_pgbar exception
# include <iostream>    // std::cerr, the output stream object used.

# include <atomic>      // std::atomic<bool>
# include <thread>      // std::thread
# include <mutex>       // std::mutex & std::unique_lock std::lock_guard
# include <condition_variable> // std::condition_variable

#if defined(__GNUC__) || defined(__clang__)
# define __PGBAR_INLINE_FUNC__ __attribute__((always_inline))
#elif defined(_MSC_VER)
# define __PGBAR_INLINE_FUNC__ __forceinline
#else
# define __PGBAR_INLINE_FUNC__
#endif

#if defined(_MSVC_VER) && defined(_MSVC_LANG) // for msvc
# define __PGBAR_CMP_V__ _MSVC_LANG
#else
# define __PGBAR_CMP_V__ __cplusplus
#endif

#if defined(_WIN32) || defined(WIN32)
# include <io.h>
# define __PGBAR_WIN__ 1
# define __PGBAR_UNIX__ 0
# define __PGBAR_UNKNOW_PLATFORM__ 0
#elif defined(__unix__) || defined(unix)
# include <unistd.h>
# define __PGBAR_WIN__ 0
# define __PGBAR_UNIX__ 1
# define __PGBAR_UNKNOW_PLATFORM__ 0
#else
# define __PGBAR_WIN__ 0
# define __PGBAR_UNIX__ 0
# define __PGBAR_UNKNOW_PLATFORM__ 1
#endif

#if __PGBAR_CMP_V__ >= 202002L
# include <concepts>
# include <format>
# define __PGBAR_CXX20__ 1
#else
# define __PGBAR_CXX20__ 0
#endif // __cplusplus >= 202002L
#if __PGBAR_CMP_V__ >= 201703L
# include <string_view>
# define __PGBAR_CXX17__ 1
# define __PGBAR_INLINE_VAR__ inline
# define __PGBAR_IF_CONSTEXPR__ constexpr
# define __PGBAR_FALLTHROUGH__ [[fallthrough]];
#else
# define __PGBAR_CXX17__ 0
# define __PGBAR_INLINE_VAR__
# define __PGBAR_IF_CONSTEXPR__
# define __PGBAR_FALLTHROUGH__ break;
#endif // __cplusplus >= 201703L
#if __PGBAR_CMP_V__ >= 201402L
# define __PGBAR_CXX14__ 1
# define __PGBAR_RET_CONSTEXPR__ constexpr
#else
# define __PGBAR_CXX14__ 0
# define __PGBAR_RET_CONSTEXPR__
#endif // __cplusplus >= 201402L

#ifndef PGBAR_NOT_COL
  /* Specify the color and font style for the status bar. */
# define __PGBAR_COL__ "\033[1;36m"
# define __PGBAR_ASSERT_FAILURE__ "\033[1;31m"
# define __PGBAR_DEFAULT_COL__ "\033[0m"
#else
# define __PGBAR_COL__ ""
# define __PGBAR_ASSERT_FAILURE__ ""
# define __PGBAR_DEFAULT_COL__ ""
#endif // PGBAR_NOT_COL

namespace pgbar {
  class bad_pgbar : public std::exception {
    std::string message;
  public:
    bad_pgbar( const std::string& _mes ) : message { _mes } {}
    virtual ~bad_pgbar() {}
    virtual const char* what() const noexcept { return message.c_str(); }
  };

  struct style {
    using Type = uint8_t;

    static constexpr Type bar = 1 << 0;
    static constexpr Type percentage = 1 << 1;
    static constexpr Type task_counter = 1 << 2;
    static constexpr Type rate = 1 << 3;
    static constexpr Type countdown = 1 << 4;
    static constexpr Type entire = UINT8_MAX;
  };

  namespace __detail {
    using SizeT = size_t;
    using StrT = std::string;
#if __PGBAR_CXX17__
    using ReadOnlyT = std::string_view;
#else
    using ReadOnlyT = const StrT&;
#endif // __PGBAR_CXX17__

    // The refresh rate is capped at about 25 Hz.
    __PGBAR_INLINE_VAR__ constexpr std::chrono::microseconds reflash_rate
      = std::chrono::microseconds( 35 );

#if __PGBAR_CXX20__
    template<typename F>
    concept FunctorType = requires(F tk) {
      { tk() } -> std::same_as<void>;
    };

    template<typename R>
    concept RenderType = requires(R rndr) {
      requires requires { R( std::declval<void()>() ); };
      { rndr.active() } -> std::same_as<void>;
      { rndr.suspend() } -> std::same_as<void>;
      { rndr.render() } -> std::same_as<void>;
    };

    template<typename T>
    concept StreamType = requires(T os) {
      { os << StrT {} } -> std::same_as<T&>;
    };
#else
    template<typename T, typename = void>
    struct is_void_functor : std::false_type {};
    template<typename T>
    struct is_void_functor<T, decltype((std::declval<T>()(), void()))> : std::true_type {};

    template<typename S, typename = void>
    struct is_stream : std::false_type {};
    template<typename S>
    struct is_stream<S, decltype((void)(std::declval<S&>() << std::declval<StrT>()), void())> : std::true_type {};
#endif // __PGBAR_CXX20__
  } // namespace __detail

  template<typename R, typename = void>
  struct is_renderer : std::false_type {};
  template<typename R>
  struct is_renderer<R,
    decltype(
      ((std::declval<R&>().active(), void())),
      ((std::declval<R&>().suspend(), void())),
      ((std::declval<R&>().render(), void()))
    )
  > : std::true_type {};

#if __PGBAR_CXX14__
  template<typename R>
  __PGBAR_INLINE_VAR__ constexpr bool is_renderer_v = is_renderer<R>::value;
#endif // __PGBAR_CXX14__

  class multithread final {
    std::atomic<bool> active_signal;
    std::atomic<bool> suspend_signal;

    std::atomic<bool> finish_signal;
    std::atomic<bool> stop_signal;
    std::condition_variable cond_var;
    std::mutex mtx;
    std::thread td;

  public:
    multithread( const multithread& ) = delete;
    multithread& operator=( const multithread& ) = delete;

#if __PGBAR_CXX20__
    template<__detail::FunctorType F>
#else
    template<typename F>
#endif // __PGBAR_CXX20__
    explicit multithread( F&& task )
      : active_signal { false }, suspend_signal { true }
      , finish_signal { false }, stop_signal { true } {
#if !(__PGBAR_CXX20__)
      static_assert(
        __detail::is_void_functor<F>::value,
        __PGBAR_ASSERT_FAILURE__
        "The type 'F' must be a callable type of 'void()'"
        __PGBAR_DEFAULT_COL__
      );
#endif // __PGBAR_CXX20__
      td = std::thread( [&, task]() -> void {
        while ( !finish_signal ) {
          {
            std::unique_lock<std::mutex> lock { mtx };
            if ( stop_signal && !finish_signal ) {
              suspend_signal = true;
              cond_var.wait( lock );
            }
          }
          active_signal = true;
          task();
          std::this_thread::sleep_for( __detail::reflash_rate );
        }
      } );
    }
    ~multithread() {
      finish_signal = true;
      stop_signal = false;
      if ( td.joinable() ) {
        cond_var.notify_one();
        td.join();
      }
    }
    void active() {
      stop_signal = false;
      cond_var.notify_one();
      // spin lock
      while ( active_signal == false );
      suspend_signal = false;
    }
    void suspend() {
      stop_signal = true;
      // spin lock
      while ( suspend_signal == false );
      active_signal = false;
    }
    void render() noexcept {}
  };

  class singlethread final {
    std::chrono::time_point<std::chrono::system_clock> last_invoke;
    std::function<void()> task;

  public:
    singlethread( const singlethread& ) = delete;
    singlethread& operator=( const singlethread& ) = delete;

    explicit singlethread( std::function<void()> tsk )
      : task { std::move( tsk ) } {}
    ~singlethread() {}
    void active() {
      last_invoke = std::chrono::system_clock::now();
    }
    void suspend() {
      task();
    }
    void render() {
      auto current_time = std::chrono::system_clock::now();
      if ( current_time - last_invoke < __detail::reflash_rate )
        return;
      last_invoke = std::move( current_time );
      task();
    }
  };

#if __PGBAR_CXX20__
  template<__detail::StreamType StreamObj = std::ostream, __detail::RenderType RenderMode = multithread>
#else
  template<typename StreamObj = std::ostream, typename RenderMode = multithread>
#endif // __PGBAR_CXX20__
  class pgbar {
#if !(__PGBAR_CXX20__)
    static_assert(
      __detail::is_stream<StreamObj>::value,
      __PGBAR_ASSERT_FAILURE__
      "The 'StreamObj' must be a type that supports 'operator<<' to insert '__detail::StrT'"
      __PGBAR_DEFAULT_COL__
    );
    static_assert(
      is_renderer<RenderMode>::value,
      __PGBAR_ASSERT_FAILURE__
      "The 'RenderMode' must satisfy the constraint of the type predicate 'is_renderer'"
      __PGBAR_DEFAULT_COL__
    );
#endif // __PGBAR_CXX20__

    enum class txt_layut { align_left, align_right, align_center }; // text layout

    static constexpr char blank = ' ';
    static constexpr char reboot = '\r';
    static constexpr char backspace = '\b';
    static constexpr __detail::SizeT ratio_len = 7; // The length of `100.00%`.
    static constexpr __detail::SizeT time_len = 11; // The length of `9.9m < 9.9m`.
    static constexpr __detail::SizeT rate_len = 10; // The length of `999.99 kHz`.
    static const __detail::StrT rightward;          // ASCII Escape Sequence: moves the cursor right.
    static const __detail::StrT l_status;           // The left bracket of status bar.
    static const __detail::StrT r_status;           // The right bracket of status bar.
    static const __detail::StrT division;           // The default division character.
    static const __detail::StrT col_fmt;            // The color and font style of status bar.
    static const __detail::StrT default_col;        // The default color and font style.

    RenderMode rndrer;
    StreamObj* stream;
    __detail::StrT todo_ch, done_ch;
    __detail::StrT l_bracket, r_bracket;
    __detail::SizeT step; // The number of steps per invoke `update()`.
    __detail::SizeT total_tsk, done_cnt;
    __detail::SizeT bar_length; // The length of the progress bar.
    style::Type option;
    bool in_terminal;

    /// @brief Format the `_str`.
    /// @tparam _style Format mode.
    /// @param _width Target length, do nothing if `_width` less than the length of `_str`.
    /// @param _str The string will be formatted.
    /// @return Formatted string.
    template<txt_layut _style>
    __PGBAR_INLINE_FUNC__ static __detail::StrT formatter( __detail::SizeT _width, __detail::StrT _str ) {
      if ( _width == 0 ) return {};
      if ( _str.size() >= _width ) return _str;
#if __PGBAR_CXX20__
      if __PGBAR_IF_CONSTEXPR__( _style == txt_layut::align_right )
        return std::format( "{:>{}}", std::move( _str ), _width );
      else if __PGBAR_IF_CONSTEXPR__( _style == txt_layut::align_left )
        return std::format( "{:<{}}", std::move( _str ), _width );
      else return std::format( "{:^{}}", std::move( _str ), _width );
#else
      __detail::SizeT str_size = _str.size();
      if __PGBAR_IF_CONSTEXPR__( _style == txt_layut::align_right )
        return __detail::StrT( _width - str_size, blank ) + std::move( _str );
      else if __PGBAR_IF_CONSTEXPR__( _style == txt_layut::align_left )
        return std::move( _str ) + __detail::StrT( _width - str_size, blank );
      else {
        _width -= _str.size();
        __detail::SizeT r_blank = _width / 2;
        return __detail::StrT( _width - r_blank, blank ) + std::move( _str ) + __detail::StrT( r_blank, blank );
      }
#endif // __PGBAR_CXX20__
    }

    /// @brief Copy a string mutiple times and concatenate them together.
    /// @tparam S The type of the string.
    /// @param _time Copy times.
    /// @param _src The string to be copied.
    /// @return The string copied mutiple times.
    static __detail::StrT bulk_copy( __detail::SizeT _time, __detail::ReadOnlyT _src ) {
      if ( _time == 0 || _src.size() == 0 ) return {};
      __detail::StrT ret; ret.reserve( _src.size() * _time );
      for ( __detail::SizeT _ = 0; _ < _time; ++_ )
        ret.append( _src, 0 );
      return ret;
    }

    __PGBAR_INLINE_FUNC__ static bool check_output_stream( const StreamObj* const os ) {
      if ( std::is_same<StreamObj, std::ostream>::value == false )
        return true; // Custom object, the program does not block output.
#if __PGBAR_WIN__
      if ( _isatty( _fileno( stdout ) ) )
        return true;
#elif __PGBAR_UNIX__
      if ( isatty( fileno( stdout ) ) )
        return true;
#elif __PGBAR_UNKNOW_PLATFORM__
      if ( true ) return true;
#endif // PLATFORM
      else return false;
    }

    /// @brief Will never return an empty string.
    __PGBAR_INLINE_FUNC__ __detail::StrT show_bar( double percent ) const {
      __detail::StrT buf; buf.reserve( l_bracket.size() + r_bracket.size() + bar_length + 1 );
      __detail::SizeT done_len = std::round( bar_length * percent );
      buf.append(
        bulk_copy( 1, l_bracket ) + bulk_copy( done_len, done_ch ) +
        bulk_copy( bar_length - done_len, todo_ch ) + bulk_copy( 1, r_bracket ) +
        __detail::StrT( 1, blank )
      );
      return buf;
    }

    /// @brief Will never return an empty string.
    __PGBAR_INLINE_FUNC__ __detail::StrT show_proportion( double percent ) const {
      if ( !is_update() ) {
        static const __detail::StrT default_str =
          formatter<txt_layut::align_left>( ratio_len, "0.00%" );
        return default_str;
      }

      __detail::StrT proportion = std::to_string( percent * 100 );
      proportion.resize( proportion.find( '.' ) + 3 );

      return formatter<txt_layut::align_left>(
        ratio_len - proportion.size(),
        std::move( proportion ) + __detail::StrT( 1, '%' )
      );
    }

    /// @brief Will never return an empty string.
    __PGBAR_INLINE_FUNC__ __detail::StrT show_remain_task() const {
      __detail::StrT total_str = std::to_string( total_tsk );
      __detail::SizeT size = total_str.size();
      return (
        formatter<txt_layut::align_right>( size, std::to_string( done_cnt ) ) +
        __detail::StrT( 1, '/' ) + std::move( total_str )
      );
    }

    /// @brief Will never return an empty string.
    __PGBAR_INLINE_FUNC__ __detail::StrT show_rate( std::chrono::duration<__detail::SizeT, std::nano> interval ) const {
      static std::chrono::duration<__detail::SizeT, std::nano> invoke_interval;

      if ( !is_update() ) {
        invoke_interval = {};
        static const __detail::StrT default_str =
          formatter<txt_layut::align_center>( rate_len, "0.00 Hz" );
        return default_str;
      }

      invoke_interval = (invoke_interval + interval) / 2; // each invoke interval
      __detail::SizeT frequency = invoke_interval.count() != 0 ? std::chrono::duration_cast<
        std::chrono::nanoseconds>(std::chrono::seconds( 1 )) / invoke_interval
        : ~static_cast<__detail::SizeT>(0); // The invoking rate is too fast to calculate.

      auto splice = []( double val ) -> __detail::StrT {
        __detail::StrT str = std::to_string( val );
        str.resize( str.find( '.' ) + 3 ); // Keep two decimal places.
        return str;
        };

      __detail::StrT rate; rate.reserve( rate_len );
      if ( frequency < 1e3 ) // < 1Hz => '999.99 Hz'
        rate.append( splice( frequency ) + __detail::StrT( " Hz" ) );
      else if ( frequency < 1e6 ) // < 1 kHz => '999.99 kHz'
        rate.append( splice( frequency / 1e3 ) + __detail::StrT( " kHz" ) );
      else if ( frequency < 1e9 ) // < 1 MHz => '999.99 MHz'
        rate.append( splice( frequency / 1e6 ) + __detail::StrT( " MHz" ) );
      else { // < 1 GHz => '> 1.00 GHz'
        double temp = frequency / 1e9;
        if ( temp > 999.99 ) rate.append( "> 1.00 GHz" );
        else rate.append( splice( temp ) + __detail::StrT( " GHz" ) );
      }

      return formatter<txt_layut::align_center>( rate_len, std::move( rate ) );
    }

    /* will never return an empty string */
    __PGBAR_INLINE_FUNC__ __detail::StrT show_time( std::chrono::duration<__detail::SizeT, std::nano> interval ) const {
      if ( !is_update() ) {
        static const __detail::StrT default_str =
          formatter<txt_layut::align_center>( time_len, "0s < 99h" );
        return default_str;
      }
      auto splice = []( double val ) -> __detail::StrT {
        __detail::StrT str = std::to_string( val );
        str.resize( str.find( '.' ) + 2 ); // Keep one decimal places.
        return str;
        };
      auto to_time = [&splice]( int64_t sec ) -> __detail::StrT {
        if ( sec < 60 ) // < 1 minute => 59s
          return std::to_string( sec ) + __detail::StrT( 1, 's' );
        else if ( sec < 60 * 9 ) // < 9 minutes => 9.9m
          return splice( static_cast<double>(sec) / 60.0 ) + __detail::StrT( 1, 'm' );
        else if ( sec < 60 * 60 ) // >= 9 minutes => 59m
          return std::to_string( sec / 60 ) + __detail::StrT( 1, 'm' );
        else if ( sec < 60 * 60 * 9 ) // < 9 hour => 9.9h
          return splice( static_cast<double>(sec) / (60.0 * 60.0) ) + __detail::StrT( 1, 'h' );
        else { // >= 9 hour => 999h
          if ( sec > 60 * 60 * 99 ) return "99h";
          else return std::to_string( sec / (60 * 60) ) + __detail::StrT( 1, 'h' );
        }
        };

      return formatter<txt_layut::align_center>(
        time_len, to_time( std::chrono::duration_cast<std::chrono::seconds>(interval * done_cnt).count() ) +
        __detail::StrT( " < " ) + to_time( std::chrono::duration_cast<std::chrono::seconds>(interval * (total_tsk - done_cnt)).count() )
      );
    }

    /// @brief Based on the value of `option` and bitwise operations,
    /// @brief determine which part of the string needs to be concatenated.
    /// @param percent The percentage of the current task execution.
    /// @return The progress bar that has been assembled but is pending output.
    std::pair<__detail::StrT, __detail::StrT> switch_feature( double percent, std::chrono::duration<__detail::SizeT, std::nano> interval ) const {
      static __detail::StrT backtrack;
      static __detail::SizeT cnt_length = 0, total_length = 0, divi_cnt = 0;
      static bool has_status = false;
      if ( !is_update() ) {
        total_length = 0; divi_cnt = 0;
        has_status = false; // To determine whether to insert the strings `l_status` and `r_status`
        // Used to assist in calculating how many variable `division` need to be inserted.
        bool has_divi = false;
        /* The progress bar has a different number of tasks each time it is restarted,
         * so the `cnt_length` needs to be updated dynamically. */
        cnt_length = std::to_string( total_tsk ).size() * 2 + 1;

        auto update_dynamically = [&has_divi]( __detail::SizeT len ) -> void {
          total_length += len;
          has_status = true;
          if ( has_divi ) ++divi_cnt;
          else has_divi = true;
          };

        // A double negative is a positive
        if ( option & style::percentage )
          update_dynamically( ratio_len );
        if ( option & style::task_counter )
          update_dynamically( cnt_length );
        if ( option & style::rate )
          update_dynamically( rate_len );
        if ( option & style::countdown )
          update_dynamically( time_len );

        total_length += divi_cnt * division.size();
        total_length += has_status ? l_status.size() + r_status.size() : 0;
        backtrack = bulk_copy( total_length, __detail::StrT( 1, backspace ) );
      }

      enum class status { // state machine
        start, done,
        dis_bar, dis_perc, dis_cnt, dis_rate, dis_cntdwn
      } state = status::start;

      __detail::StrT status_str;
      __detail::StrT bar_str;

      bool will_dis_bar = false;
      auto goto_nxt = [
        &status_str, &will_dis_bar, this
      ](status& now, status nxt) {
        if ( (now == status::start && nxt != status::dis_bar) || // has status bar.
            (now == status::dis_bar && nxt != status::done) ) // ditto.
          if ( will_dis_bar || !is_update() ) status_str.append( col_fmt + l_status );
          else status_str.append( backtrack + col_fmt + l_status );
        else if ( now != status::start && nxt != status::done )
          status_str.append( division ); // The `division` will be added during state machine jump.
        else if ( nxt == status::done && has_status )
          status_str.append( r_status + default_col );
        now = nxt;
        };

        auto opt = option;
        auto get_nxt = [&opt]() -> status {
          if ( opt & style::bar ) {
            opt &= ~style::bar;
            return status::dis_bar;
          } else if ( opt & style::percentage ) {
            opt &= ~style::percentage;
            return status::dis_perc;
          } else if ( opt & style::task_counter ) {
            opt &= ~style::task_counter;
            return status::dis_cnt;
          } else if ( opt & style::rate ) {
            opt &= ~style::rate;
            return status::dis_rate;
          } else if ( opt & style::countdown ) {
            opt &= ~style::countdown;
            return status::dis_cntdwn;
          } else return status::done;
          };

        /* The operations are the same in most states.
         * All diffcult operations are reflected in the state jump process. */
        do {
          __detail::StrT aped_str;
          switch ( state ) {
          case status::dis_bar: {
            bar_str = show_bar( percent );
            will_dis_bar = !bar_str.empty();
            if ( !bar_str.empty() )
              bar_str.insert( bar_str.begin(), reboot );
          } break;
          case status::dis_perc: {
            static const __detail::StrT skip_perc { bulk_copy( ratio_len, rightward ) };
            aped_str = show_proportion( percent );
            aped_str = aped_str.empty() ? skip_perc : aped_str;
          } break;
          case status::dis_cnt: {
            aped_str = show_remain_task();
          } break;
          case status::dis_rate: {
            aped_str = show_rate( interval );
          } break;
          case status::dis_cntdwn: {
            aped_str = show_time( std::move( interval ) );
          } break;
          case status::start: // nothing here, just skip it.
            __PGBAR_FALLTHROUGH__
          case status::done:
            __PGBAR_FALLTHROUGH__
          default: break;
          }
          status_str.append( aped_str );
          goto_nxt( state, get_nxt() );
        } while ( state != status::done );

        return { bar_str, status_str };
    }

    /// @brief This function only will be invoked by the rendering thread.
    __PGBAR_INLINE_FUNC__ void rendering() const {
      static bool done_flag = false;
      static std::chrono::duration<__detail::SizeT, std::nano> invoke_interval;
      static std::chrono::system_clock::time_point first_invoked;

      if ( !is_update() ) {
        done_flag = false;
        if ( in_terminal ) {
          invoke_interval = {};
          first_invoked = std::chrono::system_clock::now();
          const auto info = switch_feature( 0, {} );
          *stream << info.first << info.second;
        }
      }

      if ( done_flag ) return;

      if ( in_terminal ) {
        auto now = std::chrono::system_clock::now();
        invoke_interval = done_cnt != 0 ? (now - first_invoked) / done_cnt
          : (now - first_invoked) / static_cast<__detail::SizeT>(1);

        double perc = done_cnt / static_cast<double>(total_tsk);
        const auto info = switch_feature( perc, invoke_interval );

        *stream << info.first << info.second;
      }

      if ( is_full() ) {
        if ( in_terminal ) {
          const auto info = switch_feature( 1, invoke_interval );
          *stream << info.first << info.second << '\n';
        }
        done_flag = true;
      }
    }

#if __PGBAR_CXX20__
    template<__detail::FunctorType F>
#else
    template<typename F>
#endif // __PGBAR_CXX20__
    __PGBAR_INLINE_FUNC__ void do_update( F&& invokable ) {
#if !(__PGBAR_CXX20__)
      static_assert(
        __detail::is_void_functor<F>::value,
        __PGBAR_ASSERT_FAILURE__
        "The type 'F' must be a callable type of 'void()'"
        __PGBAR_DEFAULT_COL__
      );
#endif // __PGBAR_CXX20__

      if ( is_full() )
        throw bad_pgbar { "bad_pgbar: updating a full progress bar" };
      else if ( total_tsk == 0 )
        throw bad_pgbar { "bad_pgbar: the number of tasks is zero" };
      if ( !is_update() )
        rndrer.active();

      invokable();
      rndrer.render();

      if ( done_cnt >= total_tsk )
        rndrer.suspend(); // wait for sub thread to finish
    }

  public:
    using StreamType = StreamObj;
    using RendererType = RenderMode;
    pgbar( pgbar&& ) = delete;
    pgbar& operator=( pgbar&& ) = delete;

    pgbar( __detail::SizeT _total_tsk, StreamObj& _ostream )
      : rndrer { [this]() -> void { this->rendering(); } } {
      stream = std::addressof( _ostream );
      todo_ch = __detail::StrT( 1, blank ); done_ch = __detail::StrT( 1, '#' );
      l_bracket = __detail::StrT( 1, '[' ); r_bracket = __detail::StrT( 1, ']' );
      step = 1; total_tsk = _total_tsk; done_cnt = 0;

      bar_length = 50; // default value
      option = style::entire;
      in_terminal = check_output_stream( stream );
    }
    pgbar( const pgbar& _other )
      : pgbar( _other.total_tsk, (*_other.stream) ) { // style copy
      todo_ch = _other.todo_ch;
      done_ch = _other.done_ch;
      l_bracket = _other.l_bracket;
      r_bracket = _other.r_bracket;
      option = _other.option;
    }
    ~pgbar() {}

    bool is_update() const noexcept {
      return done_cnt != 0;
    }
    bool is_full() const noexcept {
      return is_update() && done_cnt == total_tsk;
    }
    /// @brief Reset pgbar obj, EXCLUDING the total number of tasks.
    pgbar& reset() {
      done_cnt = 0;
      rndrer.suspend();
      return *this;
    }
    /// @brief Set the number of steps the counter is updated each time `update()` is called.
    /// @throw If the `_step` is 0, it will throw an `bad_pgbar`.
    pgbar& set_step( __detail::SizeT _step ) {
      if ( is_update() ) return *this;
      else if ( _step == 0 ) throw bad_pgbar { "bad_pgbar: zero step" };
      step = _step; return *this;
    }
    /// @brief Set the number of tasks to be updated.
    /// @throw If the `_total_tsk` is 0, it will throw an `bad_pgbar`.
    pgbar& set_task( __detail::SizeT _total_tsk ) {
      if ( is_update() ) return *this;
      else if ( _total_tsk == 0 )
        throw bad_pgbar { "bad_pgbar: the number of tasks is zero" };
      total_tsk = _total_tsk; return *this;
    }
    /// @brief Set the TODO characters in the progress bar.
    pgbar& set_done_char( __detail::StrT _done_ch ) noexcept {
      if ( is_update() ) return *this;
      done_ch = std::move( _done_ch );
      return *this;
    }
    /// @brief Set the DONE characters in the progress bar.
    pgbar& set_todo_char( __detail::StrT _todo_ch ) noexcept {
      if ( is_update() ) return *this;
      todo_ch = std::move( _todo_ch );
      return *this;
    }
    /// @brief Set the left bracket of the progress bar.
    pgbar& set_left_bracket( __detail::StrT _l_bracket ) noexcept {
      if ( is_update() ) return *this;
      l_bracket = std::move( _l_bracket );
      return *this;
    }
    /// @brief Set the right bracket of the progress bar.
    pgbar& set_right_bracket( __detail::StrT _r_bracket ) noexcept {
      if ( is_update() ) return *this;
      r_bracket = std::move( _r_bracket );
      return *this;
    }
    /// @brief Set the length of the progress bar.
    pgbar& set_bar_length( __detail::SizeT _length ) noexcept {
      if ( is_update() ) return *this;
      bar_length = _length; return *this;
    }
    /// @brief Select the display style by using bit operations.
    pgbar& set_style( style::Type _selection ) noexcept {
      if ( is_update() ) return *this;
      option = _selection; return *this;
    }
    /// @brief Copy the style options from the other bar object.
    /// @param _other The other bar object.
    pgbar& operator=( const pgbar& _other ) {
      if ( this == &_other || is_update() )
        return *this;
      stream = _other.stream;
      todo_ch = _other.todo_ch;
      done_ch = _other.done_ch;
      l_bracket = _other.l_bracket;
      r_bracket = _other.r_bracket;
      in_terminal = check_output_stream( stream );
      option = _other.option;
      return *this;
    }

    /// @brief Update progress bar.
    void update() {
      do_update( [&]() -> void { done_cnt += step; } );
    }

    /// @brief Ignore the effect of `set_step()`, increment forward several progresses,
    /// @brief and any `next_step` portions that exceed the total number of tasks are ignored.
    /// @param next_step The number that will increment forward the progresses.
    void update( __detail::SizeT next_step ) {
      do_update( [&]() -> void {
        done_cnt += done_cnt + next_step <= total_tsk ? next_step :
          total_tsk - done_cnt;
      } );
    }
  };

#if __PGBAR_CXX20__
# define __PGBAR_TEMPLATE_DECL__ \
  template<__detail::StreamType StreamObj, __detail::RenderType RenderMode> \
    const __detail::StrT
#else
# define __PGBAR_TEMPLATE_DECL__ \
  template<typename StreamObj, typename RenderMode> \
    const __detail::StrT
#endif // __PGBAR_CXX20__

  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::rightward { "\033[C" };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::l_status { "[ " };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::r_status { " ]" };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::division { " | " };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::col_fmt { __PGBAR_COL__ };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::default_col { __PGBAR_DEFAULT_COL__ };

  template<typename BarT>
  struct is_pgbar : std::false_type {};
  template<typename StreamObj, typename RenderMode>
  struct is_pgbar<pgbar<StreamObj, RenderMode>> : std::true_type {};

#if __PGBAR_CXX14__
  template<typename BarT>
  __PGBAR_INLINE_VAR__ constexpr bool is_pgbar_v = is_pgbar<BarT>::value;
#endif // __PGBAR_CXX14__
} // namespace pgbar

#undef __PGBAR_TYPE_DECL__
#undef __PGBAR_TEMPLATE_DECL__

#undef __PGBAR_WIN__
#undef __PGBAR_UNIX__
#undef __PGBAR_UNKNOW_PLATFORM__

#undef __PGBAR_CXX14__
#undef __PGBAR_CXX17__
#undef __PGBAR_CXX20__

#undef __PGBAR_COL__
#undef __PGBAR_DEFAULT_COL__
#undef __PGBAR_RET_CONSTEXPR__
#undef __PGBAR_IF_CONSTEXPR__
#undef __PGBAR_INLINE_VAR__
#undef __PGBAR_FALLTHROUGH__
#undef __PGBAR_INLINE_FUNC__

#endif // __PROGRESSBAR_HPP__
