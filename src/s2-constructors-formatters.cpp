
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include "s2/s2projections.h"
#include "s2/s2pointutil.h"

#include "wk-v1.h"
#include "geography.h"


#define CPP_START                         \
    char cpp_exception_error[8096];       \
    memset(cpp_exception_error, 0, 8096); \
    try {

#define CPP_END                                           \
    } catch (std::exception& e) {                         \
        strncpy(cpp_exception_error, e.what(), 8096 - 1); \
    }                                                     \
    Rf_error("%s", cpp_exception_error);                  \
    return R_NilValue;


// The other versions of CPP_START and CPP_END stack-allocate the
// error message buffer, which takes a non-trivial amount of time
// when done at this scale (at worst 4 times per coordinate). By
// keeping the buffer in the handler_data struct, we can call C++
// from every handler method without measurable overhead.
#define WK_METHOD_CPP_START                     \
    try {

#define WK_METHOD_CPP_END                                 \
    } catch (std::exception& e) {                         \
        strncpy(data->cpp_exception_error, e.what(), 8096 - 1); \
    }                                                     \
    Rf_error("%s", data->cpp_exception_error);            \
    return R_NilValue;

#define WK_METHOD_CPP_END_INT                                 \
    } catch (std::exception& e) {                         \
        strncpy(data->cpp_exception_error, e.what(), 8096 - 1); \
    }                                                     \
    Rf_error("%s", data->cpp_exception_error);            \
    return WK_ABORT;


typedef struct {
    s2geography::util::FeatureConstructor* builder;
    SEXP result;
    R_xlen_t feat_id;
    int coord_size;
    char cpp_exception_error[8096];
} builder_handler_t;


// TODO: Both of these allocate in a way that could longjmp and possibly leak memory
static inline void builder_result_append(builder_handler_t* data, SEXP value) {
    R_xlen_t current_size = Rf_xlength(data->result);
    if (data->feat_id >= current_size) {
        SEXP new_result = PROTECT(Rf_allocVector(VECSXP, current_size * 2 + 1));
        for (R_xlen_t i = 0; i < current_size; i++) {
            SET_VECTOR_ELT(new_result, i, VECTOR_ELT(data->result, i));
        }
        R_ReleaseObject(data->result);
        data->result = new_result;
        R_PreserveObject(data->result);
        UNPROTECT(1);
    }

    SET_VECTOR_ELT(data->result, data->feat_id, value);
    data->feat_id++;
}

static inline void builder_result_finalize(builder_handler_t* data) {
    R_xlen_t current_size = Rf_xlength(data->result);
    if (data->feat_id != current_size) {
        SEXP new_result = PROTECT(Rf_allocVector(VECSXP, data->feat_id));
        for (R_xlen_t i = 0; i < data->feat_id; i++) {
            SET_VECTOR_ELT(new_result, i, VECTOR_ELT(data->result, i));
        }
        R_ReleaseObject(data->result);
        data->result = new_result;
        R_PreserveObject(data->result);
        UNPROTECT(1);
    }
}

int builder_vector_start(const wk_vector_meta_t* meta, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;

  if (data->result != R_NilValue) {
      Rf_error("Destination vector was already allocated"); // # nocov
  }

  if (meta->size == WK_VECTOR_SIZE_UNKNOWN) {
      data->result = PROTECT(Rf_allocVector(VECSXP, 1024));
  } else {
      data->result = PROTECT(Rf_allocVector(VECSXP, meta->size));
  }
  R_PreserveObject(data->result);
  UNPROTECT(1);

  data->feat_id = 0;

  return WK_CONTINUE;
}

SEXP builder_vector_end(const wk_vector_meta_t* meta, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  builder_result_finalize(data);
  SEXP cls = PROTECT(Rf_allocVector(STRSXP, 2));
  SET_STRING_ELT(cls, 0, Rf_mkChar("s2_geography"));
  SET_STRING_ELT(cls, 1, Rf_mkChar("wk_vctr"));
  Rf_setAttrib(data->result, R_ClassSymbol, cls);
  UNPROTECT(1);
  return data->result;
}

int builder_feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START
  data->builder->feat_start();
  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_feature_null(void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  builder_result_append(data, R_NilValue);
  return WK_ABORT_FEATURE;
}

int builder_feature_end(const wk_vector_meta_t* meta, R_xlen_t feat_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START
  std::unique_ptr<s2geography::Geography> feat = data->builder->finish_feature();
  SEXP feature_xptr = PROTECT(RGeography::MakeXPtr(std::move(feat)));
  builder_result_append(data, feature_xptr);
  UNPROTECT(1);
  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_geometry_start(const wk_meta_t* meta, uint32_t part_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START

  auto geometry_type = static_cast<s2geography::util::GeometryType>(meta->geometry_type);

  int32_t size;
  if (meta->size == WK_SIZE_UNKNOWN) {
      size = -1;
  } else {
      size = meta->size;
  }

  if (meta->flags & WK_FLAG_HAS_Z && meta->flags & WK_FLAG_HAS_M) {
    data->coord_size = 4;
  } else if (meta->flags & WK_FLAG_HAS_Z) {
    data->coord_size = 3;
  } else if (meta->flags & WK_FLAG_HAS_M) {
    data->coord_size = 3;
  } else {
    data->coord_size = 2;
  }

  data->builder->geom_start(geometry_type, size);
  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_geometry_end(const wk_meta_t* meta, uint32_t part_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START
  data->builder->geom_end();
  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_ring_start(const wk_meta_t* meta, uint32_t size, uint32_t ring_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START

  if (size == WK_SIZE_UNKNOWN) {
      data->builder->ring_start(-1);
  } else {
      data->builder->ring_start(size);
  }

  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_ring_end(const wk_meta_t* meta, uint32_t size, uint32_t ring_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START
  data->builder->ring_end();
  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_coord(const wk_meta_t* meta, const double* coord, uint32_t coord_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START
  data->builder->coords(coord, 1, data->coord_size);
  return WK_CONTINUE;
  WK_METHOD_CPP_END_INT
}

int builder_error(const char* message, void* handler_data) {
  Rf_error("%s", message);
  return WK_ABORT;
}

void builder_deinitialize(void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  if (data->result != R_NilValue) {
    R_ReleaseObject(data->result);
    data->result = R_NilValue;
  }
}

void builder_finalize(void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  if (data != nullptr) {
    free(data);
  }
}

void delete_vector_constructor(SEXP xptr) {
    auto ptr = reinterpret_cast<s2geography::util::FeatureConstructor*>(R_ExternalPtrAddr(xptr));
    if (ptr != nullptr) {
        delete ptr;
    }
}

extern "C" SEXP c_s2_geography_writer_new(SEXP oriented_sexp, SEXP check_sexp,
                                          SEXP projection_xptr,
                                          SEXP tessellate_tolerance_sexp) {
  CPP_START

  int oriented = LOGICAL(oriented_sexp)[0];
  int check = LOGICAL(check_sexp)[0];
  S2::Projection* projection = NULL;
  if (projection_xptr != R_NilValue) {
    projection = reinterpret_cast<S2::Projection*>(R_ExternalPtrAddr(projection_xptr));
  }
  double tessellate_tolerance = REAL(tessellate_tolerance_sexp)[0];

  s2geography::util::Constructor::Options options;
  options.set_oriented(oriented);
  options.set_check(check);
  options.set_projection(projection);
  if (tessellate_tolerance != R_PosInf) {
    options.set_tessellate_tolerance(S1Angle::Radians(tessellate_tolerance));
  }

  auto builder = new s2geography::util::FeatureConstructor(options);
  SEXP builder_xptr = PROTECT(R_MakeExternalPtr(builder, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(builder_xptr, &delete_vector_constructor);

  wk_handler_t* handler = wk_handler_create();

  handler->vector_start = &builder_vector_start;
  handler->vector_end = &builder_vector_end;

  handler->feature_start = &builder_feature_start;
  handler->null_feature = &builder_feature_null;
  handler->feature_end = &builder_feature_end;

  handler->geometry_start = &builder_geometry_start;
  handler->geometry_end = &builder_geometry_end;

  handler->ring_start = &builder_ring_start;
  handler->ring_end = &builder_ring_end;

  handler->coord = &builder_coord;

  handler->error = &builder_error;

  handler->deinitialize = &builder_deinitialize;
  handler->finalizer = &builder_finalize;

  builder_handler_t* data = (builder_handler_t*) malloc(sizeof(builder_handler_t));
  if (data == NULL) {
    wk_handler_destroy(handler); // # nocov
    Rf_error("Failed to alloc handler data"); // # nocov
  }

  data->coord_size = 2;
  data->builder = builder;
  data->result = R_NilValue;
  memset(data->cpp_exception_error, 0, 8096);

  handler->handler_data = data;

  // include the builder pointer as a tag for this external pointer
  // which guarnatees that it will not be garbage collected until
  // this object is garbage collected
  SEXP handler_xptr = wk_handler_create_xptr(handler, builder_xptr, projection_xptr);
  UNPROTECT(1);
  return handler_xptr;

  CPP_END
}

// The following defines exporting...it will hopefully be subsumed by a more general
// approach supported by geoarrow and/or s2geography

#define HANDLE_OR_RETURN(expr)                                 \
    result = expr;                                             \
    if (result != WK_CONTINUE) return result

#define HANDLE_CONTINUE_OR_BREAK(expr)                         \
    result = expr;                                             \
    if (result == WK_ABORT_FEATURE) continue; else if (result == WK_ABORT) break

class S2Exporter {
public:

  void set_vector_meta_flags(wk_vector_meta_t* vector_meta) {
    vector_meta->flags |= WK_FLAG_HAS_Z;
  }

  void set_meta_flags(wk_meta_t* vector_meta) {
    vector_meta->flags |= WK_FLAG_HAS_Z;
  }

  int coord_point(const wk_meta_t* meta, const S2Point& point, uint32_t coord_id, wk_handler_t* handler) {
    int result;
    coord_[0] = point.x();
    coord_[1] = point.y();
    coord_[2] = point.z();
    HANDLE_OR_RETURN(handler->coord(meta, coord_, coord_id, handler->handler_data));
    return WK_CONTINUE;
  }

  void reset() {
    coord_id_ = -1;
  }

  int coord_in_series(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    coord_id_++;
    return coord_point(meta, point, coord_id_, handler);
  }

  int last_coord_in_series(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    coord_id_++;
    return coord_point(meta, point, coord_id_, handler);
  }

  int last_coord_in_loop(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    coord_id_++;
    return coord_point(meta, point, coord_id_, handler);
  }

protected:
  int32_t coord_id_;
  double coord_[4];
};

class SimpleExporter: public S2Exporter {
public:
  SimpleExporter(const s2geography::util::Constructor::Options& options): options_(options) {}

  void set_vector_meta_flags(wk_vector_meta_t* vector_meta) {}

  void set_meta_flags(wk_meta_t* vector_meta) {}

  int coord_point(const wk_meta_t* meta, const S2Point& point, uint32_t coord_id, wk_handler_t* handler) {
    int result;
    R2Point out = options_.projection()->Project(point);
    coord_[0] = out.x();
    coord_[1] = out.y();
    HANDLE_OR_RETURN(handler->coord(meta, coord_, coord_id, handler->handler_data));
    return WK_CONTINUE;
  }

  void reset() {
    coord_id_ = -1;
  }

  int coord_in_series(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    coord_id_++;
    return coord_point(meta, point, coord_id_, handler);
  }

  int last_coord_in_series(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    coord_id_++;
    return coord_point(meta, point, coord_id_, handler);
  }

  int last_coord_in_loop(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    coord_id_++;
    return coord_point(meta, point, coord_id_, handler);
  }

private:
  s2geography::util::Constructor::Options options_;
};

class TessellatingExporter {
public:
  TessellatingExporter(const s2geography::util::Constructor::Options& options):
    options_(options),
    tessellator_(new S2EdgeTessellator(options.projection(), options.tessellate_tolerance())) {}

  void set_vector_meta_flags(wk_vector_meta_t* vector_meta) {}

  void set_meta_flags(wk_meta_t* vector_meta) {}

  int coord_point(const wk_meta_t* meta, const S2Point& point, uint32_t coord_id, wk_handler_t* handler) {
    int result;
    R2Point out = options_.projection()->Project(point);
    coord_[0] = out.x();
    coord_[1] = out.y();
    HANDLE_OR_RETURN(handler->coord(meta, coord_, coord_id, handler->handler_data));
    return WK_CONTINUE;
  }

  void reset() {
    points_out_.clear();
    is_first_point_ = true;
  }

  int coord_in_series(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    if (is_first_point_) {
      is_first_point_ = false;
      most_recent_ = point;
      first_in_loop_ = point;
      return WK_CONTINUE;
    }

    tessellator_->AppendProjected(most_recent_, point, &points_out_);
    most_recent_ = point;
    return WK_CONTINUE;
  }

  int last_coord_in_series(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    int result;
    HANDLE_OR_RETURN(coord_in_series(meta, point, handler));

    for (int i = 0; i < points_out_.size(); i++) {
      coord_[0] = points_out_[i].x();
      coord_[1] = points_out_[i].y();
      HANDLE_OR_RETURN(handler->coord(meta, coord_, i, handler->handler_data));
    }

    return WK_CONTINUE;
  }

  int last_coord_in_loop(const wk_meta_t* meta, const S2Point& point, wk_handler_t* handler) {
    int result;
    HANDLE_OR_RETURN(coord_in_series(meta, point, handler));

    for (int i = 0; i < (points_out_.size() - 1); i++) {
      coord_[0] = points_out_[i].x();
      coord_[1] = points_out_[i].y();
      HANDLE_OR_RETURN(handler->coord(meta, coord_, i, handler->handler_data));
    }

    if (!is_first_point_) {
      HANDLE_OR_RETURN(coord_point(meta, point, points_out_.size() - 1, handler));
    }

    return WK_CONTINUE;
  }

private:
  s2geography::util::Constructor::Options options_;
  std::unique_ptr<S2EdgeTessellator> tessellator_;
  bool is_first_point_;
  S2Point first_in_loop_;
  S2Point most_recent_;
  std::vector<R2Point> points_out_;
  double coord_[4];
};

template <typename EdgeExporterT>
int handle_points(const s2geography::PointGeography& geog,
                  EdgeExporterT* exporter,
                  wk_handler_t* handler,
                  uint32_t part_id = WK_PART_ID_NONE) {
  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_MULTIPOINT);
  meta.size = geog.Points().size();
  exporter->set_meta_flags(&meta);

  wk_meta_t meta_child;
  WK_META_RESET(meta_child, WK_POINT);
  meta_child.size = 1;
  exporter->set_meta_flags(&meta_child);

  if (meta.size == 0) {
    meta_child.size = 0;
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else if (meta.size == 1) {
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(exporter->coord_point(&meta_child, geog.Points()[0], 0, handler));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else {
    HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));

    for (size_t i = 0; i < geog.Points().size(); i++) {
      HANDLE_OR_RETURN(handler->geometry_start(&meta_child, i, handler->handler_data));
      HANDLE_OR_RETURN(exporter->coord_point(&meta_child, geog.Points()[i], 0, handler));
      HANDLE_OR_RETURN(handler->geometry_end(&meta_child, i, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));
  }

  return WK_CONTINUE;
}

template <typename EdgeExporterT>
int handle_polylines(const s2geography::PolylineGeography& geog,
                     EdgeExporterT* exporter,
                     wk_handler_t* handler,
                     uint32_t part_id = WK_PART_ID_NONE) {
  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_MULTILINESTRING);
  meta.size = geog.Polylines().size();
  exporter->set_meta_flags(&meta);

  wk_meta_t meta_child;
  WK_META_RESET(meta_child, WK_LINESTRING);
  exporter->set_meta_flags(&meta_child);

  if (meta.size == 0) {
    meta_child.size = 0;
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else if (meta.size == 1) {
    const S2Polyline& poly = *geog.Polylines()[0];
    meta_child.size = poly.num_vertices();
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));

    exporter->reset();
    for (int j = 0; j < poly.num_vertices(); j++) {
      if (j < (poly.num_vertices() - 1)) {
        HANDLE_OR_RETURN(exporter->coord_in_series(&meta_child, poly.vertex(j), handler));
      } else {
        HANDLE_OR_RETURN(exporter->last_coord_in_series(&meta_child, poly.vertex(j), handler));
      }
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else {
    HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));

    for (size_t i = 0; i < geog.Polylines().size(); i++) {
      const S2Polyline& poly = *geog.Polylines()[i];
      meta_child.size = poly.num_vertices();
      HANDLE_OR_RETURN(handler->geometry_start(&meta_child, i, handler->handler_data));

      exporter->reset();
      for (int j = 0; j < poly.num_vertices(); j++) {
        if (j < (poly.num_vertices() - 1)) {
          HANDLE_OR_RETURN(exporter->coord_in_series(&meta_child, poly.vertex(j), handler));
        } else {
          HANDLE_OR_RETURN(exporter->last_coord_in_series(&meta_child, poly.vertex(j), handler));
        }
      }

      HANDLE_OR_RETURN(handler->geometry_end(&meta_child, i, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));
  }

  return WK_CONTINUE;
}

template <typename EdgeExporterT>
int handle_loop_shell(const S2Loop* loop,
                      EdgeExporterT* exporter,
                      const wk_meta_t* meta, uint32_t loop_id, wk_handler_t* handler) {
  int result;

  if (loop->num_vertices() == 0) {
    return handler->error("Unexpected S2Loop with 0 vertices", handler->handler_data);
  }

  HANDLE_OR_RETURN(handler->ring_start(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));

  exporter->reset();
  for (int i = 0; i < loop->num_vertices(); i++) {
    HANDLE_OR_RETURN(exporter->coord_in_series(meta, loop->vertex(i), handler));
  }
  HANDLE_OR_RETURN(exporter->last_coord_in_loop(meta, loop->vertex(loop->num_vertices()), handler));

  HANDLE_OR_RETURN(handler->ring_end(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));
  return WK_CONTINUE;
}

template <typename EdgeExporterT>
int handle_loop_hole(const S2Loop* loop,
                     EdgeExporterT* exporter,
                     const wk_meta_t* meta, uint32_t loop_id, wk_handler_t* handler) {
  int result;

  if (loop->num_vertices() == 0) {
    return handler->error("Unexpected S2Loop with 0 vertices", handler->handler_data);
  }

  HANDLE_OR_RETURN(handler->ring_start(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));

  exporter->reset();
  for (int i = loop->num_vertices() - 1; i >= 0; i--) {
    HANDLE_OR_RETURN(exporter->coord_in_series(meta, loop->vertex(i), handler));
  }
  HANDLE_OR_RETURN(exporter->last_coord_in_loop(meta, loop->vertex(loop->num_vertices() - 1), handler));

  HANDLE_OR_RETURN(handler->ring_end(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));
  return WK_CONTINUE;
}

template <typename EdgeExporterT>
int handle_shell(const S2Polygon& poly,
                 EdgeExporterT* exporter,
                 const wk_meta_t* meta, int loop_start, wk_handler_t* handler) {
  int result;
  const S2Loop* loop0 = poly.loop(loop_start);
  HANDLE_OR_RETURN(handle_loop_shell<EdgeExporterT>(loop0, exporter, meta, 0, handler));

  uint32_t loop_id = 1;
  for (int j = loop_start + 1; j <= poly.GetLastDescendant(loop_start); j++) {
    const S2Loop* loop = poly.loop(j);
    if (loop->depth() == (loop0->depth() + 1)) {
      HANDLE_OR_RETURN(handle_loop_hole<EdgeExporterT>(loop, exporter, meta, loop_id, handler));
      loop_id++;
    }
  }

  return WK_CONTINUE;
}

template <typename EdgeExporterT>
int handle_polygon(const s2geography::PolygonGeography& geog,
                   EdgeExporterT* exporter,
                   wk_handler_t* handler,
                   uint32_t part_id = WK_PART_ID_NONE) {
  const S2Polygon& poly = *geog.Polygon();

  // find the outer shells (loop depth = 0, 2, 4, etc.)
  std::vector<int> outer_shell_loop_ids;
  std::vector<int> outer_shell_loop_sizes;

  outer_shell_loop_ids.reserve(poly.num_loops());
  for (int i = 0; i < poly.num_loops(); i++) {
    if ((poly.loop(i)->depth() % 2) == 0) {
      outer_shell_loop_ids.push_back(i);
    }
  }

  // count the number of rings in each
  outer_shell_loop_sizes.reserve(outer_shell_loop_ids.size());
  for (const auto loop_start : outer_shell_loop_ids) {
    const S2Loop* loop0 = poly.loop(loop_start);
    int num_loops = 1;

    for (int j = loop_start + 1; j <= poly.GetLastDescendant(loop_start); j++) {
      const S2Loop* loop = poly.loop(j);
      num_loops += loop->depth() == (loop0->depth() + 1);
    }

    outer_shell_loop_sizes.push_back(num_loops);
  }

  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_MULTIPOLYGON);
  meta.size = outer_shell_loop_ids.size();
  exporter->set_meta_flags(&meta);

  wk_meta_t meta_child;
  WK_META_RESET(meta_child, WK_POLYGON);
  exporter->set_meta_flags(&meta_child);

  if (meta.size == 0) {
    meta_child.size = 0;
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else if (meta.size == 1) {
    meta_child.size = outer_shell_loop_sizes[0];
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handle_shell<EdgeExporterT>(poly, exporter, &meta_child, outer_shell_loop_ids[0], handler));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else {
    HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));

    for (size_t i = 0; i < outer_shell_loop_sizes.size(); i++) {
      meta_child.size = outer_shell_loop_sizes[i];
      HANDLE_OR_RETURN(handler->geometry_start(&meta_child, i, handler->handler_data));
      HANDLE_OR_RETURN(handle_shell<EdgeExporterT>(poly, exporter, &meta_child, outer_shell_loop_ids[i], handler));
      HANDLE_OR_RETURN(handler->geometry_end(&meta_child, i, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));
  }

  return WK_CONTINUE;
}

template <typename EdgeExporterT>
int handle_collection(const s2geography::GeographyCollection& geog,
                      EdgeExporterT* exporter,
                      wk_handler_t* handler,
                      uint32_t part_id = WK_PART_ID_NONE) {
  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_GEOMETRYCOLLECTION);
  meta.size = geog.Features().size();
  exporter->set_meta_flags(&meta);

  HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));
  for (size_t i = 0; i < geog.Features().size(); i++) {
    const s2geography::Geography* child_ptr = geog.Features()[i].get();

    auto child_point = dynamic_cast<const s2geography::PointGeography*>(child_ptr);
    if (child_point != nullptr) {
      HANDLE_OR_RETURN(handle_points<EdgeExporterT>(*child_point, exporter, handler, i));
      continue;
    }

    auto child_polyline = dynamic_cast<const s2geography::PolylineGeography*>(child_ptr);
    if (child_polyline != nullptr) {
      HANDLE_OR_RETURN(handle_polylines<EdgeExporterT>(*child_polyline, exporter, handler, i));
      continue;
    }

    auto child_polygon = dynamic_cast<const s2geography::PolygonGeography*>(child_ptr);
    if (child_polygon != nullptr) {
      HANDLE_OR_RETURN(handle_polygon<EdgeExporterT>(*child_polygon, exporter, handler, i));
      continue;
    }

    auto child_collection = dynamic_cast<const s2geography::GeographyCollection*>(child_ptr);
    if (child_collection != nullptr) {
      HANDLE_OR_RETURN(handle_collection<EdgeExporterT>(*child_collection, exporter, handler, i));
      continue;
    }

    return handler->error("Unsupported S2Geography subclass", handler->handler_data);
  }
  HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));

  return WK_CONTINUE;
}

template <typename EdgeExporterT>
SEXP handle_geography_templ(SEXP data, EdgeExporterT* exporter, wk_handler_t* handler) {
  R_xlen_t n_features = Rf_xlength(data);

    wk_vector_meta_t vector_meta;
    WK_VECTOR_META_RESET(vector_meta, WK_GEOMETRY);
    vector_meta.size = n_features;
    vector_meta.flags |= WK_FLAG_DIMS_UNKNOWN;
    exporter->set_vector_meta_flags(&vector_meta);

    if (handler->vector_start(&vector_meta, handler->handler_data) == WK_CONTINUE) {
      int result;
      SEXP item;

      for (R_xlen_t i = 0; i < n_features; i++) {
        item = VECTOR_ELT(data, i);

        HANDLE_CONTINUE_OR_BREAK(handler->feature_start(&vector_meta, i, handler->handler_data));

        if (item == R_NilValue) {
          HANDLE_CONTINUE_OR_BREAK(handler->null_feature(handler->handler_data));
        } else {
          auto item_ptr = reinterpret_cast<RGeography*>(R_ExternalPtrAddr(item));
          const s2geography::Geography* geog_ptr = &item_ptr->Geog();

          auto child_point = dynamic_cast<const s2geography::PointGeography*>(geog_ptr);
          if (child_point != nullptr) {
            HANDLE_CONTINUE_OR_BREAK(handle_points<EdgeExporterT>(*child_point, exporter, handler));
          } else {
            auto child_polyline = dynamic_cast<const s2geography::PolylineGeography*>(geog_ptr);
            if (child_polyline != nullptr) {
              HANDLE_CONTINUE_OR_BREAK(handle_polylines<EdgeExporterT>(*child_polyline, exporter, handler));
            } else {
              auto child_polygon = dynamic_cast<const s2geography::PolygonGeography*>(geog_ptr);
              if (child_polygon != nullptr) {
                HANDLE_CONTINUE_OR_BREAK(handle_polygon<EdgeExporterT>(*child_polygon, exporter, handler));
              } else {
                auto child_collection = dynamic_cast<const s2geography::GeographyCollection*>(geog_ptr);
                if (child_collection != nullptr) {
                  HANDLE_CONTINUE_OR_BREAK(handle_collection<EdgeExporterT>(*child_collection, exporter, handler));
                } else {
                  HANDLE_CONTINUE_OR_BREAK(
                    handler->error("Unsupported S2Geography subclass", handler->handler_data));
                }
              }
            }
          }
        }

        if (handler->feature_end(&vector_meta, i, handler->handler_data) == WK_ABORT) {
          break;
        }
      }
    }

    SEXP result = PROTECT(handler->vector_end(&vector_meta, handler->handler_data));
    UNPROTECT(1);
    return result;
}

template <typename T>
void finalize_cpp_xptr(SEXP xptr) {
  auto ptr = reinterpret_cast<T*>(R_ExternalPtrAddr(xptr));
  if (ptr != nullptr) {
    delete ptr;
  }
}

SEXP handle_geography(SEXP data, wk_handler_t* handler) {
  SEXP projection_xptr = Rf_getAttrib(data, Rf_install("s2_projection"));

  SEXP result;

  if (projection_xptr != R_NilValue) {
    auto projection = reinterpret_cast<S2::Projection*>(R_ExternalPtrAddr(projection_xptr));
    s2geography::util::Constructor::Options options;
    options.set_projection(projection);

    auto exporter = new SimpleExporter(options);
    SEXP exporter_shelter = PROTECT(R_MakeExternalPtr(exporter, R_NilValue, R_NilValue));
    R_RegisterCFinalizer(exporter_shelter, &finalize_cpp_xptr<SimpleExporter>);

    result = PROTECT(handle_geography_templ<SimpleExporter>(data, exporter, handler));
    UNPROTECT(2);
  } else {
    auto exporter = new S2Exporter();
    SEXP exporter_shelter = PROTECT(R_MakeExternalPtr(exporter, R_NilValue, R_NilValue));
    R_RegisterCFinalizer(exporter_shelter, &finalize_cpp_xptr<S2Exporter>);

    result = PROTECT(handle_geography_templ<S2Exporter>(data, exporter, handler));
    UNPROTECT(2);
  }

  return result;
}

extern "C" SEXP c_s2_handle_geography(SEXP data, SEXP handler_xptr) {
    return wk_handler_run_xptr(&handle_geography, data, handler_xptr);
}

SEXP handle_geography_tessellated(SEXP data, wk_handler_t* handler) {
  SEXP projection_xptr = Rf_getAttrib(data, Rf_install("s2_projection"));
  auto projection = reinterpret_cast<S2::Projection*>(R_ExternalPtrAddr(projection_xptr));
  SEXP tessellate_tolerance_sexp = Rf_getAttrib(data, Rf_install("s2_tessellate_tol"));
  double tessellate_tol = REAL(tessellate_tolerance_sexp)[0];

  s2geography::util::Constructor::Options options;
  options.set_projection(projection);
  options.set_tessellate_tolerance(S1Angle::Radians(tessellate_tol));

  auto exporter = new TessellatingExporter(options);
  SEXP exporter_shelter = PROTECT(R_MakeExternalPtr(exporter, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(exporter_shelter, &finalize_cpp_xptr<TessellatingExporter>);

  SEXP result = PROTECT(handle_geography_templ<TessellatingExporter>(data, exporter, handler));
  UNPROTECT(2);
  return result;
}

class OrthographicProjection: public S2::Projection {
public:
  OrthographicProjection(const S2LatLng& centre):
      centre_(centre) {
    z_axis_ = S2Point(0, 0, 1);
    y_axis_ = S2Point(0, 1, 0);
  }

  // Converts a point on the sphere to a projected 2D point.
  R2Point Project(const S2Point& p) const {
    S2Point out = S2::Rotate(p, z_axis_, -centre_.lng());
    out = S2::Rotate(out, y_axis_, centre_.lat());
    return R2Point(out.y(), out.z());
  }

  // Converts a projected 2D point to a point on the sphere.
  S2Point Unproject(const R2Point& p) const {
    double y = p.x();
    double z = p.y();
    double x = sqrt(1.0 - y * y - z * z);
    S2Point pp(x, y, z);
    S2Point out = S2::Rotate(pp, y_axis_, -centre_.lat());
    out = S2::Rotate(out, z_axis_, centre_.lng());
    return out;
  }

  R2Point FromLatLng(const S2LatLng& ll) const {
    return Project(ll.ToPoint());
  }

  S2LatLng ToLatLng(const R2Point& p) const {
    return S2LatLng(Unproject(p));
  }

  R2Point wrap_distance() const {return R2Point(0, 0); }

private:
  S2LatLng centre_;
  S2Point z_axis_;
  S2Point y_axis_;
};


extern "C" SEXP c_s2_handle_geography_tessellated(SEXP data, SEXP handler_xptr) {
    return wk_handler_run_xptr(&handle_geography_tessellated, data, handler_xptr);
}

extern "C" SEXP c_s2_projection_plate_carree(SEXP x_scale_sexp) {
  double x_scale = REAL(x_scale_sexp)[0];

  auto projection = new S2::PlateCarreeProjection(x_scale);
  SEXP xptr = PROTECT(R_MakeExternalPtr(projection, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(xptr, &finalize_cpp_xptr<S2::Projection>);
  UNPROTECT(1);
  return xptr;
}

extern "C" SEXP c_s2_projection_mercator(SEXP x_scale_sexp) {
  double x_scale = REAL(x_scale_sexp)[0];

  auto projection = new S2::MercatorProjection(x_scale);
  SEXP xptr = PROTECT(R_MakeExternalPtr(projection, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(xptr, &finalize_cpp_xptr<S2::Projection>);
  UNPROTECT(1);
  return xptr;
}

extern "C" SEXP c_s2_projection_orthographic(SEXP centre_sexp) {
  S2LatLng centre =
    S2LatLng::FromDegrees(REAL(centre_sexp)[1], REAL(centre_sexp)[0]);

  auto projection = new OrthographicProjection(centre);
  SEXP xptr = PROTECT(R_MakeExternalPtr(projection, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(xptr, &finalize_cpp_xptr<S2::Projection>);
  UNPROTECT(1);
  return xptr;
}
