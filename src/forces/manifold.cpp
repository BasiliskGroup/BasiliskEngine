#include <basilisk/solver/physics.h>

namespace bsk::internal {

Manifold::Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB, uint index) : Force(solver, bodyA, bodyB) {

    for (uint i = 0; i < MANIFOLD_ROWS; i++) {
        getJ()[i] = { 0, 0, 0 };
        getH()[i] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        getC()[i] = 0.0f;
        getStiffness()[i] = std::numeric_limits<float>::infinity();
        getFmax()[i] = std::numeric_limits<float>::infinity();
        getFmin()[i] = -std::numeric_limits<float>::infinity();
        getFracture()[i] = std::numeric_limits<float>::infinity();

        getPenalty()[i] = 0.0f;
        getLambda()[i] = 0.0f;
    }

    getFmax()[0] = getFmax()[2] = 0.0f;
    getFmin()[0] = getFmin()[2] = -std::numeric_limits<float>::infinity();

    type = MANIFOLD;
}

Manifold::~Manifold() {

}

void Manifold::draw() const {

}

void Manifold::initialize() {    
    getFriction() = sqrt(bodyA->getFriction() * bodyB->getFriction());

    for (uint i = 0; i < 2; i++) {
        penalty[i * 2 + JN] = penalty[i * 2 + JT] = 0.0f;
        lambda[i * 2 + JN] = lambda[i * 2 + JT] = 0.0f;

        getTangent() = { getNormal().y, -getNormal().x };
        getBasis() = {
            getNormal().x, getNormal().y,
            getTangent().x, getTangent().y
        };

        getRAW()[i] = bodyA->getRMat() * getRA()[i];
        getRBW()[i] = bodyB->getRMat() * getRB()[i];

        JAn[i] = glm::vec3{ getNormal().x, getNormal().y, cross(getRAW()[i], getNormal()) };
        JAt[i] = glm::vec3{ getTangent().y, getTangent().y, cross(getRAW()[i], getTangent()) };
        JBn[i] = glm::vec3{ -getNormal().x, -getNormal().y, -cross(getRBW()[i], getNormal()) };
        JBt[i] = glm::vec3{ -getTangent().y, -getTangent().y, -cross(getRBW()[i], getTangent()) };

        glm::vec2 posA = glm::vec2(bodyA->getPos());
        glm::vec2 posB = glm::vec2(bodyB->getPos());
        glm::vec2 diff = posA + getRAW()[i] - posB - getRBW()[i];
        getC0()[i] = getBasis() * diff;
    }
}

void Manifold::computeConstraint(float alpha) {
    for (uint i = 0; i < 2; i++) {
        glm::vec3 dpA = bodyA->getPos() - bodyA->getInitial();
        glm::vec3 dpB = bodyB->getPos() - bodyB->getInitial();

        getC()[2 * i + JN] = getC0()[i].x * (1 - alpha) + glm::dot(JAn[i], dpA) + glm::dot(JBn[i], dpB);
        getC()[2 * i + JT] = getC0()[i].y * (1 - alpha) + glm::dot(JAt[i], dpA) + glm::dot(JBt[i], dpB);

        float frictionBound = abs(getLambda()[2 * i + JN]) * getFriction();
        getFmax()[2 * i + JT] =  frictionBound;
        getFmin()[2 * i + JT] = -frictionBound;
    }
}

void Manifold::computeDerivatives(Rigid* body) {
    for (uint i = 0; i < 2; i++) {
        if (body == bodyA) {
            J[i * 2 + JN] = JAn[i];
            J[i * 2 + JT] = JAt[i];
        } else {
            J[i * 2 + JN] = JBn[i];
            J[i * 2 + JT] = JBt[i];
        }
    }
}

}