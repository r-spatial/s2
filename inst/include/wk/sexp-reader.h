
#ifndef WK_SEXP_READER
#define WK_SEXP_READER

#include "wk/reader.h"
#include <Rcpp.h>
#include "wk/rcpp-io.h"

class WKSEXPReader: public WKReader {
public:
  WKSEXPReader(WKSEXPProvider& provider): WKReader(provider), provider(provider) {}

protected:
  WKSEXPProvider& provider;

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      this->readClassedGeometry(this->provider.feature(), PART_ID_NONE);
    }

    this->handler->nextFeatureEnd(featureId);
  }

  virtual void readClassedGeometry(SEXP item, uint32_t partId) {

    WKGeometryMeta meta;
    try {
      if (Rf_inherits(item, "wk_point")) {
        meta = this->readMeta<Rcpp::NumericMatrix>(item, WKGeometryType::Point);
      } else if (Rf_inherits(item, "wk_linestring")) {
        meta = this->readMeta<Rcpp::NumericMatrix>(item, WKGeometryType::LineString);
      } else if (Rf_inherits(item, "wk_polygon")) {
        meta = this->readMeta<Rcpp::List>(item, WKGeometryType::Polygon);
      } else if (Rf_inherits(item, "wk_multipoint"))  {
        meta = this->readMeta<Rcpp::List>(item, WKGeometryType::MultiPoint);
      } else if (Rf_inherits(item, "wk_multilinestring"))  {
        meta = this->readMeta<Rcpp::List>(item, WKGeometryType::MultiLineString);
      } else if (Rf_inherits(item, "wk_multipolygon"))  {
        meta = this->readMeta<Rcpp::List>(item, WKGeometryType::MultiPolygon);
      } else if (Rf_inherits(item, "wk_geometrycollection"))  {
        meta = this->readMeta<Rcpp::List>(item, WKGeometryType::GeometryCollection);
      } else {
        throw WKParseException("Expected object with class 'wk_<geometry_type>'");
      }
    } catch(WKParseException& e) {
      throw e;
    } catch (std::exception& e) {
      throw WKParseException(ErrorFormatter() << "Unexpected classed object: " << e.what());
    }

    this->readGeometry(item, meta, partId);
  }

  virtual void readGeometry(SEXP item, const WKGeometryMeta meta, uint32_t partId) {
    this->handler->nextGeometryStart(meta, partId);

    try {
      switch (meta.geometryType) {

      case WKGeometryType::Point:
        this->readPoint(item, meta);
        break;

      case WKGeometryType::LineString:
        this->readLinestring(item, meta);
        break;

      case WKGeometryType::Polygon:
        this->readPolygon(item, meta);
        break;

      case WKGeometryType::MultiPoint:
        this->readMultiPoint(item, meta);
        break;
      case WKGeometryType::MultiLineString:
        this->readMultiLineString(item, meta);
        break;

      case WKGeometryType::MultiPolygon:
        this->readMultiPolygon(item, meta);
        break;

      case WKGeometryType::GeometryCollection:
        this->readCollection(item, meta);
        break;

      default:
        throw WKParseException(
            ErrorFormatter() <<
              "Unrecognized geometry type: " <<
                meta.geometryType
        );
      }
    } catch (WKParseException& e) {
      throw e;
    } catch(std::exception& e) {
      throw WKParseException(ErrorFormatter() <<
        "Unexpected object for geometry type " << meta.geometryType << ": " <<
          e.what()
      );
    }

    this->handler->nextGeometryEnd(meta, partId);
  }

  virtual void readPoint(Rcpp::NumericMatrix item, WKGeometryMeta meta) {
    if (meta.size > 1) {
      throw WKParseException(
          ErrorFormatter() << "Expected matrix with 0 or 1 rows but found matrix with " <<
            meta.size << " rows"
      );
    }

    this->readCoordinates(item, meta);
  }

  virtual void readLinestring(Rcpp::NumericMatrix item, const WKGeometryMeta meta) {
    this->readCoordinates(item, meta);
  }

  virtual void readPolygon(Rcpp::List item, const WKGeometryMeta meta) {
    for (R_xlen_t i = 0; i < item.size(); i++) {
      try {
        Rcpp::NumericMatrix ring = item[i];
        this->handler->nextLinearRingStart(meta, ring.nrow(), i);
        this->readCoordinates(ring, meta);
        this->handler->nextLinearRingEnd(meta, ring.nrow(), i);
      } catch(WKParseException& e) {
        throw e;
      } catch(std::exception& e) {
        throw WKParseException(ErrorFormatter() << "Unexpected item in polygon list: " << e.what());
      }
    }
  }

  virtual void readMultiPoint(Rcpp::List item, const WKGeometryMeta meta)  {
    for (R_xlen_t i = 0; i < item.size(); i++) {
      WKGeometryMeta childMeta(WKGeometryType::Point, meta.hasZ, meta.hasM, meta.hasSRID);
      childMeta.srid = meta.srid;
      try {
        Rcpp::NumericMatrix childItem = item[i];
        childMeta.hasSize = true;
        childMeta.size = this->itemSize(childItem);
        this->readGeometry(childItem, childMeta, i);

      } catch (WKParseException& e) {
        throw e;
      } catch (std::exception& e) {
        throw WKParseException(
            ErrorFormatter() << "Unexpected object within multipoint: " <<
              e.what()
        );
      }
    }
  }

  virtual void readMultiLineString(Rcpp::List item, const WKGeometryMeta meta)  {
    for (R_xlen_t i = 0; i < item.size(); i++) {
      WKGeometryMeta childMeta(WKGeometryType::LineString, meta.hasZ, meta.hasM, meta.hasSRID);
      childMeta.srid = meta.srid;
      try {
        Rcpp::NumericMatrix childItem = item[i];
        childMeta.hasSize = true;
        childMeta.size = this->itemSize(childItem);
        this->readGeometry(childItem, childMeta, i);

      } catch (WKParseException& e) {
        throw e;
      } catch (std::exception& e) {
        throw WKParseException(
            ErrorFormatter() << "Unexpected object within multilinestring: " <<
              e.what()
        );
      }
    }
  }

  virtual void readMultiPolygon(Rcpp::List item, const WKGeometryMeta meta)  {
    for (R_xlen_t i = 0; i < item.size(); i++) {
      WKGeometryMeta childMeta(WKGeometryType::Polygon, meta.hasZ, meta.hasM, meta.hasSRID);
      childMeta.srid = meta.srid;
      try {
        Rcpp::List childItem = item[i];
        childMeta.hasSize = true;
        childMeta.size = this->itemSize(childItem);
        this->readGeometry(childItem, childMeta, i);

      } catch (WKParseException& e) {
        throw e;
      } catch (std::exception& e) {
        throw WKParseException(
            ErrorFormatter() << "Unexpected object within multipolygon: " <<
              e.what()
        );
      }
    }
  }

  virtual void readCollection(Rcpp::List item, const WKGeometryMeta meta) {
    for (R_xlen_t i = 0; i < item.size(); i++) {
      this->readClassedGeometry(item[i], i);
    }
  }

  virtual void readCoordinates(Rcpp::NumericMatrix coords, const WKGeometryMeta meta) {
    WKCoord coord;
    coord.hasZ = meta.hasZ;
    coord.hasM = meta.hasM;

    for (int i = 0; i < coords.nrow(); i++) {
      try {
      coord.x = coords(i, 0);
      coord.y = coords(i, 1);
      if (meta.hasZ && meta.hasM) {
        coord.z = coords(i, 2);
        coord.m = coords(i, 3);
      } else if (meta.hasZ) {
        coord.z = coords(i, 2);
      } else if (meta.hasM) {
        coord.m = coords(i, 2);
      }

      this->handler->nextCoordinate(meta, coord, i);
      } catch(WKParseException& e) {
        throw e;
      } catch(std::exception& e) {
        throw WKParseException(
            ErrorFormatter() <<
              "Error extracting coordinates from matrix: " <<
              e.what()
        );
      }
    }
  }

private:
  template <class T>
  WKGeometryMeta readMeta(T item, int geometryType) {
    uint32_t srid = WKGeometryMeta::SRID_NONE;
    bool hasSRID = false;
    bool hasZ = false;
    bool hasM = false;

    if (item.hasAttribute("srid")) {
      try {
        srid = item.attr("srid");
        hasSRID = srid != NA_INTEGER;
      } catch(std::exception& e) {
        throw WKParseException("'srid' attribute has unexpected type");
      }
    }

    if (item.hasAttribute("has_z")) {
      try {
        hasZ = item.attr("has_z");
      } catch(std::exception& e) {
        throw WKParseException("'has_z' attribute has unexpected type");
      }
    }

    if (item.hasAttribute("has_m")) {
      try {
        hasM = item.attr("has_m");
      } catch(std::exception& e) {
        throw WKParseException("'has_m' attribute has unexpected type");
      }
    }

    WKGeometryMeta meta(geometryType, hasZ, hasM, hasSRID);
    meta.srid = srid;
    meta.hasSize = true;

    try {
      meta.size = this->itemSize(item);
    } catch(std::exception& e) {
      // probably an unexpected type
      throw WKParseException(ErrorFormatter() << "Can't calculate size: " << e.what());
    }

    return meta;
  }

  uint32_t itemSize(Rcpp::NumericMatrix item) {
    return item.nrow();
  }

  uint32_t itemSize(Rcpp::List item) {
    return item.size();
  }
};

#endif
