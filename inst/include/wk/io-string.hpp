
#ifndef WK_IO_STRING_H
#define WK_IO_STRING_H

#include <locale>
#include <sstream>

#include "wk/io.hpp"

// for now, the only option is to provide a reference to a string
// the string tokenizer operates on a string iterator, which might be
// more flexible for the WKT reader but less flexible for other applications
class WKStringProvider: public WKProvider {
public:
  virtual const std::string featureString() = 0;
};

class WKStringExporter: public WKExporter {
public:
  WKStringExporter(size_t size): WKExporter(size) {}
  virtual void writeString(std::string value) = 0;
  virtual void writeConstChar(const char* value) = 0;
  virtual void writeDouble(double value) = 0;
  virtual void writeUint32(uint32_t value) = 0;
};

class WKStringStreamExporter: public WKStringExporter {
public:
  WKStringStreamExporter(size_t size): WKStringExporter(size) {
    this->stream.imbue(std::locale::classic());
  }

  void setRoundingPrecision(int precision) {
    this->stream.precision(precision);
  }

  void setTrim(bool trim) {
    if (trim) {
      this->stream.unsetf(stream.fixed);
    } else {
      this->stream.setf(stream.fixed);
    }
  }

  void writeString(std::string value) {
    this->stream << value;
  }

  void writeConstChar(const char* value) {
    this->stream << value;
  }

  void writeDouble(double value) {
    this->stream << value;
  }

  void writeUint32(uint32_t value) {
    this->stream << value;
  }

protected:
  std::stringstream stream;
};

#endif
