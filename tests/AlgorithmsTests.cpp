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


Vector2f findIntersectionBetweenLines(const Vector2f &l1_point, const Vector2f& l1_dir, const Vector2f &l2_point, const Vector2f & l2_dir) {
    auto l1_ortogonal_axis = Vector2f(l1_dir.y, -l1_dir.x);//Rotate the edge clockwise to obtain the normal
    auto distance_l2_to_l1 = l1_point - l2_point;
    auto distance_ortogonal_component = (distance_l2_to_l1).DotProduct(l1_ortogonal_axis);
    auto direction_ortogonal_component = l2_dir.DotProduct(l1_ortogonal_axis);
    auto dt = distance_ortogonal_component / direction_ortogonal_component;//Edge necessary dt throught the normal projection (minimum distance direction to cross the face)
    return l2_point + l2_dir * dt;
}


bool checkTrajectoryAgainstEdge(const std::vector<Vector2f>& box, const Vector2f& start, const Vector2f& end, Vector2f& outNormal) {
    //FIRST PART - Choose a edge potentially crossed by the trajectory
    auto edge = box[1] - box[0];//v2 - v1;
    auto edge_direction = edge.Normalized();//v2 - v1;
    auto edge_origin = box[0]; //v1
    auto normal = edge_direction.Normalized();
    normal = Vector2f(edge_direction.y, -edge_direction.x);//Rotate the edge clockwise to obtain the normal
    auto direction = (end - start).Normalized();
    //The potentially colliding edge must have a normal with projection to the oposite side of the direction
    if ((normal).DotProduct(direction) > 0) {
        //Pick the oposite edge
        edge = box[3] - box[2]; //v4 - v3;
        edge_direction = edge.Normalized();
        edge_origin = box[2]; // v3;
        normal = edge_direction.Normalized();
        normal = Vector2f(normal.y, -normal.x);
    }

    //SECOND PART - Verify if the trajectory cross the edge at a calculated dt, where dt is calculated as a multiplier factor where the ray defined by the trajectory would cross the line containing the edge
    //Used to calculate if a trajectory cross with a given edge of the box
    auto extrapolate_trajectory_and_edge_intersection = findIntersectionBetweenLines(edge_origin, edge_direction, end, start - end);

    auto projection_intersection_point_to_edge_length = (extrapolate_trajectory_and_edge_intersection - edge_origin).DotProduct(edge_direction);
    //The extremes might represent and edge case of collision with corners
    if (projection_intersection_point_to_edge_length >= 0 && projection_intersection_point_to_edge_length <= edge.Length()) {
        //Collision and intersection with face confirmed. Normal obtained. Consider the need or not of performing the next checking.
        //Next checking would be necessary in case a collision on 2 adjacent edges happens. Meaning the intersection of the 2 edges were hit.
        //Such scenario can be found by only checking the if second condition. If the hit is at the edge's length, the next edge was hit as well. No further check is needed.
        //Interpolate the normal of both edges.
        outNormal = normal;
        return true;
    }
    return false;
}

//CheckTrajectoryAgainstBox
static bool CheckTrajectoryAgainstBox(const std::vector<Vector2f> &box, const Vector2f &c, const Vector2f &d, Vector2f& escape, Vector2f& normal) {
    Vector2f v1 = box[0];
    Vector2f v2 = box[1];
    Vector2f v3 = box[2];
    Vector2f v4 = box[3];
    auto direction = (d - c).Normalized();
    
    auto GetEscapeVectorOposeToMovement = [&](std::pair<float, float> projectionValues) {
        Vector2f edgeDir = (v2 - v1).Normalized();
        Vector2f escape1 = - projectionValues.second*edgeDir;
        Vector2f escape2 = (v2 - v1) - projectionValues.first*edgeDir;
        return (escape1).DotProduct(direction) < 0 ? escape1 : escape2;
    };
    
    bool hasIntersection = false;
    std::pair<float, float> projectionValues;
    
    if(hasProjectionOverEdge(v1, v2, c, d, projectionValues)) {
        auto escapeV1 = GetEscapeVectorOposeToMovement(projectionValues);
        
        if (hasProjectionOverEdge(v2, v3, c, d, projectionValues)) {
            auto escapeV2 = GetEscapeVectorOposeToMovement(projectionValues);
            float escape_v1_project_dir = (escapeV1).DotProduct(direction);
            float escape_v2_project_dir = (escapeV2).DotProduct(direction);
            escape = (direction * escape_v1_project_dir) + (direction * escape_v2_project_dir);
            
            return checkTrajectoryAgainstEdge(box, c, d, normal) || checkTrajectoryAgainstEdge({box[1], box[2], box[3], box[0]}, c, d, normal);
        }
    }
    return false;
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
        hasIntersection = CheckTrajectoryAgainstBox(box1, c, d, escape, normal);
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

TEST(GeneralCollisionHelpers, TraceTrajectoryIntoBox_CollisionDetected) {
    std::vector<Vector2f> box{
        {-10.f, 4.2f}, {-4.3f, 0.5f}, {-2.f, 4.f}, {-7.5f, 7.5f}};
    //Vector2f start(-9, 1);
    //Vector2f end(-9.5, 4.5);

    Vector2f start(-11, 6);
    Vector2f end(-6, 8);
    Vector2f direction(Vector2f(-5, -2) - Vector2f(-8, -4));
    Vector2f normal, escape;
    
    //ASSERT_FALSE(MyCollision::hasProjectionOverEdge(start, end, {-10.f, 4.2f}, {-4.3f, 0.5f}, value));
    ASSERT_TRUE(MyCollision::CheckTrajectoryAgainstBox(box, start, end, escape, normal));
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
