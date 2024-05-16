// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the full license text.
// Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __PROGRESSBAR_HPP__
# define __PROGRESSBAR_HPP__

# include <cstdint>
# include <limits>      // std::numeric_limits
# include <cmath>       // std::round, std::log10, std::trunc, std::ceil
# include <type_traits> // SFINAE
# include <utility>     // std::pair
# include <iterator>    // marks iterator tags
# include <bitset>      // std::bitset
# include <string>      // std::string
# include <chrono>      // as u know
# include <exception>   // std::exception
# include <iostream>    // std::cerr, the output stream object used

# include <atomic>      // std::atomic<bool>
# include <thread>      // std::thread
# include <mutex>       // std::mutex & std::unique_lock
# include <condition_variable> // std::condition_variable

#if defined(__GNUC__) || defined(__clang__)
# define __PGBAR_INLINE_FUNC__ __attribute__((always_inline)) inline
# define __PGBAR_NODISCARD__ __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
# define __PGBAR_INLINE_FUNC__ __forceinline inline
# define __PGBAR_NODISCARD__ _Check_return_
#else
# define __PGBAR_INLINE_FUNC__ inline
# define __PGBAR_NODISCARD__
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

# undef __PGBAR_NODISCARD__
# define __PGBAR_NODISCARD__ [[nodiscard]]
#else
# define __PGBAR_CXX17__ 0
# define __PGBAR_INLINE_VAR__
# define __PGBAR_ENHANCE_CONSTEXPR__
# define __PGBAR_FALLTHROUGH__
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
# define __PGBAR_BOLD__ "\x1B[1m"
# define __PGBAR_BLACK__ "\x1B[30m"
# define __PGBAR_RED__ "\x1B[31m"
# define __PGBAR_GREEN__ "\x1B[32m"
# define __PGBAR_YELLOW__ "\x1B[33m"
# define __PGBAR_BLUE__ "\x1B[34m"
# define __PGBAR_MAGENTA__ "\x1B[35m"
# define __PGBAR_CYAN__ "\x1B[36m"
# define __PGBAR_WHITE__ "\x1B[37m"
# define __PGBAR_DEFAULT_COL__ "\x1B[0m"
#else
# define __PGBAR_BOLD__ ""
# define __PGBAR_BLACK__ ""
# define __PGBAR_RED__ ""
# define __PGBAR_GREEN__ ""
# define __PGBAR_YELLOW__ ""
# define __PGBAR_BLUE__ ""
# define __PGBAR_MAGENTA__ ""
# define __PGBAR_CYAN__ ""
# define __PGBAR_WHITE__ ""
# define __PGBAR_DEFAULT_COL__ ""
#endif // PGBAR_NOT_COL

namespace pgbar {
  class bad_pgbar : public std::exception {
  protected:
    std::string message;

  public:
    bad_pgbar( std::string _mes ) : message { std::move( _mes ) } {}
    virtual ~bad_pgbar() {}
    virtual const char* what() const noexcept { return message.c_str(); }
  };

  namespace __detail {
    using SizeT = size_t;
    using StrT = std::string;
    using CharT = char;
#if __PGBAR_CXX17__
    using ROStrT = std::string_view; // read only string
    using ConstStrT = const std::string_view;
    using LiteralStrT = std::string_view;
#else
    using ROStrT = const StrT&;
    using ConstStrT = const StrT;
    using LiteralStrT = const CharT*;
#endif // __PGBAR_CXX17__

    // The refresh rate is capped at about 25 Hz.
    __PGBAR_INLINE_VAR__ constexpr std::chrono::microseconds reflash_rate
      = std::chrono::microseconds( 35 );

    template<typename T>
    __PGBAR_INLINE_FUNC__
    typename std::enable_if<
      std::is_arithmetic<typename std::decay<T>::type>::value,
      StrT
    >::type ToString( T&& value ) {
      return std::to_string( value );
    }

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
    template<typename T>
    struct is_copyable {
      static constexpr bool value = std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value;
    };
    template<typename T>
    struct copyability_extractor {
      using type = typename std::conditional<is_copyable<T>::value, T, T&>::type;
    };
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
      std::is_same<decltype(std::declval<S&>() << std::declval<__detail::StrT>()), S&>::value
    >::type
  > : std::true_type {};

  template<typename R, typename = void>
  struct is_renderer : std::false_type {};
  template<typename R>
  struct is_renderer<R,
    typename std::enable_if<
      std::is_same<decltype(R( std::declval<void()>() )), R>::value &&
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

  namespace __detail {
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
        "pgbar::__detail::functor_wrapper: template type error"
      );

      typename __detail::copyability_extractor<F>::type func_;

    public:
      template<typename U>
      functor_wrapper( U&& func )
        : func_ { std::forward<U>( func ) } {}
      void run() override final { func_(); }
    };

    template<typename NumT>
    class numeric_iterator {
      static_assert(
        std::is_arithmetic<NumT>::value,
        "pgbar::__detail::range_iterator: Only available for numeric types"
      );

      NumT start_point_, end_point_, step_;
      SizeT cnt_, extent_;

      __PGBAR_INLINE_FUNC__ void init_extent() noexcept {
        const NumT diff = std::max( start_point_, end_point_ ) - std::min( start_point_, end_point_ );
        extent_ = static_cast<SizeT>(
          std::ceil( static_cast<double>(diff) / (step_ < 0 ? -step_ : step_) )
        );
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = NumT;
      using difference_type = void;
      using pointer = void;
      using reference = value_type;

      numeric_iterator() : start_point_ {}, end_point_ {}, step_ {}, cnt_ {}, extent_ {} {}
      /// @throw pgbar::bar_pgbar If '_step' is equal to 0.
      explicit numeric_iterator( value_type _start, value_type _end, value_type _step = 1 )
        : numeric_iterator() {
        if ( _step == 0 )
          throw bad_pgbar { "pgbar::__detail::numeric_iterator: '_step' is zero" };

        start_point_ = _start;
        end_point_ = _end;
        step_ = _step;
        init_extent();
      }
      __PGBAR_NODISCARD__ numeric_iterator begin() const noexcept {
        return *this;
      }
      __PGBAR_NODISCARD__ numeric_iterator end() const noexcept {
        auto endpoint = *this;
        endpoint.cnt_ = extent_;
        return endpoint;
      }
      __PGBAR_NODISCARD__ reference operator*() const noexcept { return cnt_ * step_; }
      __PGBAR_NODISCARD__ bool operator==( value_type _num ) const noexcept {
        return (cnt_ * step_) == _num;
      }
      __PGBAR_NODISCARD__ bool operator!=( value_type _num ) const noexcept {
        return !(operator==( _num ));
      }
      __PGBAR_NODISCARD__ bool operator==( const numeric_iterator& lhs ) const noexcept {
        return extent_ == lhs.extent_ && cnt_ == lhs.cnt_;
      }
      __PGBAR_NODISCARD__ bool operator!=( const numeric_iterator& lhs ) const noexcept {
        return !(operator==( lhs ));
      }
      __PGBAR_INLINE_FUNC__ numeric_iterator& operator++() noexcept {
        ++cnt_; return *this;
      }
      __PGBAR_INLINE_FUNC__ numeric_iterator operator++( int ) noexcept {
        auto before = *this; ++cnt_; return before;
      }
      __PGBAR_INLINE_FUNC__ numeric_iterator& operator+=( value_type _increment ) noexcept {
        const auto num_inc = static_cast<SizeT>(_increment / step_);
        cnt_ = cnt_ + num_inc > extent_ ? extent_ : num_inc;
        return *this;
      }
      __PGBAR_INLINE_FUNC__ void reset() noexcept {
        cnt_ = 0;
      }
      __PGBAR_INLINE_FUNC__ void set_step( value_type _step ) noexcept {
        step_ = _step;
        init_extent();
      }
      __PGBAR_INLINE_FUNC__ void set_upper( value_type _upper ) noexcept {
        end_point_ = _upper;
        init_extent();
      }
      __PGBAR_NODISCARD__ value_type get_step() const noexcept { return step_; }
      __PGBAR_NODISCARD__ value_type get_upper() const noexcept { return end_point_; }
      __PGBAR_NODISCARD__ SizeT get_extent() const noexcept { return extent_; }
      __PGBAR_NODISCARD__ bool is_end() const noexcept { return cnt_ == extent_; }
    };

    /// @brief A dynamic character buffer is provided for string concatenation to reduce heap allocations.
    /// @brief The core thoughts is based on `std::string::clear` does not clear the allocated memory block.
    class streambuf final {
      StrT buffer_;

    public:
      /// @brief Append several characters to the buffer.
      __PGBAR_INLINE_FUNC__ void append( SizeT _num, CharT _ch ) { buffer_.append( _num, _ch ); }
      __PGBAR_INLINE_FUNC__ void reserve( SizeT _size ) { buffer_.reserve( _size ); }
      __PGBAR_INLINE_FUNC__ void clear() { buffer_.clear(); }
      __PGBAR_INLINE_FUNC__ void release() { clear(); buffer_.shrink_to_fit(); }
      __PGBAR_NODISCARD__ __PGBAR_INLINE_FUNC__ StrT& data() noexcept { return buffer_; }

      template<typename _T>
      __PGBAR_INLINE_FUNC__ streambuf& operator<<( _T&& info ) {
        using T = typename std::decay<_T>::type;
        static_assert(
          std::is_same<T, StrT>::value ||
          std::is_same<T, ROStrT>::value ||
          std::is_same<T, ConstStrT>::value ||
          std::is_same<T, LiteralStrT>::value,
          "pgbar::__detail::streambuf: 'T' must be a type that can be appended to 'pgbar::__detail::StrT'"
        );
        buffer_.append( info );
        return *this;
      }
      template<typename S>
      __PGBAR_INLINE_FUNC__ friend S& operator<<( S& stream, streambuf& buf ) { // hidden friend
        static_assert(
          is_stream<S>::value,
          "pgbar::__detail::streambuf: 'S' must be a type that supports 'operator<<' to insert 'pgbar::__detail::StrT'"
        );
        stream << buf.data();
        buf.clear(); return stream;
      }
    };

    template<typename T>
    class generic_wrapper { // for `initr`
      T data;

    public:
      constexpr explicit generic_wrapper( T _data )
        : data { std::move( _data ) } {}
      virtual ~generic_wrapper() = 0;
      __PGBAR_INLINE_FUNC__ T& value() noexcept { return data; }
    };
    template<typename T>
    generic_wrapper<T>::~generic_wrapper() {}

    template<typename W>
    struct is_initr {
    private:
      template<typename U>
      static std::true_type check( const generic_wrapper<U>& );
      static std::false_type check( ... );
    public:
      static constexpr bool value = decltype(check( std::declval<W>() ))::value;
    };

    // The default template parameter is used to handle the case where `sizeof...(Args) == 0`.
    // `generic_wrapper` requires a template parameter, so here we fill in an impossible-to-use type `std::nullptr_t`.
    template<typename T = generic_wrapper<std::nullptr_t>, typename ...Args>
    struct all_of_initr {
      static constexpr bool value = is_initr<T>::value && all_of_initr<Args...>::value;
    };
    template<typename T>
    struct all_of_initr<T> {
      static constexpr bool value = is_initr<T>::value;
    };
  } // namespace __detail

  struct style {
    using Type = uint8_t;

    static constexpr Type bar      = 1 << 0;
    static constexpr Type ratio    = 1 << 1;
    static constexpr Type task_cnt = 1 << 2;
    static constexpr Type rate     = 1 << 3;
    static constexpr Type timer    = 1 << 4;
    static constexpr Type entire   = ~0;
  };

  struct dye {
    static constexpr __detail::LiteralStrT none    = "";
    static constexpr __detail::LiteralStrT black   = __PGBAR_BLACK__;
    static constexpr __detail::LiteralStrT red     = __PGBAR_RED__;
    static constexpr __detail::LiteralStrT green   = __PGBAR_GREEN__;
    static constexpr __detail::LiteralStrT yellow  = __PGBAR_YELLOW__;
    static constexpr __detail::LiteralStrT blue    = __PGBAR_BLUE__;
    static constexpr __detail::LiteralStrT magenta = __PGBAR_MAGENTA__;
    static constexpr __detail::LiteralStrT cyan    = __PGBAR_CYAN__;
    static constexpr __detail::LiteralStrT white   = __PGBAR_WHITE__;
  };

  namespace initr {
    struct option final : public __detail::generic_wrapper<style::Type> {
      constexpr explicit option( style::Type _data ) noexcept
        : __detail::generic_wrapper<style::Type>( _data ) {}
    };
    struct todo_color final : public __detail::generic_wrapper<__detail::LiteralStrT> {
      constexpr explicit todo_color( __detail::LiteralStrT _data )
        : __detail::generic_wrapper<__detail::LiteralStrT>( std::move( _data ) ) {}
    };
    struct done_color final : public __detail::generic_wrapper<__detail::LiteralStrT> {
      constexpr explicit done_color( __detail::LiteralStrT _data )
        : __detail::generic_wrapper<__detail::LiteralStrT>( std::move( _data ) ) {}
    };
    struct status_color final : public __detail::generic_wrapper<__detail::LiteralStrT> {
      constexpr explicit status_color( __detail::LiteralStrT _data )
        : __detail::generic_wrapper<__detail::LiteralStrT>( std::move( _data ) ) {}
    };
    struct todo_char final : public __detail::generic_wrapper<__detail::StrT> {
      explicit todo_char( __detail::StrT _data )
        : __detail::generic_wrapper<__detail::StrT>( std::move( _data ) ) {}
    };
    struct done_char final : public __detail::generic_wrapper<__detail::StrT> {
      explicit done_char( __detail::StrT _data )
        : __detail::generic_wrapper<__detail::StrT>( std::move( _data ) ) {}
    };
    struct startpoint final : public __detail::generic_wrapper<__detail::StrT> {
      explicit startpoint( __detail::StrT _data )
        : __detail::generic_wrapper<__detail::StrT>( std::move( _data ) ) {}
    };
    struct endpoint final : public __detail::generic_wrapper<__detail::StrT> {
      explicit endpoint( __detail::StrT _data )
        : __detail::generic_wrapper<__detail::StrT>( std::move( _data ) ) {}
    };
    struct left_status final : public __detail::generic_wrapper<__detail::StrT> {
      explicit left_status( __detail::StrT _data )
        : __detail::generic_wrapper<__detail::StrT>( std::move( _data ) ) {}
    };
    struct right_status final : public __detail::generic_wrapper<__detail::StrT> {
      explicit right_status( __detail::StrT _data )
        : __detail::generic_wrapper<__detail::StrT>( std::move( _data ) ) {}
    };
    struct total_tasks final : public __detail::generic_wrapper<__detail::SizeT> {
      constexpr explicit total_tasks( __detail::SizeT _data ) noexcept
        : __detail::generic_wrapper<__detail::SizeT>( _data ) {}
    };
    struct each_setp final : public __detail::generic_wrapper<__detail::SizeT> {
      constexpr explicit each_setp( __detail::SizeT _data ) noexcept
        : __detail::generic_wrapper<__detail::SizeT>( _data ) {}
    };
    struct bar_length final : public __detail::generic_wrapper<__detail::SizeT> {
      constexpr explicit bar_length( __detail::SizeT _data ) noexcept
        : __detail::generic_wrapper<__detail::SizeT>( _data ) {}
    };
  } // namespace initr

  namespace __detail {
    template<typename B> // end point
    __PGBAR_INLINE_FUNC__ void pipeline_expan( B& b ) {}

#define __PGBAR_EXPAN_FUNC__(OptionName, MethodName) \
    template<typename B, typename ...Args> \
    inline void pipeline_expan( B& b, initr::OptionName val, Args&&... args ) { \
      b.MethodName( std::move( val.value() ) ); \
      pipeline_expan( b, std::forward<Args>( args )... ); \
    }

    __PGBAR_EXPAN_FUNC__(option, set_style)
    __PGBAR_EXPAN_FUNC__(todo_color, set_todo_col)
    __PGBAR_EXPAN_FUNC__(done_color, set_done_col)
    __PGBAR_EXPAN_FUNC__(status_color, set_status_col)
    __PGBAR_EXPAN_FUNC__(todo_char, set_todo)
    __PGBAR_EXPAN_FUNC__(done_char, set_done)
    __PGBAR_EXPAN_FUNC__(startpoint, set_startpoint)
    __PGBAR_EXPAN_FUNC__(endpoint, set_endpoint)
    __PGBAR_EXPAN_FUNC__(left_status, set_lstatus)
    __PGBAR_EXPAN_FUNC__(right_status, set_rstatus)
    __PGBAR_EXPAN_FUNC__(total_tasks, set_task)
    __PGBAR_EXPAN_FUNC__(each_setp, set_step)
    __PGBAR_EXPAN_FUNC__(bar_length, set_bar_length)

#undef __PGBAR_EXPAN_FUNC__
  } // namespace __detail

  class multithread final {
    __detail::wrapper_base* task_;

    std::atomic<bool> active_flag_;
    std::atomic<bool> suspend_flag_;

    std::atomic<bool> finish_signal_;
    std::atomic<bool> stop_signal_;
    std::condition_variable cond_var_;
    std::mutex mtx_;
    std::thread td_;

    multithread() : task_ { nullptr }
      , active_flag_ { false }, suspend_flag_ { true }
      , finish_signal_ { false }, stop_signal_ { true } {}

  public:
    multithread( const multithread& ) = delete;
    multithread& operator=( multithread&& ) = delete;

#if __PGBAR_CXX20__
    template<__detail::FunctorType F>
#else
    template<
      typename F,
      typename = typename std::enable_if<
        __detail::is_void_functor<typename std::decay<F>::type>::value
      >::type
    >
#endif
    explicit multithread( F&& task )
      : multithread() {
      auto new_res = new __detail::functor_wrapper<typename std::decay<F>::type>( std::forward<F>( task ) );
      task_ = new_res;
      td_ = std::thread( [this]() -> void {
        do {
          {
            std::unique_lock<std::mutex> lock { mtx_ };
            if ( stop_signal_ && !finish_signal_ ) {
              if ( active_flag_ ) // it means child thread has been printed already
                task_->run(); // so output the last progress bar before suspend
              suspend_flag_ = true;
              cond_var_.wait( lock );
            }
          }
          if ( finish_signal_ )
            break;
          active_flag_ = true;
          task_->run();
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
      delete task_;
    }
    void active() {
      stop_signal_ = false;
      cond_var_.notify_one();
      // spin lock
      while ( active_flag_ == false ) {}
      suspend_flag_ = false;
    }
    void suspend() {
      { // there are multiple atomic variables entering the critical region
        std::unique_lock<std::mutex> lock { mtx_ };
        stop_signal_ = true;
      }
      while ( suspend_flag_ == false ) {}
      { // ensure that the thread has been suspended
        std::unique_lock<std::mutex> lock { mtx_ };
        active_flag_ = false;
      }
    }
    void render() noexcept {}
  };

  class singlethread final {
    __detail::wrapper_base* task_;
    bool active_flag_;
    std::chrono::time_point<std::chrono::system_clock> last_invoke_;

  public:
    singlethread( const singlethread& ) = delete;
    singlethread& operator=( singlethread&& ) = delete;

#if __PGBAR_CXX20__
    template<__detail::FunctorType F>
#else
    template<
      typename F,
      typename = typename std::enable_if<
        __detail::is_void_functor<F>::value
      >::type
    >
#endif
    explicit singlethread( F&& tsk )
      : task_ { nullptr }, active_flag_ { false } {
      auto new_res = new __detail::functor_wrapper<typename std::decay<F>::type>( std::forward<F>( tsk ) );
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
      "pgbar::pgbar: The 'StreamObj' must be a type that supports 'operator<<' to insert 'pgbar::__detail::StrT'"
    );
    static_assert(
      is_renderer<RenderMode>::value,
      "pgbar::pgbar: The 'RenderMode' must satisfy the constraint of the type predicate 'pgbar::is_renderer'"
    );

    enum class txt_layout { align_left, align_right, align_center }; // text layout
    enum bit_index : style::Type { bar = 0, per, cnt, rate, timer };
    using BitVector = std::bitset<sizeof( style::Type ) * 8>;
    class rendering_core final { // nested class
      const pgbar& bar_;
      enum class render_state {
        beginning, refreshing, ending, stopped
      } cur_state_;
      double last_bar_progress_;
      std::chrono::system_clock::time_point first_invoked_;

      __PGBAR_INLINE_FUNC__ render_state transition( render_state current_state ) const noexcept;

      __PGBAR_INLINE_FUNC__ void init_member() {
        cur_state_ = render_state::stopped;
        last_bar_progress_ = {};
        first_invoked_ = {};
      }

    public:
      rendering_core( const rendering_core& ) = delete;
      rendering_core& operator=( const rendering_core& ) = delete;

      rendering_core( pgbar<StreamObj, RenderMode>& _bar ) : bar_ { _bar } {
        init_member();
      }
      __PGBAR_INLINE_FUNC__ void reset() noexcept { cur_state_ = render_state::stopped; }
      void operator()();
    };

#define __PGBAR_DEFAULT_RATIO__ " 0.00% "
#define __PGBAR_DEFAULT_TIMER__ "00:00:00 < 99:60:60"
#define __PGBAR_DEFAULT_RATE__ "   inf Hz "

    static constexpr __detail::CharT blank = ' ';
    static constexpr __detail::CharT backspace = '\b';
    static constexpr __detail::SizeT ratio_len = sizeof( __PGBAR_DEFAULT_RATIO__ ) - sizeof( __detail::CharT );
    static constexpr __detail::SizeT timer_len = sizeof( __PGBAR_DEFAULT_TIMER__ ) - sizeof( __detail::CharT );
    static constexpr __detail::SizeT rate_len = sizeof( __PGBAR_DEFAULT_RATE__ ) - sizeof( __detail::CharT );
    static __detail::ConstStrT division; // The default division character.
    // The font style of status bar.
    static constexpr __detail::LiteralStrT font_fmt = __PGBAR_BOLD__;
    // The default color and font style.
    static constexpr __detail::LiteralStrT default_col = __PGBAR_DEFAULT_COL__;

    rendering_core machine_;
    mutable std::atomic<bool> update_flag_;
    RenderMode rndrer_;
    StreamObj& stream_;
    mutable __detail::streambuf buffer_;

    BitVector option_;
    __detail::LiteralStrT todo_col_, done_col_;
    __detail::LiteralStrT status_col_;
    __detail::StrT todo_ch_, done_ch_;
    __detail::StrT startpoint_, endpoint_;
    __detail::StrT lstatus_, rstatus_;
    __detail::numeric_iterator<__detail::SizeT> task_cnt_;
    __detail::SizeT bar_length_;    // The length of the progress bar.
    __detail::SizeT cnt_length_;    // The length of the task counter.
    __detail::SizeT status_length_; // The length of the status bar.

    bool in_tty_, reset_signal_;

    /// @brief Format the `_str`.
    /// @tparam Style Format mode.
    /// @param _width Target length, do nothing if `_width` less than the length of `_str`.
    /// @param _str The string will be formatted.
    /// @return Formatted string.
    template<txt_layout Style>
    __PGBAR_INLINE_FUNC__ static __detail::StrT formatter( __detail::SizeT _width, __detail::ROStrT _str ) {
      if ( _width == 0 ) return {};
      if ( _str.size() >= _width ) return __detail::StrT( _str );
#if __PGBAR_CXX20__
      if __PGBAR_ENHANCE_CONSTEXPR__ ( Style == txt_layout::align_right )
        return std::format( "{:>{}}", _str, _width );
      else if __PGBAR_ENHANCE_CONSTEXPR__ ( Style == txt_layout::align_left )
        return std::format( "{:<{}}", _str, _width );
      else return std::format( "{:^{}}", _str, _width );
#else
      __detail::SizeT str_size = _str.size();
      if __PGBAR_ENHANCE_CONSTEXPR__( Style == txt_layout::align_right )
        return __detail::StrT( _width - str_size, blank ).append( _str );
      else if __PGBAR_ENHANCE_CONSTEXPR__ ( Style == txt_layout::align_left )
        return __detail::StrT( _str ) + __detail::StrT( _width - str_size, blank );
      else {
        _width -= _str.size();
        __detail::SizeT r_blank = _width / 2;
        return __detail::StrT( _width - r_blank, blank ) + __detail::StrT( _str ) + __detail::StrT( r_blank, blank );
      }
#endif // __PGBAR_CXX20__
    }

    /// @brief Copy a string mutiple times and concatenate them together.
    /// @tparam S The type of the string.
    /// @param _time Copy times.
    /// @param _src The string to be copied.
    /// @return The string copied mutiple times.
    static __detail::StrT bulk_copy( __detail::SizeT _time, __detail::ROStrT _src ) {
      if ( _time == 0 || _src.size() == 0 ) return {};
      __detail::StrT ret; ret.reserve( _src.size() * _time );
      for ( __detail::SizeT _ = 0; _ < _time; ++_ )
        ret.append( _src );
      return ret;
    }

    __PGBAR_INLINE_FUNC__ static bool check_output_stream( const StreamObj* const os ) {
      if __PGBAR_ENHANCE_CONSTEXPR__( std::is_same<StreamObj, std::ostream>::value == false )
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

    __PGBAR_NODISCARD__ __PGBAR_INLINE_FUNC__ __detail::StrT show_bar( double num_per ) const {
      const __detail::SizeT done_len = std::round( bar_length_ * num_per );
      return (
        startpoint_ +
        __detail::StrT( done_col_ ) + bulk_copy( done_len, done_ch_ ) +
        __detail::StrT( todo_col_ ) + bulk_copy( bar_length_ - done_len, todo_ch_ ) +
        __detail::StrT( __PGBAR_DEFAULT_COL__ ) + endpoint_ +
        __detail::StrT( 1, blank )
      );
    }

    __PGBAR_NODISCARD__ __PGBAR_INLINE_FUNC__ __detail::StrT show_ratio( double num_per ) const {
      if ( !is_updated() )
        return { __PGBAR_DEFAULT_RATIO__ };

      __detail::StrT proportion = __detail::ToString( num_per * 100.0 );
      proportion.resize( proportion.find( '.' ) + 3 );

      return formatter<txt_layout::align_right>(
        ratio_len,
        std::move( proportion ) + __detail::StrT( 1, '%' )
      );
    }

    __PGBAR_NODISCARD__ __PGBAR_INLINE_FUNC__ __detail::StrT show_task_counter( __detail::SizeT num_done ) const {
      __detail::StrT total_str = __detail::ToString( get_tasks() );
      __detail::SizeT size = total_str.size();
      return (
        formatter<txt_layout::align_right>( size, __detail::ToString( num_done ) ) +
        __detail::StrT( 1, '/' ) + std::move( total_str )
      );
    }

    __PGBAR_NODISCARD__ __detail::StrT show_rate( std::chrono::nanoseconds time_passed,
                                                  __detail::SizeT num_done ) const {
      if ( !is_updated() )
        return { __PGBAR_DEFAULT_RATE__ };

      auto rate2str = []( double val ) -> __detail::StrT {
        __detail::StrT str = __detail::ToString( val );
        str.resize( str.find( '.' ) + 3 ); // Keep two decimal places.
        return str;
      };

      const double seconds_passed = std::chrono::duration<double>( time_passed ).count();
      // zero or negetive is invalid.
      const double frequency = seconds_passed <= 0.0 ? (std::numeric_limits<double>::max)() : num_done / seconds_passed;
      __detail::StrT rate_str;
      if ( frequency < 1e3 ) // < 1Hz => '999.99 Hz'
        rate_str = rate2str( frequency ) + " Hz";
      else if ( frequency < 1e6 ) // < 1 kHz => '999.99 kHz'
        rate_str = rate2str( frequency / 1e3 ) + " kHz";
      else if ( frequency < 1e9 ) // < 1 MHz => '999.99 MHz'
        rate_str = rate2str( frequency / 1e6 ) + " MHz";
      else { // > 999 GHz => infinity
        const double temp = frequency / 1e9;
        if ( temp > 999.99 ) rate_str = __PGBAR_DEFAULT_RATE__; // it's impossible I think
        else rate_str = rate2str( temp ) + __detail::StrT( " GHz" );
      }

      return formatter<txt_layout::align_center>( rate_len, std::move( rate_str ) );
    }

    __PGBAR_NODISCARD__ __detail::StrT show_timer( std::chrono::nanoseconds time_passed,
                                                   __detail::SizeT num_done ) const {
      if ( !is_updated() )
        return { __PGBAR_DEFAULT_TIMER__ };

      auto time2str = []( int64_t num_time ) -> __detail::StrT {
        __detail::StrT ret = __detail::ToString( num_time );
        if ( ret.size() < 2 ) return "0" + ret;
        return ret;
      };
      auto to_time = [&time2str]( std::chrono::nanoseconds duration ) -> __detail::StrT {
        const auto hours = std::chrono::duration_cast<std::chrono::hours>( duration );
        duration -= hours;
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>( duration );
        duration -= minutes;
        return (
          ((hours.count() > 99 ? __detail::StrT( "99" ) : time2str( hours.count() )) + ":") +
          (time2str( minutes.count() ) + ":") +
          time2str( std::chrono::duration_cast<std::chrono::seconds>(duration).count() )
        );
      };

      auto time_per_task = time_passed / num_done;
      if ( time_per_task.count() == 0 )
        time_per_task = std::chrono::duration<__detail::SizeT, std::nano>( 1 );
      std::chrono::nanoseconds estimated_time = time_per_task * (get_tasks() - num_done);

      return formatter<txt_layout::align_center>(
        timer_len,
        to_time( std::move( time_passed ) ) + __detail::StrT( " < " ) +
        to_time( std::move( estimated_time ) )
      );
    }

    /// @brief Based on the value of `option` and bitwise operations,
    /// @brief determine which part of the string needs to be concatenated.
    void generate_barcode( BitVector ctrller, double num_per, __detail::SizeT num_done,
                           std::chrono::nanoseconds time_passed ) const {
      const __detail::SizeT total_length = (
        (ctrller[bit_index::bar] ?
          (bar_length_ + startpoint_.size() + endpoint_.size() + 1) : 0)
        + status_length_);
      buffer_.reserve( total_length * 2 + 1 ); // The extra 1 is for '\n'.

      if ( is_updated() )
        buffer_.append( total_length, backspace );

      if ( ctrller[bit_index::bar] )
        buffer_ << show_bar( num_per );
      if ( status_length_ != 0 )
        buffer_ << font_fmt << status_col_ << lstatus_;
      if ( ctrller[bit_index::per] ) {
        buffer_ << show_ratio( num_per );
        if ( ctrller[bit_index::cnt] || ctrller[bit_index::rate] || ctrller[bit_index::timer] )
          buffer_ << division;
      }
      if ( ctrller[bit_index::cnt] ) {
        buffer_ << show_task_counter( num_done );
        if ( ctrller[bit_index::rate] || ctrller[bit_index::timer] )
          buffer_ << division;
      }
      if ( ctrller[bit_index::rate] ) {
        buffer_ << show_rate( time_passed, num_done );
        if ( ctrller[bit_index::timer] )
          buffer_ << division;
      }
      if ( ctrller[bit_index::timer] )
        buffer_ << show_timer( std::move( time_passed ), num_done );
      if ( status_length_ != 0 )
        buffer_ << rstatus_ << default_col;
    }

    template<typename F>
    /// @throw pgbar::bad_pgbar If the updating is done or the number of tasks is 0.
    __PGBAR_INLINE_FUNC__ void do_update( F&& updating_task ) {
      static_assert(
#if __PGBAR_CXX20__
        __detail::FunctorType<F>,
#else
        __detail::is_void_functor<F>::value,
#endif // __PGBAR_CXX20__
        "pgbar::pgbar::do_update: template type error"
      );

      if ( is_done() )
        throw bad_pgbar { "pgbar::do_update: updating a full progress bar" };
      if ( !is_updated() ) {
        if ( task_cnt_.get_upper() == 0 )
          throw bad_pgbar { "pgbar::do_update: the number of tasks is zero" };
        rndrer_.active();
      }

      updating_task();
      rndrer_.render();

      if ( task_cnt_.is_end() )
        rndrer_.suspend(); // wait for child thread to finish
    }

    __PGBAR_INLINE_FUNC__ void pod_copy( const pgbar& _from ) noexcept {
      bar_length_ = _from.bar_length_;
      cnt_length_ = _from.cnt_length_;
      status_length_ = _from.status_length_;
    }
    __PGBAR_INLINE_FUNC__ void npod_copy( const pgbar& _from ) {
      option_ = _from.option_;
      todo_col_ = _from.todo_col_;
      done_col_ = _from.done_col_;
      status_col_ = _from.status_col_;
      todo_ch_ = _from.todo_ch_;
      done_ch_ = _from.done_ch_;
      startpoint_ = _from.startpoint_;
      endpoint_ = _from.endpoint_;
      lstatus_ = _from.lstatus_;
      rstatus_ = _from.rstatus_;
      task_cnt_ = _from.task_cnt_;
      task_cnt_.reset();
    }
    __PGBAR_INLINE_FUNC__ void npod_move( pgbar& _from ) {
      option_ = std::move( _from.option_ );
      todo_col_ = std::move( _from.todo_col_ );
      done_col_ = std::move( _from.done_col_ );
      status_col_ = std::move( _from.status_col_ );
      todo_ch_ = std::move( _from.todo_ch_ );
      done_ch_ = std::move( _from.done_ch_ );
      startpoint_ = std::move( _from.startpoint_ );
      endpoint_ = std::move( _from.endpoint_ );
      lstatus_ = std::move( _from.lstatus_ );
      rstatus_ = std::move( _from.rstatus_ );
      task_cnt_ = std::move( _from.task_cnt_ );
      task_cnt_.reset();
    }
    __PGBAR_INLINE_FUNC__ void init_length( bool update_cnt_len = true ) {
      if ( update_cnt_len )
        cnt_length_ = static_cast<__detail::SizeT>(
          std::log10( get_tasks() ) + 1) * 2 + 1;

      status_length_ = (
        (option_[bit_index::per] ? ratio_len : 0) +
        (option_[bit_index::cnt] ? cnt_length_ : 0) +
        (option_[bit_index::rate] ? rate_len : 0) +
        (option_[bit_index::timer] ? timer_len : 0)
      );
      if ( status_length_ != 0 ) {
        status_length_ += lstatus_.size() + rstatus_.size();
        const __detail::SizeT status_num =
          option_[bit_index::per] + option_[bit_index::cnt] + option_[bit_index::rate] + option_[bit_index::timer];
        status_length_ += status_num > 1 ? (status_num - 1) * division.size() : 0;
      }
    }

    pgbar( StreamObj* _ostream )
      : machine_ { *this }, update_flag_ { false }
      , rndrer_ { machine_ }, stream_ { *_ostream } {
      option_ = style::entire;
      todo_col_ = dye::none;
      done_col_ = dye::none;
      status_col_ = dye::cyan;
      bar_length_ = 30;
      cnt_length_ = 1;
      status_length_ = 0;
      in_tty_ = check_output_stream( _ostream );
      reset_signal_ = false;
    }

  public:
    using StreamType = StreamObj;
    using RendererType = RenderMode;

    pgbar( __detail::SizeT _total_tsk, __detail::SizeT _each_step, StreamObj& _ostream = std::cerr )
      : pgbar( std::addressof( _ostream ) ) {
      todo_ch_ = __detail::StrT( 1, blank );
      done_ch_ = __detail::StrT( 1, '-' );
      startpoint_ = __detail::StrT( 1, '[' );
      endpoint_ = __detail::StrT( 1, ']' );
      lstatus_ = __detail::StrT( "[ " );
      rstatus_ = __detail::StrT( " ]" );
      task_cnt_ = __detail::numeric_iterator<__detail::SizeT>( 0, _total_tsk, _each_step );

      init_length();
    }
    pgbar( __detail::SizeT _total_tsk, StreamObj& _ostream = std::cerr )
      : pgbar( _total_tsk, 1, _ostream ) {}
    template<typename ...Args, typename =
      typename std::enable_if<
        __detail::all_of_initr<typename std::decay<Args>::type...>::value
      >::type
    > pgbar( StreamObj& _ostream = std::cerr, Args&&... args )
      : pgbar( 0, _ostream ) { // = default constructor
      __detail::pipeline_expan( *this, std::forward<Args>( args )... );
      init_length();
    }
    pgbar( const pgbar& _lhs )
      : pgbar( std::addressof( _lhs.stream_ ) ) {
      npod_copy( _lhs );
      pod_copy( _lhs );
    }
    pgbar( pgbar&& _rhs )
      : pgbar( std::addressof( _rhs.stream_ ) ) {
      npod_move( _rhs );
      pod_copy( _rhs );
    }
    ~pgbar() { reset(); }
    pgbar& operator=( const pgbar& _lhs ) {
      if ( this == &_lhs || is_updated() )
        return *this;
      npod_copy( *this, _lhs );
      pod_copy( *this, _lhs );
      return *this;
    }
    pgbar& operator=( pgbar&& _rhs ) {
      if ( this == &_rhs || is_updated() )
        return *this;
      npod_move( *this, _rhs );
      pod_copy( *this, _rhs );
      return *this;
    }

    bool is_updated() const noexcept {
      return update_flag_;
    }
    bool is_done() const noexcept {
      return is_updated() && task_cnt_.is_end();
    }
    /// @brief Reset pgbar obj, EXCLUDING the total number of tasks.
    pgbar& reset() {
      reset_signal_ = true;
      rndrer_.suspend();
      machine_.reset();
      task_cnt_.reset();
      update_flag_ = false;
      reset_signal_ = false;
      return *this;
    }
    /// @brief Set the number of steps the counter is updated each time `update()` is called.
    /// @throw pgbar::bad_pgbar If the `_step` is zero.
    pgbar& set_step( __detail::SizeT _step ) {
      if ( is_updated() ) return *this;
      else if ( _step == 0 ) throw bad_pgbar { "pgbar::set_step: zero step" };
      task_cnt_.set_step( _step ); return *this;
    }
    /// @brief Set the number of tasks to be updated.
    /// @throw pgbar::bad_pgbar If the `_total_tsk` is zero.
    pgbar& set_task( __detail::SizeT _total_tsk ) {
      if ( is_updated() ) return *this;
      else if ( _total_tsk == 0 )
        throw bad_pgbar { "pgbar::set_task: the number of tasks is zero" };
      task_cnt_.set_upper( _total_tsk );
      init_length(); return *this;
    }
    /// @brief Set the TODO characters in the progress bar.
    pgbar& set_done( __detail::StrT _done_ch ) noexcept {
      if ( !is_updated() )
        done_ch_ = std::move( _done_ch );
      return *this;
    }
    /// @brief Set the DONE characters in the progress bar.
    pgbar& set_todo( __detail::StrT _todo_ch ) noexcept {
      if ( !is_updated() )
        todo_ch_ = std::move( _todo_ch );
      return *this;
    }
    /// @brief Set the start point of the progress bar.
    pgbar& set_startpoint( __detail::StrT _startpoint ) noexcept {
      if ( !is_updated() )
        startpoint_ = std::move( _startpoint );
      return *this;
    }
    /// @brief Set the endpoint of the progress bar.
    pgbar& set_endpoint( __detail::StrT _endpoint ) noexcept {
      if ( !is_updated() )
        endpoint_ = std::move( _endpoint );
      return *this;
    }
    /// @brief Set the left bracket of the status bar.
    pgbar& set_lstatus( __detail::StrT _lstatus ) noexcept {
      if ( !is_updated() )
        lstatus_ = std::move( _lstatus );
      init_length( false );
      return *this;
    }
    /// @brief Set the right bracket of the status bar.
    pgbar& set_rstatus( __detail::StrT _rstatus ) noexcept {
      if ( !is_updated() )
        rstatus_ = std::move( _rstatus );
      init_length( false );
      return *this;
    }
    /// @brief Set the character length of the whole progress bar
    pgbar& set_bar_length( __detail::SizeT _length ) noexcept {
      if ( !is_updated() )
        bar_length_ = _length;
      return *this;
    }
    /// @brief Set the color of the todo characters in the progress bar
    pgbar& set_todo_col( __detail::LiteralStrT _dye ) noexcept {
      if ( !is_updated() )
        todo_col_ = std::move( _dye );
      return *this;
    }
    /// @brief Set the color of the done characters in the progress bar
    pgbar& set_done_col( __detail::LiteralStrT _dye ) noexcept {
      if ( !is_updated() )
        done_col_ = std::move( _dye );
      return *this;
    }
    /// @brief Set the color of the status bar
    pgbar& set_status_col( __detail::LiteralStrT _dye ) noexcept {
      if ( !is_updated() )
        status_col_ = std::move( _dye );
      return *this;
    }
    /// @brief Select the display style by using bit operations.
    pgbar& set_style( style::Type _selection ) noexcept {
      if ( !is_updated() )
        option_ = _selection;
      init_length( false );
      return *this;
    }
    template<typename ...Args>
    typename std::enable_if<
      __detail::all_of_initr<typename std::decay<Args>::type...>::value,
      pgbar&
    >::type set_style( Args&&... args ) {
      __detail::pipeline_expan( *this, std::forward<Args>( args )... );
      init_length();
      return *this;
    }
    /// @brief Get the total number of tasks.
    __PGBAR_NODISCARD__ __detail::SizeT get_tasks() const noexcept {
      return task_cnt_.get_upper();
    }
    /// @brief Get the number of steps the iteration advances.
    __PGBAR_NODISCARD__ __detail::SizeT get_step() const noexcept {
      return task_cnt_.get_step();
    }
    /// @brief Get the number of tasks that have been updated.
    __PGBAR_NODISCARD__ __detail::SizeT get_current() const noexcept {
      return *task_cnt_;
    }
    /// @brief Update progress bar.
    void update() {
      do_update( [this]() -> void { ++task_cnt_; } );
    }
    /// @brief Ignore the effect of `set_step()`, increment forward several progresses,
    /// @brief and any `next_step` portions that exceed the total number of tasks are ignored.
    /// @param next_step The number that will increment forward the progresses.
    void update( __detail::SizeT next_step ) {
      do_update(
        [this, &next_step]() -> void {
          task_cnt_ += next_step;
        }
      );
    }
    /// @brief Set the iteration steps of the progress bar to a specified percentage.
    /// @brief Ignore the call if the iteration count exceeds the given percentage.
    /// @brief If `percentage` is bigger than 100, it will be set to 100.
    /// @param percentage Value range: [0, 100].
    void update_to( __detail::SizeT percentage ) {
      if ( percentage < 100 ) {
        const __detail::SizeT current_percent = std::trunc(
          (static_cast<double>(get_current()) / get_tasks()) * 100.0 );
        const __detail::SizeT differ = percentage - current_percent;
        if ( differ > 1 ) // calculates the next step
          update( differ * 0.01 * get_tasks() );
      } else do_update( [this]() -> void { task_cnt_ = task_cnt_.end(); } );
    }
  };

  template<typename StreamObj, typename RenderMode>
  __detail::ConstStrT pgbar<StreamObj, RenderMode>::division { " | " };

#define __PGBAR_NAME_PREFIX__ pgbar<StreamObj, RenderMode>::rendering_core
  template<typename StreamObj, typename RenderMode>
  typename __PGBAR_NAME_PREFIX__::render_state
  __PGBAR_NAME_PREFIX__::transition( render_state current_state ) const noexcept {
    switch ( current_state ) {
    case render_state::beginning: // fallthrough
      __PGBAR_FALLTHROUGH__
    case render_state::refreshing:
      return bar_.reset_signal_ || bar_.is_done() ? render_state::ending : render_state::refreshing;
    case render_state::ending: // fallthrough
      __PGBAR_FALLTHROUGH__
    case render_state::stopped:
      return !bar_.in_tty_ || bar_.reset_signal_ || bar_.is_done() ?
             render_state::stopped : render_state::beginning;
    default: break;
    }
    return render_state::stopped;
  }

  template<typename StreamObj, typename RenderMode>
  void __PGBAR_NAME_PREFIX__::operator()() {
    switch ( cur_state_ = transition( cur_state_ ) ) {
    case render_state::beginning: { // intermediate state
      last_bar_progress_ = {};
      first_invoked_ = std::chrono::system_clock::now();

      bar_.generate_barcode( bar_.option_, 0.0, 0, {} );
      bar_.stream_ << bar_.buffer_; // For visual purposes, output the full progress bar at the beginning.
      bar_.update_flag_ = true;
      cur_state_ = render_state::refreshing; // unconditional jump
    } break;

    case render_state::refreshing: {
      const auto current = bar_.get_current();
      const double num_percent = current / static_cast<double>(bar_.get_tasks());

      auto controller = bar_.option_;
      if ( num_percent - last_bar_progress_ < 0.01 )
        controller.reset( pgbar<StreamObj, RenderMode>::bit_index::bar );
      else last_bar_progress_ = num_percent;

      // Then normally output the progress bar.
      bar_.generate_barcode( std::move( controller ), num_percent, current,
                             std::chrono::system_clock::now() - first_invoked_ );
      bar_.stream_ << bar_.buffer_;
    } break;

    case render_state::ending: { // intermediate state
      if ( !bar_.reset_signal_ )
        bar_.generate_barcode( bar_.option_, 1, bar_.get_tasks(),
                               std::chrono::system_clock::now() - first_invoked_ );
      bar_.buffer_.append( 1, '\n' );
      bar_.stream_ << bar_.buffer_; // Same, for visual purposes.
      bar_.buffer_.release(); // releases the buffer
      cur_state_ = render_state::stopped; // unconditional jump
    } break;

    case render_state::stopped: // fallthrough
      __PGBAR_FALLTHROUGH__
    default: return;
    }
  }
#undef __PGBAR_NAME_PREFIX__

#if __PGBAR_CXX20__
  namespace __detail {
    template <typename B>
    concept PgbarType = requires {
      typename B::StreamType;
      typename B::RendererType;
      requires (
        std::conjunction_v<
          is_stream<typename B::StreamType>,
          is_renderer<typename B::RendererType>,
          std::is_same<pgbar<typename B::StreamType, typename B::RendererType>, B>
        >
      );
    };
  }

  template<typename B>
  struct is_pgbar : std::bool_constant<__detail::PgbarType<B>> {};
#else
  template<typename B, typename = void>
  struct is_pgbar : std::false_type {};
  template<typename B>
  struct is_pgbar<B,
    typename std::enable_if<
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

  template<typename R = multithread, typename S, typename ...Args>
#if __PGBAR_CXX20__
  requires std::conjunction_v<
    is_stream<S>,
    is_renderer<R>,
    __detail::all_of_initr<Args...>
  > __PGBAR_NODISCARD__ inline pgbar<S, R>
#else
  __PGBAR_NODISCARD__ inline typename std::enable_if <
    is_stream<S>::value &&
    is_renderer<R>::value &&
    __detail::all_of_initr<Args...>::value,
    pgbar<S, R>
  >::type
#endif // __PGBAR_CXX20__
  make_pgbar( S& stream_obj, Args&&... args ) {
    pgbar<S, R> bar { stream_obj };
    __detail::pipeline_expan( bar, std::forward<Args>( args )... );
    return bar;
  }
} // namespace pgbar

#undef __PGBAR_DEFAULT_RATIO__
#undef __PGBAR_DEFAULT_TIMER__
#undef __PGBAR_DEFAULT_RATE__

#undef __PGBAR_DEFAULT_COL__
#undef __PGBAR_WHITE__
#undef __PGBAR_CYAN__
#undef __PGBAR_MAGENTA__
#undef __PGBAR_BLUE__
#undef __PGBAR_YELLOW__
#undef __PGBAR_GREEN__
#undef __PGBAR_RED__
#undef __PGBAR_BLACK__
#undef __PGBAR_BOLD__

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
#undef __PGBAR_NODISCARD__
#undef __PGBAR_INLINE_FUNC__

#endif // __PROGRESSBAR_HPP__
