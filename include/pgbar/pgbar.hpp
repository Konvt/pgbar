// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __PROGRESSBAR_HPP__
# define __PROGRESSBAR_HPP__

# include <cstdint>
# include <cmath>       // std::round
# include <type_traits> // SFINAE
# include <utility>     // std::pair
# include <bitset>      // std::bitset
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
# include <concepts> // std::same_as
# include <format>   // std::format
# include <optional> // std::optional
# define __PGBAR_CXX20__ 1
#else
# define __PGBAR_CXX20__ 0
#endif // __cplusplus >= 202002L
#if __PGBAR_CMP_V__ >= 201703L
# include <string_view> // std::string_view
# define __PGBAR_CXX17__ 1
# define __PGBAR_INLINE_VAR__ inline
# define __PGBAR_ENHANCE_CONSTEXPR__ constexpr
# define __PGBAR_FALLTHROUGH__ [[fallthrough]];
#else
# define __PGBAR_CXX17__ 0
# define __PGBAR_INLINE_VAR__
# define __PGBAR_ENHANCE_CONSTEXPR__
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

  namespace __detail {
    using SizeT = size_t;
    using StrT = std::string;
#if __PGBAR_CXX17__
    using ReadOnlyT = std::string_view;
    using ConstStrT = const std::string_view;
#else
    using ReadOnlyT = const StrT&;
    using ConstStrT = const StrT;
#endif // __PGBAR_CXX17__

    // The refresh rate is capped at about 25 Hz.
    __PGBAR_INLINE_VAR__ constexpr std::chrono::microseconds reflash_rate
      = std::chrono::microseconds( 35 );

#if __PGBAR_CXX20__
    // these concepts are like duck types
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

    template<typename S>
    concept StreamType = requires(S os) {
      { os << StrT {} } -> std::same_as<S&>;
    };
#else
    template<typename F, typename = void>
    struct is_void_functor : std::false_type {};
    template<typename F>
    struct is_void_functor<F,
      typename std::enable_if<
        std::is_void<decltype(std::declval<F>()())>::value
      >::type
    > : std::true_type {};
#endif // __PGBAR_CXX20__
  } // namespace __detail

#if __PGBAR_CXX20__
  template<typename S>
  struct is_stream : std::bool_constant<__detail::StreamType<S>> {};

  template<typename R>
  struct is_renderer : std::bool_constant<__detail::RenderType<R>> {};
#else
  template<typename S, typename = void>
  struct is_stream : std::false_type {};
  template<typename S>
  struct is_stream<S,
    typename std::enable_if<
      std::is_same<decltype(std::declval<S&>() << std::declval<std::string>()), S&>::value
    >::type
  > : std::true_type {};

  template<typename R, typename = void>
  struct is_renderer : std::false_type {};
  template<typename R>
  struct is_renderer<R,
    typename std::enable_if<
      std::is_void<decltype(std::declval<R&>().active())>::value &&
      std::is_void<decltype(std::declval<R&>().suspend())>::value &&
      std::is_void<decltype(std::declval<R&>().render())>::value
    >::type
  > : std::true_type {};
#endif // __PGBAR_CXX20__

#if __PGBAR_CXX14__
  template<typename R>
  __PGBAR_INLINE_VAR__ constexpr bool is_renderer_v = is_renderer<R>::value;

  template<typename S>
  __PGBAR_INLINE_VAR__ constexpr bool is_stream_v = is_stream<S>::value;
#endif // __PGBAR_CXX14__

  struct style {
    using Type = uint8_t;

    static constexpr Type bar = 1 << 0;
    static constexpr Type percentage = 1 << 1;
    static constexpr Type task_counter = 1 << 2;
    static constexpr Type rate = 1 << 3;
    static constexpr Type countdown = 1 << 4;
    static constexpr Type entire = UINT8_MAX;

#if __PGBAR_CXX20__
    std::optional<__detail::StrT> todo_char;
    std::optional<__detail::StrT> done_char;
    std::optional<__detail::StrT> left_bracket;
    std::optional<__detail::StrT> right_bracket;
    std::optional<__detail::SizeT> total_tasks;
    std::optional<__detail::SizeT> each_setp;
    std::optional<__detail::SizeT> bar_length;
    std::optional<Type> option;
#endif // __PGBAR_CXX20__
  };

  class multithread final {
    std::atomic<bool> active_flag_;
    std::atomic<bool> suspend_flag_;

    std::atomic<bool> finish_signal_;
    std::atomic<bool> stop_signal_;
    std::condition_variable cond_var_;
    std::mutex mtx_;
    std::thread td_;

  public:
    multithread( const multithread& ) = delete;
    multithread& operator=( const multithread& ) = delete;

    template<
#if __PGBAR_CXX20__
      __detail::FunctorType F
#else
      typename F,
      typename = typename std::enable_if<
        __detail::is_void_functor<F>::value
      >::type
#endif
    >
    explicit multithread( F&& task )
      : active_flag_ { false }, suspend_flag_ { true }
      , finish_signal_ { false }, stop_signal_ { true } {
      td_ = std::thread( [&, task]() -> void {
        do {
          {
            std::unique_lock<std::mutex> lock { mtx_ };
            if ( stop_signal_ && !finish_signal_ ) {
              if ( active_flag_ || stop_signal_ ) // it means subthread has been printed already
                task(); // so output the last progress bar before suspend
              suspend_flag_ = true;
              cond_var_.wait( lock );
            }
          }
          active_flag_ = true;
          task();
          if ( finish_signal_ ) break;
          std::this_thread::sleep_for( __detail::reflash_rate );
        } while ( true );
      } );
    }
    ~multithread() {
      {
        std::unique_lock<std::mutex> lock { mtx_ };
        finish_signal_ = true;
        stop_signal_ = false;
      }
      cond_var_.notify_all();
      if ( td_.joinable() )
        td_.join();
    }
    void active() {
      stop_signal_ = false;
      cond_var_.notify_one();
      // spin lock
      while ( active_flag_ == false );
      suspend_flag_ = false;
    }
    void suspend() {
      { // there are multiple atomic variables entering the critical region
        std::unique_lock<std::mutex> lock { mtx_ };
        stop_signal_ = true;
      }
      while ( suspend_flag_ == false );
      { // ensure that the thread has been suspended
        std::unique_lock<std::mutex> lock { mtx_ };
        active_flag_ = false;
      }
    }
    void render() noexcept {}
  };

  class singlethread final {
    struct wrapper_base {
      virtual ~wrapper_base() {}
      virtual void run() = 0;
    };
    template<typename F>
    class functor_wrapper final : public wrapper_base {
      static_assert(
#if __PGBAR_CXX20__
        __detail::FunctorType<F>,
#else
        __detail::is_void_functor<F>::value,
#endif // __PGBAR_CXX20__
        __PGBAR_ASSERT_FAILURE__
        "singlethread::functor_wrapper: template type error"
        __PGBAR_DEFAULT_COL__
      );

      F func_;

    public:
      template<typename U>
      functor_wrapper( U&& func )
        : func_ { std::forward<U>( func ) } {}
      void run() override final { func_(); }
    };

    wrapper_base* task_;
    bool active_flag_;
    std::chrono::time_point<std::chrono::system_clock> last_invoke_;

  public:
    singlethread( const singlethread& ) = delete;
    singlethread& operator=( const singlethread& ) = delete;

    template<
#if __PGBAR_CXX20__
      __detail::FunctorType F
#else
      typename F,
      typename = typename std::enable_if<
        __detail::is_void_functor<F>::value
      >::type
#endif
    >
    explicit singlethread( F&& tsk )
      : task_ { nullptr }, active_flag_ { false } {
      auto new_res = new functor_wrapper<typename std::decay<F>::type>( std::forward<F>( tsk ) );
      task_ = new_res;
    }
    ~singlethread() {
      delete task_;
    }
    void active() {
      if ( active_flag_ )
        return;
      last_invoke_ = std::chrono::system_clock::now();
      task_->run();
      active_flag_ = true;
    }
    void suspend() {
      if ( !active_flag_ )
        return;
      task_->run();
      active_flag_ = false;
    }
    void render() {
      if ( !active_flag_ )
        return;
      auto current_time = std::chrono::system_clock::now();
      if ( current_time - last_invoke_ < __detail::reflash_rate )
        return;
      last_invoke_ = std::move( current_time );
      task_->run();
    }
  };

  template<typename StreamObj = std::ostream, typename RenderMode = multithread>
  class pgbar {
    static_assert(
      is_stream<StreamObj>::value,
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

    enum class txt_layut { align_left, align_right, align_center }; // text layout

    static constexpr char blank = ' ';
    static constexpr char backspace = '\b';
    static constexpr __detail::SizeT ratio_len = 7; // The length of `100.00%`.
    static constexpr __detail::SizeT time_len = 11; // The length of `9.9m < 9.9m`.
    static constexpr __detail::SizeT rate_len = 10; // The length of `999.99 kHz`.
    static __detail::ConstStrT lstatus;    // The left bracket of status bar.
    static __detail::ConstStrT rstatus;    // The right bracket of status bar.
    static __detail::ConstStrT division;    // The default division character.
    static __detail::ConstStrT col_fmt;     // The color and font style of status bar.
    static __detail::ConstStrT default_col; // The default color and font style.

    mutable std::atomic<bool> update_flag_;
    RenderMode rndrer_;
    StreamObj& stream_;

    __detail::StrT todo_ch_, done_ch_;
    __detail::StrT lbracket_, rbracket_;
    __detail::SizeT num_tasks_;
    __detail::SizeT step_; // The number of steps per invoke `update()`.
    __detail::SizeT bar_length_; // The length of the progress bar.
    style::Type option_;
    __detail::SizeT done_cnt_;
    __detail::SizeT cnt_length_; // The length of the task counter.

    bool in_tty_;

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
      if __PGBAR_ENHANCE_CONSTEXPR__( _style == txt_layut::align_right )
        return std::format( "{:>{}}", std::move( _str ), _width );
      else if __PGBAR_ENHANCE_CONSTEXPR__( _style == txt_layut::align_left )
        return std::format( "{:<{}}", std::move( _str ), _width );
      else return std::format( "{:^{}}", std::move( _str ), _width );
#else
      __detail::SizeT str_size = _str.size();
      if __PGBAR_ENHANCE_CONSTEXPR__( _style == txt_layut::align_right )
        return __detail::StrT( _width - str_size, blank ) + std::move( _str );
      else if __PGBAR_ENHANCE_CONSTEXPR__( _style == txt_layut::align_left )
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
      if __PGBAR_ENHANCE_CONSTEXPR__ ( std::is_same<StreamObj, std::ostream>::value == false )
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

    __PGBAR_INLINE_FUNC__ __detail::StrT show_bar( double percent ) const {
      __detail::StrT buf; buf.reserve( lbracket_.size() + rbracket_.size() + bar_length_ + 1 );
      const __detail::SizeT done_len = std::round( bar_length_ * percent );
      buf.append(
        lbracket_ + bulk_copy( done_len, done_ch_ ) +
        bulk_copy( bar_length_ - done_len, todo_ch_ ) + rbracket_ +
        __detail::StrT( 1, blank )
      );
      return buf;
    }

    __PGBAR_INLINE_FUNC__ __detail::StrT show_percentage( double percent ) const {
      if ( !is_updated() ) {
        static const __detail::StrT default_str =
          formatter<txt_layut::align_left>( ratio_len, "0.00%" );
        return default_str;
      }

      __detail::StrT proportion = std::to_string( percent * 100 );
      proportion.resize( proportion.find( '.' ) + 3 );

      return formatter<txt_layut::align_left>(
        ratio_len,
        std::move( proportion ) + __detail::StrT( 1, '%' )
      );
    }

    __PGBAR_INLINE_FUNC__ __detail::StrT show_task_counter( __detail::SizeT num_done ) const {
      __detail::StrT total_str = std::to_string( num_tasks_ );
      __detail::SizeT size = total_str.size();
      return (
        formatter<txt_layut::align_right>( size, std::to_string( num_done ) ) +
        __detail::StrT( 1, '/' ) + std::move( total_str )
      );
    }

    __detail::StrT show_rate( std::chrono::duration<__detail::SizeT, std::nano> interval ) const {
      static std::chrono::duration<__detail::SizeT, std::nano> invoke_interval;

      if ( !is_updated() ) {
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

    __detail::StrT show_countdown( std::chrono::duration<__detail::SizeT, std::nano> interval,
                                   __detail::SizeT num_done ) const {
      if ( !is_updated() ) {
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
        time_len, to_time( std::chrono::duration_cast<std::chrono::seconds>(interval * num_done).count() ) +
        __detail::StrT( " < " ) + to_time( std::chrono::duration_cast<std::chrono::seconds>(interval * (num_tasks_ - num_done)).count() )
      );
    }

    /// @brief Based on the value of `option` and bitwise operations,
    /// @brief determine which part of the string needs to be concatenated.
    /// @param percent The percentage of the current task execution.
    /// @return The progress bar that has been assembled but is pending output.
    std::pair<__detail::StrT, __detail::StrT>  generate_barcode( double percent, __detail::SizeT num_done,
                                                                 std::chrono::duration<__detail::SizeT, std::nano> interval ) const {
      enum bit_index : style::Type { _bar = 0, _per, _cnt, _rate, _timer, _terminus };
      static std::bitset<sizeof( style::Type ) * 8> control_bit;
      static __detail::SizeT total_length;

      __detail::StrT rollback, progress_bar;
      if ( !is_updated() ) {
        control_bit = option_;
        control_bit[_terminus] = // indicates whether a status bar exists
          control_bit[_per] || control_bit[_cnt] ||
          control_bit[_rate] || control_bit[_timer];
        total_length = (
          (control_bit[_bar] ? bar_length_ + lbracket_.size() + rbracket_.size() + 1 : 0) +
          (control_bit[_per] ? ratio_len : 0) +
          (control_bit[_cnt] ? cnt_length_ : 0) +
          (control_bit[_rate] ? rate_len : 0) +
          (control_bit[_timer] ? time_len : 0) +
          (control_bit[_terminus] ? lstatus.size() + rstatus.size() : 0)
        );
        const __detail::SizeT status_info_num =
          control_bit[_per] + control_bit[_cnt] + control_bit[_rate] + control_bit[_timer];
        total_length += status_info_num > 1 ? (status_info_num - 1) * division.size() : 0;
      } else rollback = __detail::StrT( total_length, backspace );

      if ( control_bit[_bar] )
        progress_bar.append( show_bar( percent ) );
      if ( control_bit[_terminus] )
        progress_bar.append( __detail::StrT( col_fmt ) + __detail::StrT( lstatus ) );
      if ( control_bit[_per] ) {
        progress_bar.append(
          control_bit[_cnt] || control_bit[_rate] || control_bit[_timer] ?
            show_percentage( percent ) + __detail::StrT( division ) :
            show_percentage( percent )
        );
      }
      if ( control_bit[_cnt] ) {
        progress_bar.append(
          control_bit[_rate] || control_bit[_timer] ?
            show_task_counter( num_done ) + __detail::StrT( division ) :
            show_task_counter( num_done )
        );
      }
      if ( control_bit[_rate] ) {
        progress_bar.append(
          control_bit[_timer] ?
            show_rate( interval ) + __detail::StrT( division ) :
            show_rate( interval )
        );
      }
      if ( control_bit[_timer] )
        progress_bar.append( show_countdown( std::move( interval ), done_cnt_ ) );
      if ( control_bit[_terminus] )
        progress_bar.append( __detail::StrT( rstatus ) + __detail::StrT( default_col ) );

      return { std::move( rollback ), std::move( progress_bar ) };
    }

    /// @brief This function only will be invoked by the rendering thread.
    void rendering() const {
      static bool done_flag = false;
      static std::chrono::duration<__detail::SizeT, std::nano> invoke_interval;
      static std::chrono::system_clock::time_point first_invoked;

      if ( !is_updated() ) {
        done_flag = false;
        if ( in_tty_ ) {
          invoke_interval = {};
          first_invoked = std::chrono::system_clock::now();
          const auto info = generate_barcode( 0.0, 0, {} );
          stream_ << info.first << info.second;
        }
        update_flag_ = true;
      }

      if ( done_flag ) return;

      if ( in_tty_ ) {
        auto now = std::chrono::system_clock::now();
        invoke_interval = done_cnt_ != 0 ? (now - first_invoked) / done_cnt_
          : (now - first_invoked) / static_cast<__detail::SizeT>(1);

        const auto info =
          generate_barcode( done_cnt_ / static_cast<double>(num_tasks_),
                            done_cnt_, invoke_interval );

        stream_ << info.first << info.second;
      }

      if ( is_done() ) {
        if ( in_tty_ ) {
          auto info = generate_barcode( 1, num_tasks_, invoke_interval );
          info.second.append( "\n" );
          stream_ << info.first << info.second;
        }
        done_flag = true;
      }
    }

    template<typename F>
    __PGBAR_INLINE_FUNC__ void do_update( F&& invokable ) {
      static_assert(
#if __PGBAR_CXX20__
        __detail::FunctorType<F>,
#else
        __detail::is_void_functor<F>::value,
#endif // __PGBAR_CXX20__
        __PGBAR_ASSERT_FAILURE__
        "pgbar::do_update: template type error"
        __PGBAR_DEFAULT_COL__
      );

      if ( is_done() )
        throw bad_pgbar { "bad_pgbar: updating a full progress bar" };
      else if ( num_tasks_ == 0 )
        throw bad_pgbar { "bad_pgbar: the number of tasks is zero" };
      if ( !is_updated() )
        rndrer_.active();

      invokable();
      rndrer_.render();

      if ( (done_cnt_ / step_) == (num_tasks_ / step_) )
        rndrer_.suspend(); // wait for sub thread to finish
    }

    static void copy( pgbar& _to, const pgbar& _from ) noexcept {
      _to.num_tasks_ = _from.num_tasks_;
      _to.step_ = _from.step_;
      _to.bar_length_ = _from.bar_length_;
      _to.option_ = _from.option_;
      _to.cnt_length_ = _from.cnt_length_;
    }

    pgbar( StreamObj* _ostream )
      : update_flag_ { false }, rndrer_ { [this]() -> void { this->rendering(); } }, stream_ { *_ostream } {
      num_tasks_ = 0;
      step_ = 0;
      bar_length_ = 50;
      option_ = style::entire;
      done_cnt_ = 0;
      cnt_length_ = 1;
      in_tty_ = check_output_stream( _ostream );
    }

  public:
    using StreamType = StreamObj;
    using RendererType = RenderMode;

    pgbar( __detail::SizeT _total_tsk, __detail::SizeT _each_step, StreamObj& _ostream = std::cerr )
      : pgbar( std::addressof( _ostream ) ) {
      todo_ch_ = __detail::StrT( 1, blank );
      done_ch_ = __detail::StrT( 1, '-' );
      lbracket_ = __detail::StrT( 1, '[' );
      rbracket_ = __detail::StrT( 1, ']' );
      num_tasks_ = _total_tsk;
      step_ = _each_step;

      cnt_length_ = std::to_string( num_tasks_ ).size() * 2 + 1;
    }
    pgbar( __detail::SizeT _total_tsk, StreamObj& _ostream = std::cerr )
      : pgbar(_total_tsk, 1, _ostream) {}
    pgbar( StreamObj& _ostream = std::cerr )
      : pgbar( 0, 1, _ostream) {} // = default constructor
#if __PGBAR_CXX20__
    pgbar( style _initializer, StreamObj& _ostream = std::cerr )
      : pgbar( std::addressof( _ostream ) ) {
      todo_ch_ = _initializer.todo_char.has_value()
        ? std::move( _initializer.todo_char.value() ) : __detail::StrT( 1, blank );
      done_ch_ = _initializer.done_char.has_value()
        ? std::move( _initializer.done_char.value() ) : __detail::StrT( 1, '-' );
      lbracket_ = _initializer.left_bracket.has_value()
        ? std::move( _initializer.left_bracket.value() ) : __detail::StrT( 1, '[' );
      rbracket_ = _initializer.right_bracket.has_value()
        ? std::move( _initializer.right_bracket.value() ) : __detail::StrT( 1, ']' );
      num_tasks_ = _initializer.total_tasks.has_value()
        ? _initializer.total_tasks.value() : num_tasks_;
      step_ = _initializer.each_setp.has_value()
        ? _initializer.each_setp.value() : step_;
      bar_length_ = _initializer.bar_length.has_value()
        ? _initializer.bar_length.value() : bar_length_;
      option_ = _initializer.option.has_value()
        ? _initializer.option.value() : option_;

      cnt_length_ = std::to_string( num_tasks_ ).size() * 2 + 1;
    }
#endif // __PGBAR_CXX20__
    pgbar( const pgbar& _lhs )
      : pgbar( std::addressof( _lhs.stream_ ) ) { // style copy
      todo_ch_ = _lhs.todo_ch_;
      done_ch_ = _lhs.done_ch_;
      lbracket_ = _lhs.lbracket_;
      rbracket_ = _lhs.rbracket_;
      copy( *this, _lhs );
    }
    pgbar( pgbar&& _rhs )
      : pgbar( std::addressof( _rhs.stream_ ) ) {
      todo_ch_ = std::move( _rhs.todo_ch_ );
      done_ch_ = std::move( _rhs.done_ch_ );
      lbracket_ = std::move( _rhs.lbracket_ );
      rbracket_ = std::move( _rhs.rbracket );
      copy( *this, _rhs );
    }
    ~pgbar() {}

    bool is_updated() const noexcept {
      return update_flag_;
    }
    bool is_done() const noexcept {
      return is_updated() && ((done_cnt_ / step_) == (num_tasks_ / step_));
    }
    /// @brief Reset pgbar obj, EXCLUDING the total number of tasks.
    pgbar& reset() {
      if ( !is_updated() )
        return *this;
      done_cnt_ = 0;
      rndrer_.suspend();
      update_flag_ = false;
      return *this;
    }
    /// @brief Set the number of steps the counter is updated each time `update()` is called.
    /// @throw If the `_step` is 0, it will throw an `bad_pgbar`.
    pgbar& set_step( __detail::SizeT _step ) {
      if ( is_updated() ) return *this;
      else if ( _step == 0 ) throw bad_pgbar { "bad_pgbar: zero step_" };
      step_ = _step; return *this;
    }
    /// @brief Set the number of tasks to be updated.
    /// @throw If the `_total_tsk` is 0, it will throw an `bad_pgbar`.
    pgbar& set_task( __detail::SizeT _total_tsk ) {
      if ( is_updated() ) return *this;
      else if ( _total_tsk == 0 )
        throw bad_pgbar { "bad_pgbar: the number of tasks is zero" };
      num_tasks_ = _total_tsk;
      cnt_length_ = std::to_string( num_tasks_ ).size() * 2 + 1;
      return *this;
    }
    /// @brief Set the TODO characters in the progress bar.
    pgbar& set_done( __detail::StrT _done_ch ) noexcept {
      if ( is_updated() ) return *this;
      done_ch_ = std::move( _done_ch );
      return *this;
    }
    /// @brief Set the DONE characters in the progress bar.
    pgbar& set_todo( __detail::StrT _todo_ch ) noexcept {
      if ( is_updated() ) return *this;
      todo_ch_ = std::move( _todo_ch );
      return *this;
    }
    /// @brief Set the left bracket of the progress bar.
    pgbar& set_lbracket( __detail::StrT _l_bracket ) noexcept {
      if ( is_updated() ) return *this;
      lbracket_ = std::move( _l_bracket );
      return *this;
    }
    /// @brief Set the right bracket of the progress bar.
    pgbar& set_rbracket( __detail::StrT _r_bracket ) noexcept {
      if ( is_updated() ) return *this;
      rbracket_ = std::move( _r_bracket );
      return *this;
    }
    /// @brief Set the character length of the whole progress bar
    pgbar& set_bar_length( __detail::SizeT _length ) noexcept {
      if ( is_updated() ) return *this;
      bar_length_ = _length; return *this;
    }
    /// @brief Select the display style by using bit operations.
    pgbar& set_style( style::Type _selection ) noexcept {
      if ( is_updated() ) return *this;
      option_ = _selection; return *this;
    }
#if __PGBAR_CXX20__
    pgbar& set_style( style _selection ) {
      if ( is_updated() ) return *this;
      todo_ch_ = _selection.todo_char.has_value()
        ? std::move( _selection.todo_char.value() ) : todo_ch_;
      done_ch_ = _selection.done_char.has_value()
        ? std::move( _selection.done_char.value() ) : done_ch_;
      lbracket_ = _selection.left_bracket.has_value()
        ? std::move( _selection.left_bracket.value() ) : lbracket_;
      rbracket_ = _selection.right_bracket.has_value()
        ? std::move( _selection.right_bracket.value() ) : rbracket_;
      num_tasks_ = _selection.total_tasks.has_value()
        ? _selection.total_tasks.value() : num_tasks_;
      step_ = _selection.each_setp.has_value()
        ? _selection.each_setp.value() : step_;
      bar_length_ = _selection.bar_length.has_value()
        ? _selection.bar_length.value() : bar_length_;
      option_ = _selection.option.has_value()
        ? _selection.option.value() : option_;

      cnt_length_ = std::to_string( num_tasks_ ).size() * 2 + 1;
      return *this;
    }
#endif // __PGBAR_CXX20__
    pgbar& operator=( const pgbar& _lhs ) {
      if ( this == &_lhs || is_updated() )
        return *this;
      todo_ch_ = _lhs.todo_ch_;
      done_ch_ = _lhs.done_ch_;
      lbracket_ = _lhs.lbracket_;
      rbracket_ = _lhs.rbracket_;
      copy( *this, _lhs );
      return *this;
    }
    pgbar& operator=( pgbar&& _rhs ) {
      todo_ch_ = std::move( _rhs.todo_ch_ );
      done_ch_ = std::move( _rhs.done_ch_ );
      lbracket_ = std::move( _rhs.lbracket_ );
      rbracket_ = std::move( _rhs.rbracket_ );
      copy( *this, _rhs );
      return *this;
    }

    /// @brief Update progress bar.
    void update() {
      do_update( [&]() -> void { done_cnt_ += step_; } );
    }

    /// @brief Ignore the effect of `set_step()`, increment forward several progresses,
    /// @brief and any `next_step` portions that exceed the total number of tasks are ignored.
    /// @param next_step The number that will increment forward the progresses.
    void update( __detail::SizeT next_step ) {
      do_update( [&]() -> void {
        done_cnt_ += ((done_cnt_ + next_step) <= num_tasks_) ? next_step :
          num_tasks_ - done_cnt_;
      } );
    }
  };

#define __PGBAR_TEMPLATE_DECL__ \
  template<typename StreamObj, typename RenderMode> \
    __detail::ConstStrT

  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::lstatus { "[ " };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::rstatus { " ]" };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::division { " | " };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::col_fmt { __PGBAR_COL__ };
  __PGBAR_TEMPLATE_DECL__
    pgbar<StreamObj, RenderMode>::default_col { __PGBAR_DEFAULT_COL__ };

#if __PGBAR_CXX20__
  namespace __detail {
    template <typename B>
    concept PgbarType = requires {
      typename B::StreamType;
      typename B::RendererType;
      requires (
        is_stream_v<typename B::StreamType> &&
        is_renderer_v<typename B::RendererType> &&
        std::is_same_v<pgbar<typename B::StreamType, typename B::RendererType>, B>
      );
    };
  }

  template<typename B>
  struct is_pgbar : std::bool_constant<__detail::PgbarType<B>> {};
#else
  template<typename B, typename = void>
  struct is_pgbar : std::false_type {};
  template<typename B>
  struct is_pgbar<
    B, typename std::enable_if<
      is_stream<typename B::StreamType>::value &&
      is_renderer<typename B::RendererType>::value &&
      std::is_same<pgbar<typename B::StreamType, typename B::RendererType>, B>::value
    >::type
  > : std::true_type {};
#endif // __PGBAR_CXX20__

#if __PGBAR_CXX14__
  template<typename B>
  __PGBAR_INLINE_VAR__ constexpr bool is_pgbar_v = is_pgbar<B>::value;
#endif // __PGBAR_CXX14__
} // namespace pgbar

#undef __PGBAR_TEMPLATE_DECL__

#undef __PGBAR_DEFAULT_COL__
#undef __PGBAR_ASSERT_FAILURE__
#undef __PGBAR_COL__

#undef __PGBAR_RET_CONSTEXPR__
#undef __PGBAR_CXX14__

#undef __PGBAR_FALLTHROUGH__
#undef __PGBAR_ENHANCE_CONSTEXPR__
#undef __PGBAR_INLINE_VAR__
#undef __PGBAR_CXX17__

#undef __PGBAR_CXX20__

#undef __PGBAR_UNKNOW_PLATFORM__
#undef __PGBAR_UNIX__
#undef __PGBAR_WIN__

#undef __PGBAR_CMP_V__
#undef __PGBAR_INLINE_FUNC__

#endif // __PROGRESSBAR_HPP__
