#ifndef __PGBAR_TERMCONTEXT
#define __PGBAR_TERMCONTEXT

#include "../types/Types.hpp"
#include <atomic>
#if __PGBAR_WIN
# include <mutex>
# ifndef NOMINMAX
#  define NOMINMAX 1
# endif
# include <windows.h>
#else
# include <sys/ioctl.h>
# include <unistd.h>
#endif

namespace pgbar {
  namespace __details {
    namespace console {
      template<Channel Outlet>
      class TermContext {
        std::atomic<bool> cache_;

        TermContext() noexcept { detect(); }

      public:
        TermContext( const TermContext& )              = delete;
        TermContext& operator=( const TermContext& ) & = delete;

        ~TermContext() = default;

        static TermContext& itself() noexcept
        {
          static TermContext self;
          return self;
        }

        // Detect whether the specified output stream is bound to a terminal.
        bool detect() noexcept
        {
          const bool value = []() noexcept {
#if defined( PGBAR_INTTY ) || __PGBAR_UNKNOWN
            return true;
#elif __PGBAR_WIN
            HANDLE hConsole;
            if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
              hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
            else
              hConsole = GetStdHandle( STD_ERROR_HANDLE );
            if ( hConsole == INVALID_HANDLE_VALUE )
              __PGBAR_UNLIKELY return false;
            return GetFileType( hConsole ) == FILE_TYPE_CHAR;
#else
            return isatty( static_cast<int>( Outlet ) );
#endif
          }();
          cache_.store( value, std::memory_order_release );
          return value;
        }
        __PGBAR_INLINE_FN bool connected() const noexcept { return cache_.load( std::memory_order_acquire ); }

        /**
         * Enable virtual terminal processing on the specified output channel (Windows only).
         * Guaranteed to be thread-safe and performed only once.
         */
        void virtual_term() const noexcept
        {
#if __PGBAR_WIN && !defined( PGBAR_NOCOLOR ) && defined( ENABLE_VIRTUAL_TERMINAL_PROCESSING )
          static std::once_flag flag;
          std::call_once( flag, []() noexcept {
            HANDLE stream_handle =
              GetStdHandle( Outlet == Channel::Stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE );
            if ( stream_handle == INVALID_HANDLE_VALUE )
              __PGBAR_UNLIKELY return;

            DWORD mode {};
            if ( !GetConsoleMode( stream_handle, &mode ) )
              __PGBAR_UNLIKELY return;
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode( stream_handle, mode );
          } );
#endif
        }

        __PGBAR_NODISCARD types::Size width() noexcept
        {
          if ( !detect() )
            return 0;
#if __PGBAR_WIN
          HANDLE hConsole;
          if __PGBAR_CXX17_CNSTXPR ( Outlet == Channel::Stdout )
            hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
          else
            hConsole = GetStdHandle( STD_ERROR_HANDLE );
          if ( hConsole != INVALID_HANDLE_VALUE ) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if ( GetConsoleScreenBufferInfo( hConsole, &csbi ) )
              return csbi.srWindow.Right - csbi.srWindow.Left + 1;
          }
#elif __PGBAR_UNIX
          struct winsize ws;
          auto fd = static_cast<int>( Outlet );
          if ( ioctl( fd, TIOCGWINSZ, &ws ) != -1 )
            return ws.ws_col;
#endif
          return 100;
        }
      };
    } // namespace console
  } // namespace __details
} // namespace pgbar

#endif
