// This code is licensed under the MIT License.
// Please see the LICENSE file in the root of the repository for the
// full license text. Copyright (c) 2023-2024 Konvt
#pragma once

#ifndef __KONVT_PGBAR
# define __KONVT_PGBAR

# if defined( __GNUC__ ) || defined( __clang__ )
#  define __PGBAR_NODISCARD __attribute__( ( warn_unused_result ) )
#  define __PGBAR_INLINE_FN __attribute__( ( always_inline ) ) inline
# elif defined( _MSC_VER )
#  define __PGBAR_NODISCARD _Check_return_
#  define __PGBAR_INLINE_FN __forceline inline
# else
#  define __PGBAR_NODISCARD
#  define __PGBAR_INLINE_FN inline
# endif

# if defined( _MSVC_VER ) && defined( _MSVC_LANG ) // for msvc
#  define __PGBAR_CC_STD _MSVC_LANG
# else
#  define __PGBAR_CC_STD __cplusplus
# endif

# if defined( _WIN32 ) || defined( _WIN64 )
#  ifndef PGBAR_INTTY
#   include <io.h>
#  endif

#  define __PGBAR_WIN     1
#  define __PGBAR_UNIX    0
#  define __PGBAR_UNKNOWN 0
# elif defined( __unix__ )
#  ifndef PGBAR_INTTY
#   include <sys/io.h>
#   include <unistd.h>
#  endif

#  define __PGBAR_WIN     0
#  define __PGBAR_UNIX    1
#  define __PGBAR_UNKNOWN 0
# else
#  define __PGBAR_WIN     0
#  define __PGBAR_UNIX    0
#  define __PGBAR_UNKNOWN 1
# endif

# if __PGBAR_CC_STD >= 202002L
#  include <concepts>
#  include <format>
#  define __PGBAR_CXX20        1
#  define __PGBAR_NOUNIQUEADDR [[no_unique_address]]
# else
#  define __PGBAR_CXX20 0
#  define __PGBAR_NOUNIQUEADDR
# endif
# if __PGBAR_CC_STD >= 201703L
#  include <string_view>
#  define __PGBAR_CXX17        1
#  define __PGBAR_INLINE_VAR   inline
#  define __PGBAR_CONSTEXPR_IF constexpr
#  define __PGBAR_FALLTHROUGH  [[fallthrough]]
#  define __PGBAR_UNLIKELY     [[unlikely]]

#  undef __PGBAR_NODISCARD
#  define __PGBAR_NODISCARD [[nodiscard]]
# else
#  define __PGBAR_CXX17 0
#  define __PGBAR_INLINE_VAR
#  define __PGBAR_CONSTEXPR_IF
#  define __PGBAR_FALLTHROUGH
#  define __PGBAR_UNLIKELY
# endif
# if __PGBAR_CC_STD >= 201402L
#  define __PGBAR_CXX14                1
#  define __PGBAR_RELAXED_CONSTEXPR_FN constexpr
# else
#  define __PGBAR_CXX14 0
#  define __PGBAR_RELAXED_CONSTEXPR_FN
# endif
# if __PGBAR_CC_STD >= 201103L
#  define __PGBAR_CXX11 1
# else
#  error "The library 'pgbar' requires C++11"
# endif

# include <algorithm>
# include <atomic>
# include <bitset>
# include <chrono>
# include <cmath>
# include <condition_variable>
# include <cstdint>
# include <exception>
# include <initializer_list>
# include <iostream>
# include <iterator>
# include <limits>
# include <memory>
# include <mutex>
# include <queue>
# include <string>
# include <thread>
# include <type_traits>
# include <utility>
# include <vector>

# ifdef PGBAR_DEBUG
#  include <cassert>
#  define __PGBAR_ASSERT( expr ) assert( expr )
#  undef __PGBAR_INLINE_FN
#  define __PGBAR_INLINE_FN
# else
#  define __PGBAR_ASSERT( expr )
# endif

# define __PGBAR_BOLD    0xB01DFACE // bold face
# define __PGBAR_DEFAULT 0xC105EA11 // C1O5E -> ClOSE, A11 -> All
# define __PGBAR_BLACK   0x000000
# define __PGBAR_RED     0xFF0000
# define __PGBAR_GREEN   0x00FF00
# define __PGBAR_YELLOW  0xFFFF00
# define __PGBAR_BLUE    0x0000FF
# define __PGBAR_MAGENTA 0x800080
# define __PGBAR_CYAN    0x00FFFF
# define __PGBAR_WHITE   0xFFFFFF

namespace pgbar {
  namespace exceptions {
    /**
     * The base exception class.
     */
    class BarError : public std::exception {
    protected:
# if __PGBAR_CXX17
      std::string_view message_;

    public:
      BarError( std::string_view mes ) noexcept : message_ { std::move( mes ) } {}
      virtual const char* what() const noexcept { return message_.data(); }
# else
      const char* message_;

    public:
      BarError( const char* mes ) noexcept : message_ { mes } {}
      virtual const char* what() const noexcept { return message_; }
# endif
      virtual ~BarError() noexcept = default;
    };

    /**
     * Exception for invalid function arguments.
     */
    class InvalidArgument : public BarError {
    public:
      using BarError::BarError;
      virtual ~InvalidArgument() noexcept = default;
    };

    /**
     * Exception for error state of object.
     */
    class InvalidState : public BarError {
    public:
      using BarError::BarError;
      virtual ~InvalidState() noexcept = default;
    };
  } // namespace exceptions

  namespace __detail {
    namespace types {
      using Size   = std::size_t;
      using String = std::string;
      using Char   = char;
# if __PGBAR_CXX17
      using ROStr  = std::string_view;
      using LitStr = ROStr; // literal strings
# else
      using ROStr = typename std::add_lvalue_reference<typename std::add_const<String>::type>::type;
      using LitStr = typename std::add_pointer<typename std::add_const<Char>::type>::type;
# endif
      // a constant string type with `size()` method
      using ConstStr   = typename std::add_const<typename std::decay<ROStr>::type>::type;
      using HexRGB     = std::uint32_t;
      using Float      = double;
      using TimeUnit   = std::chrono::nanoseconds;
      using BitwiseSet = std::uint8_t;
    } // namespace types

    namespace constants {
      __PGBAR_INLINE_VAR constexpr types::Char blank            = ' ';
      __PGBAR_INLINE_VAR constexpr types::LitStr cursor_save    = "\x1b[s";
      __PGBAR_INLINE_VAR constexpr types::LitStr cursor_restore = "\x1b[u";
    }

    namespace traits {
      template<typename, typename>
      struct is_in_tuple;
      template<typename T>
      struct is_in_tuple<T, std::tuple<>> : std::false_type {};
      template<typename T, typename U, typename... Us>
      struct is_in_tuple<T, std::tuple<U, Us...>>
        : std::conditional<std::is_same<T, U>::value,
                           std::true_type,
                           is_in_tuple<T, std::tuple<Us...>>>::type {};

      template<typename, typename...>
      struct all_in_tuple;
      template<typename Tuple>
      struct all_in_tuple<Tuple> : std::true_type {};
      template<typename Tuple, typename First, typename... Rest>
      struct all_in_tuple<Tuple, First, Rest...>
        : std::conditional<is_in_tuple<First, Tuple>::value,
                           all_in_tuple<Tuple, Rest...>,
                           std::false_type>::type {};

      template<typename T, typename = void>
      struct iterator_type {
        using type = typename std::decay<T>::type;
      };
      template<typename T>
      struct iterator_type<T,
                           typename std::enable_if<
                             std::is_same<typename std::decay<T>::type::iterator,
                                          typename std::decay<T>::type::iterator>::value>::type> {
        using type = typename std::decay<T>::type::iterator;
      };

# if __PGBAR_CXX20
      template<typename F>
      concept TaskFunctor = requires( F fn ) {
        { fn() } -> std::same_as<void>;
      };
      template<typename F>
      struct is_void_functor : std::bool_constant<TaskFunctor<F>> {};

      template<typename M>
      concept Mutex = requires( M mtx ) {
        requires !std::is_reference_v<M>;
        { mtx.lock() } -> std::same_as<void>;
        { mtx.unlock() } -> std::same_as<void>;
      };
      template<typename M>
      struct is_mutex : std::bool_constant<Mutex<M>> {};

      template<typename S>
      concept OStream = requires( S stream ) {
        requires !std::is_reference_v<S>;
        { stream << types::String {} } -> std::same_as<S&>;
      };
      template<typename S>
      struct is_ostream : std::bool_constant<OStream<S>> {};
# else
      template<typename, typename = void>
      struct is_void_functor : std::false_type {};
      template<typename F>
      struct is_void_functor<
        F,
        typename std::enable_if<std::is_void<decltype( std::declval<F>()() )>::value>::type>
        : std::true_type {};

      template<typename, typename = void>
      struct is_mutex : std::false_type {};
      template<typename M>
      struct is_mutex<
        M,
        typename std::enable_if<
          !std::is_reference<M>::value && std::is_void<decltype( std::declval<M&>().lock() )>::value
          && std::is_void<decltype( std::declval<M&>().unlock() )>::value>::type>
        : std::true_type {};

      template<typename, typename = void>
      struct is_ostream : std::false_type {};
      template<typename S>
      struct is_ostream<
        S,
        typename std::enable_if<
          !std::is_reference<S>::value
          && std::is_same<decltype( std::declval<S&>() << std::declval<types::String>() ),
                          S&>::value>::type> : std::true_type {};
# endif
    } // namespace traits

    template<typename I>
    class IterSpanBase {
      static_assert( !std::is_arithmetic<I>::value,
                     "pgbar::__detail::IterSpanBase: Only available for iterator types" );
      static_assert( !std::is_void<typename std::iterator_traits<I>::difference_type>::value,
                     "pgbar::__detail::IterSpanBase: The 'difference_type' of the given "
                     "iterators cannot be 'void'" );

      /**
       * Measure the length of the iteration range.
       */
      __PGBAR_INLINE_FN types::Size measure() const noexcept
      {
        const auto length = std::distance( start_, end_ );
        if __PGBAR_CONSTEXPR_IF ( std::is_pointer<I>::value )
          return length >= 0 ? length : -length;
        else
          return length;
      }

    protected:
      I start_, end_;
      types::Size size_;

    public:
      IterSpanBase( I startpoint, I endpoint ) noexcept
        : start_ { std::move( startpoint ) }, end_ { std::move( endpoint ) }, size_ { 0 }
      {
        size_ = measure();
        __PGBAR_ASSERT( size_ >= 0 );
      }
      virtual ~IterSpanBase() noexcept( std::is_nothrow_destructible<I>::value ) = 0;

      __PGBAR_INLINE_FN I& start_iter() noexcept { return start_; }
      __PGBAR_INLINE_FN I& end_iter() noexcept { return end_; }

      __PGBAR_INLINE_FN IterSpanBase& start_iter( I startpoint )
        noexcept( std::is_nothrow_move_assignable<I>::value )
      {
        start_ = std::move( startpoint );
        size_  = measure();
        __PGBAR_ASSERT( size_ >= 0 );
        return *this;
      }
      __PGBAR_INLINE_FN IterSpanBase& end_iter( I endpoint )
        noexcept( std::is_nothrow_move_constructible<I>::value )
      {
        end_  = std::move( endpoint );
        size_ = measure();
        __PGBAR_ASSERT( size_ >= 0 );
        return *this;
      }

      __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size step() const noexcept { return 1; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN types::Size size() const noexcept { return size_; }

      void swap( IterSpanBase<I>& lhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        using std::swap;
        swap( start_, lhs.start_ );
        swap( end_, lhs.end_ );
        swap( size_, lhs.size_ );
      }
      friend void swap( IterSpanBase<I>& a, IterSpanBase<I>& b ) noexcept { a.swap( b ); }
    };
    template<typename I>
    IterSpanBase<I>::~IterSpanBase() noexcept( std::is_nothrow_destructible<I>::value ) = default;
  } // namespace __detail

  namespace iterators {
    /**
     * An undirectional range delimited by an numeric interval [start, end).
     *
     * The `end` can be less than the `start` only if the `step` is negative,
     * otherwise it throws exceptions `pgbar::exceptions::InvalidArgument`.
     */
    template<typename N>
    class NumericSpan {
      static_assert( std::is_arithmetic<N>::value,
                     "pgbar::iterators::NumericSpan: Only available for arithmetic types" );

      N start_, end_, step_;

    public:
      class iterator final {
        N itr_start_, itr_step_;
        __detail::types::Size itr_cnt_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = N;
        using difference_type   = value_type;
        using pointer           = void;
        using reference         = value_type;

        constexpr iterator( N startpoint, N step, __detail::types::Size iterated = 0 ) noexcept
          : itr_start_ { startpoint }, itr_step_ { step }, itr_cnt_ { iterated }
        {}

        constexpr iterator() noexcept : iterator( {}, {}, {} ) {}
        ~iterator() noexcept = default;

        // In C++11 this `constexpr` context is inferred to be `const`,
        // but it should obviously be writable
        __PGBAR_INLINE_FN __PGBAR_RELAXED_CONSTEXPR_FN iterator& operator++() noexcept
        {
          ++itr_cnt_;
          return *this;
        }
        __PGBAR_INLINE_FN __PGBAR_RELAXED_CONSTEXPR_FN iterator operator++( int ) noexcept
        {
          auto before = *this;
          operator++();
          return before;
        }
        __PGBAR_INLINE_FN __PGBAR_RELAXED_CONSTEXPR_FN iterator& operator+=(
          value_type increment ) noexcept
        {
          itr_cnt_ +=
            increment > 0 ? static_cast<__detail::types::Size>( increment / itr_step_ ) : 0;
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr reference operator*() const noexcept
        {
          return itr_start_ + itr_cnt_ * itr_step_;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==(
          value_type num ) const noexcept
        {
          return operator*() == num;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=(
          value_type num ) const noexcept
        {
          return !operator==( num );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator==(
          const iterator& lhs ) const noexcept
        {
          return itr_start_ == lhs.itr_start_ && itr_step_ == lhs.itr_step_
              && itr_cnt_ == lhs.itr_cnt_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr bool operator!=(
          const iterator& lhs ) const noexcept
        {
          return !operator==( lhs );
        }
      };

      constexpr NumericSpan() noexcept : start_ {}, end_ {}, step_ { 1 } {}
      NumericSpan( N startpoint, N endpoint, N step ) : NumericSpan()
      {
        __PGBAR_UNLIKELY if ( step > 0 && startpoint > endpoint ) throw exceptions::InvalidArgument(
          "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else __PGBAR_UNLIKELY if ( step < 0 && startpoint < endpoint ) throw exceptions::
          InvalidArgument( "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        __PGBAR_UNLIKELY if ( step
                              == 0 ) throw exceptions::InvalidArgument( "pgbar: 'step' is zero" );

        start_ = startpoint;
        step_  = step;
        end_   = endpoint;
      }
      NumericSpan( N startpoint, N endpoint ) : NumericSpan( startpoint, endpoint, 1 ) {}
      explicit NumericSpan( N endpoint ) : NumericSpan( {}, endpoint, 1 ) {}
      virtual ~NumericSpan() noexcept = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator begin() const noexcept
      {
        return iterator( start_, step_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN constexpr iterator end() const noexcept
      {
        return iterator( start_, step_, size() );
      }

      NumericSpan& step( N step )
      {
        __PGBAR_UNLIKELY if ( step < 0 && start_ < end_ ) throw exceptions::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else __PGBAR_UNLIKELY if ( step > 0 && start_ > end_ ) throw exceptions::InvalidArgument(
          "pgbar: 'end' is less than 'start' while 'step' is positive" );
        else __PGBAR_UNLIKELY if ( step == 0 ) throw exceptions::InvalidArgument(
          "pgbar: 'step' is zero" );

        step_ = step;
        return *this;
      }
      NumericSpan& start_value( N startpoint )
      {
        __PGBAR_UNLIKELY if ( step_ < 0 && startpoint < end_ ) throw exceptions::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else __PGBAR_UNLIKELY if ( step_ > 0 && startpoint > end_ ) throw exceptions::
          InvalidArgument( "pgbar: 'end' is less than 'start' while 'step' is positive" );

        start_ = startpoint;
        return *this;
      }
      NumericSpan& end_value( N endpoint )
      {
        __PGBAR_UNLIKELY if ( step_ < 0 && start_ < endpoint ) throw exceptions::InvalidArgument(
          "pgbar: 'end' is greater than 'start' while 'step' is negative" );
        else __PGBAR_UNLIKELY if ( step_ > 0 && start_ > endpoint ) throw exceptions::
          InvalidArgument( "pgbar: 'end' is less than 'start' while 'step' is positive" );

        end_ = endpoint;
        return *this;
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN N start_value() const noexcept { return start_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN N end_value() const noexcept { return end_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN N step() const noexcept { return step_; }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN __detail::types::Size size() const noexcept
      {
        if __PGBAR_CONSTEXPR_IF ( std::is_integral<N>::value )
          return ( ( end_ - start_ + step_ ) - 1 ) / step_;
        else
          return static_cast<__detail::types::Size>( std::ceil( ( end_ - start_ ) / step_ ) );
      }

      void swap( NumericSpan<N>& lhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        std::swap( start_, lhs.start_ );
        std::swap( end_, lhs.end_ );
        std::swap( step_, lhs.step_ );
      }
      friend void swap( NumericSpan<N>& a, NumericSpan<N>& b ) noexcept { a.swap( b ); }
    };

    /**
     * An undirectional range delimited by a pair of iterators, including pointer types.
     *
     * When the type of iterator is pointer, it can figure out whether the iterator is reversed,
     * and "increments" it normally.
     *
     * Accepted iterator types must be able to obtain a `difference_type`.
     */
    template<typename I>
    class IterSpan : public __detail::IterSpanBase<I> {
      static_assert( !std::is_pointer<I>::value,
                     "pgbar::iterators::IterSpan<I>: Only available for iterator types" );

    public:
      class iterator final {
        I current_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = typename std::iterator_traits<I>::value_type;
        using difference_type   = void;
        using pointer           = typename std::iterator_traits<I>::pointer;
        using reference         = typename std::iterator_traits<I>::reference;

        explicit iterator( I startpoint, const I& = I {} )
          noexcept( std::is_nothrow_move_constructible<I>::value )
          : current_ { std::move( startpoint ) }
        {}
        ~iterator() noexcept( std::is_nothrow_destructible<I>::value ) = default;

        __PGBAR_INLINE_FN iterator& operator++()
        {
          ++current_;
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator operator++( int )
        {
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN reference operator*() noexcept { return *current_; }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN pointer operator->() noexcept
        {
          return std::addressof( current_ );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator==( const iterator& lhs ) const noexcept
        {
          return current_ == lhs.current_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator!=( const iterator& lhs ) const noexcept
        {
          return !operator==( lhs );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator==( const I& lhs ) const noexcept
        {
          return current_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator!=( const I& lhs ) const noexcept
        {
          return !operator==( lhs );
        }
      };

      using __detail::IterSpanBase<I>::IterSpanBase;
      virtual ~IterSpan() noexcept( std::is_nothrow_destructible<I>::value ) = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator begin() const
        noexcept( std::is_nothrow_move_constructible<I>::value
                  && std::is_nothrow_copy_constructible<I>::value )
      {
        return iterator( this->start_, this->end_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator end() const
        noexcept( std::is_nothrow_move_constructible<I>::value
                  && std::is_nothrow_copy_constructible<I>::value )
      {
        return iterator( this->end_, this->end_ );
      }
    };
    template<typename P>
    class IterSpan<P*> : public __detail::IterSpanBase<P*> {
      static_assert( std::is_pointer<P*>::value,
                     "pgbar::iterators::IterSpan<P*>: Only available for pointer types" );

    public:
      class iterator final {
      public:
        P* current_;
        bool reversed_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = P;
        using difference_type   = void;
        using pointer           = P*;
        using reference         = typename std::add_lvalue_reference<value_type>::type;

        iterator( P* startpoint, P* endpoint ) noexcept
          : current_ { startpoint }, reversed_ { false }
        {
          __PGBAR_ASSERT( startpoint != nullptr );
          __PGBAR_ASSERT( endpoint != nullptr );
          reversed_ = endpoint < startpoint;
        }
        ~iterator() noexcept = default;

        __PGBAR_INLINE_FN iterator& operator++() noexcept
        {
          if ( reversed_ )
            --current_;
          else
            ++current_;
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator operator++( int ) noexcept
        {
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN reference operator*() noexcept { return *current_; }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN pointer operator->() noexcept
        {
          return std::addressof( current_ );
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator==( const iterator& lhs ) const noexcept
        {
          return current_ == lhs.current_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator!=( const iterator& lhs ) const noexcept
        {
          return !operator==( lhs );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator==( const P* lhs ) const noexcept
        {
          return current_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator!=( const P* lhs ) const noexcept
        {
          return !operator==( lhs );
        }
      };

      IterSpan( P* startpoint, P* endpoint ) : __detail::IterSpanBase<P*>( startpoint, endpoint )
      {
        __PGBAR_UNLIKELY if ( startpoint == nullptr || endpoint == nullptr ) throw exceptions::
          InvalidArgument( "pgbar: null pointer cannot generate a range" );
      }
      virtual ~IterSpan() noexcept = default;

      __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator begin() const noexcept
      {
        return iterator( this->start_, this->end_ );
      }
      __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator end() const noexcept
      {
        return iterator( this->end_, this->end_ );
      }
    };

    template<typename, typename>
    class ProxySpan;
  } // namespace iterators

  namespace __detail {
    namespace traits {
      template<typename T>
      struct is_arith_range {
      private:
        template<typename N>
        static std::true_type check( const iterators::NumericSpan<N>& );
        static std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check( std::declval<T>() ) )::value;
      };

      template<typename T>
      struct is_iter_range {
      private:
        template<typename I>
        static std::true_type check( const iterators::IterSpan<I>& );
        static std::false_type check( ... );

      public:
        static constexpr bool value = decltype( check( std::declval<T>() ) )::value;
      };
    } // namespace traits
  } // namespace __detail

  namespace __detail {
    types::String rgb2ansi( types::HexRGB rgb )
    {
# ifdef PGBAR_COLORLESS
      return {};
# else
      switch ( rgb ) {
      case __PGBAR_DEFAULT: return "\x1B[0m";
      case __PGBAR_BOLD:    return "\x1B[1m";
      default:              rgb &= 0x00FFFFFF; // discard the high 8 bits
      }

      switch ( rgb ) {
      case __PGBAR_BLACK:   return "\x1B[30m";
      case __PGBAR_RED:     return "\x1B[31m";
      case __PGBAR_GREEN:   return "\x1B[32m";
      case __PGBAR_YELLOW:  return "\x1B[33m";
      case __PGBAR_BLUE:    return "\x1B[34m";
      case __PGBAR_MAGENTA: return "\x1B[35m";
      case __PGBAR_CYAN:    return "\x1B[36m";
      case __PGBAR_WHITE:   return "\x1B[37m";
      default:
#  if __PGBAR_CXX20
        return std::format( "\x1B[38;2;{};{};{}m",
                            ( rgb >> 16 ) & 0xFF,
                            ( rgb >> 8 ) & 0xFF,
                            rgb & 0xFF );
#  else
        return types::String( "\x1B[38;2;" )
          .append( std::to_string( ( rgb >> 16 ) & 0xFF ) )
          .append( ";" )
          .append( std::to_string( ( rgb >> 8 ) & 0xFF ) )
          .append( ";" )
          .append( std::to_string( rgb & 0xFF ) )
          .append( "m" );
#  endif
      }
# endif
    }

    types::HexRGB hex2rgb( types::ROStr hex )
    {
      if ( ( hex.size() != 7 && hex.size() != 4 ) || hex.front() != '#' )
        throw exceptions::InvalidArgument( "pgbar: invalid hex color format" );

      for ( std::size_t i = 1; i < hex.size(); i++ ) {
        if ( ( hex[i] < '0' || hex[i] > '9' ) && ( hex[i] < 'A' || hex[i] > 'F' )
             && ( hex[i] < 'a' || hex[i] > 'f' ) )
          throw exceptions::InvalidArgument( "pgbar: invalid hexadecimal letter" );
      }

# ifdef PGBAR_COLORLESS
      return {};
# else
      std::uint32_t ret = 0;
      if ( hex.size() == 4 ) {
        for ( types::Size i = 1; i < hex.size(); ++i ) {
          ret <<= 4;
          if ( hex[i] >= '0' && hex[i] <= '9' )
            ret = ( ( ret | ( hex[i] - '0' ) ) << 4 ) | ( hex[i] - '0' );
          else if ( hex[i] >= 'A' && hex[i] <= 'F' )
            ret = ( ( ret | ( hex[i] - 'A' + 10 ) ) << 4 ) | ( hex[i] - 'A' + 10 );
          else // no need to check whether it's valid or not
            ret = ( ( ret | ( hex[i] - 'a' + 10 ) ) << 4 ) | ( hex[i] - 'a' + 10 );
        }
      } else {
        for ( types::Size i = 1; i < hex.size(); ++i ) {
          ret <<= 4;
          if ( hex[i] >= '0' && hex[i] <= '9' )
            ret |= hex[i] - '0';
          else if ( hex[i] >= 'A' && hex[i] <= 'F' )
            ret |= hex[i] - 'A' + 10;
          else
            ret |= hex[i] - 'a' + 10;
        }
      }
      return ret;
# endif
    }

    enum class TxtLayout { left, right, center }; // text layout
    /**
     * Format the `str`.
     *
     * @tparam Style Format mode.
     *
     * @param width Target length, do nothing if `width` less than the length of `str`.
     *
     * @param str The string will be formatted.
     */
    template<TxtLayout Style>
    __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String formatting( types::Size width,
                                                                  types::ROStr str )
    {
      __PGBAR_UNLIKELY if ( width == 0 ) return {};
      __PGBAR_UNLIKELY if ( str.size() >= width ) return types::String( str );
# if __PGBAR_CXX20
      if __PGBAR_CONSTEXPR_IF ( Style == TxtLayout::right )
        return std::format( "{:>{}}", str, width );
      else if __PGBAR_CONSTEXPR_IF ( Style == TxtLayout::left )
        return std::format( "{:<{}}", str, width );
      else
        return std::format( "{:^{}}", str, width );
# else
      if __PGBAR_CONSTEXPR_IF ( Style == TxtLayout::right )
        return types::String( width - str.size(), constants::blank ).append( str );
      else if __PGBAR_CONSTEXPR_IF ( Style == TxtLayout::left )
        return types::String( str ) + types::String( width - str.size(), constants::blank );
      else {
        width -= str.size();
        const types::Size l_blank = width / 2;
        return types::String( l_blank, constants::blank ) + types::String( str )
             + types::String( width - l_blank, constants::blank );
      }
# endif
    }

    /**
     * Determine whether the program is running in the tty based on the platform api.
     */
    __PGBAR_NODISCARD __PGBAR_INLINE_FN bool intty()
    {
# if defined( PGBAR_INTTY ) || __PGBAR_UNKNOWN
      return true;
# else
      return isatty( fileno( stdout ) ) && isatty( fileno( stderr ) );
# endif
    }

    /**
     * A dynamic string buffer is provided for string concatenation to reduce heap allocations.
     *
     * The core thoughts is based on `std::string::clear` does not clear the allocated memory block.
     */
    class StringBuffer final {
      using self = StringBuffer;

      types::String buffer_;

    public:
      StringBuffer() noexcept = default;

      StringBuffer( const self& lhs ) { operator=( lhs ); }
      StringBuffer( self&& rhs ) noexcept : StringBuffer() { swap( rhs ); }
      self& operator=( const self& lhs ) &
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        buffer_ = lhs.buffer_;
        return *this;
      }
      self& operator=( self&& rhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( rhs ) );
        swap( rhs );
        return *this;
      }

      __PGBAR_INLINE_FN bool empty() const noexcept { return buffer_.empty(); }
      __PGBAR_INLINE_FN types::String& data() noexcept { return buffer_; }
      __PGBAR_INLINE_FN self& append( types::Size num, types::Char ch )
      {
        buffer_.append( num, ch );
        return *this;
      }
      __PGBAR_INLINE_FN self& reserve( types::Size size ) &
      {
        buffer_.reserve( size );
        return *this;
      }
      __PGBAR_INLINE_FN void clear() & noexcept { buffer_.clear(); }
      /**
       * Release the buffer space completely
       */
      __PGBAR_INLINE_FN void release() & noexcept
      {
        clear();
        buffer_.shrink_to_fit();
      }

      void swap( StringBuffer& lhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        buffer_.swap( lhs.buffer_ );
      }
      friend void swap( StringBuffer& a, StringBuffer& b ) noexcept { a.swap( b ); }

      template<typename T>
      __PGBAR_INLINE_FN typename std::enable_if<
        std::is_same<typename std::decay<T>::type, types::String>::value
          || std::is_same<typename std::decay<T>::type, types::ROStr>::value
          || std::is_same<typename std::decay<T>::type, types::ConstStr>::value
          || std::is_same<typename std::decay<T>::type, const types::Char*>::value,
        self&>::type
        append( T&& info )
      {
        buffer_.append( std::forward<T>( info ) );
        return *this;
      }

      template<typename T>
      __PGBAR_INLINE_FN typename std::enable_if<
        std::is_same<typename std::decay<T>::type, types::String>::value
          || std::is_same<typename std::decay<T>::type, types::ROStr>::value
          || std::is_same<typename std::decay<T>::type, types::ConstStr>::value
          || std::is_same<typename std::decay<T>::type, const types::Char*>::value,
        self&>::type
        append( types::Size num, T&& info )
      {
        for ( types::Size _ = 0; _ < num; ++_ )
          buffer_.append( info );
        return *this;
      }

      template<typename T>
      __PGBAR_INLINE_FN typename std::enable_if<
        std::is_same<typename std::decay<T>::type, types::String>::value
          || std::is_same<typename std::decay<T>::type, types::ROStr>::value
          || std::is_same<typename std::decay<T>::type, types::ConstStr>::value
          || std::is_same<typename std::decay<T>::type, const types::Char*>::value,
        self&>::type
        operator<<( T&& info )
      {
        buffer_.append( std::forward<T>( info ) );
        return *this;
      }
      __PGBAR_INLINE_FN self& operator<<( types::Char character )
      {
        buffer_.push_back( character );
        return *this;
      }
      template<typename S>
      friend __PGBAR_INLINE_FN S& operator<<( S& stream, StringBuffer& buf )
      { // hidden friend
        stream << buf.data();
        buf.clear();
        return stream;
      }
    };

    namespace concurrency {
      /**
       * A simple `Shared Mutex` implementation for any C++ version.
       */
      class SharedMutex {
        using self = SharedMutex;

      protected:
        std::atomic<types::Size> num_readers_;
        std::atomic<bool> lock_stat_;
        std::mutex writer_mtx_;

      public:
        SharedMutex( const self& )     = delete;
        self& operator=( const self& ) = delete;

        SharedMutex() noexcept : num_readers_ { 0 }, lock_stat_ { false } {}
        virtual ~SharedMutex() noexcept = default;

        void lock() & noexcept
        {
          while ( true ) {
            while ( num_readers_.load( std::memory_order_acquire ) != 0 )
              std::this_thread::yield();

            writer_mtx_.lock();
            if ( num_readers_.load( std::memory_order_acquire ) == 0 ) {
              lock_stat_.store( true, std::memory_order_release );
              break;
            } else // unlock it and wait for readers to finish
              writer_mtx_.unlock();
          }
        }
        __PGBAR_NODISCARD bool try_lock() & noexcept
        {
          if ( num_readers_.load( std::memory_order_acquire ) == 0 && writer_mtx_.try_lock() ) {
            lock_stat_.store( true, std::memory_order_release );
            return true;
          }
          return false;
        }
        void unlock() & noexcept
        {
          writer_mtx_.unlock();
          lock_stat_.store( false, std::memory_order_release );
        }

        void lock_shared() & noexcept
        {
          writer_mtx_.lock();

          num_readers_.fetch_add( 1, std::memory_order_release );
          __PGBAR_ASSERT( num_readers_ > 0 ); // overflow checking

          writer_mtx_.unlock();
        }
        __PGBAR_NODISCARD bool try_lock_shared() & noexcept
        {
          if ( !lock_stat_.load( std::memory_order_acquire ) && writer_mtx_.try_lock() ) {
            if ( lock_stat_.load( std::memory_order_acquire ) ) {
              writer_mtx_.unlock();
              return false;
            }

            num_readers_.fetch_add( 1, std::memory_order_release );
            __PGBAR_ASSERT( num_readers_ > 0 );
            writer_mtx_.unlock();
            return true;
          }
          return false;
        }
        void unlock_shared() & noexcept
        {
          __PGBAR_ASSERT( num_readers_ > 0 ); // underflow checking
          num_readers_.fetch_sub( 1, std::memory_order_release );
        }
      };
      class SharedMutexRef final {
        SharedMutex& mtx_;

      public:
        SharedMutexRef( SharedMutex& mtx ) : mtx_ { mtx } {}
        ~SharedMutexRef() noexcept = default;

        __PGBAR_INLINE_FN void lock() & noexcept { mtx_.lock_shared(); }
        __PGBAR_INLINE_FN void unlock() & noexcept { mtx_.unlock_shared(); }
      };

      /**
       * A pipe that transmits exceptions between different threads.
       */
      class ExceptionPipe final {
        using self = ExceptionPipe;

        std::queue<std::exception_ptr> exceptions_;
        mutable SharedMutex mtx_;

      public:
        ExceptionPipe()
          noexcept( std::is_nothrow_default_constructible<SharedMutex>::value )       = default;
        ~ExceptionPipe() noexcept( std::is_nothrow_destructible<SharedMutex>::value ) = default;

        ExceptionPipe( const ExceptionPipe& lhs )
          noexcept( std::is_nothrow_default_constructible<SharedMutex>::value )
          : exceptions_ { lhs.exceptions_ }
        {}
        ExceptionPipe( ExceptionPipe&& rhs )
          noexcept( std::is_nothrow_default_constructible<SharedMutex>::value )
          : ExceptionPipe()
        {
          swap( rhs );
        }
        ExceptionPipe& operator=( const ExceptionPipe& lhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( lhs ) );
          exceptions_ = lhs.exceptions_;
          return *this;
        }
        ExceptionPipe& operator=( ExceptionPipe&& rhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( rhs ) );
          swap( rhs );
          return *this;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN std::size_t size() const
        {
          auto shared_end = SharedMutexRef( mtx_ );
          std::lock_guard<SharedMutexRef> lock { shared_end };
          return exceptions_.size();
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool empty() const
        {
          auto shared_end = SharedMutexRef( mtx_ );
          std::lock_guard<SharedMutexRef> lock { shared_end };
          return exceptions_.empty();
        }

        __PGBAR_INLINE_FN self& push( std::exception_ptr e ) &
        {
          std::lock_guard<SharedMutex> lock { mtx_ };
          exceptions_.push( e );
          return *this;
        }
        __PGBAR_INLINE_FN self& pop() &
        {
          std::lock_guard<SharedMutex> lock { mtx_ };
          exceptions_.pop();
          return *this;
        }
        __PGBAR_INLINE_FN std::exception_ptr front()
        {
          auto shared_end = SharedMutexRef( mtx_ );
          std::lock_guard<SharedMutexRef> lock { shared_end };
          return exceptions_.front();
        }

        /**
         * Pop up the head of the queue element and throw it as an exception.
         */
        __PGBAR_INLINE_FN void pop_throw() &
        {
          __PGBAR_ASSERT( !empty() );
          auto exception_ptr = front();
          pop();
          if ( exception_ptr )
            std::rethrow_exception( exception_ptr );
        }

        void swap( ExceptionPipe& lhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( lhs ) );
          exceptions_.swap( lhs.exceptions_ );
        }
        friend void swap( ExceptionPipe& a, ExceptionPipe& b ) noexcept { a.swap( b ); }
      };
    } // namespace concurrency

    namespace wrappers {
      // Used for type erasure
      struct RenderFn {
        virtual ~RenderFn() noexcept = default;
        virtual void run()           = 0;
      };
      template<typename F>
      class RenderFnWrapper final : public RenderFn {
        static_assert( traits::is_void_functor<F>::value,
                       "pgbar::__detail::wrappers::RenderFnWrapper: Invalid template type" );

        F fntor_;

      public:
        explicit RenderFnWrapper( F fn ) : fntor_ { std::move( fn ) } {}
        ~RenderFnWrapper() noexcept = default;
        virtual void run() { fntor_(); }
      };

      template<typename T>
      class GenericWrapper {
      protected:
        using BaseSelf = GenericWrapper<T>;

        T data_;

      public:
        constexpr explicit GenericWrapper( T data ) : data_ { std::move( data ) } {}
        virtual ~GenericWrapper() noexcept = 0;
        __PGBAR_INLINE_FN T& value() noexcept { return data_; }

        void swap( GenericWrapper<T>& lhs ) & noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( lhs ) );
          using std::swap;
          swap( data_, lhs.data_ );
        }
        friend void swap( GenericWrapper<T>& a, GenericWrapper<T>& b ) noexcept { a.swap( b ); }
      };
      template<typename T>
      GenericWrapper<T>::~GenericWrapper() noexcept = default;
    } // namespace wrappers
  } // namespace __detail

  namespace colors {
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB None    = __PGBAR_DEFAULT;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Black   = __PGBAR_BLACK;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Red     = __PGBAR_RED;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Green   = __PGBAR_GREEN;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Yellow  = __PGBAR_YELLOW;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Blue    = __PGBAR_BLUE;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Magenta = __PGBAR_MAGENTA;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB Cyan    = __PGBAR_CYAN;
    __PGBAR_INLINE_VAR constexpr __detail::types::HexRGB White   = __PGBAR_WHITE;
  } // namespace colors

  namespace options {
# define __PGBAR_OPTIONS( StructName, ValueType )                                   \
   struct StructName final : public __detail::wrappers::GenericWrapper<ValueType> { \
     using BaseSelf::GenericWrapper;                                                \
   };
    __PGBAR_OPTIONS( Colored, bool )
    __PGBAR_OPTIONS( Bolded, bool )
    __PGBAR_OPTIONS( Styles, __detail::types::BitwiseSet )
    __PGBAR_OPTIONS( TodoChar, __detail::types::String )
    __PGBAR_OPTIONS( DoneChar, __detail::types::String )
    __PGBAR_OPTIONS( StartPoint, __detail::types::String )
    __PGBAR_OPTIONS( EndPoint, __detail::types::String )
    __PGBAR_OPTIONS( LeftStatus, __detail::types::String )
    __PGBAR_OPTIONS( RightStatus, __detail::types::String )
    __PGBAR_OPTIONS( Divider, __detail::types::String )
    __PGBAR_OPTIONS( Tasks, __detail::types::Size )
    __PGBAR_OPTIONS( BarLength, __detail::types::Size )

    __PGBAR_OPTIONS( Suffix, __detail::types::String )
    __PGBAR_OPTIONS( TrueFrame, __detail::types::String )
    __PGBAR_OPTIONS( FalseFrame, __detail::types::String )
# undef __PGBAR_OPTIONS
# define __PGBAR_OPTIONS( StructName )                                                            \
   struct StructName final : public __detail::wrappers::GenericWrapper<__detail::types::String> { \
     StructName( __detail::types::String val )                                                    \
       : __detail::wrappers::GenericWrapper<__detail::types::String>(                             \
           __detail::rgb2ansi( __detail::hex2rgb( std::move( val ) ) ) )                          \
     {}                                                                                           \
     StructName( __detail::types::HexRGB val )                                                    \
       : __detail::wrappers::GenericWrapper<__detail::types::String>( __detail::rgb2ansi( val ) ) \
     {}                                                                                           \
   };
    __PGBAR_OPTIONS( TodoColor )
    __PGBAR_OPTIONS( DoneColor )
    __PGBAR_OPTIONS( StatusColor )

    __PGBAR_OPTIONS( FramesColor )
    __PGBAR_OPTIONS( TrueColor )
    __PGBAR_OPTIONS( FalseColor )
# undef __PGBAR_OPTIONS
    struct Frames final
      : public __detail::wrappers::GenericWrapper<std::vector<__detail::types::String>> {
      Frames( std::vector<__detail::types::String> val )
        : __detail::wrappers::GenericWrapper<std::vector<__detail::types::String>>( {} )
      { // Here we need to throw an exception sometimes, so we cannot generate codes with macro.
        __PGBAR_UNLIKELY if ( val.empty() ) throw exceptions::InvalidArgument(
          "pgbar: the frames are empty" );
        data_ = std::move( val );
      }
    };
  } // namespace options

  namespace __detail {
    namespace traits {
      template<typename... Ts>
      using all_of_progress = all_in_tuple<std::tuple<options::Colored,
                                                      options::Bolded,
                                                      options::Styles,
                                                      options::TodoChar,
                                                      options::DoneChar,
                                                      options::StartPoint,
                                                      options::EndPoint,
                                                      options::LeftStatus,
                                                      options::RightStatus,
                                                      options::Divider,
                                                      options::Tasks,
                                                      options::BarLength,
                                                      options::TodoColor,
                                                      options::DoneColor,
                                                      options::StatusColor>,
                                           Ts...>;

      template<typename... Ts>
      using all_of_spinner = all_in_tuple<std::tuple<options::Colored,
                                                     options::Bolded,
                                                     options::FramesColor,
                                                     options::TrueColor,
                                                     options::FalseColor,
                                                     options::Frames,
                                                     options::Suffix,
                                                     options::TrueFrame,
                                                     options::FalseFrame>,
                                          Ts...>;
    }
  }

  namespace configs {
    class Global {
      static const bool _in_tty;

      static __detail::types::TimeUnit _refresh_interval;
      static __detail::concurrency::SharedMutex _rw_mtx;

    protected:
      enum FrontOption : __detail::types::Size { color = 0, bold };
      std::bitset<2> front_options_;

      mutable __detail::concurrency::SharedMutex rw_mtx_;

    public:
      __PGBAR_INLINE_FN static const __detail::types::TimeUnit& refresh_interval()
      {
        auto shared_end = __detail::concurrency::SharedMutexRef( _rw_mtx );
        std::lock_guard<__detail::concurrency::SharedMutexRef> lock { shared_end };
        return _refresh_interval;
      }
      static void refresh_interval( std::chrono::nanoseconds new_rate )
      {
        //__PGBAR_ASSERT( new_rate != std::chrono::nanoseconds( 0 ) );
        std::lock_guard<__detail::concurrency::SharedMutex> lock { _rw_mtx };
        _refresh_interval = std::move( new_rate );
      }
      __PGBAR_NODISCARD static bool intty() noexcept { return _in_tty; }

      Global() : front_options_ {} { front_options_.set(); }
      virtual ~Global() noexcept = default;

      Global( const Global& lhs ) { operator=( lhs ); }
      Global( Global&& rhs ) noexcept { operator=( std::move( rhs ) ); }
      Global& operator=( const Global& lhs )
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        front_options_ = lhs.front_options_;
        return *this;
      }
      Global& operator=( Global&& rhs ) noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( rhs ) );
        front_options_ = rhs.front_options_;
        return *this;
      }

      virtual Global& colored( bool ) & = 0;
      virtual Global& bolded( bool ) &  = 0;

      __PGBAR_NODISCARD bool colored() const noexcept { return front_options_[FrontOption::color]; }
      __PGBAR_NODISCARD bool bolded() const noexcept { return front_options_[FrontOption::bold]; }
    };
    typename __detail::types::TimeUnit Global::_refresh_interval =
      std::chrono::duration_cast<__detail::types::TimeUnit>( std::chrono::milliseconds( 25 ) );
    __detail::concurrency::SharedMutex Global::_rw_mtx {};
    const bool Global::_in_tty = __detail::intty();

    class Progress : public Global {
      friend void unpacking( Progress& cfg ) {}
# define __PGBAR_UNPAKING( OptionName, MemberName, Operation )                     \
   template<typename... Args>                                                      \
   friend void unpacking( Progress& cfg, options::OptionName val, Args&&... args ) \
   {                                                                               \
     cfg.MemberName Operation( std::move( val.value() ) );                         \
     unpacking( cfg, std::forward<Args>( args )... );                              \
   }
      __PGBAR_UNPAKING( Colored, front_options_[FrontOption::color], = )
      __PGBAR_UNPAKING( Bolded, front_options_[FrontOption::bold], = )
      __PGBAR_UNPAKING( Styles, visibilities_, = )
      __PGBAR_UNPAKING( TodoColor, todo_col_, = )
      __PGBAR_UNPAKING( DoneColor, done_col_, = )
      __PGBAR_UNPAKING( StatusColor, status_col_, = )
      __PGBAR_UNPAKING( TodoChar, todo_ch_, = )
      __PGBAR_UNPAKING( DoneChar, done_ch_, = )
      __PGBAR_UNPAKING( StartPoint, startpoint_, = )
      __PGBAR_UNPAKING( EndPoint, endpoint_, = )
      __PGBAR_UNPAKING( LeftStatus, lstatus_, = )
      __PGBAR_UNPAKING( RightStatus, rstatus_, = )
      __PGBAR_UNPAKING( Divider, divider_, = )
      __PGBAR_UNPAKING( BarLength, bar_length_, = )
      __PGBAR_UNPAKING( Tasks, task_range_.end_value, )
# undef __PGBAR_UNPAKING

    protected:
# define __PGBAR_DEFAULT_RATIO " 0.00% "
# define __PGBAR_DEFAULT_TIMER "00:00:00 < --:--:--"
# define __PGBAR_DEFAULT_RATE  "   inf Hz "

      static constexpr __detail::types::Size _ratio_len = sizeof( __PGBAR_DEFAULT_RATIO ) - 1;
      static constexpr __detail::types::Size _timer_len = sizeof( __PGBAR_DEFAULT_TIMER ) - 1;
      static constexpr __detail::types::Size _rate_len  = sizeof( __PGBAR_DEFAULT_RATE ) - 1;

      enum BitIndex : __detail::types::Size { bar = 0, per, cnt, rate, timer };
      using BitVector = std::bitset<sizeof( __detail::types::BitwiseSet ) * 8>;

      BitVector visibilities_;
      __detail::types::String todo_col_, done_col_;
      __detail::types::String status_col_;
      __detail::types::String todo_ch_, done_ch_;
      __detail::types::String startpoint_, endpoint_;
      __detail::types::String lstatus_, rstatus_;
      __detail::types::String divider_;
      __detail::types::Size bar_length_; // The length of the progress bar.

      iterators::NumericSpan<__detail::types::Size> task_range_;

    public:
      static constexpr __detail::types::BitwiseSet Bar     = 1 << 0;
      static constexpr __detail::types::BitwiseSet Ratio   = 1 << 1;
      static constexpr __detail::types::BitwiseSet TaskCnt = 1 << 2;
      static constexpr __detail::types::BitwiseSet Rate    = 1 << 3;
      static constexpr __detail::types::BitwiseSet Timer   = 1 << 4;
      static constexpr __detail::types::BitwiseSet Entire  = ~0;

      Progress( __detail::types::Size num_tasks )
        : visibilities_ { Entire }
        , todo_col_ {}
        , done_col_ {}
        , status_col_ { __detail::rgb2ansi( colors::Cyan ) }
        , bar_length_ { 30 }
        , task_range_ {}
      {
        todo_ch_    = __detail::types::String( 1, __detail::constants::blank );
        done_ch_    = __detail::types::String( 1, '-' );
        startpoint_ = __detail::types::String( 1, '[' );
        endpoint_   = __detail::types::String( 1, ']' );
        lstatus_    = __detail::types::String( "[ " );
        rstatus_    = __detail::types::String( " ]" );
        divider_    = __detail::types::String( " | " );

        task_range_.end_value( num_tasks );
      }
      template<typename... Args,
               typename = typename std::enable_if<__detail::traits::all_of_progress<
                 typename std::decay<Args>::type...>::value>::type>
      Progress( Args&&... args ) : Progress( 0 )
      {
        // trigger ADL
        unpacking( *this, std::forward<Args>( args )... );
      }

      Progress( const Progress& lhs ) { operator=( lhs ); }
      Progress( Progress&& rhs ) noexcept : Progress( 0 ) { swap( rhs ); }
      Progress& operator=( const Progress& lhs ) &
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        // WRANING: self-assignment will immediately and definitely result in deadlock
        std::lock_guard<__detail::concurrency::SharedMutex> lock1 { rw_mtx_ };
        auto shared_end = __detail::concurrency::SharedMutexRef( lhs.rw_mtx_ );
        std::lock_guard<__detail::concurrency::SharedMutexRef> lock { shared_end };

        Global::operator=( lhs );
        visibilities_ = lhs.visibilities_;
        todo_col_     = lhs.todo_col_;
        done_col_     = lhs.done_col_;
        status_col_   = lhs.status_col_;
        todo_ch_      = lhs.todo_ch_;
        done_ch_      = lhs.done_ch_;
        startpoint_   = lhs.startpoint_;
        endpoint_     = lhs.endpoint_;
        lstatus_      = lhs.lstatus_;
        rstatus_      = lhs.rstatus_;
        divider_      = lhs.divider_;
        bar_length_   = lhs.bar_length_;
        task_range_   = lhs.task_range_;
        return *this;
      }
      Progress& operator=( Progress&& rhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( rhs ) );
        swap( rhs );
        return *this;
      }

      virtual ~Progress() noexcept = default;

      using Global::bolded;
      using Global::colored;

// automatically generate getter/setter
# define __PGBAR_METHOD( MethodName, ParamType, MemberName, Operator, Operation, Override ) \
   Progress& MethodName( ParamType param ) & Override                                       \
   {                                                                                        \
     std::lock_guard<__detail::concurrency::SharedMutex> lock { rw_mtx_ };                  \
     MemberName Operator Operation( std::move( param ) );                                   \
     return *this;                                                                          \
   }
      __PGBAR_METHOD( colored, bool, front_options_[FrontOption::color], =, , override )
      __PGBAR_METHOD( bolded, bool, front_options_[FrontOption::bold], =, , override )
      __PGBAR_METHOD( styles, __detail::types::BitwiseSet, visibilities_, =, , )
      __PGBAR_METHOD( todo, __detail::types::String, todo_ch_, =, , )
      __PGBAR_METHOD( done, __detail::types::String, done_ch_, =, , )
      __PGBAR_METHOD( startpoint, __detail::types::String, startpoint_, =, , )
      __PGBAR_METHOD( endpoint, __detail::types::String, endpoint_, =, , )
      __PGBAR_METHOD( lstatus, __detail::types::String, lstatus_, =, , )
      __PGBAR_METHOD( rstatus, __detail::types::String, rstatus_, =, , )
      __PGBAR_METHOD( divider, __detail::types::String, divider_, =, , )
      __PGBAR_METHOD( bar_length, __detail::types::Size, bar_length_, =, , )
      __PGBAR_METHOD( tasks, __detail::types::Size, task_range_, ., end_value, )
# undef __PGBAR_METHOD
# define __PGBAR_METHOD( MethodName, ParamType, MemberName, Operation1, Operation2 ) \
   Progress& MethodName( ParamType param )&                                          \
   {                                                                                 \
     std::lock_guard<__detail::concurrency::SharedMutex> lock { rw_mtx_ };           \
     MemberName = Operation1( Operation2( param ) );                                 \
     return *this;                                                                   \
   }
      __PGBAR_METHOD( todo_color, __detail::types::HexRGB, todo_col_, __detail::rgb2ansi, )
      __PGBAR_METHOD( done_color, __detail::types::HexRGB, done_col_, __detail::rgb2ansi, )
      __PGBAR_METHOD( status_color, __detail::types::HexRGB, done_col_, __detail::rgb2ansi, )
      __PGBAR_METHOD( todo_color,
                      __detail::types::String,
                      todo_col_,
                      __detail::rgb2ansi,
                      __detail::hex2rgb )
      __PGBAR_METHOD( done_color,
                      __detail::types::String,
                      done_col_,
                      __detail::rgb2ansi,
                      __detail::hex2rgb )
      __PGBAR_METHOD( status_color,
                      __detail::types::String,
                      status_col_,
                      __detail::rgb2ansi,
                      __detail::hex2rgb )
# undef __PGBAR_METHOD

      template<typename... Args>
      typename std::enable_if<
        __detail::traits::all_of_progress<typename std::decay<Args>::type...>::value,
        Progress&>::type
        set( Args&&... args ) &
      {
        std::lock_guard<__detail::concurrency::SharedMutex> lock { rw_mtx_ };
        unpacking( *this, std::forward<Args>( args )... );
        return *this;
      }

# define __PGBAR_METHOD( MethodName, MemberName, ReturnType, Operation )         \
   __PGBAR_NODISCARD ReturnType MethodName() const                               \
   {                                                                             \
     auto shared_end = __detail::concurrency::SharedMutexRef( rw_mtx_ );         \
     std::lock_guard<__detail::concurrency::SharedMutexRef> lock { shared_end }; \
     return Operation( MemberName );                                             \
   }
      __PGBAR_METHOD( tasks, task_range_.end_value(), __detail::types::Size, )
      __PGBAR_METHOD( bar_length, bar_length_, __detail::types::Size, )
# undef __PGBAR_METHOD

      /**
       * Return the number of characters excluding the bar in the whole progress bar.
       */
      __PGBAR_NODISCARD __detail::types::Size fixed_size() const noexcept
      {
        auto status_length =
          ( ( visibilities_[BitIndex::per] ? _ratio_len : 0 )
            + ( visibilities_[BitIndex::cnt]
                  ? ( task_range_.end_value() == 0 ? 1
                                                   : static_cast<__detail::types::Size>(
                                                       std::log10( task_range_.end_value() ) )
                                                       + 1 )
                        * 2
                      + 1
                  : 0 )
            + ( visibilities_[BitIndex::rate] ? _rate_len : 0 )
            + ( visibilities_[BitIndex::timer] ? _timer_len : 0 ) );
        if ( status_length != 0 ) {
          status_length += lstatus_.size() + rstatus_.size();
          const __detail::types::Size status_num =
            visibilities_[BitIndex::per] + visibilities_[BitIndex::cnt]
            + visibilities_[BitIndex::rate] + visibilities_[BitIndex::timer];
          status_length += ( status_num > 1 ) * ( status_num - 1 ) * divider_.size();
        }
        return /* The extra 1 is necessary */ 1 + status_length
             + ( visibilities_[BitIndex::bar] ? startpoint_.size() + endpoint_.size() + 1 : 0 );
      }

      void swap( Progress& lhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        std::lock_guard<__detail::concurrency::SharedMutex> lock1 { rw_mtx_ };
        std::lock_guard<__detail::concurrency::SharedMutex> lock2 { lhs.rw_mtx_ };

        std::swap( front_options_, lhs.front_options_ );
        std::swap( visibilities_, lhs.visibilities_ );
        todo_col_.swap( lhs.todo_col_ );
        done_col_.swap( lhs.done_col_ );
        status_col_.swap( lhs.status_col_ );
        todo_ch_.swap( lhs.todo_ch_ );
        done_ch_.swap( lhs.done_ch_ );
        startpoint_.swap( lhs.startpoint_ );
        endpoint_.swap( lhs.endpoint_ );
        lstatus_.swap( lhs.lstatus_ );
        rstatus_.swap( lhs.rstatus_ );
        divider_.swap( lhs.divider_ );
        std::swap( bar_length_, lhs.bar_length_ );
        task_range_.swap( lhs.task_range_ );
      }
      friend void swap( Progress& a, Progress& b ) noexcept { a.swap( b ); }
    };

    class Spinner : public Global {
      friend void unpacking( Spinner& cfg ) {}
# define __PGBAR_UNPAKING( OptionName, MemberName )                               \
   template<typename... Args>                                                     \
   friend void unpacking( Spinner& cfg, options::OptionName val, Args&&... args ) \
   {                                                                              \
     cfg.MemberName = std::move( val.value() );                                   \
     unpacking( cfg, std::forward<Args>( args )... );                             \
   }
      __PGBAR_UNPAKING( Colored, front_options_[FrontOption::color] )
      __PGBAR_UNPAKING( Bolded, front_options_[FrontOption::bold] )
      __PGBAR_UNPAKING( Frames, frames_ )
      __PGBAR_UNPAKING( FramesColor, frames_col_ )
      __PGBAR_UNPAKING( Suffix, suffix_ )
      __PGBAR_UNPAKING( TrueColor, true_col_ )
      __PGBAR_UNPAKING( FalseColor, false_col_ )
      __PGBAR_UNPAKING( TrueFrame, true_frame_ )
      __PGBAR_UNPAKING( FalseFrame, false_frame_ )
# undef __PGBAR_UNPAKING

    protected:
      __detail::types::String frames_col_;
      __detail::types::String true_col_;
      __detail::types::String false_col_;
      std::vector<__detail::types::String> frames_;
      __detail::types::String suffix_;
      __detail::types::String true_frame_;
      __detail::types::String false_frame_;

    public:
      Spinner( std::vector<__detail::types::String> frames )
      {
        __PGBAR_UNLIKELY if ( frames.empty() ) throw exceptions::InvalidArgument(
          "pgbar: the frames are empty" );
        frames_ = std::move( frames );
      }
      template<typename... Args,
               typename = typename std::enable_if<
                 __detail::traits::all_of_spinner<typename std::decay<Args>::type...>::value>::type>
      Spinner( Args&&... args )
        : Spinner( { "/", "/", "/", "/", "-", "-", "-", "-", "\\", "\\", "|", "|", "|", "|" } )
      {
        unpacking( *this, std::forward<Args>( args )... );
      }

      Spinner( const Spinner& lhs ) { operator=( lhs ); }
      Spinner( Spinner&& rhs ) noexcept { swap( rhs ); }
      Spinner& operator=( const Spinner& lhs ) &
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        std::lock_guard<__detail::concurrency::SharedMutex> lock1 { rw_mtx_ };
        auto shared_end = __detail::concurrency::SharedMutexRef( lhs.rw_mtx_ );
        std::lock_guard<__detail::concurrency::SharedMutexRef> lock2 { shared_end };

        Global::operator=( lhs );

        frames_col_  = lhs.frames_col_;
        true_col_    = lhs.true_col_;
        false_col_   = lhs.false_col_;
        frames_      = lhs.frames_;
        suffix_      = lhs.suffix_;
        true_frame_  = lhs.true_frame_;
        false_frame_ = lhs.false_frame_;
        return *this;
      }
      Spinner& operator=( Spinner&& rhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( rhs ) );
        swap( rhs );
        return *this;
      }

      virtual ~Spinner() noexcept = default;

      Spinner& frames( std::vector<__detail::types::String> frames ) &
      {
        __PGBAR_UNLIKELY if ( frames.empty() ) throw exceptions::InvalidArgument(
          "pgbar: the frames are empty" );
        std::lock_guard<__detail::concurrency::SharedMutex> lock { rw_mtx_ };
        frames_ = std::move( frames );
        return *this;
      }

      using Global::bolded;
      using Global::colored;

# define __PGBAR_METHOD( MethodName, ParamType, MemberName, Operation1, Operation2, Override ) \
   Spinner& MethodName( ParamType param )&                                                     \
   {                                                                                           \
     std::lock_guard<__detail::concurrency::SharedMutex> lock { rw_mtx_ };                     \
     MemberName = Operation1( Operation2( param ) );                                           \
     return *this;                                                                             \
   }
      __PGBAR_METHOD( colored, bool, front_options_[FrontOption::color], , , override )
      __PGBAR_METHOD( bolded, bool, front_options_[FrontOption::bold], , , override )
      __PGBAR_METHOD( suffix, __detail::types::String, suffix_, , , )
      __PGBAR_METHOD( true_frame, __detail::types::String, true_frame_, , , )
      __PGBAR_METHOD( false_frame, __detail::types::String, false_frame_, , , )
      __PGBAR_METHOD( frames_color, __detail::types::HexRGB, true_col_, __detail::rgb2ansi, , )
      __PGBAR_METHOD( true_color, __detail::types::HexRGB, true_col_, __detail::rgb2ansi, , )
      __PGBAR_METHOD( false_color, __detail::types::HexRGB, false_col_, __detail::rgb2ansi, , )
      __PGBAR_METHOD( frames_color,
                      __detail::types::String,
                      true_col_,
                      __detail::rgb2ansi,
                      __detail::hex2rgb, )
      __PGBAR_METHOD( true_color,
                      __detail::types::String,
                      true_col_,
                      __detail::rgb2ansi,
                      __detail::hex2rgb, )
      __PGBAR_METHOD( false_color,
                      __detail::types::String,
                      false_col_,
                      __detail::rgb2ansi,
                      __detail::hex2rgb, )
# undef __PGBAR_METHOD

      template<typename... Args>
      typename std::enable_if<
        __detail::traits::all_of_spinner<typename std::decay<Args>::type...>::value>::type
        set( Args&&... args ) &
      {
        std::lock_guard<__detail::concurrency::SharedMutex> lock { rw_mtx_ };
        unpacking( *this, std::forward<Args>( args )... );
      }

      virtual void swap( Spinner& lhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        std::lock_guard<__detail::concurrency::SharedMutex> lock1 { rw_mtx_ };
        std::lock_guard<__detail::concurrency::SharedMutex> lock2 { lhs.rw_mtx_ };

        std::swap( front_options_, lhs.front_options_ );
        frames_col_.swap( lhs.frames_col_ );
        true_col_.swap( lhs.true_col_ );
        false_col_.swap( lhs.false_col_ );
        frames_.swap( lhs.frames_ );
        suffix_.swap( lhs.suffix_ );
        true_frame_.swap( lhs.true_frame_ );
        false_frame_.swap( lhs.false_frame_ );
      }
      friend void swap( Spinner& a, Spinner& b ) noexcept { a.swap( b ); }
    };
  } // namespace configs

  namespace __detail {
    namespace render {
      template<typename>
      class Builder;

      template<>
      class Builder<configs::Progress> : public configs::Progress {
        __PGBAR_NODISCARD __PGBAR_INLINE_FN std::pair<types::Size, types::Size> produce_bar(
          types::Float num_per ) const noexcept
        {
          __PGBAR_ASSERT( num_per >= 0.0 );
          __PGBAR_ASSERT( num_per <= 1.0 );
          const types::Size done_len = std::round( bar_length_ * num_per );
          return std::make_pair( done_len, bar_length_ - done_len );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String produce_ratio(
          types::Float num_per ) const
        {
          __PGBAR_ASSERT( num_per >= 0.0 );
          __PGBAR_ASSERT( num_per <= 1.0 );
          // The minimum resolution is 0.01%
          __PGBAR_UNLIKELY if ( num_per < 0.01 ) return { __PGBAR_DEFAULT_RATIO };

          types::String proportion = std::to_string( num_per * 100.0 );
          proportion.resize( proportion.find( '.' ) + 3 );

          return formatting<TxtLayout::right>( _ratio_len,
                                               std::move( proportion ) + types::String( 1, '%' ) );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String produce_progress(
          types::Size num_done ) const
        {
          __PGBAR_ASSERT( num_done <= task_range_.end_value() );
          types::String total_str = std::to_string( task_range_.end_value() );
          const types::Size size  = total_str.size();
          return ( formatting<TxtLayout::right>( size, std::to_string( num_done ) )
                   + types::String( 1, '/' ) + std::move( total_str ) );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String produce_rate( types::TimeUnit time_passed,
                                                                        types::Size num_done ) const
        {
          __PGBAR_ASSERT( num_done <= task_range_.end_value() );
          __PGBAR_UNLIKELY if ( num_done == 0 ) return { __PGBAR_DEFAULT_RATE };

          const auto rate2str = []( types::Float val ) -> types::String {
            types::String str = std::to_string( val );
            str.resize( str.find( '.' ) + 3 ); // Keep two decimal places.
            return str;
          };

          const auto seconds_passed    = std::chrono::duration<types::Float>( time_passed ).count();
          // zero or negetive is invalid
          const types::Float frequency = seconds_passed <= 0.0
                                         ? ( std::numeric_limits<types::Float>::max )()
                                         : num_done / seconds_passed;
          types::String rate_str;
          if ( frequency < 1e3 ) // < 1Hz => '999.99 Hz'
            rate_str = rate2str( frequency ) + " Hz";
          else if ( frequency < 1e6 ) // < 1 kHz => '999.99 kHz'
            rate_str = rate2str( frequency / 1e3 ) + " kHz";
          else if ( frequency < 1e9 ) // < 1 MHz => '999.99 MHz'
            rate_str = rate2str( frequency / 1e6 ) + " MHz";
          else { // > 999 GHz => infinity
            const types::Float temp = frequency / 1e9;
            __PGBAR_UNLIKELY if ( temp > 999.99 ) // it's impossible I think
              rate_str    = __PGBAR_DEFAULT_RATE;
            else rate_str = rate2str( temp ) + types::String( " GHz" );
          }

          return formatting<TxtLayout::center>( _rate_len, std::move( rate_str ) );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN types::String produce_timer(
          types::TimeUnit time_passed,
          types::Size num_done ) const
        {
          __PGBAR_ASSERT( num_done <= task_range_.end_value() );
          __PGBAR_UNLIKELY if ( num_done == 0 ) return { __PGBAR_DEFAULT_TIMER };

          const auto time2str = []( std::int64_t num_time ) -> types::String {
            types::String ret = std::to_string( num_time );
            if ( ret.size() < 2 )
              return "0" + ret;
            return ret;
          };
          const auto to_time = [&time2str]( types::TimeUnit duration ) -> types::String {
            const auto hours = std::chrono::duration_cast<std::chrono::hours>( duration );
            duration -= hours;
            const auto minutes = std::chrono::duration_cast<std::chrono::minutes>( duration );
            duration -= minutes;
            return (
              ( ( hours.count() > 99 ? types::String( "--" ) : time2str( hours.count() ) ) + ":" )
              + ( time2str( minutes.count() ) + ":" )
              + time2str( std::chrono::duration_cast<std::chrono::seconds>( duration ).count() ) );
          };

          auto time_per_task = time_passed / num_done;
          if ( time_per_task.count() == 0 )
            time_per_task = std::chrono::nanoseconds( 1 );

          const auto remaining_tasks = task_range_.end_value() - num_done;
          // overflow check
          if ( remaining_tasks > std::numeric_limits<std::int64_t>::max() / time_per_task.count() )
            return formatting<TxtLayout::center>( _timer_len,
                                                  to_time( std::move( time_passed ) )
                                                    + " < --:--:--" );
          else
            return formatting<TxtLayout::center>( _timer_len,
                                                  to_time( std::move( time_passed ) ) + " < "
                                                    + to_time( time_per_task * remaining_tasks ) );
        }

      public:
        Builder( configs::Progress cfg ) noexcept : configs::Progress( std::move( cfg ) ) {}
        virtual ~Builder() noexcept = default;

        Builder( const Builder& lhs ) : configs::Progress( lhs ) {}
        Builder( Builder&& rhs ) noexcept : configs::Progress( std::move( rhs ) ) {}

        using configs::Progress::operator=;

        /**
         * Based on the value of `visibilities_`, determine which part of the string needs to be
         * concatenated.
         */
        StringBuffer& build( StringBuffer& buffer,
                             types::Float num_per,
                             types::Size num_done,
                             types::TimeUnit time_passed ) const
        {
          __PGBAR_ASSERT( num_per >= 0.0 );
          __PGBAR_ASSERT( num_per <= 1.0 );
          __PGBAR_ASSERT( num_done <= task_range_.end_value() );

          const types::ROStr placeholder = "";
          if ( visibilities_[BitIndex::bar] ) {
            auto info = produce_bar( num_per );
            buffer.append( startpoint_ )
              .append( front_options_[FrontOption::color] ? done_col_ : placeholder )
              .append( info.first, done_ch_ )
              .append( front_options_[FrontOption::color] ? todo_col_ : placeholder )
              .append( info.second, todo_ch_ )
              .append( front_options_[FrontOption::color] ? rgb2ansi( __PGBAR_DEFAULT )
                                                          : placeholder )
              .append( endpoint_ )
              .append( 1, constants::blank );
          }

          const bool status_flag = visibilities_[BitIndex::per] || visibilities_[BitIndex::cnt]
                                || visibilities_[BitIndex::rate] || visibilities_[BitIndex::timer];
          if ( status_flag )
            buffer << ( front_options_[FrontOption::bold] ? rgb2ansi( __PGBAR_BOLD ) : placeholder )
                   << ( front_options_[FrontOption::color] ? status_col_ : placeholder )
                   << lstatus_;
          if ( visibilities_[BitIndex::per] ) {
            buffer << produce_ratio( num_per );
            if ( visibilities_[BitIndex::cnt] || visibilities_[BitIndex::rate]
                 || visibilities_[BitIndex::timer] )
              buffer << divider_;
          }
          if ( visibilities_[BitIndex::cnt] ) {
            buffer << produce_progress( num_done );
            if ( visibilities_[BitIndex::rate] || visibilities_[BitIndex::timer] )
              buffer << divider_;
          }
          if ( visibilities_[BitIndex::rate] ) {
            buffer << produce_rate( time_passed, num_done );
            if ( visibilities_[BitIndex::timer] )
              buffer << divider_;
          }
          if ( visibilities_[BitIndex::timer] )
            buffer << produce_timer( std::move( time_passed ), num_done );
          if ( status_flag )
            buffer << rstatus_
                   << ( front_options_.any() ? rgb2ansi( __PGBAR_DEFAULT ) : placeholder );

          return buffer;
        }
      };

      template<>
      class Builder<configs::Spinner> : public configs::Spinner {
      public:
        Builder( configs::Spinner cfg ) noexcept : configs::Spinner( std::move( cfg ) ) {}
        virtual ~Builder() noexcept = default;

        Builder( const Builder& lhs ) : configs::Spinner( lhs ) {}
        Builder( Builder&& rhs ) noexcept : configs::Spinner( std::move( rhs ) ) {}

        using configs::Spinner::operator=;

        types::ROStr true_frame() const noexcept { return true_frame_; }
        types::ROStr false_frame() const noexcept { return false_frame_; }

        __PGBAR_NODISCARD types::Size num_frames() const noexcept { return frames_.size(); }

        __PGBAR_NODISCARD types::Size max_width() const noexcept
        {
          __PGBAR_ASSERT( frames_.empty() == false );
          return std::max_element(
                   frames_.cbegin(),
                   frames_.cend(),
                   []( types::ROStr a, types::ROStr b ) { return a.size() < b.size(); } )
            ->size();
        }
        __PGBAR_NODISCARD types::Size total_max_width() const noexcept
        {
          __PGBAR_ASSERT( frames_.empty() == false );
          return std::max(
            { max_width() + suffix_.size() + 2, true_frame_.size(), false_frame_.size() } );
        }

        StringBuffer& build( StringBuffer& buffer,
                             types::ROStr next_frame,
                             types::ROStr color_frame,
                             types::Size frame_width ) const
        {
          __PGBAR_ASSERT( frame_width >= next_frame.size() );
          const types::ROStr placeholder = "";
          buffer << ( front_options_[FrontOption::color] ? color_frame : placeholder ) << ' '
                 << ( front_options_[FrontOption::bold] ? rgb2ansi( __PGBAR_BOLD ) : placeholder )
                 << formatting<TxtLayout::left>( frame_width, next_frame ) << ' ' << suffix_
                 << ( front_options_.any() ? rgb2ansi( __PGBAR_DEFAULT ) : placeholder );
          return buffer;
        }

        StringBuffer& build( StringBuffer& buffer,
                             types::Size idx_frame,
                             types::Size frame_width ) const
        {
          __PGBAR_ASSERT( frames_.empty() == false );
          __PGBAR_ASSERT( idx_frame < frames_.size() );
          return build( buffer, frames_[idx_frame], frames_col_, frame_width );
        }

        StringBuffer& build( StringBuffer& buffer, bool frame_flag ) const
        {
          const auto& frame = frame_flag ? true_frame_ : false_frame_;
          if ( frame.empty() )
            return buffer;
          const types::ROStr placeholder = "";
          const auto& color              = frame_flag ? true_col_ : false_col_;
          return buffer << ( front_options_[FrontOption::color] ? color : placeholder )
                        << ( front_options_[FrontOption::bold] ? rgb2ansi( __PGBAR_BOLD )
                                                               : placeholder )
                        << frame
                        << ( front_options_.any() ? rgb2ansi( __PGBAR_DEFAULT ) : placeholder );
        }
      };

      /**
       * A manager class used to synchronize the rendering thread and main thread.
       */
      class Renderer final {
        using self = Renderer;
        /* The state transfer process is:
         *                   activate()                   suspend()
         * dormant(default) -----------> awake -> active ----------> suspend -> dormat
         *              dctor
         * (any state) ------> finish */
        enum class state : types::BitwiseSet { dormant, awake, active, suspend, finish };

        std::unique_ptr<wrappers::RenderFn> task_;

        std::atomic<state> state_;
        concurrency::ExceptionPipe pipe_;

        mutable std::condition_variable cond_var_;
        mutable std::mutex mtx_;

        std::thread td_;

      public:
        Renderer( const self& )        = delete;
        self& operator=( const self& ) = delete;

        // Lazily initialize.
        Renderer() noexcept : task_ { nullptr }, state_ { state::dormant } {}

        template<typename F>
        explicit Renderer( F&& task ) : Renderer()
        {
          reset( std::forward<F>( task ) );
        }
        ~Renderer() noexcept { reset(); }

# if __PGBAR_CXX20
        template<traits::TaskFunctor F>
        void
# else
        template<typename F>
        typename std::enable_if<traits::is_void_functor<typename std::decay<F>::type>::value>::type
# endif
          reset( F&& task ) &
        {
          __PGBAR_ASSERT( state_ == state::dormant );
# if __PGBAR_CXX14
          task_ = std::make_unique<wrappers::RenderFnWrapper<typename std::decay<F>::type>>(
            std::forward<F>( task ) );
# else
          auto new_res =
            new wrappers::RenderFnWrapper<typename std::decay<F>::type>( std::forward<F>( task ) );

          // incredibly that `std::make_unique` was forgotten in c++11 :/
          task_ = std::unique_ptr<wrappers::RenderFn>( new_res );
# endif
          td_ = std::thread( [this]() -> void {
            while ( state_.load( std::memory_order_acquire ) != state::finish ) {
              try {
                switch ( state_.load( std::memory_order_acquire ) ) {
                case state::dormant: {
                  std::unique_lock<std::mutex> lock { mtx_ };
                  cond_var_.wait( lock, [this]() -> bool {
                    return state_.load( std::memory_order_acquire ) != state::dormant;
                  } );
                } break;

                case state::awake: {
                  // Intermediate state
                  // Used to tell other threads that the current thread has woken up.
                  task_->run();
                  auto expected = state::active;
                  state_.compare_exchange_strong( expected,
                                                  state::active,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed );
                } break;
                  /* The state `awake` does not jump to `active` by using `fallthrough`,
                   * because we need to ensure that `suspend` must be transferred from `active`.

                   * In some cases the `state` transition
                   * won't work as well as we'd like if we use `fallthrough` here. */

                case state::active: {
                  task_->run();
                  std::this_thread::sleep_for( configs::Global::refresh_interval() );
                } break;

                case state::suspend: {
                  task_->run();
                  /* We expect the progress bar to be waiting for output to show that
                   * the iteration is complete at this point,
                   * so we should render it one last time before moving to `dormat` here. */

                  state_.store( state::dormant, std::memory_order_release );
                } break;

                case state::finish: __PGBAR_FALLTHROUGH;
                default: // immediately terminate
                  return;
                }
              } catch ( ... ) {
                // keep object valid
                if ( state_.load( std::memory_order_acquire ) != state::finish )
                  state_.store( state::dormant, std::memory_order_release );
                // Avoid deadlock in main thread
                // when the child thread catchs exceptions.
                auto exception = std::current_exception();
                if ( exception )
                  pipe_.push( exception );
              }
            }
          } );
        }

        void reset() & noexcept
        {
          // terminate rendered
          state_.store( state::finish, std::memory_order_release );
          {
            std::lock_guard<std::mutex> lock { mtx_ };
            cond_var_.notify_all();
          }
          if ( td_.joinable() )
            td_.join();

          task_.reset();
          td_ = std::thread();
        }

        /**
         * Check whether the lazy initialization object state is valid.
         */
        __PGBAR_NODISCARD bool valid() const noexcept { return task_ != nullptr; }

        void activate() &
        {
          __PGBAR_ASSERT( valid() );
          auto expected = state::dormant;
          if ( state_.compare_exchange_strong( expected,
                                               state::awake,
                                               std::memory_order_acq_rel,
                                               std::memory_order_relaxed ) ) {
            // If `if` evaluates to false, another thread has already activated it.
            // Otherwise, the rendering thread has terminated abnormally.
            {
              std::lock_guard<std::mutex> lock { mtx_ };
              cond_var_.notify_one();
            }
            // spin wait, ensure taht the thread has moved to the new state
            do {
              // avoid deadlock and throw the exception the thread received
              __PGBAR_UNLIKELY if ( pipe_.empty() == false ) pipe_.pop_throw();
              // busy wait
            } while ( state_.load( std::memory_order_acquire ) == state::awake );
          }
        }

        void suspend() &
        {
          __PGBAR_ASSERT( valid() );
          auto expected = state::active;
          if ( state_.compare_exchange_strong( expected,
                                               state::suspend,
                                               std::memory_order_acq_rel,
                                               std::memory_order_relaxed ) ) {
            do {
              __PGBAR_UNLIKELY if ( pipe_.empty() == false ) pipe_.pop_throw();
            } while ( state_.load( std::memory_order_acquire ) == state::suspend );
          }
        }
      };

      template<typename Config, typename StreamObj, typename MutexMode, class Derived>
      class Director { // CRTP
# if !__PGBAR_CXX20
        static_assert( traits::is_ostream<StreamObj>::value,
                       "pgbar::__detail::render::Director: Invalid tmeplate type" );
# endif
        using self = Director<Config, StreamObj, MutexMode, Derived>;

      protected:
        render::Renderer executor_;
        render::Builder<Config> builder_;

        typename std::add_pointer<typename std::decay<StreamObj>::type>::type stream_;
        StringBuffer buffer_;

        __PGBAR_NOUNIQUEADDR mutable MutexMode mtx_;

      public:
        using StreamType = StreamObj;
        using MutexType  = MutexMode;

        Director( StreamObj& stream, Config cfg )
          noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
          : executor_ {}, builder_ { std::move( cfg ) }, stream_ { std::addressof( stream ) }
        {}
        Director( Config cfg ) noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
          : Director( std::cerr, std::move( cfg ) )
        {}

        Director( const self& lhs )
          : executor_ {}, builder_ { lhs.builder_ }, stream_ { lhs.stream_ }
        {}
        Director( self&& rhs ) noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
          : Director( *rhs.stream_, std::move( rhs.builder_ ) )
        {}
        self& operator=( const self& lhs )
        {
          __PGBAR_ASSERT( this != std::addressof( lhs ) );
          builder_ = lhs.builder_;
          stream_  = lhs.stream_;
          return *this;
        }
        self& operator=( self&& rhs ) noexcept
        {
          __PGBAR_ASSERT( this != std::addressof( rhs ) );
          builder_.swap( rhs.builder_ );
          std::swap( stream_, rhs.stream_ );
          return *this;
        }

        virtual bool is_running() const noexcept = 0;

        virtual void tick() & = 0;
        virtual void reset()  = 0;

        Config& configure() & noexcept { return builder_; }
        const Config& configure() const& noexcept { return builder_; }
        Derived& configure( Config cfg ) & noexcept
        {
          builder_.swap( cfg );
          return static_cast<Derived&>( *this );
        };

        virtual ~Director() noexcept = default;
      };
    } // namespace render
  } // namespace __detail

  class Threadsafe {
    using self = Threadsafe;

    std::atomic_flag lock_stat_ = ATOMIC_FLAG_INIT;

  public:
    Threadsafe( const Threadsafe& )            = delete;
    Threadsafe& operator=( const Threadsafe& ) = delete;

    Threadsafe() noexcept          = default;
    virtual ~Threadsafe() noexcept = default;

    void lock() & noexcept
    {
      while ( lock_stat_.test_and_set( std::memory_order_acq_rel ) )
        std::this_thread::yield();
    }

    void unlock() & noexcept { lock_stat_.clear( std::memory_order_release ); }

    bool try_lock() & noexcept { return !lock_stat_.test_and_set( std::memory_order_acq_rel ); }
  };
  // A empty class that satisfies the "basic lockable" requirement.
  class Threadunsafe {
  public:
    Threadunsafe() noexcept          = default;
    virtual ~Threadunsafe() noexcept = default;
    __PGBAR_INLINE_FN void lock() noexcept {}
    __PGBAR_INLINE_FN void unlock() noexcept {}
  };

# if __PGBAR_CXX20
  template<typename,
           __detail::traits::OStream = std::ostream,
           __detail::traits::Mutex   = Threadunsafe>
# else
  template<typename, typename = std::ostream, typename = Threadunsafe>
# endif
  class Indicator;

# if __PGBAR_CXX20
  template<__detail::traits::OStream StreamObj, __detail::traits::Mutex MutexMode>
# else
  template<typename StreamObj, typename MutexMode>
# endif
  class Indicator<configs::Progress, StreamObj, MutexMode>
    : public __detail::render::Director<configs::Progress,
                                        StreamObj,
                                        MutexMode,
                                        Indicator<configs::Progress, StreamObj, MutexMode>> {
    using self = Indicator<configs::Progress, StreamObj, MutexMode>;
    using base = __detail::render::Director<configs::Progress, StreamObj, MutexMode, self>;

    /* The state transfer process is:
     *                   tick()                    automatically/reset()
     * stopped(default) -------> begin -> refresh ----------------------> finish -> stopped
     *
     * And normally, the state `stopped` will not terminate function `rendering`,
     * excepting the dctor is invoked. */
    enum class state : uint8_t { begin, refresh, finish, stopped };

    std::atomic<state> state_;

    std::atomic<__detail::types::Size> task_cnt_, task_end_;
    std::chrono::system_clock::time_point zero_point_;

    void rendering() &
    {
      switch ( state_.load( std::memory_order_acquire ) ) {
      case state::begin: { // intermediate state
        zero_point_ = std::chrono::system_clock::now();

        // For visual purposes, output the full progress bar at the beginning.
        *this->stream_ << this->builder_.build(
          this->buffer_.reserve( this->builder_.fixed_size() + this->builder_.bar_length() + 7 )
            .append( __detail::constants::cursor_save ),
          0.0,
          0,
          {} );

        auto expected = state::begin;
        state_.compare_exchange_strong( expected,
                                        state::refresh,
                                        std::memory_order_release,
                                        std::memory_order_relaxed );
        /* If the main thread finds that the iteration is complete immediately,
         * it will set the `state_` to `finish`.
         * Therefore we cannot use `store` here. */
      }
        __PGBAR_FALLTHROUGH;

      case state::refresh: {
        const auto num_percent = progress() / static_cast<__detail::types::Float>( task_end_ );
        __PGBAR_ASSERT( num_percent <= 100.0 && num_percent >= 0.0 );

        // Then normally output the progress bar.
        *this->stream_ << this->builder_.build(
          this->buffer_.append( __detail::constants::cursor_restore ),
          num_percent,
          progress(),
          std::chrono::system_clock::now() - zero_point_ );
      } break;

      case state::finish: { // intermediate state
        const auto num_percent = progress() / static_cast<__detail::types::Float>( task_end_ );
        __PGBAR_ASSERT( num_percent <= 100.0 && num_percent >= 0.0 );

        *this->stream_ << this->builder_
                            .build( this->buffer_.append( __detail::constants::cursor_restore ),
                                    num_percent,
                                    progress(),
                                    std::chrono::system_clock::now() - zero_point_ )
                            .append( 1, '\n' );
        this->buffer_.release(); // releases the buffer

        state_.store( state::stopped, std::memory_order_release );
      } break;

      // It only reaches here when the rendering thread is destroyed.
      case state::stopped: __PGBAR_FALLTHROUGH;
      default:             return;
      }
    }

    void unlock_reset()
    {
      if ( this->executor_.valid() ) {
        auto expected = state::begin;
        state_.compare_exchange_strong( expected,
                                        state::finish,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed )
          || state_.compare_exchange_strong( expected = state::refresh,
                                             state::finish,
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed );
        this->executor_.suspend();
      } else
        __PGBAR_ASSERT( state_ == state::stopped );
    }

    template<typename F>
    __PGBAR_INLINE_FN void do_tick( F&& custom_action ) &
    {
      static_assert( __detail::traits::is_void_functor<F>::value,
                     "pgbar::Indicator<Progress>::do_tick: Invalid template type error" );

      std::lock_guard<MutexMode> lock { this->mtx_ };
      switch ( state_.load( std::memory_order_acquire ) ) {
      case state::stopped: {
        /* The `intty()` is thread safe, and if the program is not in tty,
         * the code flow should return as soon as possible.

         * Any activities that attempt to modify the state of `pgbar` in this case is meaningless.
         */
        __PGBAR_UNLIKELY if ( !configs::Global::intty() ) return;
        else
        {
          task_end_.store( this->builder_.tasks(), std::memory_order_release );
          __PGBAR_UNLIKELY if ( task_end_.load( std::memory_order_acquire )
                                == 0 ) throw exceptions::
            InvalidState( "pgbar: the number of tasks is zero" );
        }

        task_cnt_.store( 0, std::memory_order_release );
        __PGBAR_UNLIKELY if ( !this->executor_.valid() ) this->executor_.reset(
          [this]() { rendering(); } );

        state_.store( state::begin, std::memory_order_release );
        this->executor_.activate();
      }
        __PGBAR_FALLTHROUGH;
      case state::begin:   __PGBAR_FALLTHROUGH;
      case state::refresh: {
        custom_action();

        __PGBAR_UNLIKELY if ( task_cnt_.load( std::memory_order_acquire )
                              == task_end_.load( std::memory_order_acquire ) ) unlock_reset();
      } break;

      case state::finish: __PGBAR_FALLTHROUGH;
      default:            return;
      }
    }

  public:
    Indicator( StreamObj& stream, configs::Progress cfg )
      noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : base( stream, std::move( cfg ) )
    {
      state_.store( state::stopped, std::memory_order_relaxed );
      task_cnt_.store( 0, std::memory_order_relaxed );
    }
    Indicator( configs::Progress cfg )
      noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : Indicator( std::cerr, std::move( cfg ) )
    {}
    Indicator( StreamObj& stream, __detail::types::Size num_tasks )
      : Indicator( stream, configs::Progress( num_tasks ) )
    {}
    Indicator( __detail::types::Size num_tasks ) : Indicator( std::cerr, num_tasks ) {}
    template<typename... Args,
             typename = typename std::enable_if<
               __detail::traits::all_of_progress<typename std::decay<Args>::type...>::value>::type>
    Indicator( StreamObj& stream, Args&&... args )
      : Indicator( stream, configs::Progress( std::forward<Args>( args )... ) )
    {}
    template<typename... Args,
             typename = typename std::enable_if<
               __detail::traits::all_of_progress<typename std::decay<Args>::type...>::value>::type>
    Indicator( Args&&... args )
      : Indicator( std::cerr, configs::Progress( std::forward<Args>( args )... ) )
    {}

    Indicator( self&& rhs ) noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : Indicator( *rhs.stream_, std::move( rhs.builder_ ) )
    {}
    self& operator=( self&& rhs ) & noexcept
    {
      __PGBAR_ASSERT( this != std::addressof( rhs ) );
      swap( rhs );
      return *this;
    }

    virtual ~Indicator() noexcept // let it crash
    {
      if ( is_running() )
        this->executor_.suspend();
    }

    __PGBAR_NODISCARD bool is_running() const noexcept override
    {
      return state_.load( std::memory_order_acquire ) != state::stopped;
    }

    void tick() & override
    {
      do_tick( [this]() { task_cnt_.fetch_add( 1, std::memory_order_release ); } );
    }
    void tick( __detail::types::Size next_step ) &
    {
      do_tick( [&]() {
        task_cnt_.fetch_add( next_step + task_cnt_.load( std::memory_order_acquire )
                                 > task_end_.load( std::memory_order_acquire )
                               ? task_end_.load( std::memory_order_acquire )
                                   - task_cnt_.load( std::memory_order_acquire )
                               : next_step,
                             std::memory_order_release );
      } );
    }
    /**
     * Set the iteration step of the progress bar to a specified percentage.
     *
     * Ignore the call if the iteration count exceeds the given percentage.
     *
     * If `percentage` is bigger than 100, it will be set to 100.
     *
     * @param percentage Value range: [0, 100].
     */
    void tick_to( __detail::types::Size percentage ) &
    {
      if ( percentage < 100 ) {
        const auto target_progress = static_cast<__detail::types::Size>(
          task_end_.load( std::memory_order_acquire ) * percentage * 0.01 );

        __PGBAR_ASSERT( target_progress <= task_end_.load( std::memory_order_acquire ) );

        if ( target_progress > task_cnt_.load( std::memory_order_acquire ) )
          do_tick( [this, target_progress]() {
            task_cnt_.store( target_progress, std::memory_order_release );
          } );
      } else
        do_tick( [this]() { task_cnt_.store( task_end_.load( std::memory_order_acquire ) ); } );
    }

    /**
     * Reset pgbar obj, EXCLUDING the total number of tasks.
     *
     * It will immediately TERMINATE the current rendering.
     */
    void reset() override
    {
      std::lock_guard<MutexMode> lock { this->mtx_ };
      unlock_reset();
    }

    /**
     * Get the number of tasks that have been completed.
     */
    __PGBAR_NODISCARD __detail::types::Size progress() const noexcept
    {
      return task_cnt_.load( std::memory_order_acquire );
    }

    void swap( self& lhs ) & noexcept
    {
      __PGBAR_ASSERT( this != std::addressof( lhs ) );
      this->builder_.swap( lhs.builder_ );
      std::swap( this->stream_, lhs.stream_ );
    }
    friend void swap( self& a, self& b ) noexcept { a.swap( b ); }

    /**
     * Visualize unidirectional traversal of a numeric interval defined by parameters.
     *
     * @return Return a range `[startpoint, endpoint)` that moves unidirectionally.
     */
    template<typename N>
# if __PGBAR_CXX20
      requires std::is_arithmetic_v<N>
    __PGBAR_NODISCARD iterators::ProxySpan<iterators::NumericSpan<N>, self>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<std::is_arithmetic<N>::value,
                              iterators::ProxySpan<iterators::NumericSpan<N>, self>>::type
# endif
      foreach ( N startpoint, N endpoint, N step ) &
    { // default parameter will cause ambiguous overloads
      // so we have to write them all
      return { iterators::NumericSpan<typename std::decay<N>::type>( startpoint, endpoint, step ),
               *this };
    }
    template<typename N, typename F>
# if __PGBAR_CXX20
      requires std::is_arithmetic_v<N>
    void
# else
    typename std::enable_if<std::is_arithmetic<N>::value>::type
# endif
      foreach ( N startpoint, N endpoint, N step, F && unary_fn ) &
    {
      for ( N e : foreach ( startpoint, endpoint, step ) )
        unary_fn( e );
    }

    template<typename N>
# if __PGBAR_CXX20
      requires std::is_floating_point_v<N>
    __PGBAR_NODISCARD iterators::ProxySpan<iterators::NumericSpan<N>, self>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<std::is_floating_point<N>::value,
                              iterators::ProxySpan<iterators::NumericSpan<N>, self>>::type
# endif
      foreach ( N endpoint, N step ) &
    {
      return { iterators::NumericSpan<typename std::decay<N>::type>( {}, endpoint, step ), *this };
    }
    template<typename N, typename F>
# if __PGBAR_CXX20
      requires std::is_floating_point_v<N>
    void
# else
    typename std::enable_if<std::is_floating_point<N>::value>::type
# endif
      foreach ( N endpoint, N step, F && unary_fn ) &
    {
      for ( N e : foreach ( endpoint, step ) )
        unary_fn( e );
    }

    /**
     * Only available for integer types.
     */
    template<typename N>
# if __PGBAR_CXX20
      requires std::is_integral_v<N>
    __PGBAR_NODISCARD iterators::ProxySpan<iterators::NumericSpan<N>, self>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<std::is_integral<N>::value,
                              iterators::ProxySpan<iterators::NumericSpan<N>, self>>::type
# endif
      foreach ( N startpoint, N endpoint ) &
    {
      return { iterators::NumericSpan<typename std::decay<N>::type>( {}, endpoint, 1 ), *this };
    }
    template<typename N, typename F>
# if __PGBAR_CXX20
      requires std::is_integral_v<N>
    void
# else
    typename std::enable_if<std::is_integral<N>::value>::type
# endif
      foreach ( N startpoint, N endpoint, F && unary_fn ) &
    {
      for ( N e : foreach ( startpoint, endpoint ) )
        unary_fn( e );
    }

    template<typename N>
# if __PGBAR_CXX20
      requires std::is_integral_v<N>
    __PGBAR_NODISCARD iterators::ProxySpan<iterators::NumericSpan<N>, self>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<std::is_integral<N>::value,
                              iterators::ProxySpan<iterators::NumericSpan<N>, self>>::type
# endif
      foreach ( N endpoint ) &
    {
      return { iterators::NumericSpan<typename std::decay<N>::type>( {}, endpoint, 1 ), *this };
    }
    template<typename N, typename F>
# if __PGBAR_CXX20
      requires std::is_integral_v<N>
    void
# else
    typename std::enable_if<std::is_integral<N>::value>::type
# endif
      foreach ( N endpoint, F && unary_fn ) &
    {
      for ( N e : foreach ( endpoint ) )
        unary_fn( e );
    }

    /**
     * Visualize unidirectional traversal of a iterator interval defined by parameters.
     */
    template<typename I>
# if __PGBAR_CXX20
      requires std::negation_v<std::is_arithmetic<I>>
    __PGBAR_NODISCARD iterators::ProxySpan<iterators::IterSpan<I>, self>
# else
    __PGBAR_NODISCARD
      typename std::enable_if<!std::is_arithmetic<I>::value,
                              iterators::ProxySpan<iterators::IterSpan<I>, self>>::type
# endif
      foreach ( I startpoint, I endpoint ) & noexcept(
        !std::is_pointer<typename std::decay<I>::type>::value
        && std::is_nothrow_move_constructible<typename std::decay<I>::type>::value )
    {
      return { iterators::IterSpan<typename std::decay<I>::type>( std::move( startpoint ),
                                                                  std::move( endpoint ) ),
               *this };
    }
    template<typename I, typename F>
# if __PGBAR_CXX20
      requires std::negation_v<std::is_arithmetic<I>>
    void
# else
    typename std::enable_if<!std::is_arithmetic<I>::value>::type
# endif
      foreach ( I startpoint, I endpoint, F && unary_fn ) &
    {
      for ( auto&& e : foreach ( std::move( startpoint ), std::move( endpoint ) ) )
        unary_fn( std::forward<decltype( e )>( e ) );
    }

    /**
     * Visualize unidirectional traversal of a abstract range interval defined by `container`'s
     * iterators.
     */
    template<class R>
# if __PGBAR_CXX20
      requires std::disjunction_v<std::is_class<std::decay_t<R>>,
                                  std::is_array<std::remove_reference_t<R>>>
            && std::is_lvalue_reference_v<R>
    __PGBAR_NODISCARD iterators::ProxySpan<
      iterators::IterSpan<
        typename __detail::traits::iterator_type<std::remove_reference_t<R>>::type>,
      self>
# else
    __PGBAR_NODISCARD typename std::enable_if<
      ( std::is_class<typename std::decay<R>::type>::value
        || std::is_array<typename std::remove_reference<R>::type>::value )
        && std::is_lvalue_reference<R>::value,
      iterators::ProxySpan<iterators::IterSpan<typename __detail::traits::iterator_type<
                             typename std::remove_reference<R>::type>::type>,
                           self>>::type
# endif
      foreach ( R&& container ) &
    { // forward it to the iterator overload
# if __PGBAR_CXX20
      return foreach ( std::ranges::begin( container ), std::ranges::end( container ) );
# else
      using std::begin;
      using std::end; // for ADL
      return foreach ( begin( container ), end( container ) );
# endif
    }
    template<class R, typename F>
# if __PGBAR_CXX20
      requires std::disjunction_v<std::is_class<std::decay_t<R>>,
                                  std::is_array<std::remove_reference_t<R>>>
            && std::is_lvalue_reference_v<R>
    void
# else
    typename std::enable_if<( std::is_class<typename std::decay<R>::type>::value
                              || std::is_array<typename std::remove_reference<R>::type>::value )
                            && std::is_lvalue_reference<R>::value>::type
# endif
      foreach ( R&& container, F && unary_fn ) &
    {
      for ( auto&& e : foreach ( container ) )
        unary_fn( std::forward<decltype( e )>( e ) );
    }
  };

  namespace __detail {
    namespace traits {
# if __PGBAR_CXX20
      template<typename B>
      concept Progress = requires {
        typename B::StreamType;
        typename B::MutexType;
        requires std::conjunction_v<
          std::negation<std::is_reference<B>>,
          is_ostream<typename B::StreamType>,
          is_mutex<typename B::MutexType>,
          std::is_same<
            B,
            Indicator<configs::Progress, typename B::StreamType, typename B::MutexType>>>;
      };
      template<typename B>
      struct is_progress : std::bool_constant<Progress<B>> {};
# else
      template<typename B, typename = void>
      struct is_progress : std::false_type {};
      template<typename B>
      struct is_progress<
        B,
        typename std::enable_if<
          !std::is_reference<B>::value && is_ostream<typename B::StreamType>::value
          && is_mutex<typename B::MutexType>::value
          && std::is_same<
            B,
            Indicator<configs::Progress, typename B::StreamType, typename B::MutexType>>::value>::
          type> : std::true_type {};
# endif
    } // namespace traits
  } // namespace __detail

# if __PGBAR_CXX20
  template<__detail::traits::OStream StreamObj, __detail::traits::Mutex MutexMode>
# else
  template<typename StreamObj, typename MutexMode>
# endif
  class Indicator<configs::Spinner, StreamObj, MutexMode>
    : public __detail::render::Director<configs::Spinner,
                                        StreamObj,
                                        MutexMode,
                                        Indicator<configs::Spinner, StreamObj, MutexMode>> {
    using self = Indicator<configs::Spinner, StreamObj, MutexMode>;
    using base = __detail::render::Director<configs::Spinner, StreamObj, MutexMode, self>;

    enum class state : __detail::types::BitwiseSet { begin, refresh, finish, stopped };
    std::atomic<state> state_;

    bool reset_flag_;
    __detail::types::Size idx_frame_;
    __detail::types::Size widest_frame_size_;

    void rendering() &
    {
      switch ( state_.load( std::memory_order_acquire ) ) {
      case state::begin: {
        widest_frame_size_ = this->builder_.max_width();

        *this->stream_ << this->builder_.build(
          this->buffer_
            .reserve( std::max( { widest_frame_size_,
                                  this->builder_.true_frame().size(),
                                  this->builder_.false_frame().size() } ) )
            .append( __detail::constants::cursor_save ),
          idx_frame_ = 0,
          widest_frame_size_ );

        auto expected = state::begin;
        state_.compare_exchange_strong( expected,
                                        state::refresh,
                                        std::memory_order_release,
                                        std::memory_order_relaxed );
      }
        __PGBAR_FALLTHROUGH;

      case state::refresh: {
        *this->stream_ << this->builder_.build(
          this->buffer_.append( __detail::constants::cursor_restore ),
          idx_frame_++,
          widest_frame_size_ );

        idx_frame_ %= this->builder_.num_frames();
      } break;

      case state::finish: {
        this->buffer_.append( __detail::constants::cursor_restore );
        if ( !( reset_flag_ ? this->builder_.true_frame() : this->builder_.false_frame() ).empty() )
          this->buffer_.append( this->builder_.total_max_width(), __detail::constants::blank )
            .append( __detail::constants::cursor_restore );
        *this->stream_ << this->builder_.build( this->buffer_, reset_flag_ ).append( 1, '\n' );

        this->buffer_.release();
        state_.store( state::stopped, std::memory_order_release );
      } break;

      case state::stopped: __PGBAR_FALLTHROUGH;
      default:             return;
      }
    }

    void unlock_reset()
    {
      if ( this->executor_.valid() ) {
        auto expected = state::begin;
        state_.compare_exchange_strong( expected,
                                        state::finish,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed )
          || state_.compare_exchange_strong( expected = state::refresh,
                                             state::finish,
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed );
        this->executor_.suspend();
      } else
        __PGBAR_ASSERT( state_ == state::stopped );
    }

  public:
    Indicator( StreamObj& stream, configs::Spinner cfg )
      noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : base( stream, std::move( cfg ) )
    {
      state_.store( state::stopped, std::memory_order_relaxed );
    }
    Indicator( configs::Spinner cfg )
      noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : Indicator( std::cerr, std::move( cfg ) )
    {}
    Indicator( StreamObj& stream, std::vector<__detail::types::String> frames )
      : Indicator( stream, configs::Spinner( std::move( frames ) ) )
    {}
    Indicator( std::vector<__detail::types::String> frames )
      : Indicator( std::cerr, std::move( frames ) )
    {}
    template<typename... Args,
             typename = typename std::enable_if<
               __detail::traits::all_of_spinner<typename std::decay<Args>::type...>::value>::type>
    Indicator( StreamObj& stream, Args&&... args )
      : Indicator( stream, configs::Spinner( std::forward<Args>( args )... ) )
    {}
    template<typename... Args,
             typename = typename std::enable_if<
               __detail::traits::all_of_spinner<typename std::decay<Args>::type...>::value>::type>
    Indicator( Args&&... args )
      : Indicator( std::cerr, configs::Spinner( std::forward<Args>( args )... ) )
    {}

    Indicator( self&& rhs ) noexcept( std::is_nothrow_default_constructible<MutexMode>::value )
      : Indicator( *rhs.stream_, std::move( rhs.builder_ ) )
    {}
    self& operator=( self&& rhs ) & noexcept
    {
      __PGBAR_ASSERT( this != std::addressof( rhs ) );
      swap( rhs );
      return *this;
    }

    virtual ~Indicator() noexcept
    {
      if ( is_running() )
        this->executor_.suspend();
    }

    __PGBAR_NODISCARD bool is_running() const noexcept override
    {
      return state_.load( std::memory_order_acquire ) != state::stopped;
    }

    void tick() & override
    {
      std::lock_guard<MutexMode> lock { this->mtx_ };
      switch ( state_.load( std::memory_order_acquire ) ) {
      case state::stopped: {
        __PGBAR_UNLIKELY if ( !configs::Global::intty() ) return;

        __PGBAR_UNLIKELY if ( !this->executor_.valid() ) this->executor_.reset(
          [this]() { rendering(); } );

        state_.store( state::begin, std::memory_order_release );
        this->executor_.activate();
      } break;

      case state::begin:   __PGBAR_FALLTHROUGH;
      case state::refresh: __PGBAR_FALLTHROUGH;
      case state::finish:  __PGBAR_FALLTHROUGH;
      default:             return;
      }
    }

    void reset() override { reset( true ); }
    void reset( bool endframe )
    {
      std::lock_guard<MutexMode> lock { this->mtx_ };
      reset_flag_ = endframe;
      unlock_reset();
    }

    void swap( self& lhs ) & noexcept
    {
      __PGBAR_ASSERT( this != std::addressof( lhs ) );
      this->builder_.swap( lhs.builder_ );
      std::swap( this->stream_, lhs.stream_ );
    }
    friend void swap( self& a, self& b ) noexcept { a.swap( b ); }
  };

  namespace iterators {
    /**
     * A range that contains a `pgbar` and an UNIDIRECTIONAL abstract range,
     * which transforms the iterations in the abstract into a visual display of `pgbar`.
     */
    template<typename R, typename B>
    class ProxySpan {
      static_assert( __detail::traits::is_arith_range<R>::value
                       || __detail::traits::is_iter_range<R>::value,
                     "pgbar::iterators::ProxySpan: Only available for certain range types" );
      static_assert( __detail::traits::is_progress<B>::value,
                     "pgbar::iterators::ProxySpan: Only available for Progress types" );

      B* itr_bar_;
      R itr_range_;

    public:
      class iterator final {
        typename R::iterator itr_;
        B* itr_bar_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = typename std::iterator_traits<typename R::iterator>::value_type;
        using difference_type   = void;
        using pointer           = typename std::iterator_traits<typename R::iterator>::pointer;
        using reference         = typename std::iterator_traits<typename R::iterator>::reference;

        iterator( typename R::iterator itr, B& itr_bar )
          noexcept( std::is_nothrow_move_constructible<typename R::iterator>::value )
          : itr_ { std::move( itr ) }, itr_bar_ { std::addressof( itr_bar ) }
        {}
        ~iterator() noexcept( std::is_nothrow_destructible<R>::value ) = default;

        __PGBAR_INLINE_FN iterator& operator++()
        {
          __PGBAR_ASSERT( itr_bar_ != nullptr );
          ++itr_;
          itr_bar_->tick();
          return *this;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN iterator operator++( int )
        {
          __PGBAR_ASSERT( itr_bar_ != nullptr );
          auto before = *this;
          operator++();
          return before;
        }

        __PGBAR_NODISCARD __PGBAR_INLINE_FN reference operator*() noexcept { return *itr_; }
        __PGBAR_INLINE_FN pointer operator->() noexcept { return std::addressof( itr_ ); }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator==( const iterator& lhs ) const noexcept
        {
          return itr_ == lhs.itr_;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator!=( const iterator& lhs ) const noexcept
        {
          return !operator==( lhs );
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator==(
          const typename R::iterator& lhs ) const noexcept
        {
          return itr_ == lhs;
        }
        __PGBAR_NODISCARD __PGBAR_INLINE_FN bool operator!=(
          const typename R::iterator& lhs ) const noexcept
        {
          return itr_ != lhs;
        }
      };

      ProxySpan( R itr_range, B& itr_bar ) noexcept( std::is_nothrow_move_constructible<R>::value )
        : itr_bar_ { std::addressof( itr_bar ) }, itr_range_ { std::move( itr_range ) }
      {}
      ProxySpan( ProxySpan&& rhs ) noexcept( std::is_nothrow_move_constructible<R>::value )
        : ProxySpan( std::move( rhs.itr_range_ ), *rhs.itr_bar_ )
      {
        __PGBAR_ASSERT( rhs.itr_bar_ != nullptr );
      }
      ProxySpan& operator=( ProxySpan&& rhs ) & noexcept(
        std::is_nothrow_move_assignable<R>::value )
      {
        __PGBAR_ASSERT( this != std::addressof( rhs ) );
        swap( rhs );
        return *this;
      }
      virtual ~ProxySpan() noexcept( std::is_nothrow_destructible<R>::value ) = default;

      /**
       * This function CHANGES the state of the pgbar object it holds.
       */
      __PGBAR_NODISCARD inline iterator begin() &
      {
        itr_bar_->configure().tasks( itr_range_.size() );
        return iterator( itr_range_.begin(), *itr_bar_ );
      }
      __PGBAR_NODISCARD inline iterator end() const
      {
        return iterator( itr_range_.end(), *itr_bar_ );
      }
      void swap( ProxySpan<R, B>& lhs ) & noexcept
      {
        __PGBAR_ASSERT( this != std::addressof( lhs ) );
        std::swap( itr_bar_, lhs.itr_bar_ );
        itr_range_.swap( lhs.itr_range_ );
      }
      friend void swap( ProxySpan<R, B>& a, ProxySpan<R, B>& b ) noexcept { a.swap( b ); }
    };
  } // namespace iterators

  namespace traits {
    template<typename T>
    using is_ostream = __detail::traits::is_ostream<T>;

    template<typename T>
    using is_mutex = __detail::traits::is_mutex<T>;

# if __PGBAR_CXX14
    template<typename T>
    __PGBAR_INLINE_VAR constexpr bool is_ostream_v = is_ostream<T>::value;

    template<typename T>
    __PGBAR_INLINE_VAR constexpr bool is_mutex_v = is_mutex<T>::value;
# endif
  } // namespace traits

  template<typename OStream = std::ostream, typename Mutex = Threadunsafe>
  using ProgressBar = Indicator<configs::Progress, OStream, Mutex>;

  template<typename OStream = std::ostream, typename Mutex = Threadunsafe>
  using SpinnerBar = Indicator<configs::Spinner, OStream, Mutex>;
} // namespace pgbar

# undef __PGBAR_DEFAULT_RATIO
# undef __PGBAR_DEFAULT_TIMER
# undef __PGBAR_DEFAULT_RATE

# undef __PGBAR_INLINE_FN
# undef __PGBAR_NODISCARD
# undef __PGBAR_CC_STD
# undef __PGBAR_WIN
# undef __PGBAR_UNIX
# undef __PGBAR_UNKNOWN
# undef __PGBAR_CXX20
# undef __PGBAR_NOUNIQUEADDR
# undef __PGBAR_CXX17
# undef __PGBAR_INLINE_VAR
# undef __PGBAR_CONSTEXPR_IF
# undef __PGBAR_FALLTHROUGH
# undef __PGBAR_UNLIKELY
# undef __PGBAR_CXX14
# undef __PGBAR_RELAXED_CONSTEXPR_FN
# undef __PGBAR_CXX11

# undef __PGBAR_BOLD
# undef __PGBAR_BLACK
# undef __PGBAR_RED
# undef __PGBAR_GREEN
# undef __PGBAR_YELLOW
# undef __PGBAR_BLUE
# undef __PGBAR_MAGENTA
# undef __PGBAR_CYAN
# undef __PGBAR_WHITE
# undef __PGBAR_DEFAULT

# undef __PGBAR_ASSERT

#endif
