#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Matrix.h>

TEST(MatrixTest, intantiate_zeroed) {
    float sum = 0;
    Matrix3f matrix3f;
    for (int i = 0; i < 9; ++i) {
        sum += matrix3f.at(i);
    }
    Matrix3f matrix4f;
    for (int i = 0; i < 9; ++i) {
        sum += matrix4f.at(i);
    }
    ASSERT_EQ(sum, 0);
}

TEST(MatrixTest, intantiate_array) {
    auto arr = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f};
    Matrix4f matrix4f(arr);
    bool not_equal = false;
    for (size_t i = 0; i < matrix4f.array_size(); ++i) {
        if (i < 9) {
            if (data(arr)[i] != matrix4f.at(i)) {
                not_equal = true;
            }
        } else {
            EXPECT_EQ(matrix4f.at(i), 0);
        }
    }
    ASSERT_FALSE(not_equal);
}

TEST(MatrixTest, instantiate_matrix3f_with_vector3d) {
    Vector2Df vector2d(2, 4);
    Matrix3f matrix(vector2d);
//    Matrix4f m(Vector3Df (1,2,3));
//
//    ASSERT_EQ(matrix.at(0, 2), vector2d.x);
//    ASSERT_EQ(matrix.at(1, 2), vector2d.y);
}
