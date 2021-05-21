
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include "wk-v1.h"

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

// Expose projections as external pointers so that they can theoretically be generated
// by other packages.
void s2_projection_xptr_destroy(SEXP projection_xptr) {
  s2_projection_destroy((s2_projection_t*) R_ExternalPtrAddr(projection_xptr));
}

SEXP c_s2_projection_plate_carree() {
  SEXP projection_xptr = PROTECT(R_MakeExternalPtr(s2_projection_create_plate_carree(180), R_NilValue, R_NilValue));
  R_RegisterCFinalizer(projection_xptr, &s2_projection_xptr_destroy);
  UNPROTECT(1);
  return projection_xptr;
}

SEXP c_s2_projection_mercator() {
  SEXP projection_xptr = PROTECT(R_MakeExternalPtr(s2_projection_create_mercator(20037508), R_NilValue, R_NilValue));
  R_RegisterCFinalizer(projection_xptr, &s2_projection_xptr_destroy);
  UNPROTECT(1);
  return projection_xptr;
}

// The s2_coord_filter at the C level is used both for projecting (i.e.,
// simple mapping of points from x, y to an x, y, z on the unit sphere)
// and tessellating (i.e., projecting AND adding additional points to
// ensure that "straight" lines in the current projection refer to the same
// line on the surface of the earth when unprojected on to the unit
// sphere. Going the other direction, points are added to ensure that
// great circle edges (S2's assumption) refer to the same line on the
// surface of the earth when exported to the specified projection.
// This is similar to D3's "adaptive resampling" which also handles
// segmentizing and projecting as a single operation.
typedef struct {
  s2_projection_t* projection;
  s2_tessellator_t* tessellator;
  wk_handler_t* next;
  wk_meta_t meta_copy;
  wk_vector_meta_t vector_meta_copy;
  int unproject;
  uint32_t coord_id;
} coord_filter_t;

static inline void modify_meta_for_filter(coord_filter_t* coord_filter, const wk_meta_t* meta) {
  memcpy(&(coord_filter->meta_copy), meta, sizeof(wk_meta_t));

  // if we're projecting we're going to end up with an XY point
  // if we're unprojecting, we're going to get an XYZ point
  if (coord_filter->unproject) {
    coord_filter->meta_copy.flags |= WK_FLAG_HAS_Z;
  } else {
    coord_filter->meta_copy.flags &= ~WK_FLAG_HAS_Z;
  }

  // if we're tesselating a polyline, the 'size' should be set to unknown
  if (meta->geometry_type == WK_LINESTRING) {
    coord_filter->meta_copy.size = WK_SIZE_UNKNOWN;
  }

  // any srid that initially existed is definitely no longer valid
  coord_filter->meta_copy.srid = WK_SRID_NONE;
}

void s2_coord_filter_initialize(int* dirty, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  *dirty = 1;
  coord_filter->next->initialize(&coord_filter->next->dirty, coord_filter->next->handler_data);
}

int s2_coord_filter_vector_start(const wk_vector_meta_t* meta, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;

  // if we're projecting we're going to end up with an XY point
  // if we're unprojecting, we're going to get an XYZ point
  memcpy(&coord_filter->vector_meta_copy, meta, sizeof(wk_vector_meta_t));
  if (coord_filter->unproject) {
    coord_filter->vector_meta_copy.flags |= WK_FLAG_HAS_Z;
  } else {
    coord_filter->vector_meta_copy.flags &= ~WK_FLAG_HAS_Z;
  }

  return coord_filter->next->vector_start(&(coord_filter->vector_meta_copy), coord_filter->next->handler_data);
}

SEXP s2_coord_filter_vector_end(const wk_vector_meta_t* meta, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  // use the modified vector meta from vector_start
  return coord_filter->next->vector_end(&(coord_filter->vector_meta_copy), coord_filter->next->handler_data);
}

int s2_coord_filter_feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  return coord_filter->next->feature_start(&(coord_filter->vector_meta_copy), feat_id, coord_filter->next->handler_data);
}

int s2_coord_filter_feature_null(void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  return coord_filter->next->null_feature(coord_filter->next->handler_data);
}

int s2_coord_filter_feature_end(const wk_vector_meta_t* meta, R_xlen_t feat_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  return coord_filter->next->feature_end(&(coord_filter->vector_meta_copy), feat_id, coord_filter->next->handler_data);
}

int s2_coord_filter_geometry_start(const wk_meta_t* meta, uint32_t part_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  modify_meta_for_filter(coord_filter, meta);
  if (coord_filter->tessellator) {
    s2_tessellator_reset(coord_filter->tessellator);
    coord_filter->coord_id = 0;
  }
  return coord_filter->next->geometry_start(&(coord_filter->meta_copy), part_id, coord_filter->next->handler_data);
}

int s2_coord_filter_geometry_end(const wk_meta_t* meta, uint32_t part_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  modify_meta_for_filter(coord_filter, meta);
  return coord_filter->next->geometry_end(&(coord_filter->meta_copy), part_id, coord_filter->next->handler_data);
}

int s2_coord_filter_ring_start(const wk_meta_t* meta, uint32_t size, uint32_t ring_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  modify_meta_for_filter(coord_filter, meta);
  if (coord_filter->tessellator) {
    s2_tessellator_reset(coord_filter->tessellator);
    coord_filter->coord_id = 0;
  }
  return coord_filter->next->ring_start(&(coord_filter->meta_copy), WK_SIZE_UNKNOWN, ring_id, coord_filter->next->handler_data);
}

int s2_coord_filter_ring_end(const wk_meta_t* meta, uint32_t size, uint32_t ring_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  modify_meta_for_filter(coord_filter, meta);
  return coord_filter->next->ring_end(&(coord_filter->meta_copy), WK_SIZE_UNKNOWN, ring_id, coord_filter->next->handler_data);
}

int s2_coord_filter_error(const char* message, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  return coord_filter->next->error(message, coord_filter->next->handler_data);
}

void s2_coord_filter_deinitialize(void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  coord_filter->next->deinitialize(coord_filter->next->handler_data);
}

void s2_coord_filter_finalize(void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  if (coord_filter != NULL) {
    s2_tessellator_destroy(coord_filter->tessellator);
    // coord_filter->projection is managed using an external pointer
    // coord_filter->next is managed using an external pointer
    free(coord_filter);
  }
}

// The main show for the coord filter! We define separate functions for the
// projecting and unprojecting case as it's difficult to keep the code
// clean otherwise.
int s2_coord_filter_coord_unproject(const wk_meta_t* meta, const double* coord, uint32_t coord_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  modify_meta_for_filter(coord_filter, meta);
  double coord_out[4];

  // if we're not tessellating, skip adding the edge to the tessellator
  if ((coord_filter->tessellator == NULL) || (meta->geometry_type == WK_POINT)) {
    s2_projection_unproject(coord_filter->projection, coord, coord_out);
    return coord_filter->next->coord(
      &(coord_filter->meta_copy),
      coord_out, coord_id, coord_filter->next->handler_data
    );
  }

  // Add this point to the tessellator
  s2_tessellator_add_r2_point(coord_filter->tessellator, coord);

  // If we have an edge in the tessellator, reguritate all the points
  // except the first one to the next handler. If we have zero points,
  // we send the it to the handler right away since we'll skip it when
  // the next point arrives.
  int size = s2_tessellator_s2_points_size(coord_filter->tessellator);
  int result;

  if (size < 2) {
    s2_projection_unproject(coord_filter->projection, coord, coord_out);
    result = coord_filter->next->coord(
      &(coord_filter->meta_copy),
      coord_out, coord_id, coord_filter->next->handler_data
    );
    coord_filter->coord_id++;

    if (result != WK_CONTINUE) {
      return result;
    }
  } else {
    for (int i = 1; i < size; i++) {
      s2_tessellator_s2_point(coord_filter->tessellator, i, coord_out);
      result = coord_filter->next->coord(
        &(coord_filter->meta_copy),
        coord_out, coord_filter->coord_id, coord_filter->next->handler_data
      );
      coord_filter->coord_id++;

      if (result != WK_CONTINUE) {
        return result;
      }
    }

    // Clear the tessellator and re-add this point to be ready for the next
    // point that forms an edge
    s2_tessellator_reset(coord_filter->tessellator);
    s2_tessellator_add_r2_point(coord_filter->tessellator, coord);
  }

  return WK_CONTINUE;
}

int s2_coord_filter_coord_project(const wk_meta_t* meta, const double* coord, uint32_t coord_id, void* handler_data) {
  coord_filter_t* coord_filter = (coord_filter_t*) handler_data;
  modify_meta_for_filter(coord_filter, meta);
  double coord_out[4];

  if ((coord_filter->tessellator == NULL) || (meta->geometry_type == WK_POINT)) {
    s2_projection_project(coord_filter->projection, coord, coord_out);
    return coord_filter->next->coord(
      &(coord_filter->meta_copy),
      coord_out, coord_id, coord_filter->next->handler_data
    );
  }

  s2_tessellator_add_s2_point(coord_filter->tessellator, coord);

  int size = s2_tessellator_r2_points_size(coord_filter->tessellator);
  int result;

  if (size < 2) {
    s2_projection_project(coord_filter->projection, coord, coord_out);
    result = coord_filter->next->coord(
      &(coord_filter->meta_copy),
      coord_out, coord_id, coord_filter->next->handler_data
    );
    coord_filter->coord_id++;

    if (result != WK_CONTINUE) {
      return result;
    }
  } else {
    for (int i = 1; i < size; i++) {
      s2_tessellator_r2_point(coord_filter->tessellator, i, coord_out);
      result = coord_filter->next->coord(
        &(coord_filter->meta_copy),
        coord_out, coord_filter->coord_id, coord_filter->next->handler_data
      );
      coord_filter->coord_id++;

      if (result != WK_CONTINUE) {
        return result;
      }
    }

    // Clear the tessellator and re-add this point to be ready for the next
    // point that forms an edge
    s2_tessellator_reset(coord_filter->tessellator);
    s2_tessellator_add_s2_point(coord_filter->tessellator, coord);
  }

  return WK_CONTINUE;
}

SEXP c_s2_coord_filter_new(SEXP handler_xptr, SEXP projection_xptr, SEXP unproject, SEXP tessellate_tol) {
  if (TYPEOF(handler_xptr) != EXTPTRSXP) {
    Rf_error("`handler` must be a wk_handler pointer");
  }

  if (TYPEOF(projection_xptr) != EXTPTRSXP) {
    Rf_error("`projection` must be an external pointer");
  }

  if (!IS_SIMPLE_SCALAR(unproject, LGLSXP)) {
    Rf_error("`unproject` must be TRUE or FALSE"); // # nocov
  }

  if (!IS_SIMPLE_SCALAR(tessellate_tol, REALSXP) || (REAL(tessellate_tol)[0] < 1e-9)) {
    Rf_error("`tessellate` must be a double() greter than 1e-9");
  }

  wk_handler_t* handler = wk_handler_create();

  handler->initialize = &s2_coord_filter_initialize;
  handler->vector_start = &s2_coord_filter_vector_start;
  handler->vector_end = &s2_coord_filter_vector_end;

  handler->feature_start = &s2_coord_filter_feature_start;
  handler->null_feature = &s2_coord_filter_feature_null;
  handler->feature_end = &s2_coord_filter_feature_end;

  handler->geometry_start = &s2_coord_filter_geometry_start;
  handler->geometry_end = &s2_coord_filter_geometry_end;

  handler->ring_start = &s2_coord_filter_ring_start;
  handler->ring_end = &s2_coord_filter_ring_end;

  handler->error = &s2_coord_filter_error;

  handler->deinitialize = &s2_coord_filter_deinitialize;
  handler->finalizer = &s2_coord_filter_finalize;

  coord_filter_t* coord_filter = (coord_filter_t*) malloc(sizeof(coord_filter_t));
  if (coord_filter == NULL) {
    wk_handler_destroy(handler); // # nocov
    Rf_error("Failed to alloc handler data"); // # nocov
  }

  coord_filter->projection = (s2_projection_t*) R_ExternalPtrAddr(projection_xptr);
  if (REAL(tessellate_tol)[0] < R_PosInf) {
    coord_filter->tessellator = s2_tessellator_create(coord_filter->projection, REAL(tessellate_tol)[0]);
  } else {
    coord_filter->tessellator = NULL;
  }
  
  coord_filter->unproject = LOGICAL(unproject)[0];
  if (coord_filter->unproject) {
    handler->coord = &s2_coord_filter_coord_unproject;
  } else {
    handler->coord = &s2_coord_filter_coord_project;
  }

  coord_filter->next = R_ExternalPtrAddr(handler_xptr);

  if (coord_filter->next->api_version != 1) {
    Rf_error("Can't run a wk_handler with api_version '%d'", coord_filter->next->api_version); // # nocov
  }

  handler->handler_data = coord_filter;

  // include the external pointers as a tag for this external pointer
  // which guarnatees that they will not be garbage collected until
  // this object is garbage collected
  return wk_handler_create_xptr(handler, handler_xptr, projection_xptr);
}
