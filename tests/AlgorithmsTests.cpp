#include <gtest/gtest.h>

#include <BoundingBox.h>
#include <UVector.h>

#include <algorithm>

bool AssessIntersectionBoxWithRay(const BoundingBox &boundingBox, const Vector3f &raySrc, const Vector3f &rayDir) {
    //Derivative from p >= min, p >= max, p within [min, max], the 2 points defining the box. And p = O + Dx/Dt, from the ray, with p representing any point of intersection
    Vector3f upper = (boundingBox.getFirst() + boundingBox.getSecond()) - 2 * raySrc;
    upper = Vector3f(upper.x / rayDir.x, upper.y / rayDir.y, upper.z / rayDir.z);
    return upper.x > 0 && upper.y > 0 && upper.z > 0;
}

TEST(CubeRayIntersectionTest, DetectIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(2, 4, 2);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(1, 1, -1);

    ASSERT_TRUE(AssessIntersectionBoxWithRay(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

TEST(CubeRayIntersectionTest, NoIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(2, 4, 2);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(1, 1, 4);

    ASSERT_FALSE(AssessIntersectionBoxWithRay(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

TEST(CubeRayIntersectionTest, CloseCallNoIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(1, 1, 1);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(-0.1, 1, -4);

    ASSERT_FALSE(AssessIntersectionBoxWithRay(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

TEST(CubeRayIntersectionTest, TangentIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(1, 1, 1);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(0, 1, -4);

    ASSERT_TRUE(AssessIntersectionBoxWithRay(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

//Algorithm for checking if an edge ab, has an intersecting projection over the other, cd
bool hasProjectingIntersection(const Vector3f &a, const Vector3f &b, const Vector3f &c, const Vector3f &d) {
    Vector3f edge = b - a;
    float ab = edge.Length();
    edge = edge.Normalized();
    float e = (c - a).DotProduct(edge);
    float f = (d - a).DotProduct(edge);
    float max = std::max(e, f);
    float min = std::min(e, f);
    return max >= 0 && min <= ab;
}

TEST(EdgeProjectingIntersection, EdgeProjectionHasIntersection) {
    Vector3f a(0, 0, 0);
    Vector3f b(10, 0, 0);
    Vector3f c(8, 5, 0);
    Vector3f d(15, 10, 0);
    ASSERT_TRUE(hasProjectingIntersection(a, b, c, d));
}

TEST(CubeRayIntersectionTest, FaceBoxIntersection) {
    BoundingBox box1(Vector3f(0, 0, 0), Vector3f(5, 7, 5));
    BoundingBox box2(Vector3f(0, 0, 4.9), Vector3f(8, 10, 9));

    //Algorithm for checking if there is an intersection between the bounding boxes by projecting one's edges against any 2 orthogonal faces of the other.
    //If any of the first's edges has such projection intersecting both orthogonal faces, the bounding boxes intersect each other
    bool hasIntersection = false;
    for (int f = 0; f < 6; f++) {
        for (int v = 1; v < 6; v++) {
            Vector3f c = box1.GetVertex(static_cast<EFace>(f), v - 1);
            Vector3f d = box1.GetVertex(static_cast<EFace>(f), v);

            //Since the bounding box is a cuboid, calculating the projection to one triangle, by symmetry, the other triangle will also contain the projection
            auto checkTriangleIntersection = [&c, &d, &box2](EFace face) {
                Vector3f v1 = box2.GetVertex(face, 0);
                Vector3f v2 = box2.GetVertex(face, 1);
                Vector3f v3 = box2.GetVertex(face, 2);
                return hasProjectingIntersection(v1, v2, c, d) && hasProjectingIntersection(v2, v3, c, d);
            };
            if (checkTriangleIntersection(EFace::BOTTOM) && checkTriangleIntersection(EFace::BACK)) {
                hasIntersection = true;
                break;
            }
        }
    }
    ASSERT_TRUE(hasIntersection);
}
