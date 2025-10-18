#ifndef PGBAR__OSTREAM
#define PGBAR__OSTREAM

#include "Stringbuf.hpp"
#if PGBAR__CXX20
# include <span>
#endif
#if PGBAR__WIN
# include "../console/TermContext.hpp"
# ifndef NOMINMAX
#  define NOMINMAX 1
# endif
# include <windows.h>
#elif PGBAR__UNIX
# include <unistd.h>
#else
# include <iostream>
#endif

namespace pgbar {
  namespace _details {
    namespace io {
      template<Channel Outlet>
      class OStream;
      template<Channel Outlet>
      OStream<Outlet>& flush( OStream<Outlet>& stream )
      {
        return stream.flush();
      }
      template<Channel Outlet>
      PGBAR__CXX20_CNSTXPR OStream<Outlet>& release( OStream<Outlet>& stream ) noexcept
      {
        stream.release();
        return stream;
      }

      /**
       * A helper output stream that writes the data to `stdout` or `stderr` directly.
       *
       * It holds a proprietary buffer
       * so that don't have to use the common output buffers in the standard library.
       *
       * If the local platform is neither `Windows` nor `unix-like`,
       * the class still uses the method `write` of `std::ostream` in standard library.
       */
      template<Channel Outlet>
      class OStream final : public Stringbuf {
        using Self = OStream;

#if PGBAR__WIN && !defined( PGBAR_UTF8 )
        std::vector<WCHAR> wb_buffer_;
        std::vector<types::Char> localized_;
#endif

        PGBAR__CXX20_CNSTXPR OStream() = default;

      public:
#if PGBAR__CXX20
        using SinkBuffer = std::span<const types::Char>;
#else
        using SinkBuffer = const std::vector<types::Char>&;
#endif

        static Self& itself() noexcept( std::is_nothrow_default_constructible<Stringbuf>::value )
        {
          static OStream instance;
          return instance;
        }

        static void writeout( SinkBuffer bytes )
        {
#if PGBAR__WIN
          types::Size total_written = 0;
          do {
            DWORD num_written = 0;
            if PGBAR__CXX17_CNSTXPR ( Outlet == Channel::Stdout ) {
              auto h_stdout = GetStdHandle( STD_OUTPUT_HANDLE );
              if ( h_stdout == INVALID_HANDLE_VALUE )
                PGBAR__UNLIKELY throw exception::SystemError(
                  "pgbar: cannot open the standard output stream" );
              WriteFile( h_stdout,
                         bytes.data() + total_written,
                         static_cast<DWORD>( bytes.size() - total_written ),
                         &num_written,
                         nullptr );
            } else {
              auto h_stderr = GetStdHandle( STD_ERROR_HANDLE );
              if ( h_stderr == INVALID_HANDLE_VALUE )
                PGBAR__UNLIKELY throw exception::SystemError(
                  "pgbar: cannot open the standard error stream" );
              WriteFile( h_stderr,
                         bytes.data() + total_written,
                         static_cast<DWORD>( bytes.size() - total_written ),
                         &num_written,
                         nullptr );
            }
            if ( num_written <= 0 )
              break; // ignore it
            total_written += static_cast<types::Size>( num_written );
          } while ( total_written < bytes.size() );
#elif PGBAR__UNIX
          types::Size total_written = 0;
          do {
            ssize_t num_written = 0;
            if PGBAR__CXX17_CNSTXPR ( Outlet == Channel::Stdout )
              num_written =
                write( STDOUT_FILENO, bytes.data() + total_written, bytes.size() - total_written );
            else
              num_written =
                write( STDERR_FILENO, bytes.data() + total_written, bytes.size() - total_written );
            if ( errno == EINTR )
              num_written = num_written < 0 ? 0 : num_written;
            else if ( num_written < 0 )
              break;
            total_written += static_cast<types::Size>( num_written );
          } while ( total_written < bytes.size() );
#else
          if PGBAR__CXX17_CNSTXPR ( Outlet == Channel::Stdout )
            std::cout.write( bytes.data(), bytes.size() ).flush();
          else
            std::cerr.write( bytes.data(), bytes.size() ).flush();
#endif
        }

        PGBAR__CXX20_CNSTXPR OStream( const Self& )           = delete;
        PGBAR__CXX20_CNSTXPR Self& operator=( const Self& ) & = delete;
        // Intentional non-virtual destructors.
        PGBAR__CXX20_CNSTXPR ~OStream()                       = default;

#if PGBAR__WIN && !defined( PGBAR_UTF8 )
        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void release() noexcept
        {
          Stringbuf::release();
          wb_buffer_.clear();
          wb_buffer_.shrink_to_fit();
          localized_.clear();
          localized_.shrink_to_fit();
        }

        PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR void clear() & noexcept
        {
          Stringbuf::clear();
          wb_buffer_.clear();
          localized_.clear();
        }
#endif

        PGBAR__INLINE_FN Self& flush() &
        {
          if ( this->buffer_.empty() )
            return *this;

#if PGBAR__WIN && !defined( PGBAR_UTF8 )
          if ( !console::TermContext<Outlet>::itself().connected() ) {
            writeout( this->buffer_ );
            Stringbuf::clear();
            return *this;
          }
          const auto codepage = GetConsoleOutputCP();
          if ( codepage == CP_UTF8 ) {
            writeout( this->buffer_ );
            Stringbuf::clear();
            return *this;
          }

          const auto wlen =
            MultiByteToWideChar( CP_UTF8, 0, buffer_.data(), static_cast<int>( buffer_.size() ), nullptr, 0 );
          PGBAR__ASSERT( wlen > 0 );
          wb_buffer_.resize( static_cast<types::Size>( wlen ) );
          MultiByteToWideChar( CP_UTF8,
                               0,
                               buffer_.data(),
                               static_cast<int>( buffer_.size() ),
                               wb_buffer_.data(),
                               wlen );

          const auto mblen =
            WideCharToMultiByte( codepage, 0, wb_buffer_.data(), wlen, nullptr, 0, nullptr, nullptr );
          PGBAR__ASSERT( mblen > 0 );
          localized_.resize( static_cast<types::Size>( mblen ) );
          WideCharToMultiByte( codepage,
                               0,
                               wb_buffer_.data(),
                               wlen,
                               localized_.data(),
                               mblen,
                               nullptr,
                               nullptr );
          writeout( localized_ );
#else
          writeout( this->buffer_ );
#endif
          clear();
          return *this;
        }

        friend PGBAR__INLINE_FN PGBAR__CXX20_CNSTXPR Self& operator<<( OStream& stream,
                                                                       OStream& ( *fnptr )(OStream&))
        {
          PGBAR__TRUST( fnptr != nullptr );
          return fnptr( stream );
        }
      };
    } // namespace io
  } // namespace _details
} // namespace pgbar

#endif
