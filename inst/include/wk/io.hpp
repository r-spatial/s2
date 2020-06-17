
#ifndef WK_IO_H
#define WK_IO_H

class WKProvider {
public:
  virtual bool seekNextFeature() = 0;
  virtual bool featureIsNull() = 0;
  virtual size_t nFeatures() = 0;
  virtual void reset() = 0;
  virtual ~WKProvider() {}
};

class WKExporter {
public:
  WKExporter(size_t size): size(size) {}
  virtual void prepareNextFeature() = 0;
  virtual void writeNull() = 0;
  virtual void writeNextFeature() = 0;
  size_t nFeatures() {
    return this->size;
  }

  virtual ~WKExporter() {}

private:
  size_t size;
};

#endif
