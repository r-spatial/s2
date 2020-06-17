
#ifndef WK_SEXP_WRITER_H
#define WK_SEXP_WRITER_H

#include "wk/writer.hpp"
#include "wk/rcpp-io.hpp"


class WKRcppSEXPWriter: public WKWriter {
public:
  WKRcppSEXPWriter(WKSEXPExporter& exporter): WKWriter(exporter), feature(R_NilValue), exporter(exporter) {}

protected:
  // I'm sure there's a way to do this without as much copying
  std::vector<Rcpp::List> stack;
  SEXP feature;
  Rcpp::NumericMatrix currentCoordinates;
  WKSEXPExporter& exporter;

  void nextFeatureStart(size_t featureId) {
    WKWriter::nextFeatureStart(featureId);
    this->stack.clear();
    feature = R_NilValue;
  }

  void nextGeometryStart(const WKGeometryMeta& meta, uint32_t partId) {
    // make sure meta has a valid size
    if (!meta.hasSize || meta.size == WKGeometryMeta::SIZE_UNKNOWN) {
      throw std::runtime_error("Can't write WKL wihout a valid meta.size");
    }

    // make a new geometry type based on the creation options
    this->newMeta = this->getNewMeta(meta);
    bool nestingMulti = false;

    switch (meta.geometryType) {

    case WKGeometryType::Point:
    case WKGeometryType::LineString:
      this->initCoords(meta, meta.size);
      if (!this->isNestingMulti()) {
        this->currentCoordinates.attr("class") = this->metaAsClass(this->newMeta);

        if (this->newMeta.hasSRID) {
          this->currentCoordinates.attr("srid") = this->newMeta.srid;
        }

        if (this->newMeta.hasZ) {
          this->currentCoordinates.attr("has_z") = true;
        }

        if (this->newMeta.hasM) {
          this->currentCoordinates.attr("has_m") = true;
        }
      }
      break;
    case WKGeometryType::Polygon:
    case WKGeometryType::MultiPoint:
    case WKGeometryType::MultiLineString:
    case WKGeometryType::MultiPolygon:
    case WKGeometryType::GeometryCollection:
      nestingMulti = this->isNestingMulti();
      this->stack.push_back(Rcpp::List(this->newMeta.size));
      if (this->newMeta.geometryType != WKGeometryType::Polygon || !nestingMulti) {
        this->stack[this->stack.size() - 1].attr("class") = this->metaAsClass(this->newMeta);

        if (this->newMeta.hasSRID) {
          this->stack[this->stack.size() - 1].attr("srid") = this->newMeta.srid;
        }

        if (this->newMeta.hasZ) {
          this->stack[this->stack.size() - 1].attr("has_z") = true;
        }

        if (this->newMeta.hasM) {
          this->stack[this->stack.size() - 1].attr("has_m") = true;
        }
      }
      break;

    default:
      throw WKParseException(
          ErrorFormatter() <<
            "Unrecognized geometry type: " <<
              meta.geometryType
      );
    }
  }

  void nextLinearRingStart(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->initCoords(meta, size);
  }

  void nextCoordinate(const WKGeometryMeta& meta, const WKCoord& coord, uint32_t coordId) {
    for (int i = 0; i < coord.size(); i++) {
      this->currentCoordinates(coordId, i) = coord[i];
    }
  }

  void nextLinearRingEnd(const WKGeometryMeta& meta, uint32_t size, uint32_t ringId) {
    this->stack[this->stack.size() - 1][ringId] = this->currentCoordinates;
  }

  void nextGeometryEnd(const WKGeometryMeta& meta, uint32_t partId) {
    SEXP geometry;

    switch (meta.geometryType) {

    case WKGeometryType::Point:
    case WKGeometryType::LineString:
      geometry = this->currentCoordinates;
      break;
    case WKGeometryType::Polygon:
    case WKGeometryType::MultiPoint:
    case WKGeometryType::MultiLineString:
    case WKGeometryType::MultiPolygon:
    case WKGeometryType::GeometryCollection:
      geometry = this->stack[this->stack.size() - 1];
      this->stack.pop_back();
      break;

    default:
      throw WKParseException(
          ErrorFormatter() <<
            "Unrecognized geometry type: " <<
              meta.geometryType
      );
    }

    if (this->stack.size() > 0) {
      this->stack[this->stack.size() - 1][partId] = geometry;
    } else {
      this->feature = geometry;
    }
  }

  void nextFeatureEnd(size_t featureId) {
    this->exporter.setFeature(this->feature);
    WKWriter::nextFeatureEnd(featureId);
  }

  std::string metaAsClass(const WKGeometryMeta& meta) {
    switch (meta.geometryType) {

    case WKGeometryType::Point:
      return "wk_point";
    case WKGeometryType::LineString:
      return "wk_linestring";
    case WKGeometryType::Polygon:
      return "wk_polygon";
    case WKGeometryType::MultiPoint:
      return "wk_multipoint";
    case WKGeometryType::MultiLineString:
      return "wk_multilinestring";
    case WKGeometryType::MultiPolygon:
      return "wk_multipolygon";
    case WKGeometryType::GeometryCollection:
      return "wk_geometrycollection";
    default:
      throw WKParseException(
          ErrorFormatter() <<
            "Unrecognized geometry type: " <<
              meta.geometryType
      );
    }
  }

  void initCoords(const WKGeometryMeta& meta, uint32_t size) {
    int coordSize = 2 + meta.hasZ + meta.hasM;
    currentCoordinates = Rcpp::NumericMatrix(size, coordSize);
  }

  bool isNestingMulti() {
    if (stack.size() > 0) {
        Rcpp::List nestingGeometry = this->stack[this->stack.size() - 1];
        if (nestingGeometry.hasAttribute("class")) {
        std::string nestingClass = Rcpp::as<std::string>(this->stack[this->stack.size() - 1].attr("class"));
        return nestingClass == "wk_multipoint" ||
          nestingClass == "wk_multilinestring" ||
          nestingClass == "wk_multipolygon";
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  bool isNestingCollection() {
    if (stack.size() > 0) {
      Rcpp::List nestingGeometry = this->stack[this->stack.size() - 1];
      if (nestingGeometry.hasAttribute("class")) {
        std::string nestingClass = Rcpp::as<std::string>(nestingGeometry.attr("class"));
        return nestingClass == "wk_geometrycollection";
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

private:
  static std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
    );
    return s;
  }
};

#endif
