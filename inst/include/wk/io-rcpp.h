
#ifndef WK_IO_RCPP_H
#define WK_IO_RCPP_H

#include "wk/parse-exception.h"
#include "wk/io-bytes.h"
#include "wk/io-string.h"

#include <Rcpp.h>
using namespace Rcpp;

class WKRawVectorListProvider: public WKBytesProvider {
public:

  WKRawVectorListProvider(List container) {
    this->container = container;
    this->index = -1;
    this->featureNull = true;
    this->offset = 0;
  }

  unsigned char readCharRaw() {
    return readBinary<unsigned char>();
  }

  double readDoubleRaw() {
    return readBinary<double>();
  }

  uint32_t readUint32Raw() {
    return readBinary<uint32_t>();
  }

  bool seekNextFeature() {
    this->index += 1;
    if (this->index >= this->container.size()) {
      return false;
    }

    SEXP item = this->container[this->index];

    if (item == R_NilValue) {
      this->featureNull = true;
      this->data = RawVector::create();
    } else {
      this->featureNull = false;
      this->data = (RawVector)item;
    }

    this->offset = 0;
    return true;
  }

  bool featureIsNull() {
    return this->featureNull;
  }

  size_t nFeatures() {
    return container.size();
  }

private:
  List container;
  R_xlen_t index;
  RawVector data;
  size_t offset;
  bool featureNull;

  template<typename T>
  T readBinary() {
    // Rcout << "Reading " << sizeof(T) << " starting at " << this->offset << "\n";
    if ((this->offset + sizeof(T)) > ((size_t) this->data.size())) {
      throw WKParseException("Reached end of RawVector input");
    }

    T dst;
    memcpy(&dst, &(this->data[this->offset]), sizeof(T));
    this->offset += sizeof(T);
    return dst;
  }
};

class WKRawVectorListExporter: public WKBytesExporter {
public:
  List output;
  RawVector buffer;
  bool featureNull;

  R_xlen_t index;
  size_t offset;

  WKRawVectorListExporter(size_t size): WKBytesExporter(size) {
    this->featureNull = false;
    this->index = 0;
    this->offset = 0;
    output = List(size);
    this->setBufferSize(2048);
  }

  void prepareNextFeature() {
    this->offset = 0;
    this->featureNull = false;
  }

  void writeNull() {
    this->featureNull = true;
  }

  void writeNextFeature() {
    if (this->index >= output.size()) {
      stop("Attempt to set index out of range (WKRawVectorListExporter)");
    }

    if (this->featureNull) {
      this->output[this->index] = R_NilValue;
    } else if (this->offset == 0) {
      this->output[this->index] = RawVector::create();
    } else {
      this->output[this->index] = this->buffer[Range(0, this->offset - 1)];
    }

    this->index++;
  }

  void setBufferSize(R_xlen_t bufferSize) {
    if (bufferSize <= 0) {
      throw std::runtime_error("Attempt to set zero or negative buffer size");
    }

    RawVector newBuffer = RawVector(bufferSize);
    this->buffer = newBuffer;
  }

  void extendBufferSize(R_xlen_t bufferSize) {
    if (bufferSize < this->buffer.size()) {
      throw std::runtime_error("Attempt to shrink RawVector buffer size");
    }

    RawVector newBuffer = RawVector(bufferSize);
    for (size_t i=0; i < this->offset; i++) {
      newBuffer[i] = this->buffer[i];
    }

    this->buffer = newBuffer;
  }

  size_t writeCharRaw(unsigned char value) {
    return this->writeBinary<unsigned char>(value);
  }

  size_t writeDoubleRaw(double value) {
    return this->writeBinary<double>(value);
  }

  size_t writeUint32Raw(uint32_t value) {
    return this->writeBinary<uint32_t>(value);
  }

  template<typename T>
  size_t writeBinary(T value) {
    // Rcout << "Writing " << sizeof(T) << "(" << value << ") starting at " << this->offset << "\n";
    while ((this->offset + sizeof(T)) > ((size_t) this->buffer.size())) {
      // we're going to need a bigger boat
      this->extendBufferSize(this->buffer.size() * 2);
    }

    memcpy(&(this->buffer[this->offset]), &value, sizeof(T));
    this->offset += sizeof(T);
    return sizeof(T);
  }
};

class WKCharacterVectorProvider: public WKStringProvider {
public:
  CharacterVector container;
  R_xlen_t index;
  bool featureNull;
  std::string data;

  WKCharacterVectorProvider(CharacterVector container):
    container(container), index(-1), featureNull(false) {}

  bool seekNextFeature() {
    this->index++;
    if (this->index >= this->container.size()) {
      return false;
    }

    if (CharacterVector::is_na(this->container[this->index])) {
      this->featureNull = true;
      this->data = std::string("");
    } else {
      this->featureNull = false;
      this->data = as<std::string>(this->container[this->index]);
    }

    return true;
  }

  const std::string featureString() {
    return this->data;
  }

  bool featureIsNull() {
    return this->featureNull;
  }

  size_t nFeatures() {
    return container.size();
  }
};

class WKCharacterVectorExporter: public WKStringStreamExporter {
public:
  CharacterVector output;
  R_xlen_t index;
  bool featureNull;

  WKCharacterVectorExporter(size_t size):
    WKStringStreamExporter(size), output(size), index(0), featureNull(false) {}

  void prepareNextFeature() {
    this->featureNull = false;
  }

  void writeNull() {
    this->featureNull = true;
  }

  void writeNextFeature() {
    if (this->index >= output.size()) {
      stop("Attempt to set index out of range (WKCharacterVectorExporter)");
    }

    if (this->featureNull) {
      this->output[this->index] = NA_STRING;
    } else {
      this->output[this->index] = this->stream.str();
    }

    this->index++;
  }
};

#endif
