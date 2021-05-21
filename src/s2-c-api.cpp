
#include <vector>

#include "s2/s2projections.h"
#include "s2/s2edge_tessellator.h"

typedef struct s2_projection_t s2_projection_t;
typedef struct s2_tessellator_t s2_tessellator_t;

#ifdef __cplusplus
extern "C" {
#endif

s2_projection_t* s2_projection_create_plate_carree(double scale);
s2_projection_t* s2_projection_create_mercator(double max_x);
void s2_projection_destroy(s2_projection_t* projection);
int s2_projection_project(s2_projection_t* projection, const double* coord_in, double* coord_out);
int s2_projection_unproject(s2_projection_t* projection, const double* coord_in, double* coord_out);

s2_tessellator_t* s2_tessellator_create(s2_projection_t* projection, double tolerance_radians);
void s2_tessellator_destroy(s2_tessellator_t* tessellator);
int s2_tessellator_reset(s2_tessellator_t* tessellator);
int s2_tessellator_add_r2_point(s2_tessellator_t* tessellator, const double* coord);
int s2_tessellator_add_s2_point(s2_tessellator_t* tessellator, const double* coord);
int s2_tessellator_r2_points_size(s2_tessellator_t* tessellator);
int s2_tessellator_s2_points_size(s2_tessellator_t* tessellator);
int s2_tessellator_r2_point(s2_tessellator_t* tessellator, int i, double* coord);
int s2_tessellator_s2_point(s2_tessellator_t* tessellator, int i, double* coord);

#ifdef __cplusplus
}
#endif

s2_projection_t* s2_projection_create_plate_carree(double scale) {
    return (s2_projection_t*) new S2::PlateCarreeProjection(scale);
}

s2_projection_t* s2_projection_create_mercator(double max_x) {
    return (s2_projection_t*) new S2::MercatorProjection(max_x);
}

void s2_projection_destroy(s2_projection_t* projection) {
    if (projection != nullptr) {
        delete ((S2::Projection*) projection);
    }
}

int s2_projection_project(s2_projection_t* projection, const double* coord_in, double* coord_out) {
    S2Point p(coord_in[0], coord_in[1], coord_in[2]);
    R2Point result = ((S2::Projection*) projection)->Project(p);
    coord_out[0] = result.x();
    coord_out[1] = result.y();
    return true;
}

int s2_projection_unproject(s2_projection_t* projection, const double* coord_in, double* coord_out) {
    R2Point p(coord_in[0], coord_in[1]);
    S2Point result = ((S2::Projection*) projection)->Unproject(p);
    coord_out[0] = result.x();
    coord_out[1] = result.y();
    coord_out[2] = result.z();
    return true;
}

// Wrapper class around a tessellator that also keeps coordinate buffers
// and methods to iterate through them.
class TessellatorWrapper {
public:
    TessellatorWrapper(s2_projection_t* projection, double tolerance_radians):
        tessellator((S2::Projection*) projection, S1Angle::Radians(tolerance_radians)),
        has_r2_last(false), has_s2_last(false) {}
    
    void reset() {
        s2points.clear();
        r2points.clear();
        has_r2_last = false;
        has_s2_last = false;
    }

    void add_r2_point(const double* coord) {
        R2Point pt(coord[0], coord[1]);
        if (has_r2_last) {
            this->tessellator.AppendUnprojected(r2last, pt, &(this->s2points));
        }
        this->r2last = pt;
        this->has_r2_last = true;
    }

    void add_s2_point(const double* coord) {
        S2Point pt(coord[0], coord[1], coord[2]);
        if (has_s2_last) {
            this->tessellator.AppendProjected(s2last, pt, &(this->r2points));
        }
        this->s2last = pt;
        this->has_s2_last = true;
    }

    int r2_points_size() {
        return this->r2points.size();
    }

    void r2_point(int i, double* coord) {
        const R2Point& pt = r2points[i];
        coord[0] = pt.x();
        coord[1] = pt.y();
    }

    int s2_points_size() {
        return this->s2points.size();
    }

    void s2_point(int i, double* coord) {
        const S2Point& pt = s2points[i];
        coord[0] = pt.x();
        coord[1] = pt.y();
        coord[2] = pt.z();
    }

private:
    S2EdgeTessellator tessellator;
    std::vector<S2Point> s2points;
    std::vector<R2Point> r2points;
    R2Point r2last;
    S2Point s2last;
    bool has_r2_last;
    bool has_s2_last;
};

s2_tessellator_t* s2_tessellator_create(s2_projection_t* projection, double tolerance_radians) {
    return (s2_tessellator_t*) new TessellatorWrapper(projection, tolerance_radians);
}

void s2_tessellator_destroy(s2_tessellator_t* tessellator) {
    if (tessellator != nullptr) {
        delete ((TessellatorWrapper*) tessellator);
    }
}

int s2_tessellator_reset(s2_tessellator_t* tessellator) {
    ((TessellatorWrapper*) tessellator)->reset();
    return true;
}

int s2_tessellator_add_r2_point(s2_tessellator_t* tessellator, const double* coord) {
    ((TessellatorWrapper*) tessellator)->add_r2_point(coord);
    return true;
}

int s2_tessellator_add_s2_point(s2_tessellator_t* tessellator, const double* coord) {
    ((TessellatorWrapper*) tessellator)->add_s2_point(coord);
    return true;
}

int s2_tessellator_r2_points_size(s2_tessellator_t* tessellator) {
    return ((TessellatorWrapper*) tessellator)->r2_points_size();
}

int s2_tessellator_s2_points_size(s2_tessellator_t* tessellator) {
    return ((TessellatorWrapper*) tessellator)->s2_points_size();
}

int s2_tessellator_r2_point(s2_tessellator_t* tessellator, int i, double* coord) {
    ((TessellatorWrapper*) tessellator)->r2_point(i, coord);
    return true;
}

int s2_tessellator_s2_point(s2_tessellator_t* tessellator, int i, double* coord) {
    ((TessellatorWrapper*) tessellator)->s2_point(i, coord);
    return true;
}
