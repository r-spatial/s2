
#include "s2/s2latlng.h"
#include "s2/s2shape_index.h"
#include "s2/mutable_s2shape_index.h"
#include "wk/geometry-handler.h"

class LibS2Geography {
public:

  // accessors need to be methods, since their calculation
  // depends on the geometry type
  virtual bool IsCollection() = 0;
  virtual int Dimension() = 0;
  virtual int NumPoints() = 0;
  virtual double Area() = 0;
  virtual double Length() = 0;
  virtual double Perimeter() = 0;
  virtual double X() = 0;
  virtual double Y() = 0;

  // every type will build the index differently based on
  // the underlying data, and this can (should?) be done
  // lazily
  virtual void BuildShapeIndex(MutableS2ShapeIndex* index) = 0;

  // the factory handler is responsible for building these objects
  // but exporting can be done here
  virtual void Export(WKGeometryHandler* handler, uint32_t partId) = 0;

  virtual ~LibS2Geography() {}

  // other calculations use ShapeIndex
  virtual S2ShapeIndex* ShapeIndex() {
    this->BuildShapeIndex(&this->shape_index_);
    return &this->shape_index_;
  };

protected:
  MutableS2ShapeIndex shape_index_;
};
