#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Matrix.h>

//Compare two matrices, m1, m2, given the desired precision, p
template <typename T, int R, int C>
bool matrix_match_precision(Matrix<T, R, C> m0, Matrix<T, R, C> m1, float p) {
    for (int i = 0; i < m0.array_size(); ++i) {
        if(std::abs(m1.at(i) - m0.at(i)) > p) {
            return false;
        }
    }
    return true;
}

TEST(MatrixTest, intantiate_zeroed) {//NOLINT (gtest static memory warning for test_info_)
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

TEST(MatrixTest, intantiate_array) {//NOLINT (gtest static memory warning for test_info_)
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

TEST(MatrixTest, instantiate_matrix3f_with_vector2d) {//NOLINT (gtest static memory warning for test_info_)
    Vector2f vector2d(2, 4);
    Matrix3f matrix(vector2d);

    ASSERT_EQ(matrix.at(0, 2), vector2d.x);
    ASSERT_EQ(matrix.at(1, 2), vector2d.y);
    ASSERT_EQ(matrix.at(2, 2), 1);
}

TEST(MatrixTest, instantiate_matrix4f_with_vector3d) {//NOLINT (gtest static memory warning for test_info_)
    Vector3f vector3d(2, 4, 5);
    Matrix4f matrix(vector3d);

    ASSERT_EQ(matrix.at(0, 3), vector3d.x);
    ASSERT_EQ(matrix.at(1, 3), vector3d.y);
    ASSERT_EQ(matrix.at(2, 3), vector3d.z);
    ASSERT_EQ(matrix.at(3, 3), 1);
}

TEST(MatrixTest, create_identity_matrix) {//NOLINT (gtest static memory warning for test_info_)
    Matrix4f matrix = Matrix4f::Identity();
    bool is_correct = true;
    for (int i = 0; i < matrix.rows; i++) {
        for (int j = 0; j < matrix.cols; j++) {
            if (i == j) {
                is_correct = matrix.at(i, j) == 1 && is_correct;
            } else {
                is_correct = matrix.at(i, j) == 0 && is_correct;
            }
        }
    }
    ASSERT_TRUE(is_correct);
}

//NOLINTNEXTLINE (gtest static memory warning for test_info_)
TEST(MatrixTest, create_rotation_matrix_from_quarernion) {
    Quaternion q(35, Vector3f(1, 0, 0));
    Matrix3f rotation_matrix = Matrix3f::RotationMatrix(q.normalizado());

    Matrix3f expected_solution{1.00000, 0.00000, 0.00000,
                               0.00000, 0.81915, -0.57357,
                               0.00000, 0.57357, 0.81915};

    ASSERT_TRUE(matrix_match_precision(rotation_matrix, expected_solution, 0.00001));
}

//NOLINTNEXTLINE (gtest static memory warning for test_info_)
TEST(MatrixTest, calculate_matrix_inverse) {
    Matrix3f matrix({4, -2, 1,
                     5, 0, 3,
                     -1, 2, 6});

    auto solution = matrix.Inverse();

    Matrix3f inverse({-0.11538, 0.26923, -0.11538,
                      -0.63461, 0.48076, -0.13461,
                      0.19230, -0.11538, 0.19230});
    ASSERT_TRUE(matrix_match_precision(solution, inverse, 0.00001));
}

//NOLINTNEXTLINE (gtest static memory warning for test_info_)
TEST(MatrixTest, matrix_non_inverseble) {
    Matrix3f matrix({4, -2, 1,
                     0, 0, 0,
                     -1, 2, 6});

    auto solution = matrix.Inverse();
    ASSERT_EQ(solution, Matrix3f::INVALID);
}

TEST(MatrixTest, multiply_by_vector) {
    Matrix4f transform = Matrix4f::ScaleMatrix(Vector3f(0.5f, 0.5f, 0.5f));
    EXPECT_EQ(transform, Matrix4f({
            0.5f, 0, 0, 0, 
            0, 0.5f, 0, 0, 
            0, 0, 0.5f, 0, 
            0, 0, 0, 1}));

   Vector3f point(0.5f, 0.5f, 0.5f);
   Vector3f scaled_point = transform * point;
   ASSERT_EQ(scaled_point, Vector3f({0.25, 0.25, 0.25}));
}
