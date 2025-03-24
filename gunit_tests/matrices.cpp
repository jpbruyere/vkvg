#include "vkvg.h"
#include <gtest/gtest.h>

#define CHECK_MAT(_xx,_yx,_xy,_yy,_x0,_y0) \
EXPECT_FLOAT_EQ (mat.xx, _xx);EXPECT_FLOAT_EQ (mat.yx, _yx);\
EXPECT_FLOAT_EQ (mat.xy, _xy);EXPECT_FLOAT_EQ (mat.yy, _yy);\
EXPECT_FLOAT_EQ (mat.x0, _x0);EXPECT_FLOAT_EQ (mat.y0, _y0);


TEST(MatrixTest, MatixInit) {
    vkvg_matrix_t mat = {1,0,0,1,0,0};
    CHECK_MAT(1,0,0,1,0,0);

    vkvg_matrix_init_identity(&mat);
    CHECK_MAT(1,0,0,1,0,0);

    vkvg_matrix_init(&mat, 1.3f, 2.5f, 0.3f, 0.7f, 1.2f, 1.7f);
    CHECK_MAT(1.3f, 2.5f, 0.3f, 0.7f, 1.2f, 1.7f);

    float a,b;

    vkvg_matrix_init_translate(&mat, 1.3f, 2.5f);
    CHECK_MAT(1,0,0,1,1.3f,2.5f);

    vkvg_matrix_init_scale(&mat, 2.1f, 1.5f);
    CHECK_MAT(2.1f,0,0,1.5f,0,0);
    vkvg_matrix_get_scale(&mat, &a, &b);
    EXPECT_FLOAT_EQ(a,2.1f);
    EXPECT_FLOAT_EQ(b,1.5f);

    vkvg_matrix_init_rotate(&mat, 2.f);
    CHECK_MAT(cosf(2.f),sinf(2.f),-sinf(2.f),cosf(2.f),0,0);
}

TEST(MatrixTest, MatixOperation) {
    vkvg_matrix_t mat = {0,0,0,0,0,0};
    //TODO

}
TEST(MatrixTest, MatixIvert) {
    vkvg_matrix_t mat = {0,0,0,0,0,0};
    vkvg_status_t res = vkvg_matrix_invert(&mat);
    EXPECT_EQ(res,VKVG_STATUS_INVALID_MATRIX);

    mat = {1,1,0,0,0,0};
    res = vkvg_matrix_invert(&mat);
    EXPECT_EQ(res,VKVG_STATUS_INVALID_MATRIX);

    mat = {1,0,0,1,0,0};
    res = vkvg_matrix_invert(&mat);
    EXPECT_EQ(res,VKVG_STATUS_SUCCESS);
    CHECK_MAT(1,0,0,1,0,0);

    vkvg_matrix_init_scale(&mat, 2.1f, 1.5f);
    res = vkvg_matrix_invert(&mat);
    EXPECT_EQ(res,VKVG_STATUS_SUCCESS);
    CHECK_MAT(1.f/2.1f,0,0,1.f/1.5f,0,0);

    vkvg_matrix_init_translate(&mat, 2.1f, 1.5f);
    res = vkvg_matrix_invert(&mat);
    EXPECT_EQ(res,VKVG_STATUS_SUCCESS);
    CHECK_MAT(1,0,0,1,-2.1f,-1.5f);

    vkvg_matrix_init_rotate(&mat, 2.f);
    res = vkvg_matrix_invert(&mat);
    EXPECT_EQ(res,VKVG_STATUS_SUCCESS);
    CHECK_MAT(cosf(2.f),-sinf(2.f),sinf(2.f),cosf(2.f),0,0);


}
