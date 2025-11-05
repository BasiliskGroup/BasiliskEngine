#include <basilisk/solver/physics.h>

namespace bsk::internal {

Collider::Collider(Solver* solver, std::vector<glm::vec2> verts) : solver(solver) {
    index = getColliderTable()->insert(this, verts);
}

Collider::~Collider() {
    // remove self from ColliderFlat
    getColliderTable()->markAsDeleted(index);
}

ColliderTable* Collider::getColliderTable() {
    return solver->getColliderTable();
}

}