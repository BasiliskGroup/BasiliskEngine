#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/physics/solver.h>

namespace bsk::internal {

Collider::Collider(Solver* solver, std::vector<glm::vec2> vertices)
    : table(solver->getColliderTable())
{
    table->insert(this, vertices); // sets index
}

Collider::~Collider() {
    // ColliderTable destructor handles cleanup of this collider
}

}