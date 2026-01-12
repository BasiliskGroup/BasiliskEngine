#ifndef BSK_COLLISION_GEOMETRY_H
#define BSK_COLLISION_GEOMETRY_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

/**
 * @brief Computes the axis-aligned bounding box (AABB) for a set of 2D vertices
 * 
 * @param vertices Vector of 2D vertices representing a polygon or point cloud
 * @return std::pair<glm::vec2, glm::vec2> First element is the minimum corner (min.x, min.y), second is the maximum corner (max.x, max.y)
 */
std::pair<glm::vec2, glm::vec2> getAABB(const std::vector<glm::vec2>& vertices);

/**
 * @brief Computes mass properties (area and moment of inertia) for a CCW-oriented 2D polygon
 * 
 * Computes the area and moment of inertia about the centroid for a counter-clockwise oriented polygon.
 * Uses the shoelace formula for area and the parallel axis theorem for moment of inertia.
 * 
 * @param vertices Vector of 2D vertices representing a CCW-oriented polygon (must have at least 3 vertices)
 * @param com Output parameter that will be set to the center of mass (centroid) of the polygon
 * @return std::pair<float, float> First element is the area, second is the moment of inertia about the centroid
 */
std::pair<float, float> getMassProperties(const std::vector<glm::vec2>& vertices, glm::vec2& com);

}

#endif