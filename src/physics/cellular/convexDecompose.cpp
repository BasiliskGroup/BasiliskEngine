#include <basilisk/physics/cellular/convexDecompose.h>

namespace bsk::internal {

Convex::Convex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
    : vertices{a, b, c}
{
}

} // namespace bsk::internal
