#include <gtest/gtest.h>

#include <BoundingBox.h>
#include <UVector.h>

#include <algorithm>
#include <assert.h>

namespace MyCollision {

template <typename T, int dimension>
struct SCollisionResult {
    std::vector<Vector<T, dimension>> vertices;// Vertices representing the edge(s) or shape hit
    Vector<T, dimension> normal;// Resulting normal on the hit point
    Vector<T, dimension> intersection;   // Intersection point
    Vector<T, dimension> escape;      //Vector to which direction and lenth the collision can be undone
};

/// <summary>
/// 
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="p1"></param>
/// <param name="p2"></param>
/// <param name="projectionLimits"></param>
/// <returns></returns>
template <typename T, int dimention>
bool findProjectedPointsOverSegment(const Vector<T, dimention> &start, const Vector<T, dimention> &end, const std::vector<Vector<T, dimention>> &points, std::vector<T> &projected_points) {
    if (points.size() == 0) {
        return false;
    }
    auto segment_direction = end - start;
    auto segment_length = segment_direction.Length();
    segment_direction = segment_direction.Normalized();
    
    for (const auto& p : points) {
        projected_points.emplace_back((p - start).DotProduct(segment_direction));
    }
    std::sort(projected_points.begin(), projected_points.end());
    return *(projected_points.end() - 1) > 0 && *projected_points.begin() < segment_length;
}

/// <summary>
/// 
/// </summary>
/// <param name="l1_point"></param>
/// <param name="l1_dir"></param>
/// <param name="l2_point"></param>
/// <param name="l2_dir"></param>
/// <returns></returns>
template <typename T, int dimension>
Vector<T, dimension> findIntersectionBetweenLines(const Vector<T, dimension> &l1_point, const Vector<T, dimension> &l1_dir, const Vector<T, dimension> &l2_point, const Vector<T, dimension> &l2_dir) {
    auto l1_ortogonal_axis = Vector<T, dimension>(l1_dir.y, -l1_dir.x);//Rotate the edge clockwise to obtain the normal
    auto distance_l2_to_l1 = l1_point - l2_point;
    auto distance_ortogonal_component = (distance_l2_to_l1).DotProduct(l1_ortogonal_axis);
    auto direction_ortogonal_component = l2_dir.DotProduct(l1_ortogonal_axis);
    auto dt = distance_ortogonal_component / direction_ortogonal_component;//Edge necessary dt throught the normal projection (minimum distance direction to cross the face)
    return l2_point + l2_dir * dt;
}

template <typename T, int dimension>
Vector<T, dimension> getResultingNormalFromVertices(const std::vector<Vector<T, dimension>> &vertices) {
    if (vertices.size() < 2) {
        return {};
    }
    Vector<T, dimension> normal;
    auto v = vertices.begin();
    for (; v != vertices.end() - 1; v++) {
        auto direction = *(v + 1) - *v;
        normal = normal + Vector<T, dimension>(direction.y, -direction.x).Normalized();
    }
    return normal.Normalized();
}

/// <summary>
/// Algorithm for checking collision of a list of edges agains a trajectory defined by @start and @end parameters. 
/// </summary>
/// <param name="vertices"> List of vertices representing interconnected edges. </param>
/// <param name="start"> Start point of the trajectory. </param>
/// <param name="end"> End point of the trajectory. </param>
/// <param name="connectLastVertex"> Define if the last vertex of the list should be considered connected to the first. The default values is true. </param>
/// <returns>Returns a SCollisionResult structure with the information of vertices belonging to the edges hit, resulting normal and point of intersection.
/// The only case more than one edge is considered to be hit is if the intersection occurs at the point of intersections between the two edges.</returns>
template <typename T, int dimension>
SCollisionResult<T, dimension> findEdgesHitOnTrajectory(const std::vector<Vector<T, dimension>> &vertices, const Vector<T, dimension> &start, const Vector<T, dimension> &end, bool connectLastVertex = true) {
    if (vertices.size() < 2) {
        return {};
    }

    Vector<T, dimension> previous;
    std::vector<Vector<T, dimension>>::const_iterator v;
    if (connectLastVertex) {
        previous = *(vertices.end() - 1);
        v = vertices.begin();
    } else {
        previous = *vertices.begin();
        v = vertices.begin() + 1;
    }

    for (; v != vertices.end(); v++) {
        //FIRST PART - Choose a edge potentially crossed by the trajectory
        auto edge = *v - previous; //v2 - v1;
        auto edge_direction = edge.Normalized();
        auto edge_origin = previous; //v1
        auto normal = edge_direction.Normalized();
        normal = Vector<T, dimension>(edge_direction.y, -edge_direction.x);//Rotate the edge clockwise to obtain the normal
        auto direction = (end - start).Normalized();
        //The potentially colliding edge must have a normal with projection to the oposite side of the direction
        if ((normal).DotProduct(direction) > 0) {
            previous = *v;
            continue;
        }

        //SECOND PART - Verify if the trajectory cross the edge at a calculated dt, where dt is calculated as a multiplier factor where the ray defined by the trajectory would cross the line containing the edge
        //Used to calculate if a trajectory cross with a given edge of the box
        auto extrapolate_trajectory_and_edge_intersection = findIntersectionBetweenLines(edge_origin, edge_direction, end, start - end);

        auto projection_intersection_point_to_edge_length = (extrapolate_trajectory_and_edge_intersection - edge_origin).DotProduct(edge_direction);
        //The extremes might represent and edge case of collision with corners
        auto edge_length = edge.Length();
        if (projection_intersection_point_to_edge_length >= 0 && projection_intersection_point_to_edge_length <= edge_length) {
            //Collision and intersection with face confirmed. Normal obtained. Consider the need or not of performing the next checking.
            //Next checking would be necessary in case a collision on 2 adjacent edges happens. Meaning the intersection of the 2 edges were hit.
            //Such scenario can be found by only checking the if second condition. If the hit is at the edge's length, the next edge was hit as well. No further check is needed.
            //Interpolate the normal of both edges.
            if (projection_intersection_point_to_edge_length == edge_length && (v + 1 !=  vertices.end())) {
                return {{previous, *v, *(v + 1)}, getResultingNormalFromVertices<T, dimension>({previous, *v, *(v + 1)}), extrapolate_trajectory_and_edge_intersection};
            }
            return {{previous, *v}, normal, extrapolate_trajectory_and_edge_intersection};
        }
        previous = *v;
    }
    return {};
}

/// <summary>
/// 
/// </summary>
/// <param name="box"></param>
/// <param name="c"></param>
/// <param name="d"></param>
/// <param name="direction"></param>
/// <returns></returns>
template <typename T, int dimension>
static SCollisionResult<T, dimension> checkSegmentCollisionAgainstRect(const std::vector<Vector<T, dimension>> &box, const Vector<T, dimension> &c, const Vector<T, dimension> &d, const Vector<T, dimension> &direction) {   
    auto find_escape_vector_component_oposing_direction = [&direction](std::pair<float, float> projected_vertices, const Vector<T, dimension> &edge) {
        auto edge_direction = edge.Normalized();
        auto escape_towards = - projected_vertices.second*edge_direction;
        auto escape_backwards = edge - projected_vertices.first*edge_direction;
        return (escape_towards).DotProduct(direction) < 0 ? escape_towards : escape_backwards;
    };
    
    bool hasIntersection = false;
    std::vector<float> projected_points;
    
    if (findProjectedPointsOverSegment(box[0], box[1], {c, d}, projected_points)) {
        assert(projected_points.size() == 2 && "Number of projected points do not match expectation.");
        auto escape_component_v1_v2 = find_escape_vector_component_oposing_direction({projected_points[0], projected_points[1]}, box[1] - box[0]);

        projected_points.clear();
        if (findProjectedPointsOverSegment(box[1], box[2], {c, d}, projected_points)) {
            assert(projected_points.size() == 2 && "Number of projected points do not match expectation.");
            auto escape_component_v2_v3 = find_escape_vector_component_oposing_direction({projected_points[0], projected_points[1]}, box[2] - box[1]);
            T escape_v1_project_dir = (escape_component_v1_v2).DotProduct(direction);
            T escape_v2_project_dir = (escape_component_v2_v3).DotProduct(direction);

            auto escape = std::abs(escape_v1_project_dir) < std::abs(escape_v2_project_dir) ? 
               findIntersectionBetweenLines({0, 0}, direction, escape_component_v1_v2, Vector<T, dimension>(escape_component_v1_v2.y, -escape_component_v1_v2.x, escape_component_v1_v2.z).Normalized()) : 
                findIntersectionBetweenLines({0, 0}, direction, escape_component_v2_v3, Vector<T, dimension>(escape_component_v2_v3.y, -escape_component_v2_v3.x, escape_component_v2_v3.z).Normalized());
            auto result = findEdgesHitOnTrajectory({box[0], box[1], box[2], box[3]}, c, d);
            result.escape = escape;

            return result;
        }
    }
    return {};
}

/// <summary>
/// 
/// </summary>
/// <param name="box1"></param>
/// <param name="box2"></param>
/// <param name="direction"></param>
/// <returns></returns>
template <typename T, int dimension>
bool CheckBoxToBoxCollision(std::vector<Vector<T, dimension>> box1, std::vector<Vector<T, dimension>> box2, const Vector<T, dimension> &direction, SCollisionResult<T, dimension> &result) {
    if(box1.size() < 4 && box2.size() < 4) {
        return false;
    }
    
    //Algorithm for checking if there is an intersection between the bounding boxes by projecting one's edges against any 2 orthogonal edges of the other.
    //If any of the first's edges has such projection intersecting both orthogonal faces, the bounding boxes intersect each other
    for (int v = 1; v < 4; v++) {
        auto c = box2[v-1];
        auto d = box2[v];
        
        //Since the bounding box is a rectangulum, calculating the projection to one triangle, by symmetry, the other triangle will also contain the projection
        auto escape;
        auto normal;
        result = findEdgesHitOnTrajectory(box1, c, d);

        if (result.vertices.size() > 0) {
            return true;
        }
    }
    return false;
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

TEST(GeneralCollisionHelpers, TraceTrajectoryIntoBox_) {
    std::vector<Vector2f> box{
        {-9.7, 4.1}, {-4.16f, 0.59f}, {-2.f, 4.f}, {-7.5f, 7.5f}};

    Vector2f start(-11, 6);
    Vector2f end(-6, 8);
    Vector2f direction = Vector2f(1.5f, 0.5f).Normalized();
    auto result = MyCollision::checkSegmentCollisionAgainstRect(box, start, end, direction);
    if (result.vertices.size() == 2) {
        ASSERT_EQ(result.vertices[0], box[3]);
        ASSERT_EQ(result.vertices[1], box[0]);
    }
    ASSERT_FLOAT_EQ(result.intersection.x, -7.5873);
    ASSERT_FLOAT_EQ(result.intersection.y, 7.3650794);
    ASSERT_FLOAT_EQ(result.escape.x, -1.5581235);
    ASSERT_FLOAT_EQ(result.escape.y, -0.51937455);
}

TEST(GeneralCollisionHelpers, TraceTrajectoryIntoBox_ValidateIntersectionResults) {
    std::vector<Vector2f> box{
        {-9.7, 4.1}, {-4.16f, 0.59f}, {-2.f, 4.f}, {-7.5f, 7.5f}};

    Vector2f start(-11, 6);
    Vector2f end(-6, 8);
    auto result = MyCollision::findEdgesHitOnTrajectory<float, 2>(box, start, end);
    EXPECT_EQ(result.vertices.size(), 2);
    if (result.vertices.size() == 2) {
        ASSERT_EQ(result.vertices[0], box[3]);
        ASSERT_EQ(result.vertices[1], box[0]);
    }    
    ASSERT_FLOAT_EQ(result.intersection.x, -7.5873);
    ASSERT_FLOAT_EQ(result.intersection.y, 7.3650794);
}


TEST(GeneralCollisionHelpers, HasIntersectionOverEdge_TRUE) {
    Vector2f p1(3, 5);
    Vector2f p2(7, 8);
    Vector2f start(4, 3);
    Vector2f end(10, 4);
    std::vector<float> value;
    ASSERT_TRUE(MyCollision::findProjectedPointsOverSegment(start, end, {p1, p2}, value));

}

TEST(GeneralCollisionHelpers, HasIntersectionOverEdge_False) {
    Vector2f p1(-9, 1);
    Vector2f p2(-9.5, 4.2);
    Vector2f start(-10, 4.5);
    Vector2f end(-8, 8);
    std::vector<float> value;
    ASSERT_FALSE(MyCollision::findProjectedPointsOverSegment(start, end, {p1, p2}, value));
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

//The algorithms is designed so to consider the boxes with its transformations applied
TEST(CubeRayIntersectionTest, FaceBoxIntersection) {
    BoundingBox box1(Vector3f(0, 0, 0), Vector3f(5, 7, 5));
    BoundingBox box2(Vector3f(0, 0, 4.9), Vector3f(8, 10, 9));
    Vector3f direction;

    auto find_escape_vector_component_oposing_direction = [&direction](std::pair<float, float> projected_vertices, const Vector3f &edge) {
        auto edge_direction = edge.Normalized();
        auto escape_towards = -projected_vertices.second * edge_direction;
        auto escape_backwards = edge - projected_vertices.first * edge_direction;
        return (escape_towards).DotProduct(direction) < 0 ? escape_towards : escape_backwards;
    };

    //Algorithm for checking if there is an intersection between the bounding boxes by projecting one's edges points against any 2 orthogonal faces of the other.
    //If any of the first's edge's points has such projection intersecting both orthogonal faces, the bounding boxes intersect each other
    bool hasIntersection = false;
    for (int f = 0; f < 6; f++) {
        for (int v = 1; v < 6; v++) {
            Vector3f c = box1.GetVertex(static_cast<EFace>(f), v - 1);
            Vector3f d = box1.GetVertex(static_cast<EFace>(f), v);

            //Since the bounding box is a cuboid, calculating the projection to one triangle, by symmetry, the other triangle will also contain the projection
            auto checkTriangleIntersection = [&c, &d, &box2, &direction](EFace face) {
                Vector3f v1 = box2.GetVertex(face, 0);
                Vector3f v2 = box2.GetVertex(face, 1);
                Vector3f v3 = box2.GetVertex(face, 2);
                Vector3f v4 = box2.GetVertex(face, 3);
                std::vector<float> projections;
                MyCollision::checkSegmentCollisionAgainstRect({v1, v2, v3, v4}, c, d, direction);
                return false; 
            };
            if (checkTriangleIntersection(EFace::BOTTOM) && checkTriangleIntersection(EFace::BACK)) {
                hasIntersection = true;
                break;
            }
        }
    }
    ASSERT_TRUE(hasIntersection);
}
