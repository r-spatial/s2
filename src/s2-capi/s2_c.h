
#ifdef __cplusplus
extern "C" {
#endif

typedef struct s2_geography_t s2_geography_t;
typedef struct s2_projection_t s2_projection_t;
typedef struct s2_tessellator_t s2_tessellator_t;

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
