#pragma once

#include "BoundingBox.h"
#include "BoundingBox2D.h"
#include "UVector.h"

#include <algorithm>
#include <assert.h>
#include <vector>
namespace unboxing_engine::algorithms {

template<typename T, int dimension>
struct SCollisionResult {
    std::vector<Vector<T, dimension>> vertices;// Vertices representing the edge(s) or shape hit
    Vector<T, dimension> normal;               // Resulting normal on the hit point
    Vector<T, dimension> intersection;         // Intersection point
    Vector<T, dimension> escape;               //Vector to which direction and lenth the collision can be undone
};

template<typename T, int dimension>
bool isInsideTriangle(const std::vector<Vector<T, dimension>> &vertices, const Vector<T, dimension> &p) {
    assert(vertices.size() >= 3 && "A triangle is expected to have 3 vertices.");

    auto edge_a = vertices[2] - vertices[0];
    auto edge_b = vertices[1] - vertices[0];
    auto edge_a_length = edge_a.Length();
    auto edge_b_length = edge_b.Length();
    auto edge_b_normalized = edge_b / edge_b_length;
    auto edge_a_normalized = edge_a / edge_a_length;

    auto is_inside = [](const std::vector<Vector<T, dimension>> &vertices, const auto &p) {
        auto edge_a = vertices[2] - vertices[0];
        auto edge_b = vertices[1] - vertices[0];
        auto edge_a_length = edge_a.Length();
        auto edge_b_length = edge_b.Length();
        auto edge_a_normalized = edge_a / edge_a_length;
        auto edge_b_normalized = edge_b / edge_b_length;

        auto v0_p = p - vertices[0];
        auto projection_p_b = v0_p.DotProduct(edge_b_normalized);

        if (projection_p_b >= 0 && projection_p_b <= edge_b_length) {
            auto projection_p_a = v0_p.DotProduct(edge_a_normalized);
            return projection_p_a / edge_a_length + projection_p_b / edge_b_length <= 1;
        }
        return false;
    };

    auto projection_a_b = edge_a.DotProduct(edge_b_normalized);
    //If right angle is on the vertex 0, the projection between ortogonal segments is 0.
    if (projection_a_b == 0) {
        return is_inside({vertices[0], vertices[1], vertices[2]}, p);
    }
    //If right angle is on the vertex 1, the projection of its oposite segment, v0v2, should equals its adjacenet segment, v0v1, length
    if (projection_a_b == edge_b_length) {
        return is_inside({vertices[1], vertices[2], vertices[0]}, p);
    }
    //If right angle is on the vertex 2, the projection of its oposite segment, v0v1, should equals its adjacenet segment, v0v2, length
    auto projection_b_a = edge_b.DotProduct(edge_a.Normalized());
    if (projection_b_a == edge_a_length) {
        return is_inside({vertices[2], vertices[0], vertices[1]}, p);
    }
    //If projection of edge a over b, is less than edge b length, splits the triangle on the edge b to vertice 1
    if (projection_a_b > 0 && projection_a_b < edge_b_length) {
        auto p0 = vertices[0] + edge_b_normalized * projection_a_b;
        return is_inside({p0, vertices[2], vertices[0]}, p) ||
               is_inside({p0, vertices[1], vertices[2]}, p);
    }
    //If projection of edge b over a, is less than edge a length, splits the triangle on the edge a to vertice 1
    if (projection_b_a > 0 && projection_b_a < edge_a_length) {
        auto p0 = vertices[0] + edge_a_normalized * projection_b_a;
        return is_inside({p0, vertices[0], vertices[1]}, p) ||
               is_inside({p0, vertices[1], vertices[2]}, p);
    }
    auto edge_c = vertices[2] - vertices[1];
    auto edge_c_length = edge_c.Length();
    auto edge_c_normalized = edge_c.Normalized();
    edge_b = -1 * edge_b;
    auto projection_b_c = edge_b.DotProduct(edge_c_normalized);
    //If projection of edge b over a, is less than edge a length, splits the triangle on the edge a to vertice 1
    if (projection_b_c > 0 && projection_b_c < edge_c_length) {
        auto p0 = vertices[1] + edge_c_normalized * projection_b_c;
        return is_inside({p0, vertices[2], vertices[0]}, p) ||
               is_inside({p0, vertices[0], vertices[1]}, p);
    }
}

/// <summary>
///
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="p1"></param>
/// <param name="p2"></param>
/// <param name="projectionLimits"></param>
/// <returns></returns>
template<typename T, int dimention>
bool findProjectedPointsOverSegment(const Vector<T, dimention> &start, const Vector<T, dimention> &end, const std::vector<Vector<T, dimention>> &points, std::vector<T> &projected_points) {
    if (points.size() == 0) {
        return false;
    }
    auto segment_direction = end - start;
    auto segment_length = segment_direction.Length();
    segment_direction = segment_direction.Normalized();

    for (const auto &p: points) {
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
template<typename T, int dimension>
Vector<T, dimension> findIntersectionBetweenLines(const Vector<T, dimension> &l1_point, const Vector<T, dimension> &l1_dir, const Vector<T, dimension> &l2_point, const Vector<T, dimension> &l2_dir) {
    auto l1_ortogonal_axis = Vector<T, dimension>(l1_dir.y, -l1_dir.x);//Rotate the edge clockwise to obtain the normal
    auto distance_l2_to_l1 = l1_point - l2_point;
    auto distance_ortogonal_component = (distance_l2_to_l1).DotProduct(l1_ortogonal_axis);
    auto direction_ortogonal_component = l2_dir.DotProduct(l1_ortogonal_axis);
    auto dt = distance_ortogonal_component / direction_ortogonal_component;//Edge necessary dt throught the normal projection (minimum distance direction to cross the face)
    return l2_point + l2_dir * dt;
}

template<typename T, int dimension>
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
template<typename T, int dimension>
SCollisionResult<T, dimension> checkPolygonIntersectionWithSegment(const std::vector<Vector<T, dimension>> &vertices, const Vector<T, dimension> &start, const Vector<T, dimension> &end, bool connectLastVertex = true) {
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
        auto edge = *v - previous;//v2 - v1;
        auto edge_direction = edge.Normalized();
        auto edge_origin = previous;//v1
        auto normal = edge_direction.Normalized();
        normal = Vector<T, dimension>(edge_direction.y, -edge_direction.x);//Rotate the edge clockwise to obtain the normal
        auto segment = end - start;
        auto segment_direction = segment.Normalized();
        //The potentially colliding edge must have a normal with projection to the oposite side of the direction. And the lines cannot be parallel
        if ((normal).DotProduct(segment_direction) >= 0) {
            previous = *v;
            continue;
        }


        //SECOND PART - Verify if the trajectory cross the edge at a calculated dt, where dt is calculated as a multiplier factor where the ray defined by the trajectory would cross the line containing the edge
        //Used to calculate if a trajectory cross with a given edge of the box
        auto extrapolate_trajectory_and_edge_intersection = findIntersectionBetweenLines(edge_origin, edge_direction, end, start - end);

        //Check if the intersection is within the segment points
        auto intersection_projection_on_to_segment = (extrapolate_trajectory_and_edge_intersection - start).DotProduct(segment_direction);
        if (intersection_projection_on_to_segment < 0 || intersection_projection_on_to_segment > segment.Length()) {
            previous = *v;
            continue;
        }

        //Check if the intersection is within the segment points
        auto edge_length = edge.Length();
        auto intersection_projection_on_to_edge = (extrapolate_trajectory_and_edge_intersection - edge_origin).DotProduct(edge_direction);
        if (intersection_projection_on_to_edge < 0 && intersection_projection_on_to_edge > edge_length) {
            previous = *v;
            continue;
        }

        // If the intersection is on edge's length, the next edge was hit as well. No further check is needed.
        //Interpolate the normal of both edges.
        if (intersection_projection_on_to_edge == edge_length && (v + 1 != vertices.end())) {
            return {{previous, *v, *(v + 1)}, getResultingNormalFromVertices<T, dimension>({previous, *v, *(v + 1)}), extrapolate_trajectory_and_edge_intersection};
        }
        return {{previous, *v}, normal, extrapolate_trajectory_and_edge_intersection};

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
template<typename T, int dimension>
static SCollisionResult<T, dimension> checkSegmentCollisionAgainstRect(const std::vector<Vector<T, dimension>> &box, const Vector<T, dimension> &c, const Vector<T, dimension> &d, const Vector<T, dimension> &direction) {

    auto find_edge_escape_vector = [&](std::pair<float, float> projected_vertices, const std::vector<Vector<T, dimension>> vertices, Vector<T, dimension> &escape, Vector<T, dimension> &adjacent_edge_dir) {
        auto edge = vertices[1] - vertices[0];
        auto edge_direction = edge.Normalized();
        escape = -projected_vertices.second * edge_direction;//Pointing towards v0
        adjacent_edge_dir = (vertices[0] - vertices[3]).Normalized();
        if (escape.DotProduct(direction) < 0) {
            escape = edge - projected_vertices.first * edge_direction;//pointing towards v1
            adjacent_edge_dir = (vertices[2] - vertices[1]).Normalized();
        }
    };


    bool hasIntersection = false;

    std::vector<float> projected_points_on_v1_v2;
    if (findProjectedPointsOverSegment(box[0], box[1], {c, d}, projected_points_on_v1_v2)) {
        assert(projected_points_on_v1_v2.size() == 2 && "Number of projected points do not match expectation.");
        std::vector<float> projected_points_on_v2_v3;
        if (findProjectedPointsOverSegment(box[1], box[2], {c, d}, projected_points_on_v2_v3)) {
            assert(projected_points_on_v2_v3.size() == 2 && "Number of projected points do not match expectation.");


            //escape gives a point from which the segment defined by the adjacent edge, starts.
            //A vector contrary to the direction must cross that segment at some point to revert the collision.
            //To find the point where it cross, we calculate the intersection between the direction and the segment.
            Vector<T, dimension> escape_v1, adjacent_edge_direction_v1, escape_v2, adjacent_edge_direction_v2;
            find_edge_escape_vector(
                {projected_points_on_v1_v2[0], projected_points_on_v1_v2[1]},
                box,
                escape_v1,
                adjacent_edge_direction_v1);
            //Adjacent edge now is considered main edge
            find_edge_escape_vector(
                {projected_points_on_v2_v3[0], projected_points_on_v2_v3[1]},
                {box[1], box[2], box[3], box[0]},
                escape_v2,
                adjacent_edge_direction_v2);

            SCollisionResult<T, dimension> result;
            Vector<float, dimension> escape;
            if (std::abs(escape_v1.DotProduct(direction)) < std::abs(escape_v2.DotProduct(direction))) {
                result.escape = findIntersectionBetweenLines({0, 0, 0}, direction, escape_v1, adjacent_edge_direction_v1);
                result.normal = -1 * adjacent_edge_direction_v1;
                result.intersection = box[1] + result.escape.DotProduct(adjacent_edge_direction_v1) * (box[2] - box[1]).Normalized();
            } else {
                result.escape = findIntersectionBetweenLines({0, 0, 0}, direction, escape_v2, adjacent_edge_direction_v2);
                result.normal = -1 * adjacent_edge_direction_v2;
                result.intersection = box[2] + result.escape.DotProduct(adjacent_edge_direction_v2) * (box[3] - box[2]).Normalized();
            }
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
template<typename T, int dimension>
bool CheckBoxToBoxCollision(std::vector<Vector<T, dimension>> box1, std::vector<Vector<T, dimension>> box2, const Vector<T, dimension> &direction, SCollisionResult<T, dimension> &result) {
    if (box1.size() < 4 && box2.size() < 4) {
        return false;
    }

    //Algorithm for checking if there is an intersection between the bounding boxes by projecting one's edges against any 2 orthogonal edges of the other.
    //If any of the first's edges has such projection intersecting both orthogonal faces, the bounding boxes intersect each other
    for (int v = 1; v < 4; v++) {
        auto c = box2[v - 1];
        auto d = box2[v];

        //Since the bounding box is a rectangulum, calculating the projection to one triangle, by symmetry, the other triangle will also contain the projection
        auto escape;
        auto normal;
        result = checkPolygonIntersectionWithSegment(box1, c, d);

        if (result.vertices.size() > 0) {
            return true;
        }
    }
    return false;
}

static std::vector<Vector2f> GetBoxVerticesFromCircleTrajectory(const Vector2f &p1, const Vector2f &p2, float radius, const Vector2f &direction) {
    Vector2f dir = (p2 - p1).Normalized();
    std::vector<Vector2f> vertices;
    // Obtain the quad describing the area swept by the circle during the frame
    vertices.push_back(Vector2f(-dir.y, dir.x) * radius + p1);//Conterclockwise rotation of dit
    vertices.push_back(Vector2f(dir.y, -dir.x) * radius + p1);//clockwise rotation of dir
    vertices.push_back(Vector2f(dir.y, -dir.x) * radius + p2);
    vertices.push_back(Vector2f(-dir.y, dir.x) * radius + p2);
    return vertices;
}


static bool AssessIntersectionBoxWithRay(const CBoundingBox3D &boundingBox, const Vector3f &raySrc, const Vector3f &rayDir) {
    //Derivative from p >= min, p >= max, p within [min, max], the 2 points defining the box. And p = O + Dx/Dt, from the ray, with p representing any point of intersection
    Vector3f upper = (boundingBox.getFirst() + boundingBox.getSecond()) - 2 * raySrc;
    upper = Vector3f(upper.x / rayDir.x, upper.y / rayDir.y, upper.z / rayDir.z);
    return upper.x > 0 && upper.y > 0 && upper.z > 0;
}

}// namespace unboxing_engine::algorithms
