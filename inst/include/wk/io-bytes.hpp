
#ifndef WK_IO_BYTES_H
#define WK_IO_BYTES_H

#include <cstdint>
#include "wk/io.hpp"

class WKBytesUtils {
public:
  // https://github.com/r-spatial/sf/blob/master/src/wkb.cpp
  // https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
  template <typename T>
  static T swapEndian(T u) {
    union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;
    source.u = u;
    for (size_t k = 0; k < sizeof(T); k++)
      dest.u8[k] = source.u8[sizeof(T) - k - 1];
    return dest.u;
  }

  static char nativeEndian(void) {
    const int one = 1;
    unsigned char *cp = (unsigned char *) &one;
    return (char) *cp;
  }
};

class WKBytesProvider: public WKProvider {
public:
  virtual unsigned char readCharRaw() = 0;
  virtual double readDoubleRaw() = 0;
  virtual uint32_t readUint32Raw() = 0;
};

class WKBytesExporter: public WKExporter {
public:
  WKBytesExporter(size_t size): WKExporter(size) {}
  virtual size_t writeCharRaw(unsigned char value) = 0;
  virtual size_t writeDoubleRaw(double value) = 0;
  virtual size_t writeUint32Raw(uint32_t value) = 0;
};

#endif
