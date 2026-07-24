#pragma once

#include "BoundingBox.h"
#include "BoundingBox2D.h"
#include "UVector.h"
#include "Matrix.h"

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


/// <summary>
///
/// </summary>
/// <param name="l1_point"></param>
/// <param name="l1_dir"></param>
/// <param name="l2_point"></param>
/// <param name="l2_dir"></param>
/// <returns></returns>
template<typename T, int dimension>
Vector<T, dimension> findIntersectionBetweenLines(const Vector<T, dimension> &l1_point, const Vector<T, dimension> &l1_dir, const Vector<T, dimension> &l2_point, const Vector<T, dimension> &l2_dir);

/// <summary>
/// Algorithm for checking collision of a list of edges against a trajectory defined by @start and @end parameters.
/// </summary>
/// <param name="vertices"> List of vertices representing interconnected edges. </param>
/// <param name="start"> Start point of the trajectory. </param>
/// <param name="end"> End point of the trajectory. </param>
/// <param name="connectLastVertex"> Define if the last vertex of the list should be considered connected to the first. The default values is true. </param>
/// <returns>Returns a SCollisionResult structure with the information of vertices belonging to the edges hit, resulting normal and point of intersection.
/// The only case more than one edge is considered to be hit is if the intersection occurs at the point of intersections between the two edges.</returns>
template<typename T, int dimension>
SCollisionResult<T, dimension> checkPathIntersectionWithSegment(const std::vector<Vector<T, dimension>> &vertices, const Vector<T, dimension> &start, const Vector<T, dimension> &end, bool connectLastVertex = true);

/// <summary>
///
/// </summary>
/// <param name="box"></param>]]]]]][cx 
/// <param name="c"></param>
/// <param name="d"></param>
/// <param name="direction"></param>
/// <returns></returns>
template<typename T, int dimension>
SCollisionResult<T, dimension> checkSegmentCollisionAgainstRect(const std::vector<Vector<T, dimension>> &box, const Vector<T, dimension> &c, const Vector<T, dimension> &d, const Vector<T, dimension> &direction);

/// <summary>
///
/// </summary>
/// <param name="box1"></param>
/// <param name="box2"></param>
/// <param name="direction"></param>
/// <returns></returns>
template<typename T, int dimension>
bool CheckBoxToBoxCollision(std::vector<Vector<T, dimension>> box1, std::vector<Vector<T, dimension>> box2, const Vector<T, dimension> &direction, SCollisionResult<T, dimension> &result);


std::vector<Vector3f> ApplyTransformationToVerticesArray(const std::vector<float> vertices, const Matrix4f &transformation);


}// namespace unboxing_engine::algorithms
