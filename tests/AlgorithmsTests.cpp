#include <gtest/gtest.h>

#include <BoundingBox.h>
#include <UVector.h>

#include <algorithm>


namespace MyCollision {

bool hasProjectionOverEdge(const Vector2f &start, const Vector2f &end, const Vector2f &p1, const Vector2f &p2, std::pair<float, float>& projectionLimits) {
    Vector2f axis = end - start;
    float axisLength = axis.Length();
    axis = axis.Normalized();
    float p1ProjectionOverAxis = (p1-start).DotProduct(axis);
    float p2ProjectionOverAxis = (p2-start).DotProduct(axis);
    float max = std::max(p1ProjectionOverAxis, p2ProjectionOverAxis);
    float min = std::min(p1ProjectionOverAxis, p2ProjectionOverAxis);
    projectionLimits = std::pair(min, max);
    return max > 0 && min < axisLength;
}

static bool CheckEdgeAgainstBoxCollision(const std::vector<Vector2f> &box1, const Vector2f &c, const Vector2f &d, const Vector2f &direction, Vector2f& escape, Vector2f& normalOut) {
    Vector2f v1 = box1[0];
    Vector2f v2 = box1[1];
    Vector2f v3 = box1[2];
    Vector2f v4 = box1[3];
    
    auto GetEscapeVectorOposeToMovement = [&](std::pair<float, float> projectionValues) {
        Vector2f edgeDir = (v2 - v1).Normalized();
        Vector2f escape1 = - projectionValues.second*edgeDir;
        Vector2f escape2 = (v2 -v1) - projectionValues.first*edgeDir;
        return (escape1).DotProduct(direction) < 0 ? escape1 : escape2;
    };
    
    bool hasIntersection = false;
    std::pair<float, float> projectionValues;
    
    if(hasProjectionOverEdge(v1, v2, c, d, projectionValues)) {
        auto escapeV1 = GetEscapeVectorOposeToMovement(projectionValues);
        
        auto edge = v2 - v1;
        auto pivot = v1;
        auto normal = edge.Normalized();
        normal = Vector2f(normal.y, -normal.x); //Rotate the edge clockwise to obtain the normal
        //The potentially colliding edge must have a normal with projection to the oposite side of the direction
        if((normal).DotProduct(direction) > 0) {
            //Pick the oposite edge
            edge = v4 - v3;
            pivot = v3;
            normal = edge.Normalized();
            normal = Vector2f(normal.y, -normal.x);
        }
        auto toPivot = pivot - d;
        auto pivotProjection = (toPivot).DotProduct(normal);
        auto edgeDirProjection = (c-d).Normalized().DotProduct(normal);
        auto dt = pivotProjection/edgeDirProjection; //Edge necessary dt throught the normal projection (minimum distance direction to cross the face)
        auto lineRayCrossFace = c + (c-d).Normalized()*dt;
        auto pivotToPointProjectionLength = (lineRayCrossFace - pivot).DotProduct(edge.Normalized());
        
 
        //The extremes might represent and edge case of collision with corners
        if((lineRayCrossFace - d).Length() <= (d-c).Length() && pivotToPointProjectionLength >= 0 && pivotToPointProjectionLength <= edge.Length()) {
            //Collision and intersection with face confirmed. Normal obtained. Consider the need or not of performing the next checking.
            normalOut = normal;
            return true;
        }
        
        if (hasProjectionOverEdge(v2, v3, c, d, projectionValues)) {
            auto escapeV2 = GetEscapeVectorOposeToMovement(projectionValues);
            float escape_v1_project_dir = (escapeV1).DotProduct(direction);
            float escape_v2_project_dir = (escapeV2).DotProduct(direction);
            escape = (direction * escape_v1_project_dir) + (direction * escape_v2_project_dir);
            
            
            edge = v3 - v2;
            pivot = v2;
            normal = edge.Normalized();
            normal = Vector2f(normal.y, -normal.x); //Rotate the edge clockwise to obtain the normal
            //The potentially colliding edge must have a normal with projection to the oposite side of the direction
            if((normal).DotProduct(direction) > 0) {
                //Pick the oposite edge
                edge = v1 - v4;
                pivot = v4;
                normal = edge.Normalized();
                normal = Vector2f(normal.y, -normal.x);
            }
            toPivot = pivot - d;
            pivotProjection = (toPivot).DotProduct(normal);
            edgeDirProjection = (c-d).Normalized().DotProduct(normal);
            dt = pivotProjection/edgeDirProjection; //Edge necessary dt throught the normal projection (minimum distance direction to cross the face)
            lineRayCrossFace = c + (c-d).Normalized()*dt;
            pivotToPointProjectionLength = (lineRayCrossFace - pivot).DotProduct(edge.Normalized());
            

            //The extremes might represent and edge case of collision with corners
            if((lineRayCrossFace - d).Length() <= (d-c).Length() && pivotToPointProjectionLength >= 0 && pivotToPointProjectionLength <= edge.Length()) {
                //Collision and intersection with face confirmed. Normal obtained. Consider the need or not of performing the next checking.
                normalOut = normal;
                return true;
            }
            
            
            hasIntersection = true;
        }
    }
    return hasIntersection;
}


bool CheckBoxToBoxCollision(std::vector<Vector2f> box1, std::vector<Vector2f> box2, const Vector2f& direction) {
    if(box1.size() < 4 && box2.size() < 4) {
        return false;
    }
    
    //Algorithm for checking if there is an intersection between the bounding boxes by projecting one's edges against any 2 orthogonal edges of the other.
    //If any of the first's edges has such projection intersecting both orthogonal faces, the bounding boxes intersect each other
    bool hasIntersection = false;
    for (int v = 1; v < 4; v++) {
        Vector2f c = box2[v-1];
        Vector2f d = box2[v];
        
        //Since the bounding box is a rectangulum, calculating the projection to one triangle, by symmetry, the other triangle will also contain the projection
        Vector2f escape;
        Vector2f normal;
        hasIntersection = CheckEdgeAgainstBoxCollision(box1, c, d, direction, escape, normal);
    }
    return hasIntersection;
}

static std::vector<Vector2f> GetBoxVerticesFromCircleTrajectory(const Vector2f& p1, const Vector2f& p2, float radius, const Vector2f& direction) {
    Vector2f dir = (p2-p1).Normalized();
    std::vector<Vector2f> vertices;
    // Obtain the quad describing the area swept by the circle during the frame
    vertices.push_back(Vector2f(-dir.y, dir.x)*radius + p1); //Conterclockwise rotation of dit
    vertices.push_back(Vector2f(dir.y, -dir.x)*radius + p1); //clockwise rotation of dir
    vertices.push_back(Vector2f(dir.y, -dir.x)*radius + p2);
    vertices.push_back(Vector2f(-dir.y, dir.x)*radius + p2);
    return vertices;
}

}

bool AssessIntersectionBoxWithRay(const BoundingBox &boundingBox, const Vector3f &raySrc, const Vector3f &rayDir) {
    //Derivative from p >= min, p >= max, p within [min, max], the 2 points defining the box. And p = O + Dx/Dt, from the ray, with p representing any point of intersection
    Vector3f upper = (boundingBox.getFirst() + boundingBox.getSecond()) - 2 * raySrc;
    upper = Vector3f(upper.x / rayDir.x, upper.y / rayDir.y, upper.z / rayDir.z);
    return upper.x > 0 && upper.y > 0 && upper.z > 0;
}

TEST(GeneralCollisionHelpers, HasIntersectionOverEdge_TRUE) {
    Vector2f p1(3, 5);
    Vector2f p2(7, 8);
    Vector2f start(4, 3);
    Vector2f end(10, 4);
    std::pair<float, float> value;
    ASSERT_TRUE(MyCollision::hasProjectionOverEdge(start, end, p1, p2, value));

}

TEST(GeneralCollisionHelpers, HasIntersectionOverEdge_False) {
    Vector2f p1(-9, 1);
    Vector2f p2(-9.5, 4.2);
    Vector2f start(-10, 4.5);
    Vector2f end(-8, 8);
    std::pair<float, float> value;
    ASSERT_FALSE(MyCollision::hasProjectionOverEdge(start, end, p1, p2, value));
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

//The algorithms is designed so to consider the boxes with its transformations applied
TEST(CubeRayIntersectionTest, FaceBoxIntersection) {
    BoundingBox box1(Vector3f(0, 0, 0), Vector3f(5, 7, 5));
    BoundingBox box2(Vector3f(0, 0, 4.9), Vector3f(8, 10, 9));

    //Algorithm for checking if there is an intersection between the bounding boxes by projecting one's edges points against any 2 orthogonal faces of the other.
    //If any of the first's edge's points has such projection intersecting both orthogonal faces, the bounding boxes intersect each other
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
