#include "blend.h"
#include "blend_version.h"

#include <float.h>

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return FAIL; \
    } \
} while (0)

#define ASSERT_EQ_INT(actual, expected) do { \
    int actual_value = (actual); \
    int expected_value = (expected); \
    if (actual_value != expected_value) { \
        fprintf(stderr, "Assertion failed at %s:%d: expected %d, got %d\n", \
                __FILE__, __LINE__, expected_value, actual_value); \
        return FAIL; \
    } \
} while (0)

#define ASSERT_NEAR(actual, expected, tolerance) do { \
    double actual_value = (actual); \
    double expected_value = (expected); \
    if (fabs(actual_value - expected_value) > (tolerance)) { \
        fprintf(stderr, "Assertion failed at %s:%d: expected %.12f, got %.12f\n", \
                __FILE__, __LINE__, expected_value, actual_value); \
        return FAIL; \
    } \
} while (0)

static double **alloc_points(int rows)
{
    int i;
    double **points = (double **)calloc((size_t)rows, sizeof(double *));

    if (points == NULL) return NULL;

    for (i = 0; i < rows; i++) {
        points[i] = (double *)calloc(2, sizeof(double));
        if (points[i] == NULL) return NULL;
    }

    return points;
}

static void set_point(double **points, int row, double x, double y)
{
    points[row][0] = x;
    points[row][1] = y;
}

static void free_points(double **points, int rows)
{
    int i;

    if (points == NULL) return;
    for (i = 0; i < rows; i++) {
        free(points[i]);
    }
    free(points);
}

static int polygon_test_contains(const polygon *poly, double x, double y)
{
    size_t i, j;
    int inside = 0;

    for (i = 0, j = poly->n_vertices - 1; i < poly->n_vertices; j = i++) {
        const vertex *vi = &poly->vertices[i];
        const vertex *vj = &poly->vertices[j];

        if (((vi->y > y) != (vj->y > y)) &&
            (x < (vj->x - vi->x) * (y - vi->y) / (vj->y - vi->y) + vi->x)) {
            inside = !inside;
        }
    }

    return inside;
}

static double polygon_test_iou(const polygon *a, const polygon *b)
{
    const int samples = 64;
    double xmin = fmin(a->vertices[0].x, b->vertices[0].x);
    double xmax = fmax(a->vertices[0].x, b->vertices[0].x);
    double ymin = fmin(a->vertices[0].y, b->vertices[0].y);
    double ymax = fmax(a->vertices[0].y, b->vertices[0].y);
    int intersection_count = 0;
    int union_count = 0;
    int ix, iy;
    size_t i;

    for (i = 0; i < a->n_vertices; i++) {
        if (a->vertices[i].x < xmin) xmin = a->vertices[i].x;
        if (a->vertices[i].x > xmax) xmax = a->vertices[i].x;
        if (a->vertices[i].y < ymin) ymin = a->vertices[i].y;
        if (a->vertices[i].y > ymax) ymax = a->vertices[i].y;
    }
    for (i = 0; i < b->n_vertices; i++) {
        if (b->vertices[i].x < xmin) xmin = b->vertices[i].x;
        if (b->vertices[i].x > xmax) xmax = b->vertices[i].x;
        if (b->vertices[i].y < ymin) ymin = b->vertices[i].y;
        if (b->vertices[i].y > ymax) ymax = b->vertices[i].y;
    }

    for (iy = 0; iy < samples; iy++) {
        double y = ymin + ((double)iy + 0.5) * (ymax - ymin) / (double)samples;

        for (ix = 0; ix < samples; ix++) {
            double x = xmin + ((double)ix + 0.5) * (xmax - xmin) / (double)samples;
            int inside_a = polygon_test_contains(a, x, y);
            int inside_b = polygon_test_contains(b, x, y);

            if (inside_a || inside_b) {
                union_count++;
                if (inside_a && inside_b) {
                    intersection_count++;
                }
            }
        }
    }

    return union_count == 0 ? 0.0 : (double)intersection_count / (double)union_count;
}

static void init_rectangle(window *data, int nx, int ny)
{
    data->row_size = 4;
    data->nx = nx;
    data->ny = ny;
    data->nz = 5;
    data->ratio_x1 = 0.2;
    data->ratio_x2 = 0.2;
    data->ratio_y1 = 0.2;
    data->ratio_y2 = 0.2;
    data->ratio_z1 = 0.2;
    data->ratio_z2 = 0.2;
    data->x_function = WFUNC_COSINE;
    data->y_function = WFUNC_COSINE;
    data->z_function = WFUNC_COSINE;
    data->vertices = alloc_points(data->row_size);
    set_point(data->vertices, 0, 0, 0);
    set_point(data->vertices, 1, nx - 1, 0);
    set_point(data->vertices, 2, nx - 1, ny - 1);
    set_point(data->vertices, 3, 0, ny - 1);
}

static void init_hexagon(window *data)
{
    data->row_size = 6;
    data->nx = 10;
    data->ny = 12;
    data->nz = 5;
    data->ratio_x1 = 0.2;
    data->ratio_x2 = 0.2;
    data->ratio_y1 = 0.2;
    data->ratio_y2 = 0.2;
    data->ratio_z1 = 0.2;
    data->ratio_z2 = 0.2;
    data->x_function = WFUNC_COSINE;
    data->y_function = WFUNC_COSINE;
    data->z_function = WFUNC_COSINE;
    data->vertices = alloc_points(data->row_size);
    set_point(data->vertices, 0, 2, 0);
    set_point(data->vertices, 1, 7, 0);
    set_point(data->vertices, 2, 9, 4);
    set_point(data->vertices, 3, 9, 11);
    set_point(data->vertices, 4, 0, 11);
    set_point(data->vertices, 5, 0, 3);
}

static void init_octagon_with_quadrant_vertices(window *data)
{
    data->row_size = 12;
    data->nx = 10;
    data->ny = 10;
    data->nz = 5;
    data->ratio_x1 = 0.2;
    data->ratio_x2 = 0.2;
    data->ratio_y1 = 0.2;
    data->ratio_y2 = 0.2;
    data->ratio_z1 = 0.2;
    data->ratio_z2 = 0.2;
    data->x_function = WFUNC_COSINE;
    data->y_function = WFUNC_COSINE;
    data->z_function = WFUNC_COSINE;
    data->vertices = alloc_points(data->row_size);
    set_point(data->vertices, 0, 3, 0);
    set_point(data->vertices, 1, 6, 0);
    set_point(data->vertices, 2, 8, 1);
    set_point(data->vertices, 3, 9, 3);
    set_point(data->vertices, 4, 9, 6);
    set_point(data->vertices, 5, 8, 8);
    set_point(data->vertices, 6, 6, 9);
    set_point(data->vertices, 7, 3, 9);
    set_point(data->vertices, 8, 1, 8);
    set_point(data->vertices, 9, 0, 6);
    set_point(data->vertices, 10, 0, 3);
    set_point(data->vertices, 11, 1, 1);
}

static int test_sorting_and_unique_vertices(void)
{
    double **points = alloc_points(5);
    int rows;

    ASSERT_TRUE(points != NULL);
    set_point(points, 0, 3, 2);
    set_point(points, 1, 1, 4);
    set_point(points, 2, 2, 1);
    set_point(points, 3, 1, 4);
    set_point(points, 4, 0, 0);

    rows = unique_vertices(points, 5);
    ASSERT_EQ_INT(rows, 4);

    ASSERT_EQ_INT(ascend_x(points, rows), SUCCESS);
    ASSERT_NEAR(points[0][0], 0, DBL_EPSILON);
    ASSERT_NEAR(points[rows - 1][0], 3, DBL_EPSILON);

    ASSERT_EQ_INT(descend_x(points, rows), SUCCESS);
    ASSERT_NEAR(points[0][0], 3, DBL_EPSILON);
    ASSERT_NEAR(points[rows - 1][0], 0, DBL_EPSILON);

    ASSERT_EQ_INT(ascend_y(points, rows), SUCCESS);
    ASSERT_NEAR(points[0][1], 0, DBL_EPSILON);
    ASSERT_NEAR(points[rows - 1][1], 4, DBL_EPSILON);

    ASSERT_EQ_INT(descend_y(points, rows), SUCCESS);
    ASSERT_NEAR(points[0][1], 4, DBL_EPSILON);
    ASSERT_NEAR(points[rows - 1][1], 0, DBL_EPSILON);

    free_points(points, 5);
    return SUCCESS;
}

static int test_version_header(void)
{
    ASSERT_EQ_INT(BLEND_VERSION_MAJOR, 2);
    ASSERT_EQ_INT(BLEND_VERSION_MINOR, 0);
    ASSERT_EQ_INT(BLEND_VERSION_PATCH, 1);
    ASSERT_TRUE(strcmp(BLEND_VERSION, "2.0.1") == 0);
    ASSERT_EQ_INT(BLEND_PACKAGE_VERSION_MAJOR, 2);
    ASSERT_EQ_INT(BLEND_PACKAGE_VERSION_MINOR, 0);
    ASSERT_EQ_INT(BLEND_PACKAGE_VERSION_PATCH, 1);
    ASSERT_TRUE(strcmp(BLEND_PACKAGE_VERSION, "2.0.1") == 0);
    ASSERT_TRUE(strcmp(BLEND_PACKAGE_VERSION_STRING, "BLEND 2.0.1") == 0);
    ASSERT_TRUE(strcmp(BLEND_VERSION_STRING, "BLEND 2.0.1") == 0);
    ASSERT_TRUE(strcmp(BLEND_LIBRARY_SOVERSION, "2") == 0);

    return SUCCESS;
}

static int test_polygon_data_model(void)
{
    double **points = alloc_points(3);
    polygon poly = {0};
    polygon copy = {0};
    polygon from_array = {0};
    window data = {0};
    vertex point = {0};

    ASSERT_TRUE(points != NULL);

    ASSERT_EQ_INT(blend_polygon_alloc(&poly, 3), SUCCESS);
    ASSERT_EQ_INT((int)poly.n_vertices, 3);
    ASSERT_TRUE(poly.vertices != NULL);

    ASSERT_EQ_INT(blend_polygon_set_vertex(&poly, 0, 0.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&poly, 1, 2.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&poly, 2, 1.0, 2.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&poly, 3, 1.0, 1.0), FAIL);

    ASSERT_EQ_INT(blend_polygon_get_vertex(&poly, 2, &point), SUCCESS);
    ASSERT_NEAR(point.x, 1.0, DBL_EPSILON);
    ASSERT_NEAR(point.y, 2.0, DBL_EPSILON);
    ASSERT_EQ_INT(blend_polygon_get_vertex(&poly, 3, &point), FAIL);

    ASSERT_EQ_INT(blend_polygon_copy(&poly, &copy), SUCCESS);
    ASSERT_EQ_INT((int)copy.n_vertices, 3);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&poly, 2, 5.0, 5.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_get_vertex(&copy, 2, &point), SUCCESS);
    ASSERT_NEAR(point.x, 1.0, DBL_EPSILON);
    ASSERT_NEAR(point.y, 2.0, DBL_EPSILON);

    set_point(points, 0, 3.0, 4.0);
    set_point(points, 1, 5.0, 6.0);
    set_point(points, 2, 7.0, 8.0);
    ASSERT_EQ_INT(blend_polygon_from_array(&from_array, points, 3), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_get_vertex(&from_array, 1, &point), SUCCESS);
    ASSERT_NEAR(point.x, 5.0, DBL_EPSILON);
    ASSERT_NEAR(point.y, 6.0, DBL_EPSILON);

    ASSERT_EQ_INT(blend_window_set_polygon(&data, &copy), SUCCESS);
    ASSERT_EQ_INT(data.row_size, 3);
    ASSERT_TRUE(data.vertices != NULL);
    ASSERT_NEAR(data.vertices[2][0], 1.0, DBL_EPSILON);
    ASSERT_NEAR(data.vertices[2][1], 2.0, DBL_EPSILON);
    blend_window_clear_polygon(&data);
    ASSERT_EQ_INT(data.row_size, 0);
    ASSERT_TRUE(data.vertices == NULL);

    blend_polygon_free(&poly);
    blend_polygon_free(&copy);
    blend_polygon_free(&from_array);
    free_points(points, 3);

    ASSERT_EQ_INT((int)poly.n_vertices, 0);
    ASSERT_TRUE(poly.vertices == NULL);

    return SUCCESS;
}

static int test_polygon_api_functions(void)
{
    polygon square = {0};
    polygon closed = {0};
    polygon grid = {0};
    polygon u_shape = {0};
    polygon monotone_exact = {0};
    polygon monotone_envelope = {0};
    polygon strict_envelope = {0};
    polygon piecewise_input = {0};
    polygon full_envelope = {0};
    polygon best_envelope = {0};
    polygon strict_best_envelope = {0};
    polygon reversed_input = {0};
    polygon reversed_best = {0};
    polygon bowtie = {0};
    polygon read_back = {0};
    polygon invalid = {0};
    vertex point = {0};
    double xmin, xmax, ymin, ymax;
    size_t i;
    int flag;
    char path[256];

    ASSERT_EQ_INT(blend_polygon_alloc(&square, 4), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&square, 0, 0.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&square, 1, 2.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&square, 2, 2.0, 2.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&square, 3, 0.0, 2.0), SUCCESS);

    ASSERT_EQ_INT(blend_polygon_validate(&square), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_bounds(&square, &xmin, &xmax, &ymin, &ymax), SUCCESS);
    ASSERT_NEAR(xmin, 0.0, DBL_EPSILON);
    ASSERT_NEAR(xmax, 2.0, DBL_EPSILON);
    ASSERT_NEAR(ymin, 0.0, DBL_EPSILON);
    ASSERT_NEAR(ymax, 2.0, DBL_EPSILON);

    ASSERT_EQ_INT(blend_polygon_contains_point(&square, 1.0, 1.0, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_contains_point(&square, 0.0, 1.0, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_contains_point(&square, 3.0, 1.0, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 0);

    ASSERT_EQ_INT(blend_polygon_is_closed(&square, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 0);
    ASSERT_EQ_INT(blend_polygon_copy(&square, &closed), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_close(&closed), SUCCESS);
    ASSERT_EQ_INT((int)closed.n_vertices, 5);
    ASSERT_EQ_INT(blend_polygon_is_closed(&closed, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);

    ASSERT_EQ_INT(blend_polygon_map_to_grid(&square, 0.0, 2.0, 0.0, 2.0, 5, 5, &grid), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_get_vertex(&grid, 2, &point), SUCCESS);
    ASSERT_NEAR(point.x, 4.0, DBL_EPSILON);
    ASSERT_NEAR(point.y, 4.0, DBL_EPSILON);

    ASSERT_EQ_INT(blend_polygon_is_xy_monotone(&square, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone_strict(&square, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_is_simple(&square, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_xy_monotone_envelope(&square, &monotone_exact), SUCCESS);
    ASSERT_EQ_INT((int)monotone_exact.n_vertices, 4);
    ASSERT_EQ_INT(blend_polygon_get_vertex(&monotone_exact, 2, &point), SUCCESS);
    ASSERT_NEAR(point.x, 2.0, DBL_EPSILON);
    ASSERT_NEAR(point.y, 2.0, DBL_EPSILON);

    ASSERT_EQ_INT(blend_polygon_alloc(&u_shape, 8), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 0, 0.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 1, 3.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 2, 3.0, 3.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 3, 2.0, 3.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 4, 2.0, 1.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 5, 1.0, 1.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 6, 1.0, 3.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&u_shape, 7, 0.0, 3.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone(&u_shape, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 0);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone_strict(&u_shape, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 0);
    ASSERT_EQ_INT(blend_polygon_xy_monotone_envelope(&u_shape, &monotone_envelope), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone(&monotone_envelope, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_xy_monotone_envelope_strict(&u_shape, &strict_envelope), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone_strict(&strict_envelope, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT((int)strict_envelope.n_vertices, 4);
    ASSERT_EQ_INT(blend_polygon_contains_point(&monotone_envelope, 1.5, 0.5, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_contains_point(&monotone_envelope, 1.5, 2.5, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_bounds(&monotone_envelope, &xmin, &xmax, &ymin, &ymax), SUCCESS);
    ASSERT_NEAR(xmin, 0.0, DBL_EPSILON);
    ASSERT_NEAR(xmax, 3.0, DBL_EPSILON);
    ASSERT_NEAR(ymin, 0.0, DBL_EPSILON);
    ASSERT_NEAR(ymax, 3.0, DBL_EPSILON);

    ASSERT_EQ_INT(blend_polygon_alloc(&piecewise_input, 14), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 0, 0.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 1, 4.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 2, 4.5, 0.5), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 3, 5.0, 1.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 4, 5.0, 4.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 5, 4.7, 4.4), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 6, 4.0, 5.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 7, 1.0, 5.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 8, 0.7, 4.7), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 9, 0.3, 4.4), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 10, -1.0, 4.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 11, -1.0, 1.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 12, -0.2, 0.8), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&piecewise_input, 13, -0.8, 0.4), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone(&piecewise_input, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 0);
    ASSERT_EQ_INT(blend_polygon_xy_monotone_envelope(&piecewise_input, &full_envelope), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_xy_monotone_best_piecewise_envelope(&piecewise_input, &best_envelope,
                                                                    -1.0, 5.0, 0.0, 5.0, 256, 256), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone(&best_envelope, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_EQ_INT(blend_polygon_xy_monotone_best_piecewise_envelope_strict(&piecewise_input,
                                                                            &strict_best_envelope,
                                                                            -1.0, 5.0, 0.0, 5.0, 256, 256),
                  SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone_strict(&strict_best_envelope, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_TRUE(polygon_test_iou(&piecewise_input, &best_envelope) >=
                polygon_test_iou(&piecewise_input, &full_envelope));

    ASSERT_EQ_INT(blend_polygon_alloc(&reversed_input, piecewise_input.n_vertices), SUCCESS);
    for (i = 0; i < piecewise_input.n_vertices; i++) {
        ASSERT_EQ_INT(blend_polygon_set_vertex(&reversed_input, i,
                                               piecewise_input.vertices[piecewise_input.n_vertices - 1 - i].x,
                                               piecewise_input.vertices[piecewise_input.n_vertices - 1 - i].y),
                      SUCCESS);
    }
    ASSERT_EQ_INT(blend_polygon_xy_monotone_best_piecewise_envelope(&reversed_input, &reversed_best,
                                                                    -1.0, 5.0, 0.0, 5.0, 256, 256), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_xy_monotone(&reversed_best, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);
    ASSERT_NEAR(polygon_test_iou(&piecewise_input, &reversed_best),
                polygon_test_iou(&piecewise_input, &best_envelope), 1.0e-12);

    ASSERT_EQ_INT(blend_polygon_is_simple(&closed, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 1);

    ASSERT_EQ_INT(blend_polygon_alloc(&bowtie, 4), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&bowtie, 0, 0.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&bowtie, 1, 2.0, 2.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&bowtie, 2, 0.0, 2.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&bowtie, 3, 2.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_is_simple(&bowtie, &flag), SUCCESS);
    ASSERT_EQ_INT(flag, 0);
    ASSERT_EQ_INT(blend_polygon_validate(&bowtie), FAIL);

    snprintf(path, sizeof(path), "/tmp/blend_polygon_api_%ld.txt", (long)getpid());
    ASSERT_EQ_INT(blend_polygon_write(path, &square), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_read(path, &read_back), SUCCESS);
    ASSERT_EQ_INT((int)read_back.n_vertices, 4);
    ASSERT_EQ_INT(blend_polygon_bounds(&read_back, &xmin, &xmax, &ymin, &ymax), SUCCESS);
    ASSERT_NEAR(xmax, 2.0, DBL_EPSILON);
    ASSERT_NEAR(ymax, 2.0, DBL_EPSILON);
    unlink(path);

    ASSERT_EQ_INT(blend_polygon_alloc(&invalid, 2), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&invalid, 0, 0.0, 0.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_set_vertex(&invalid, 1, 1.0, 1.0), SUCCESS);
    ASSERT_EQ_INT(blend_polygon_validate(&invalid), FAIL);
    ASSERT_EQ_INT(blend_polygon_map_to_grid(&square, 0.5, 2.0, 0.0, 2.0, 5, 5, &grid), FAIL);

    blend_polygon_free(&square);
    blend_polygon_free(&closed);
    blend_polygon_free(&grid);
    blend_polygon_free(&u_shape);
    blend_polygon_free(&monotone_exact);
    blend_polygon_free(&monotone_envelope);
    blend_polygon_free(&strict_envelope);
    blend_polygon_free(&piecewise_input);
    blend_polygon_free(&full_envelope);
    blend_polygon_free(&best_envelope);
    blend_polygon_free(&strict_best_envelope);
    blend_polygon_free(&reversed_input);
    blend_polygon_free(&reversed_best);
    blend_polygon_free(&bowtie);
    blend_polygon_free(&read_back);
    blend_polygon_free(&invalid);

    return SUCCESS;
}

static int test_window_functions(void)
{
    blend_window_function parsed = WFUNC_INVALID;
    blend_window_function functions[] = {
        WFUNC_BOXCAR,
        WFUNC_COSINE,
        WFUNC_TRAPEZOID,
        WFUNC_HAMMING,
        WFUNC_BLACKMAN,
        WFUNC_BLACKMANHARRIS,
        WFUNC_WELCH,
        WFUNC_PARZEN,
        WFUNC_GAUSSIAN,
        WFUNC_SMOOTHSTEP,
        WFUNC_SMOOTHERSTEP,
        WFUNC_EXPONENTIAL,
        WFUNC_SINE,
        WFUNC_BOHMAN,
        WFUNC_NUTTALL,
        WFUNC_KAISER,
        WFUNC_CAUCHY,
        WFUNC_QUADRATIC,
        WFUNC_CUBIC,
        WFUNC_POISSON,
        WFUNC_BARTLETT,
        WFUNC_BARTLETTHANN,
        WFUNC_EXACTBLACKMAN,
        WFUNC_BLACKMANNUTTALL,
        WFUNC_FLATTOP,
        WFUNC_LANCZOS,
        WFUNC_RIESZ,
        WFUNC_RIEMANN,
        WFUNC_FEJER,
        WFUNC_CONNES,
        WFUNC_HANNINGPOISSON,
        WFUNC_KAISERBESSEL,
        WFUNC_PLANCKTAPER,
        WFUNC_QUARTIC,
        WFUNC_QUINTIC,
        WFUNC_SEPTIC,
        WFUNC_NONIC,
        WFUNC_LOGISTIC,
        WFUNC_TANH,
        WFUNC_ERF,
        WFUNC_ARCTAN,
        WFUNC_GOMPERTZ,
        WFUNC_SOFTSIGN,
        WFUNC_AGNESI,
        WFUNC_INVERSEQUADRATIC,
        WFUNC_INVERSEMULTIQUADRIC,
        WFUNC_POWERLAW,
        WFUNC_ROOT,
        WFUNC_CIRCULAR,
        WFUNC_SECH,
        WFUNC_SECH2,
        WFUNC_STUDENT,
        WFUNC_LAPLACE
    };
    struct {
        const char *name;
        blend_window_function function;
    } parse_cases[] = {
        {"bartlett", WFUNC_BARTLETT},
        {"barthann", WFUNC_BARTLETTHANN},
        {"bartletthann", WFUNC_BARTLETTHANN},
        {"exactblackman", WFUNC_EXACTBLACKMAN},
        {"blackmannuttall", WFUNC_BLACKMANNUTTALL},
        {"flattop", WFUNC_FLATTOP},
        {"lanczos", WFUNC_LANCZOS},
        {"riesz", WFUNC_RIESZ},
        {"riemann", WFUNC_RIEMANN},
        {"fejer", WFUNC_FEJER},
        {"connes", WFUNC_CONNES},
        {"hanningpoisson", WFUNC_HANNINGPOISSON},
        {"hannpoisson", WFUNC_HANNINGPOISSON},
        {"kaiserbessel", WFUNC_KAISERBESSEL},
        {"plancktaper", WFUNC_PLANCKTAPER},
        {"planck", WFUNC_PLANCKTAPER},
        {"quartic", WFUNC_QUARTIC},
        {"quintic", WFUNC_QUINTIC},
        {"septic", WFUNC_SEPTIC},
        {"nonic", WFUNC_NONIC},
        {"logistic", WFUNC_LOGISTIC},
        {"tanh", WFUNC_TANH},
        {"erf", WFUNC_ERF},
        {"arctan", WFUNC_ARCTAN},
        {"gompertz", WFUNC_GOMPERTZ},
        {"softsign", WFUNC_SOFTSIGN},
        {"agnesi", WFUNC_AGNESI},
        {"inversequadratic", WFUNC_INVERSEQUADRATIC},
        {"inversemultiquadric", WFUNC_INVERSEMULTIQUADRIC},
        {"powerlaw", WFUNC_POWERLAW},
        {"root", WFUNC_ROOT},
        {"circular", WFUNC_CIRCULAR},
        {"sech", WFUNC_SECH},
        {"sech2", WFUNC_SECH2},
        {"student", WFUNC_STUDENT},
        {"laplace", WFUNC_LAPLACE},
        {"normal", WFUNC_GAUSSIAN}
    };
    size_t f;
    size_t p;
    int i;

    ASSERT_NEAR(boxcar(), 1.0, DBL_EPSILON);
    ASSERT_NEAR(cosine(1, 10, 10, 10, 0.2, 0.2), 0.146446609407, 1e-12);
    ASSERT_NEAR(cosine(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(cosine(9, 10, 10, 10, 0.1, 0.2), 0.853553390593, 1e-12);
    ASSERT_NEAR(trapezoid(1, 10, 10, 10, 0.2, 0.2), 0.25, DBL_EPSILON);
    ASSERT_NEAR(trapezoid(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(trapezoid(9, 10, 10, 10, 0.1, 0.2), 0.75, DBL_EPSILON);
    ASSERT_NEAR(trapezoid(33, 41, 41, 41, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(hamming(1, 10, 10, 10, 0.2, 0.2), 0.214730880654, 1e-12);
    ASSERT_NEAR(blackman(1, 10, 10, 10, 0.2, 0.2), 0.066446609407, 1e-12);
    ASSERT_NEAR(blackmanharris(1, 10, 10, 10, 0.2, 0.2), 0.021735837019, 1e-12);
    ASSERT_NEAR(welch(1, 10, 10, 10, 0.2, 0.2), 0.4375, DBL_EPSILON);
    ASSERT_NEAR(parzen(1, 10, 10, 10, 0.2, 0.2), 0.03125, DBL_EPSILON);
    ASSERT_NEAR(gaussian(1, 10, 10, 10, 0.2, 0.2), 0.069219471044, 1e-12);
    ASSERT_NEAR(smoothstep(1, 10, 10, 10, 0.2, 0.2), 0.15625, DBL_EPSILON);
    ASSERT_NEAR(smootherstep(1, 10, 10, 10, 0.2, 0.2), 0.103515625, DBL_EPSILON);
    ASSERT_NEAR(exponential(1, 10, 10, 10, 0.2, 0.2), 0.718335308375, 1e-12);
    ASSERT_NEAR(sine(1, 10, 10, 10, 0.2, 0.2), sin(0.125 * M_PI), 1e-12);
    ASSERT_NEAR(bohman(1, 10, 10, 10, 0.2, 0.2),
                0.25 * cos(0.75 * M_PI) + sin(0.75 * M_PI) / M_PI, 1e-12);
    ASSERT_TRUE(nuttall(1, 10, 10, 10, 0.2, 0.2) > 0.0);
    ASSERT_TRUE(nuttall(1, 10, 10, 10, 0.2, 0.2) < 1.0);
    ASSERT_TRUE(kaiser(1, 10, 10, 10, 0.2, 0.2) > 0.0);
    ASSERT_TRUE(kaiser(1, 10, 10, 10, 0.2, 0.2) < 1.0);
    ASSERT_TRUE(cauchy(1, 10, 10, 10, 0.2, 0.2) > 0.0);
    ASSERT_TRUE(cauchy(1, 10, 10, 10, 0.2, 0.2) < 1.0);
    ASSERT_NEAR(quadratic(1, 10, 10, 10, 0.2, 0.2), 0.0625, DBL_EPSILON);
    ASSERT_NEAR(cubic(1, 10, 10, 10, 0.2, 0.2), 0.015625, DBL_EPSILON);
    ASSERT_NEAR(poisson(1, 10, 10, 10, 0.2, 0.2),
                (exp(-3.75) - exp(-5.0)) / (1.0 - exp(-5.0)), 1e-12);
    ASSERT_NEAR(bartlett(1, 10, 10, 10, 0.2, 0.2), 0.25, DBL_EPSILON);
    ASSERT_NEAR(fejer(1, 10, 10, 10, 0.2, 0.2), 0.25, DBL_EPSILON);
    ASSERT_NEAR(quartic(1, 10, 10, 10, 0.2, 0.2), 0.00390625, DBL_EPSILON);
    ASSERT_NEAR(quintic(1, 10, 10, 10, 0.2, 0.2), 0.0009765625, DBL_EPSILON);
    ASSERT_NEAR(plancktaper(1, 10, 10, 10, 0.2, 0.2),
                1.0 / (exp((1.0 / 0.25) - (1.0 / 0.75)) + 1.0), 1e-12);
    ASSERT_TRUE(flattop(1, 10, 10, 10, 0.2, 0.2) >= 0.0);
    ASSERT_TRUE(flattop(1, 10, 10, 10, 0.2, 0.2) <= 1.0);
    ASSERT_TRUE(logistic(1, 10, 10, 10, 0.2, 0.2) > 0.0);
    ASSERT_TRUE(logistic(1, 10, 10, 10, 0.2, 0.2) < 1.0);
    ASSERT_NEAR(blackman(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(smootherstep(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(kaiser(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(poisson(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(plancktaper(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(lanczos(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);
    ASSERT_NEAR(sechwindow(5, 10, 10, 10, 0.2, 0.2), 1.0, DBL_EPSILON);

    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_COSINE), "cosine") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_TRAPEZOID), "trapezoid") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_BLACKMANHARRIS), "blackmanharris") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_SMOOTHERSTEP), "smootherstep") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_SINE), "sine") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_POISSON), "poisson") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_PLANCKTAPER), "plancktaper") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_LAPLACE), "laplace") == 0);
    ASSERT_TRUE(strcmp(blend_window_function_name(WFUNC_INVALID), "invalid") == 0);
    ASSERT_EQ_INT(blend_window_function_from_name("boxcar", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_BOXCAR);
    ASSERT_EQ_INT(blend_window_function_from_name("cosine", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_COSINE);
    ASSERT_EQ_INT(blend_window_function_from_name("hann", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_COSINE);
    ASSERT_EQ_INT(blend_window_function_from_name("tukey", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_COSINE);
    ASSERT_EQ_INT(blend_window_function_from_name("trapezoid", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_TRAPEZOID);
    ASSERT_EQ_INT(blend_window_function_from_name("linear", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_TRAPEZOID);
    ASSERT_EQ_INT(blend_window_function_from_name("hamming", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_HAMMING);
    ASSERT_EQ_INT(blend_window_function_from_name("blackman", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_BLACKMAN);
    ASSERT_EQ_INT(blend_window_function_from_name("blackmanharris", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_BLACKMANHARRIS);
    ASSERT_EQ_INT(blend_window_function_from_name("welch", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_WELCH);
    ASSERT_EQ_INT(blend_window_function_from_name("parzen", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_PARZEN);
    ASSERT_EQ_INT(blend_window_function_from_name("gaussian", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_GAUSSIAN);
    ASSERT_EQ_INT(blend_window_function_from_name("smoothstep", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_SMOOTHSTEP);
    ASSERT_EQ_INT(blend_window_function_from_name("smootherstep", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_SMOOTHERSTEP);
    ASSERT_EQ_INT(blend_window_function_from_name("exponential", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_EXPONENTIAL);
    ASSERT_EQ_INT(blend_window_function_from_name("sine", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_SINE);
    ASSERT_EQ_INT(blend_window_function_from_name("bohman", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_BOHMAN);
    ASSERT_EQ_INT(blend_window_function_from_name("nuttall", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_NUTTALL);
    ASSERT_EQ_INT(blend_window_function_from_name("kaiser", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_KAISER);
    ASSERT_EQ_INT(blend_window_function_from_name("cauchy", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_CAUCHY);
    ASSERT_EQ_INT(blend_window_function_from_name("quadratic", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_QUADRATIC);
    ASSERT_EQ_INT(blend_window_function_from_name("cubic", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_CUBIC);
    ASSERT_EQ_INT(blend_window_function_from_name("poisson", &parsed), SUCCESS);
    ASSERT_EQ_INT(parsed, WFUNC_POISSON);
    for (p = 0; p < sizeof(parse_cases) / sizeof(parse_cases[0]); p++) {
        ASSERT_EQ_INT(blend_window_function_from_name(parse_cases[p].name, &parsed), SUCCESS);
        ASSERT_EQ_INT(parsed, parse_cases[p].function);
    }
    ASSERT_EQ_INT(blend_window_function_from_name("scosine", &parsed), FAIL);
    ASSERT_EQ_INT(blend_window_function_from_name("ecosine", &parsed), FAIL);
    ASSERT_EQ_INT(blend_window_function_from_name("strapezoid", &parsed), FAIL);
    ASSERT_EQ_INT(blend_window_function_from_name("etrapezoid", &parsed), FAIL);
    ASSERT_EQ_INT(blend_window_function_from_name("invalid", &parsed), FAIL);
    ASSERT_EQ_INT(parsed, WFUNC_INVALID);

    ASSERT_NEAR(window_function(5, 1, 10, 10, 10, 0.2, 0.2, WFUNC_BOXCAR), 1.0, DBL_EPSILON);
    ASSERT_NEAR(window_function(5, 1, 10, 10, 10, 0.2, 0.2, WFUNC_COSINE), 1.0, DBL_EPSILON);
    ASSERT_NEAR(window_function(1, 1, 10, 10, 10, 0.2, 0.1, WFUNC_COSINE), 0.146446609407, 1e-12);
    ASSERT_NEAR(window_function(9, 1, 10, 10, 10, 0.1, 0.2, WFUNC_COSINE), 0.853553390593, 1e-12);
    ASSERT_NEAR(window_function(1, 1, 10, 10, 10, 0.2, 0.1, WFUNC_TRAPEZOID), 0.25, DBL_EPSILON);
    ASSERT_NEAR(window_function(9, 1, 10, 10, 10, 0.1, 0.2, WFUNC_TRAPEZOID), 0.75, DBL_EPSILON);
    ASSERT_NEAR(window_function(1, 1, 10, 10, 10, 0.2, 0.1, WFUNC_SMOOTHSTEP), 0.15625, DBL_EPSILON);
    ASSERT_NEAR(window_function(9, 1, 10, 10, 10, 0.1, 0.2, WFUNC_SMOOTHSTEP), 0.84375, DBL_EPSILON);
    ASSERT_NEAR(window_function(1, 1, 10, 10, 10, 0.2, 0.1, WFUNC_QUADRATIC), 0.0625, DBL_EPSILON);
    ASSERT_NEAR(window_function(9, 1, 10, 10, 10, 0.1, 0.2, WFUNC_QUADRATIC), 0.5625, DBL_EPSILON);
    ASSERT_NEAR(window_function(0, 1, 10, 10, 10, 0.2, 0.2, WFUNC_BOXCAR), WFUNC_ERROR, DBL_EPSILON);
    ASSERT_NEAR(window_function(11, 1, 10, 10, 10, 0.2, 0.2, WFUNC_BOXCAR), WFUNC_ERROR, DBL_EPSILON);
    ASSERT_NEAR(window_function(5, 1, 10, 10, 10, 0.5, 0.2, WFUNC_BOXCAR), WFUNC_ERROR, DBL_EPSILON);
    ASSERT_NEAR(window_function(5, 1, 10, 10, 10, 0.2, 0.5, WFUNC_BOXCAR), WFUNC_ERROR, DBL_EPSILON);
    ASSERT_NEAR(window_function(5, 1, 10, 10, 10, 0.2, 0.2, WFUNC_INVALID), WFUNC_ERROR, DBL_EPSILON);

    for (i = 1; i <= 41; i++) {
        for (f = 0; f < sizeof(functions) / sizeof(functions[0]); f++) {
            double weight = window_function(i, 1, 41, 41, 41, 0.2, 0.2, functions[f]);

            ASSERT_TRUE(weight >= 0.0 && weight <= 1.0);
        }
    }

    return SUCCESS;
}

static int test_boundary_box_and_assembly_helpers(void)
{
    window data = {0};
    permuted_vertex vertex = {0};

    init_rectangle(&data, 5, 4);

    ASSERT_EQ_INT(bounding_boxcheck(&data), SUCCESS);
    ASSERT_EQ_INT(assemble_bv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(vertex.row_size_bv, 2);
    ASSERT_NEAR(vertex.bottom_vertices[0][0], 0, DBL_EPSILON);
    ASSERT_NEAR(vertex.bottom_vertices[1][0], 4, DBL_EPSILON);

    ASSERT_EQ_INT(assemble_lv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(vertex.row_size_lv, 2);
    ASSERT_NEAR(vertex.left_vertices[0][1], 0, DBL_EPSILON);
    ASSERT_NEAR(vertex.left_vertices[1][1], 3, DBL_EPSILON);

    ASSERT_EQ_INT(assemble_tv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(vertex.row_size_tv, 2);
    ASSERT_NEAR(vertex.top_vertices[0][0], 0, DBL_EPSILON);
    ASSERT_NEAR(vertex.top_vertices[1][0], 4, DBL_EPSILON);

    ASSERT_EQ_INT(assemble_rv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(vertex.row_size_rv, 2);
    ASSERT_NEAR(vertex.right_vertices[0][1], 0, DBL_EPSILON);
    ASSERT_NEAR(vertex.right_vertices[1][1], 3, DBL_EPSILON);

    ASSERT_EQ_INT(assemble_btlv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_lttv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_rttv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_btrv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(vertex.row_size_btlv, 0);
    ASSERT_EQ_INT(vertex.row_size_lttv, 0);
    ASSERT_EQ_INT(vertex.row_size_rttv, 0);
    ASSERT_EQ_INT(vertex.row_size_btrv, 0);

    blend_permuted_vertex_free(&vertex);
    blend_window_boundary_clear(&data);

    return SUCCESS;
}

static int test_assembly_helpers_with_quadrant_vertices(void)
{
    window data = {0};
    permuted_vertex vertex = {0};

    init_octagon_with_quadrant_vertices(&data);

    ASSERT_EQ_INT(bounding_boxcheck(&data), SUCCESS);
    ASSERT_EQ_INT(assemble_bv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_lv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_tv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_rv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_btlv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_lttv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_rttv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_btrv(&data, &vertex), SUCCESS);

    ASSERT_EQ_INT(vertex.row_size_btlv, 1);
    ASSERT_NEAR(vertex.bottom_to_left_vertices[0][0], 1, DBL_EPSILON);
    ASSERT_NEAR(vertex.bottom_to_left_vertices[0][1], 1, DBL_EPSILON);

    ASSERT_EQ_INT(vertex.row_size_lttv, 1);
    ASSERT_NEAR(vertex.left_to_top_vertices[0][0], 1, DBL_EPSILON);
    ASSERT_NEAR(vertex.left_to_top_vertices[0][1], 8, DBL_EPSILON);

    ASSERT_EQ_INT(vertex.row_size_rttv, 1);
    ASSERT_NEAR(vertex.right_to_top_vertices[0][0], 8, DBL_EPSILON);
    ASSERT_NEAR(vertex.right_to_top_vertices[0][1], 8, DBL_EPSILON);

    ASSERT_EQ_INT(vertex.row_size_btrv, 1);
    ASSERT_NEAR(vertex.bottom_to_right_vertices[0][0], 8, DBL_EPSILON);
    ASSERT_NEAR(vertex.bottom_to_right_vertices[0][1], 1, DBL_EPSILON);

    blend_permuted_vertex_free(&vertex);
    blend_window_boundary_clear(&data);

    return SUCCESS;
}

static int test_hanging_sweep(void)
{
    double vector[12] = {0};
    double **prev_array = alloc_points(1);
    double **sub_array = alloc_points(1);
    double **hang_array = alloc_points(2);
    int index;

    ASSERT_TRUE(prev_array != NULL && sub_array != NULL && hang_array != NULL);
    set_point(prev_array, 0, 0, 0);
    set_point(sub_array, 0, 6, 6);
    set_point(hang_array, 0, 2, 2);
    set_point(hang_array, 1, 4, 4);
    vector[0] = 0;

    index = hanging_sweep(1, vector, prev_array, 0, sub_array, 0, hang_array, 2, ROW_STEP);
    ASSERT_EQ_INT(index, 5);
    ASSERT_NEAR(vector[2], 2, DBL_EPSILON);
    ASSERT_NEAR(vector[4], 4, DBL_EPSILON);

    memset(vector, 0, sizeof(vector));
    index = hanging_sweep(1, vector, prev_array, 0, sub_array, 0, hang_array, 2, COLUMN_STEP);
    ASSERT_EQ_INT(index, 5);
    ASSERT_NEAR(vector[2], 2, DBL_EPSILON);
    ASSERT_NEAR(vector[4], 4, DBL_EPSILON);

    index = hanging_sweep(7, vector, prev_array, 0, sub_array, 0, hang_array, 0, ROW_STEP);
    ASSERT_EQ_INT(index, 7);

    return SUCCESS;
}

static int test_boundary_assembly_and_contribution(void)
{
    window data = {0};
    permuted_vertex vertex = {0};
    int i;

    init_rectangle(&data, 5, 4);
    ASSERT_EQ_INT(boundary_assembly(&data, &vertex), SUCCESS);

    ASSERT_EQ_INT(data.minnx, 5);
    ASSERT_EQ_INT(data.minny, 4);
    for (i = 0; i < data.ny; i++) {
        ASSERT_EQ_INT(data.nnx1[i], 0);
        ASSERT_EQ_INT(data.nnx2[i], 4);
    }
    for (i = 0; i < data.nx; i++) {
        ASSERT_EQ_INT(data.nny1[i], 0);
        ASSERT_EQ_INT(data.nny2[i], 3);
    }

    ASSERT_EQ_INT(embedding_contribution1d(2, &data), SUCCESS);
    ASSERT_NEAR(data.contribution, 1.0, DBL_EPSILON);
    ASSERT_EQ_INT(embedding_contribution1d(0, &data), SUCCESS);
    ASSERT_TRUE(data.contribution > 0.0);
    ASSERT_TRUE(data.contribution < 1.0);
    ASSERT_EQ_INT(embedding_contribution1d(-1, &data), FAIL);

    ASSERT_EQ_INT(embedding_contribution2d(2, 2, &data), SUCCESS);
    ASSERT_NEAR(data.contribution, 1.0, DBL_EPSILON);
    ASSERT_EQ_INT(embedding_contribution3d(2, 2, 3, &data), SUCCESS);
    ASSERT_NEAR(data.contribution, 1.0, DBL_EPSILON);

    ASSERT_EQ_INT(embedding_contribution2d(0, 0, &data), SUCCESS);
    ASSERT_TRUE(data.contribution > 0.0);
    ASSERT_TRUE(data.contribution < 1.0);

    blend_permuted_vertex_free(&vertex);
    blend_window_boundary_clear(&data);

    return SUCCESS;
}

static int test_irregular_xy_monotone_scanlines(void)
{
    const double raw[][2] = {
        {-116.036489302, 32.7468972727},
        {-115.897830305, 32.6176071636},
        {-115.675627030, 32.6197869057},
        {-115.376887965, 32.9190953624},
        {-115.448025885, 33.3363316320},
        {-115.641389745, 33.3298816686}
    };
    const double xmin = -116.036489302, xmax = -115.376887965;
    const double ymin = 32.6176071636, ymax = 33.3363316320;
    window data = {0};
    polygon poly = {0};
    permuted_vertex boundary = {0};
    double lower_weight, upper_weight;
    int i;

    data.nx = 331;
    data.ny = 360;
    data.ratio_x1 = data.ratio_x2 = 0.2;
    data.ratio_y1 = data.ratio_y2 = 0.2;
    data.x_function = data.y_function = WFUNC_COSINE;
    ASSERT_EQ_INT(blend_polygon_alloc(&poly, 6), SUCCESS);
    for (i = 0; i < 6; i++) {
        double x = (raw[i][0] - xmin) * (double)(data.nx - 1) / (xmax - xmin);
        double y = (raw[i][1] - ymin) * (double)(data.ny - 1) / (ymax - ymin);
        ASSERT_EQ_INT(blend_polygon_set_vertex(&poly, (size_t)i, x, y), SUCCESS);
    }
    ASSERT_EQ_INT(blend_window_set_polygon(&data, &poly), SUCCESS);
    ASSERT_EQ_INT(boundary_assembly(&data, &boundary), SUCCESS);

    for (i = 0; i < data.ny; i++) {
        ASSERT_TRUE(data.nnx1[i] >= 0 && data.nnx1[i] < data.nx);
        ASSERT_TRUE(data.nnx2[i] >= 0 && data.nnx2[i] < data.nx);
        ASSERT_TRUE(data.nnx1[i] <= data.nnx2[i]);
    }
    for (i = 0; i < data.nx; i++) {
        ASSERT_TRUE(data.nny1[i] >= 0 && data.nny1[i] < data.ny);
        ASSERT_TRUE(data.nny2[i] >= 0 && data.nny2[i] < data.ny);
        ASSERT_TRUE(data.nny1[i] <= data.nny2[i]);
    }

    ASSERT_EQ_INT(embedding_contribution2d(168, 149, &data), SUCCESS);
    lower_weight = data.contribution;
    ASSERT_EQ_INT(embedding_contribution2d(168, 150, &data), SUCCESS);
    upper_weight = data.contribution;
    ASSERT_TRUE(fabs(lower_weight - upper_weight) < 0.2);

    blend_permuted_vertex_free(&boundary);
    blend_window_boundary_clear(&data);
    blend_polygon_free(&poly);
    return SUCCESS;
}

static int test_linear_interpolation(void)
{
    double value;

    ASSERT_EQ_INT(interpolate_linear(0.0, 2.0, 10.0, 12.0, 2.5, &value), SUCCESS);
    ASSERT_NEAR(value, 4.5, DBL_EPSILON);

    ASSERT_EQ_INT(interpolate_linear(10.0, 12.0, 0.0, 2.0, 2.5, &value), SUCCESS);
    ASSERT_NEAR(value, 4.5, DBL_EPSILON);

    ASSERT_EQ_INT(interpolate_linear(1.0, 2.0, 1.0, 4.0, 1.0, &value), FAIL);
    ASSERT_EQ_INT(interpolate_linear(0.0, 2.0, 1.0, 4.0, 0.5, NULL), FAIL);

    ASSERT_EQ_INT(interpolate_bilinear(0.0, 10.0, 0.0, 20.0,
                                       0.0, 10.0, 20.0, 30.0,
                                       2.5, 5.0, &value), SUCCESS);
    ASSERT_NEAR(value, 7.5, DBL_EPSILON);

    ASSERT_EQ_INT(interpolate_bilinear(10.0, 0.0, 20.0, 0.0,
                                       30.0, 20.0, 10.0, 0.0,
                                       2.5, 5.0, &value), SUCCESS);
    ASSERT_NEAR(value, 7.5, DBL_EPSILON);

    ASSERT_EQ_INT(interpolate_bilinear(1.0, 1.0, 0.0, 1.0,
                                       0.0, 1.0, 2.0, 3.0,
                                       1.0, 0.5, &value), FAIL);
    ASSERT_EQ_INT(interpolate_bilinear(0.0, 1.0, 1.0, 1.0,
                                       0.0, 1.0, 2.0, 3.0,
                                       0.5, 1.0, &value), FAIL);
    ASSERT_EQ_INT(interpolate_bilinear(0.0, 1.0, 0.0, 1.0,
                                       0.0, 1.0, 2.0, 3.0,
                                       0.5, 0.5, NULL), FAIL);

    ASSERT_EQ_INT(interpolate_trilinear(0.0, 10.0, 0.0, 20.0, 0.0, 30.0,
                                        0.0, 10.0, 20.0, 30.0,
                                        30.0, 40.0, 50.0, 60.0,
                                        2.5, 5.0, 7.5, &value), SUCCESS);
    ASSERT_NEAR(value, 15.0, DBL_EPSILON);

    ASSERT_EQ_INT(interpolate_trilinear(10.0, 0.0, 20.0, 0.0, 30.0, 0.0,
                                        60.0, 50.0, 40.0, 30.0,
                                        30.0, 20.0, 10.0, 0.0,
                                        2.5, 5.0, 7.5, &value), SUCCESS);
    ASSERT_NEAR(value, 15.0, DBL_EPSILON);

    ASSERT_EQ_INT(interpolate_trilinear(1.0, 1.0, 0.0, 1.0, 0.0, 1.0,
                                        0.0, 1.0, 2.0, 3.0,
                                        4.0, 5.0, 6.0, 7.0,
                                        1.0, 0.5, 0.5, &value), FAIL);
    ASSERT_EQ_INT(interpolate_trilinear(0.0, 1.0, 1.0, 1.0, 0.0, 1.0,
                                        0.0, 1.0, 2.0, 3.0,
                                        4.0, 5.0, 6.0, 7.0,
                                        0.5, 1.0, 0.5, &value), FAIL);
    ASSERT_EQ_INT(interpolate_trilinear(0.0, 1.0, 0.0, 1.0, 1.0, 1.0,
                                        0.0, 1.0, 2.0, 3.0,
                                        4.0, 5.0, 6.0, 7.0,
                                        0.5, 0.5, 1.0, &value), FAIL);
    ASSERT_EQ_INT(interpolate_trilinear(0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
                                        0.0, 1.0, 2.0, 3.0,
                                        4.0, 5.0, 6.0, 7.0,
                                        0.5, 0.5, 0.5, NULL), FAIL);

    return SUCCESS;
}

static int test_step_vector_functions(void)
{
    window data = {0};
    permuted_vertex vertex = {0};
    int i;

    init_hexagon(&data);
    ASSERT_EQ_INT(bounding_boxcheck(&data), SUCCESS);
    ASSERT_EQ_INT(assemble_bv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_lv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_tv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_rv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_btlv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_lttv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_rttv(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(assemble_btrv(&data, &vertex), SUCCESS);

    ASSERT_EQ_INT(vertex.row_size_btrv, 0);
    ASSERT_EQ_INT(vertex.row_size_btlv, 0);
    ASSERT_EQ_INT(vertex.row_size_lttv, 0);
    ASSERT_EQ_INT(vertex.row_size_rttv, 0);

    ASSERT_EQ_INT(minimum_rowstep(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(maximum_rowstep(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(minimum_columnstep(&data, &vertex), SUCCESS);
    ASSERT_EQ_INT(maximum_columnstep(&data, &vertex), SUCCESS);

    for (i = 0; i < data.ny; i++) {
        ASSERT_TRUE(data.nnx1[i] <= data.nnx2[i]);
    }
    for (i = 0; i < data.nx; i++) {
        ASSERT_TRUE(data.nny1[i] <= data.nny2[i]);
    }

    return SUCCESS;
}

int main(void)
{
    ASSERT_EQ_INT(test_version_header(), SUCCESS);
    ASSERT_EQ_INT(test_polygon_data_model(), SUCCESS);
    ASSERT_EQ_INT(test_polygon_api_functions(), SUCCESS);
    ASSERT_EQ_INT(test_sorting_and_unique_vertices(), SUCCESS);
    ASSERT_EQ_INT(test_window_functions(), SUCCESS);
    ASSERT_EQ_INT(test_boundary_box_and_assembly_helpers(), SUCCESS);
    ASSERT_EQ_INT(test_assembly_helpers_with_quadrant_vertices(), SUCCESS);
    ASSERT_EQ_INT(test_hanging_sweep(), SUCCESS);
    ASSERT_EQ_INT(test_boundary_assembly_and_contribution(), SUCCESS);
    ASSERT_EQ_INT(test_irregular_xy_monotone_scanlines(), SUCCESS);
    ASSERT_EQ_INT(test_linear_interpolation(), SUCCESS);
    ASSERT_EQ_INT(test_step_vector_functions(), SUCCESS);

    return SUCCESS;
}
