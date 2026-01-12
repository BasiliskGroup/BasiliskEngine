#ifndef BSK_COLLISION_H
#define BSK_COLLISION_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;

/**
 * @brief Detects collision between two rigid bodies using Clipper2 and returns contact points and normal
 * 
 * This function uses Clipper2's polygon intersection algorithm to detect collisions between two
 * rigid bodies. It transforms the collider vertices from local to world space, computes the
 * intersection, and extracts contact points and penetration normal.
 * 
 * @param bodyA First rigid body
 * @param bodyB Second rigid body
 * @param verticesA Local-space vertices for bodyA (CCW-oriented polygon)
 * @param verticesB Local-space vertices for bodyB (CCW-oriented polygon)
 * @param contacts Output array for up to 4 contact points (in world space). 
 *                 Contacts[0-1] are extreme points from bodyA, contacts[2-3] from bodyB
 * @param normal Output penetration normal vector (points from A to B, normalized)
 * @return true if the bodies are colliding, false otherwise
 */
bool collide(Rigid* bodyA, Rigid* bodyB, 
             const std::vector<glm::vec2>& verticesA, 
             const std::vector<glm::vec2>& verticesB,
             glm::vec2 contacts[4], 
             glm::vec2& normal);

}

#endif