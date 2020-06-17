
#ifndef WK_WKCOORD_H
#define WK_WKCOORD_H

#include <cmath>
#include <vector>
#include <stdexcept>

class WKCoord {
public:
  double x;
  double y;
  double z;
  double m;
  bool hasZ;
  bool hasM;

  WKCoord(): x(NAN), y(NAN), z(NAN), m(NAN), hasZ(false), hasM(false) {}
  WKCoord(double x, double y, double z, double m, bool hasZ, bool hasM):
    x(x), y(y), z(z), m(m), hasZ(hasZ), hasM(hasM) {}

  bool operator == (WKCoord& other) {
    if (this->hasZ != other.hasZ || this->hasM != other.hasM) {
      return false;
    }

    for (size_t i = 0; i < this->size(); i++) {
      if ((*this)[i] != other[i]) {
        return false;
      }
    }

    return true;
  }

  const double& operator[](std::size_t idx) const {
    switch (idx) {
    case 0: return x;
    case 1: return y;
    case 2:
      if (hasZ) {
        return z;
      } else if (hasM) {
        return m;
      }
    case 3:
      if (hasM) return m;
    default:
      throw std::runtime_error("Coordinate subscript out of range");
    }
  }

  const size_t size() const {
    return 2 + hasZ + hasM;
  }

  static const WKCoord xy(double x, double y) {
    return WKCoord(x, y, NAN, NAN, false, false);
  }

  static const WKCoord xyz(double x, double y, double z) {
    return WKCoord(x, y, z, NAN, true, false);
  }

  static const WKCoord xym(double x, double y, double m) {
    return WKCoord(x, y, NAN, m, false, true);
  }

  static const WKCoord xyzm(double x, double y, double z, double m) {
    return WKCoord(x, y, z, m, true, true);
  }
};

#endif
