#ifndef BSK_EPA_H
#define BSK_EPA_H

#include <basilisk/physics/collision/gjk.h>

#include <glm/glm.hpp>
#include <optional>

namespace bsk::internal {

// Expanding Polytope Algorithm in 2D on the Minkowski difference A − B.
// `pair.simplex` must be the terminating 3-point simplex from gjk() (origin inside).
// Returns a unit separation normal (direction from shape B toward shape A in world space)
// if EPA succeeds; std::nullopt on degeneracy or iteration limit.
std::optional<glm::vec2> epa(const ConvexShape& shapeA, const ConvexShape& shapeB, const CollisionPair& pair);

} // namespace bsk::internal

#endif
