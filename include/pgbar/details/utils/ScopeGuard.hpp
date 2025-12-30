#ifndef PGBAR__SCOPEGUARD
#define PGBAR__SCOPEGUARD

#include "../traits/Backport.hpp"
#include "Backport.hpp"
#include <limits>

namespace pgbar {
  namespace _details {
    namespace utils {
      template<typename Fn, typename = void>
      class ScpStore { // scope, not that "scp"
        Fn callback_;

      protected:
        template<typename F>
        ScpStore( std::true_type, F&& fn ) noexcept : callback_ { std::forward<F>( fn ) }
        {}
        template<typename F>
        ScpStore( std::false_type, F&& fn ) noexcept( std::is_nothrow_constructible<Fn, F&>::value )
          : callback_ { fn }
        {}

        PGBAR__FORCEINLINE Fn& callback() noexcept { return callback_; }
      };
      template<typename Fn>
      class ScpStore<Fn,
                     typename std::enable_if<
                       traits::AllOf<std::is_empty<Fn>, traits::Not<traits::is_final<Fn>>>::value>::type>
        : private Fn {
      protected:
        template<typename F>
        ScpStore( std::true_type, F&& fn ) noexcept : Fn( std::forward<F>( fn ) )
        {}
        template<typename F>
        ScpStore( std::false_type, F&& fn ) noexcept( std::is_nothrow_constructible<Fn, F&>::value )
          : Fn( fn )
        {}

        PGBAR__FORCEINLINE Fn& callback() noexcept { return static_cast<Fn&>( *this ); }
      };

      template<typename Fn>
      class
#if PGBAR__CXX17
        PGBAR__NODISCARD
#endif
          ScopeFail : private ScpStore<Fn> {
        static_assert( traits::is_invocable<typename std::remove_reference<Fn>::type&>::value,
                       "pgbar::_details::utils::ScopeFail: Invalid callback type" );
        using Self = ScopeFail;
        using Base = ScpStore<Fn>;

        int exceptions_on_entry_;

      public:
        ScopeFail( const Self& )       = delete;
        Self& operator=( Self&& )      = delete;
        Self& operator=( const Self& ) = delete;

        template<
          typename F,
          typename std::enable_if<
            traits::AllOf<
              traits::Not<
                std::is_same<typename std::remove_cv<typename std::remove_reference<F>::type>::type, Self>>,
              std::is_constructible<Fn, F>,
              traits::Not<std::is_lvalue_reference<F>>,
              std::is_nothrow_constructible<Fn, F>>::value,
            bool>::type = 0>
        explicit ScopeFail( F&& fn ) noexcept
          : Base( std::true_type(), std::forward<F>( fn ) ), exceptions_on_entry_ { uncaught_exceptions() }
        {}
        template<
          typename F,
          typename std::enable_if<
            traits::AllOf<
              traits::Not<
                std::is_same<typename std::remove_cv<typename std::remove_reference<F>::type>::type, Self>>,
              std::is_constructible<Fn, F>,
              traits::AnyOf<std::is_lvalue_reference<F>,
                            traits::Not<std::is_nothrow_constructible<Fn, F>>>>::value,
            bool>::type = 0>
        explicit ScopeFail( F&& fn ) noexcept( std::is_nothrow_constructible<Fn, F&>::value )
        try
          : Base( std::false_type(), std::forward<F>( fn ) ), exceptions_on_entry_ { uncaught_exceptions() } {
        } catch ( ... ) {
          (void)fn();
          // it would be a bit contradictory to rethrow an exception here...
          // at least the gnu experimental/scope doesn't do that.
        }
        template<typename F,
                 typename std::enable_if<
                   traits::AllOf<std::is_same<F, Fn>, std::is_nothrow_move_constructible<Fn>>::value,
                   bool>::type = 0>
        ScopeFail( ScopeFail<F>&& rhs ) noexcept
          : Base( std::true_type(), std::forward<F>( rhs.callback() ) )
          , exceptions_on_entry_ { uncaught_exceptions() }
        {}
        template<typename F,
                 typename std::enable_if<traits::AllOf<std::is_same<F, Fn>,
                                                       traits::Not<std::is_nothrow_move_constructible<Fn>>,
                                                       std::is_copy_constructible<Fn>>::value,
                                         bool>::type = 0>
        ScopeFail( ScopeFail<F>&& rhs ) noexcept( std::is_nothrow_copy_constructible<Fn>::value )
          : Base( std::false_type(), std::forward<F>( rhs.callback() ) )
          , exceptions_on_entry_ { uncaught_exceptions() }
        {}
        ~ScopeFail() noexcept
        {
          if ( exceptions_on_entry_ < uncaught_exceptions() ) {
            (void)( this->callback()() );
            this->release();
          }
        }

        PGBAR__FORCEINLINE void release() noexcept
        {
          exceptions_on_entry_ = ( std::numeric_limits<int>::max )();
        }
      };

#if PGBAR__CXX17
      template<typename F>
      ScopeFail( F fn ) -> ScopeFail<F>;
#endif

      // The move construction implementation can directly use `auto var = std::move( /* guard */ )`,
      // so the corresponding factory function overload is not provided.
      template<typename F>
      PGBAR__NODISCARD ScopeFail<F> make_scope_fail( F&& fn )
        noexcept( std::is_nothrow_constructible<ScopeFail<F>, F&&>::value )
      {
        return ScopeFail<F>( std::forward<F>( fn ) );
      }
    } // namespace utils
  } // namespace _details
} // namespace pgbar

#endif
