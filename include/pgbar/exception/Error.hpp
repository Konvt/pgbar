#ifndef PGBAR_ERROR
#define PGBAR_ERROR

#include "../details/config/Core.hpp"
#include <exception>
#if __PGBAR_CXX17
# include <string_view>
#endif

namespace pgbar {
  namespace exception {
    /**
     * The base exception class.
     *
     * It should only takes the literal strings, otherwise it isn't well-defined.
     */
    class Error : public std::exception {
    protected:
#if __PGBAR_CXX17
      std::string_view message_;
#else
      const char* message_;
#endif

    public:
      template<std::size_t N>
      Error( const char ( &mes )[N] ) noexcept : message_ { mes }
      {}
      ~Error() override = default;
      __PGBAR_NODISCARD const char* what() const noexcept override
      {
#if __PGBAR_CXX17
        return message_.data();
#else
        return message_;
#endif
      }
    };

    // Exception for invalid function arguments.
    class InvalidArgument : public Error {
    public:
      using Error::Error;
      ~InvalidArgument() override = default;
    };

    // Exception for error state of object.
    class InvalidState : public Error {
    public:
      using Error::Error;
      ~InvalidState() override = default;
    };

    // Exception for local system error.
    class SystemError : public Error {
    public:
      using Error::Error;
      ~SystemError() override = default;
    };

  } // namespace exception
} // namespace pgbar

#endif
