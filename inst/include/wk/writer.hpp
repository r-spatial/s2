
#ifndef WK_WRITER_H
#define WK_WRITER_H

#include "wk/geometry-handler.hpp"
#include "wk/geometry-meta.hpp"
#include "wk/io.hpp"

class WKWriter: public WKGeometryHandler {
public:
  // by default, leave everything as is!
  WKWriter(WKExporter& exporter): exporter(exporter), includeZ(2), includeM(2), includeSRID(2) {}

  virtual void nextFeatureStart(size_t featureId) {
    exporter.prepareNextFeature();
  }

  virtual void nextNull(size_t featureId) {
    exporter.writeNull();
  }

  virtual void nextFeatureEnd(size_t featureId) {
    exporter.writeNextFeature();
  }

  // creation options for all WKX formats
  void setIncludeSRID(int includeSRID) {
    this->includeSRID = includeSRID;
  }

  void setIncludeZ(int includeZ) {
    this->includeZ = includeZ;
  }

  void setIncludeM(int includeM) {
    this->includeM = includeM;
  }

protected:
  WKExporter& exporter;
  int includeZ;
  int includeM;
  int includeSRID;
  WKGeometryMeta newMeta;

  virtual WKGeometryMeta getNewMeta(const WKGeometryMeta& meta) {
    WKGeometryMeta newMeta(
      meta.geometryType,
      this->actuallyIncludeZ(meta),
      this->actuallyIncludeM(meta),
      this->actuallyIncludeSRID(meta)
    );

    newMeta.srid = meta.srid;
    newMeta.hasSize = meta.hasSize;
    newMeta.size = meta.size;

    return newMeta;
  }

  bool actuallyIncludeZ(const WKGeometryMeta& meta) {
    return actuallyInclude(this->includeZ, meta.hasZ, "Z");
  }

  bool actuallyIncludeM(const WKGeometryMeta& meta) {
    return actuallyInclude(this->includeM, meta.hasM, "M");
  }

  bool actuallyIncludeSRID(const WKGeometryMeta& meta) {
    return actuallyInclude(this->includeSRID, meta.hasSRID, "SRID");
  }

  bool actuallyInclude(int flag, bool hasValue, const char* label) {
    if (flag == 1 && !hasValue) {
      throw std::runtime_error(
        ErrorFormatter() << "Can't include " <<  label <<
          " values in a geometry for which " <<
          label << " values are not defined"
      );
    }

    return flag && hasValue;
  }
};

#endif

