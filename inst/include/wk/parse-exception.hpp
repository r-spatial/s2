
#ifndef WK_PARSE_EXCEPTION_H
#define WK_PARSE_EXCEPTION_H

#include <string>
#include <stdexcept>

class WKParseException: public std::runtime_error {
public:
  static const int CODE_UNSPECIFIED = 0;
  WKParseException(int code): std::runtime_error(""), exceptionCode(code) {}
  WKParseException(std::string message): std::runtime_error(message), exceptionCode(CODE_UNSPECIFIED) {}

  int code() {
    return this->exceptionCode;
  }

private:
  int exceptionCode;
};

#endif
