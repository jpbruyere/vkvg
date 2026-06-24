#pragma once

#define CHECK_MAT(_xx, _yx, _xy, _yy, _x0, _y0)                                                                        \
EXPECT_FLOAT_EQ(mat.xx, _xx);                                                                                          \
    EXPECT_FLOAT_EQ(mat.yx, _yx);                                                                                      \
    EXPECT_FLOAT_EQ(mat.xy, _xy);                                                                                      \
    EXPECT_FLOAT_EQ(mat.yy, _yy);                                                                                      \
    EXPECT_FLOAT_EQ(mat.x0, _x0);                                                                                      \
    EXPECT_FLOAT_EQ(mat.y0, _y0);


