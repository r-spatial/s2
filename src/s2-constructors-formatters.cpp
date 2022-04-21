
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include "wk-v1.h"
#include "s2-geography/s2-geography.hpp"
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
    s2geography::VectorConstructor* builder;
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
  SET_STRING_ELT(cls, 1, Rf_mkChar("s2_xptr"));
  Rf_setAttrib(data->result, R_ClassSymbol, cls);
  UNPROTECT(1);
  return data->result;
}

int builder_feature_start(const wk_vector_meta_t* meta, R_xlen_t feat_id, void* handler_data) {
  builder_handler_t* data = (builder_handler_t*) handler_data;
  WK_METHOD_CPP_START
  data->builder->start_feature();
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
  std::unique_ptr<s2geography::S2Geography> feat = data->builder->finish_feature();
  builder_result_append(data, Geography::MakeXPtr(std::move(feat)));
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
    auto ptr = reinterpret_cast<s2geography::VectorConstructor*>(R_ExternalPtrAddr(xptr));
    if (ptr != nullptr) {
        delete ptr;
    }
}

extern "C" SEXP c_s2_geography_writer_new(SEXP oriented_sexp, SEXP check_sexp) {
  CPP_START

  int oriented = LOGICAL(oriented_sexp)[0];
  int check = LOGICAL(check_sexp)[0];

  s2geography::Constructor::Options options;
  options.set_oriented(oriented);
  options.set_check(check);

  auto builder = new s2geography::VectorConstructor(options);
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
  SEXP handler_xptr = wk_handler_create_xptr(handler, builder_xptr, R_NilValue);
  UNPROTECT(1);
  return handler_xptr;

  CPP_END
}

#define HANDLE_OR_RETURN(expr)                                 \
    result = expr;                                             \
    if (result != WK_CONTINUE) return result

#define HANDLE_CONTINUE_OR_BREAK(expr)                         \
    result = expr;                                             \
    if (result == WK_ABORT_FEATURE) continue; else if (result == WK_ABORT) break


int handle_points(const s2geography::S2GeographyOwningPoint& geog, wk_handler_t* handler,
                  uint32_t part_id = WK_PART_ID_NONE) {
  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_MULTIPOINT);
  meta.size = geog.Points().size();

  wk_meta_t meta_child;
  WK_META_RESET(meta_child, WK_POINT);
  meta_child.size = 1;
  double coord[2];

  if (meta.size == 0) {
    meta_child.size = 0;
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else if (meta.size == 1) {
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    S2LatLng pt(geog.Points()[0]);
    coord[0] = pt.lng().degrees();
    coord[1] = pt.lat().degrees();
    HANDLE_OR_RETURN(handler->coord(&meta_child, coord, 0, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else {
    HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));

    for (size_t i = 0; i < geog.Points().size(); i++) {
      HANDLE_OR_RETURN(handler->geometry_start(&meta_child, i, handler->handler_data));
      S2LatLng pt(geog.Points()[i]);
      coord[0] = pt.lng().degrees();
      coord[1] = pt.lat().degrees();
      HANDLE_OR_RETURN(handler->coord(&meta_child, coord, 0, handler->handler_data));
      HANDLE_OR_RETURN(handler->geometry_end(&meta_child, i, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));
  }

  return WK_CONTINUE;
}

int handle_polylines(const s2geography::S2GeographyOwningPolyline& geog, wk_handler_t* handler,
                     uint32_t part_id = WK_PART_ID_NONE) {
  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_MULTILINESTRING);
  meta.size = geog.Polylines().size();

  wk_meta_t meta_child;
  WK_META_RESET(meta_child, WK_LINESTRING);
  double coord[2];

  if (meta.size == 0) {
    meta_child.size = 0;
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else if (meta.size == 1) {
    const S2Polyline& poly = *geog.Polylines()[0];
    meta_child.size = poly.num_vertices();
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));

    for (int j = 0; j < poly.num_vertices(); j++) {
      S2LatLng pt(poly.vertex(j));
      coord[0] = pt.lng().degrees();
      coord[1] = pt.lat().degrees();
      HANDLE_OR_RETURN(handler->coord(&meta_child, coord, j, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else {
    HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));

    for (size_t i = 0; i < geog.Polylines().size(); i++) {
      const S2Polyline& poly = *geog.Polylines()[i];
      meta_child.size = poly.num_vertices();
      HANDLE_OR_RETURN(handler->geometry_start(&meta_child, i, handler->handler_data));

      for (int j = 0; j < poly.num_vertices(); j++) {
        S2LatLng pt(poly.vertex(j));
        coord[0] = pt.lng().degrees();
        coord[1] = pt.lat().degrees();
        HANDLE_OR_RETURN(handler->coord(&meta_child, coord, j, handler->handler_data));
      }

      HANDLE_OR_RETURN(handler->geometry_end(&meta_child, i, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));
  }

  return WK_CONTINUE;
}

int handle_loop_shell(const S2Loop* loop, const wk_meta_t* meta, uint32_t loop_id, wk_handler_t* handler) {
  int result;
  double coord[2];

  if (loop->num_vertices() == 0) {
    return handler->error("Unexpected S2Loop with 0 vertices", handler->handler_data);
  }

  HANDLE_OR_RETURN(handler->ring_start(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));

  for (int i = 0; i <= loop->num_vertices(); i++) {
    S2LatLng pt(loop->vertex(i));
    coord[0] = pt.lng().degrees();
    coord[1] = pt.lat().degrees();
    HANDLE_OR_RETURN(handler->coord(meta, coord, i, handler->handler_data));
  }

  HANDLE_OR_RETURN(handler->ring_end(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));
  return WK_CONTINUE;
}

int handle_loop_hole(const S2Loop* loop, const wk_meta_t* meta, uint32_t loop_id, wk_handler_t* handler) {
  int result;
  double coord[2];

  if (loop->num_vertices() == 0) {
    return handler->error("Unexpected S2Loop with 0 vertices", handler->handler_data);
  }

  HANDLE_OR_RETURN(handler->ring_start(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));

  uint32_t coord_id = 0;
  for (int i = loop->num_vertices() - 1; i >= 0; i--) {
    S2LatLng pt(loop->vertex(i));
    coord[0] = pt.lng().degrees();
    coord[1] = pt.lat().degrees();
    HANDLE_OR_RETURN(handler->coord(meta, coord, coord_id, handler->handler_data));
    coord_id++;
  }

  S2LatLng pt(loop->vertex(loop->num_vertices() - 1));
  coord[0] = pt.lng().degrees();
  coord[1] = pt.lat().degrees();
  HANDLE_OR_RETURN(handler->coord(meta, coord, coord_id, handler->handler_data));

  HANDLE_OR_RETURN(handler->ring_end(meta, loop->num_vertices() + 1, loop_id, handler->handler_data));
  return WK_CONTINUE;
}

int handle_shell(const S2Polygon& poly, const wk_meta_t* meta, int loop_start, wk_handler_t* handler) {
  int result;
  const S2Loop* loop0 = poly.loop(loop_start);
  HANDLE_OR_RETURN(handle_loop_shell(loop0, meta, 0, handler));

  uint32_t loop_id = 1;
  for (int j = loop_start + 1; j <= poly.GetLastDescendant(loop_start); j++) {
    const S2Loop* loop = poly.loop(j);
    if (loop->depth() == (loop0->depth() + 1)) {
      HANDLE_OR_RETURN(handle_loop_hole(loop, meta, loop_id, handler));
      loop_id++;
    }
  }

  return WK_CONTINUE;
}

int handle_polygon(const s2geography::S2GeographyOwningPolygon& geog, wk_handler_t* handler,
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

  wk_meta_t meta_child;
  WK_META_RESET(meta_child, WK_POLYGON);

  if (meta.size == 0) {
    meta_child.size = 0;
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else if (meta.size == 1) {
    meta_child.size = outer_shell_loop_sizes[0];
    HANDLE_OR_RETURN(handler->geometry_start(&meta_child, part_id, handler->handler_data));
    HANDLE_OR_RETURN(handle_shell(poly, &meta_child, outer_shell_loop_ids[0], handler));
    HANDLE_OR_RETURN(handler->geometry_end(&meta_child, part_id, handler->handler_data));
  } else {
    HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));

    for (size_t i = 0; i < outer_shell_loop_sizes.size(); i++) {
      meta_child.size = outer_shell_loop_sizes[i];
      HANDLE_OR_RETURN(handler->geometry_start(&meta_child, i, handler->handler_data));
      HANDLE_OR_RETURN(handle_shell(poly, &meta_child, outer_shell_loop_ids[i], handler));
      HANDLE_OR_RETURN(handler->geometry_end(&meta_child, i, handler->handler_data));
    }

    HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));
  }

  return WK_CONTINUE;
}

int handle_collection(const s2geography::S2GeographyCollection& geog, wk_handler_t* handler,
                      uint32_t part_id = WK_PART_ID_NONE) {
  int result;

  wk_meta_t meta;
  WK_META_RESET(meta, WK_GEOMETRYCOLLECTION);
  meta.size = geog.Features().size();

  HANDLE_OR_RETURN(handler->geometry_start(&meta, part_id, handler->handler_data));
  for (size_t i = 0; i < geog.Features().size(); i++) {
    const s2geography::S2Geography* child_ptr = geog.Features()[i].get();

    auto child_point = dynamic_cast<const s2geography::S2GeographyOwningPoint*>(child_ptr);
    if (child_point != nullptr) {
      HANDLE_OR_RETURN(handle_points(*child_point, handler, i));
      continue;
    }

    auto child_polyline = dynamic_cast<const s2geography::S2GeographyOwningPolyline*>(child_ptr);
    if (child_polyline != nullptr) {
      HANDLE_OR_RETURN(handle_polylines(*child_polyline, handler, i));
      continue;
    }

    auto child_polygon = dynamic_cast<const s2geography::S2GeographyOwningPolygon*>(child_ptr);
    if (child_polygon != nullptr) {
      HANDLE_OR_RETURN(handle_polygon(*child_polygon, handler, i));
      continue;
    }

    auto child_collection = dynamic_cast<const s2geography::S2GeographyCollection*>(child_ptr);
    if (child_collection != nullptr) {
      HANDLE_OR_RETURN(handle_collection(*child_collection, handler, i));
      continue;
    }

    return handler->error("Unsupported S2Geography subclass", handler->handler_data);
  }
  HANDLE_OR_RETURN(handler->geometry_end(&meta, part_id, handler->handler_data));

  return WK_CONTINUE;
}

SEXP handle_geography(SEXP data, wk_handler_t* handler) {
  R_xlen_t n_features = Rf_xlength(data);

    wk_vector_meta_t vector_meta;
    WK_VECTOR_META_RESET(vector_meta, WK_GEOMETRY);
    vector_meta.size = n_features;
    vector_meta.flags |= WK_FLAG_DIMS_UNKNOWN;

    if (handler->vector_start(&vector_meta, handler->handler_data) == WK_CONTINUE) {
      int result;
      SEXP item;

      for (R_xlen_t i = 0; i < n_features; i++) {
        item = VECTOR_ELT(data, i);

        HANDLE_CONTINUE_OR_BREAK(handler->feature_start(&vector_meta, i, handler->handler_data));

        if (item == R_NilValue) {
          HANDLE_CONTINUE_OR_BREAK(handler->null_feature(handler->handler_data));
        } else {
          auto item_ptr = reinterpret_cast<Geography*>(R_ExternalPtrAddr(item));
          const s2geography::S2Geography* geog_ptr = &item_ptr->Geog();

          auto child_point = dynamic_cast<const s2geography::S2GeographyOwningPoint*>(geog_ptr);
          if (child_point != nullptr) {
            HANDLE_CONTINUE_OR_BREAK(handle_points(*child_point, handler));
          } else {
            auto child_polyline = dynamic_cast<const s2geography::S2GeographyOwningPolyline*>(geog_ptr);
            if (child_polyline != nullptr) {
              HANDLE_CONTINUE_OR_BREAK(handle_polylines(*child_polyline, handler));
            } else {
              auto child_polygon = dynamic_cast<const s2geography::S2GeographyOwningPolygon*>(geog_ptr);
              if (child_polygon != nullptr) {
                HANDLE_CONTINUE_OR_BREAK(handle_polygon(*child_polygon, handler));
              } else {
                auto child_collection = dynamic_cast<const s2geography::S2GeographyCollection*>(geog_ptr);
                if (child_collection != nullptr) {
                  HANDLE_CONTINUE_OR_BREAK(handle_collection(*child_collection, handler));
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

extern "C" SEXP c_s2_handle_geography(SEXP data, SEXP handler_xptr) {
    return wk_handler_run_xptr(&handle_geography, data, handler_xptr);
}
