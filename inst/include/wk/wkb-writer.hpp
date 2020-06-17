
#ifndef WK_WKB_WRITER_H
#define WK_WKB_WRITER_H

#include "wk/geometry-handler.hpp"
#include "wk/io-bytes.hpp"
#include "wk/writer.hpp"
#include "wk/wkb-reader.hpp"

class WKBWriter: public WKWriter {
public:
  WKBWriter(WKBytesExporter& exporter): WKWriter(exporter), exporter(exporter) {

  }

  void setEndian(unsigned char endian) {
    this->endian = endian;
    this->swapEndian = WKBytesUtils::nativeEndian() != endian;
  }

  virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    // make sure meta has a valid size
    if (!meta.hasSize || meta.size == WKGeometryMeta::SIZE_UNKNOWN) {
      throw std::runtime_error("Can't write WKB wihout a valid meta.size");
    }

    // make a new geometry type based on the creation options
    this->newMeta = this->getNewMeta(meta);

    this->writeEndian();
    this->writeUint32(this->newMeta.ewkbType());

    if (this->newMeta.hasSRID) this->writeUint32(this->newMeta.srid);
    if (this->newMeta.geometryType != WKGeometryType::Point) this->writeUint32(meta.size);

    // empty point hack! could also error here, but this feels more in line with
    // how these are represented in real life (certainly in R)
    if (this->newMeta.geometryType == WKGeometryType::Point && this->newMeta.size == 0) {
      this->writeDouble(NAN);
      this->writeDouble(NAN);
    }
  }

  virtual void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->writeUint32(size);
  }

  virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    this->writeDouble(coord.x);
    this->writeDouble(coord.y);
    if (this->newMeta.hasZ && coord.hasZ) {
      this->writeDouble(coord.z);
    }
    if (this->newMeta.hasM && coord.hasM) {
      this->writeDouble(coord.m);
    }
  }

private:
  bool swapEndian;
  unsigned char endian;
  WKBytesExporter& exporter;

  size_t writeEndian() {
    return this->writeChar(this->endian);
  }

  size_t writeCoord(WKCoord coord) {
    size_t bytesWritten = 0;
    for (size_t i=0; i < coord.size(); i++) {
      bytesWritten += this->writeDouble(coord[i]);
    }
    return bytesWritten;
  }

  size_t writeChar(unsigned char value) {
    return this->exporter.writeCharRaw(value);
  }

  size_t writeDouble(double value) {
    if (this->swapEndian) {
      this->exporter.writeDoubleRaw(WKBytesUtils::swapEndian<double>(value));
    } else {
      this->exporter.writeDoubleRaw(value);
    }
    return sizeof(double);
  }

  size_t writeUint32(uint32_t value) {
    if (this->swapEndian) {
      this->exporter.writeUint32Raw(WKBytesUtils::swapEndian<uint32_t>(value));
    } else {
      this->exporter.writeUint32Raw(value);
    }
    return sizeof(uint32_t);
  }
};

#endif
