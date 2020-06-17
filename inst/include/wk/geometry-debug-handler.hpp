
#ifndef WK_GEOMETRY_DEBUG_HANDLER_H
#define WK_GEOMETRY_DEBUG_HANDLER_H

#include "wk/coord.hpp"
#include "wk/geometry-handler.hpp"
#include "wk/parse-exception.hpp"
#include "wk/geometry-meta.hpp"
#include "wk/geometry-debug-handler.hpp"

class WKGeometryDebugHandler: public WKGeometryHandler {
public:
  WKGeometryDebugHandler(std::ostream& out): out(out), indentationLevel(0) {}

  virtual void nextFeatureStart(size_t featureId) {
    this->indentationLevel = 0;
    this->indent();
    out << "nextFeatureStart(" << featureId <<  ")\n";
    this->indentationLevel++;
  }

  virtual void nextFeatureEnd(size_t featureId) {
    this->indentationLevel--;
    this->indent();
    out << "nextFeatureEnd(" << featureId <<  ")\n";
  }

  virtual void nextNull(size_t featureId) {
    this->indent();
    out << "nextNull(" << featureId <<  ")\n";
  }

  virtual void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    this->indent();
    out << "nextGeometryStart(";
    this->writeMeta(meta);
    out << ", ";
    this->writeMaybeUnknown(partId, "WKReader::PART_ID_NONE");
    out << ")\n";
    this->indentationLevel++;
  }

  virtual void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
    this->indentationLevel--;
    this->indent();
    out << "nextGeometryEnd(";
    this->writeMeta(meta);
    out << ", ";
    this->writeMaybeUnknown(partId, "WKReader::PART_ID_NONE");
    out << ")\n";
  }

  virtual void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->indent();
    out << "nextLinearRingStart(";
    this->writeMeta(meta);
    out << ", ";
    this->writeMaybeUnknown(size, "WKGeometryMeta::SIZE_UNKNOWN");
    out << ", " << ringId << ")\n";
    this->indentationLevel++;
  }

  virtual void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->indentationLevel--;
    this->indent();
    out << "nextLinearRingEnd(";
    this->writeMeta(meta);
    out << ", ";
    this->writeMaybeUnknown(size, "WKGeometryMeta::SIZE_UNKNOWN");
    out << ", " << ringId << ")\n";
  }

  virtual void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    this->indent();
    out << "nextCoordinate(";
    this->writeMeta(meta);
    out << ", " << "WKCoord(x = " << coord.x << ", y = " << coord.y;
    if (coord.hasZ) {
      out << ", z = " << coord.z;
    }

    if (coord.hasM) {
      out << ", m = " << coord.m;
    }

    out << "), " << coordId << ")\n";
  }

  virtual bool nextError(WKParseException& error, size_t featureId) {
    out << "nextError('" << error.what() << "', " << featureId << ")\n";
    return true;
  }

  virtual void writeMaybeUnknown(uint32_t value, const char* ifUnknown) {
    if (value == UINT32_MAX) {
      out << ifUnknown;
    } else {
      out << value;
    }
  }

  virtual void writeMeta(const WKGeometryMeta& meta) {
    this->writeGeometryType(meta.geometryType);
    if (meta.hasSRID) {
      out << " SRID=" << meta.srid;
    }

    if (meta.hasSize) {
      out << " [" << meta.size << "]";
    } else {
      out << " [unknown]";
    }
  }

  virtual void writeGeometryType(uint32_t simpleGeometryType) {
    switch (simpleGeometryType) {
    case WKGeometryType::Point:
      out << "POINT";
      break;
    case WKGeometryType::LineString:
      out << "LINESTRING";
      break;
    case WKGeometryType::Polygon:
      out << "POLYGON";
      break;
    case WKGeometryType::MultiPoint:
      out << "MULTIPOINT";
      break;
    case WKGeometryType::MultiLineString:
      out << "MULTILINESTRING";
      break;
    case WKGeometryType::MultiPolygon:
      out << "MULTIPOLYGON";
      break;
    case WKGeometryType::GeometryCollection:
      out << "GEOMETRYCOLLECTION";
      break;
    default:
      out << "Unknown Type (" << simpleGeometryType << ")";
      break;
    }
  }

  virtual void indent() {
    for (int i=0; i < indentationLevel; i++)  {
      out << "    ";
    }
  }

protected:
  std::ostream& out;
  int indentationLevel;
};

#endif
