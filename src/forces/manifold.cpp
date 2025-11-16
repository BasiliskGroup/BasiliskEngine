#include <basilisk/solver/physics.h>

namespace bsk::internal {

Manifold::Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB, uint index) : Force(solver, bodyA, bodyB) {
    this->index = index;
}

Manifold::~Manifold() {

}

void Manifold::draw() const {

}

BskVec2Pair& Manifold::getC0() { return this->getTable()->getManifoldTable()->getC0()[getSpecialIndex()]; }
BskVec2Pair& Manifold::getRA() { return this->getTable()->getManifoldTable()->getRA()[getSpecialIndex()]; } // explicit body
BskVec2Pair& Manifold::getRB() { return this->getTable()->getManifoldTable()->getRB()[getSpecialIndex()]; }
glm::vec2& Manifold::getNormal() { return this->getTable()->getManifoldTable()->getNormal()[getSpecialIndex()]; }
float& Manifold::getFriction() { return this->getTable()->getManifoldTable()->getFriction()[getSpecialIndex()]; }
bool Manifold::getStick() { return this->getTable()->getManifoldTable()->getStick()[getSpecialIndex()]; }
BskVec2Triplet& Manifold::getSimplex() { return this->getTable()->getManifoldTable()->getSimplex()[getSpecialIndex()]; }
uint& Manifold::getForceIndex() { return this->getTable()->getManifoldTable()->getForceIndex()[getSpecialIndex()]; }

glm::vec2& Manifold::getTangent() { return this->getTable()->getManifoldTable()->getTangent()[getSpecialIndex()]; }
glm::mat2x2& Manifold::getBasis() { return this->getTable()->getManifoldTable()->getBasis()[getSpecialIndex()]; }

BskVec2Pair& Manifold::getRAW() { return this->getTable()->getManifoldTable()->getRAW()[getSpecialIndex()]; } // explicit body
BskVec2Pair& Manifold::getRBW() { return this->getTable()->getManifoldTable()->getRBW()[getSpecialIndex()]; }
BskFloatROWS& Manifold::getCdA() { return this->getTable()->getManifoldTable()->getCdA()[getSpecialIndex()]; } // explicit body
BskFloatROWS& Manifold::getCdB() { return this->getTable()->getManifoldTable()->getCdB()[getSpecialIndex()]; }

void Manifold::initialize() {
    getFriction() = sqrt(bodyA->getFriction() * bodyB->getFriction());

    // we have already collided for this to exist
    Rigid* realBodyA = isA() ? bodyA : bodyB;
    Rigid* realBodyB = isA() ? bodyB : bodyA;

    // intersection convex polygons always have exactly 2 contacts
    for (uint i = 0; i < 2; i++) {
        getTangent() = { getNormal().y, -getNormal().x };
        getBasis() = {
            getNormal().x, getNormal().y,
            getTangent().x, getTangent().y
        };

        // uses bodyA as syntax since we only caculate from our perspective ^ are shared storage so we need to differentiate
        getRAW()[i] = realBodyA->getRMat() * getRA()[i];
        getRBW()[i] = realBodyB->getRMat() * getRB()[i];
        float sign = isA() ? 1 : -1;
        glm::vec2& rXW = isA() ? getRAW()[i] : getRBW()[i];
        
        J()[2 * i + JN] = sign * glm::vec3{ getBasis()[0][0], getBasis()[0][1], cross(rXW, getNormal()) };
        J()[2 * i + JT] = sign * glm::vec3{ getBasis()[1][0], getBasis()[1][1], cross(rXW, getTangent()) };

        getC0()[i] = getBasis() * (glm::vec2(realBodyA->getPos()) + getRAW()[i] - glm::vec2(realBodyB->getPos()) - getRBW()[i]);
    }
}

// TODO set C to C0 * (1 - alpha)

void Manifold::computeConstraint(float alpha) {
    // we only record our side so we need to view it from twin
    auto& JOther = getTwin()->J();

    for (uint i = 0; i < 2; i++) {
        glm::vec3 dpA = bodyA->getPos() - bodyA->getInitial();
        glm::vec3 dpB = bodyB->getPos() - bodyB->getInitial();

        C()[2 * i + JN] = getC0()[i].x * (1 - alpha) + glm::dot(J()[2 * i + JN], dpA) + glm::dot(JOther[2 * i + JN], dpB);
        C()[2 * i + JT] = getC0()[i].y * (1 - alpha) + glm::dot(J()[2 * i + JT], dpA) + glm::dot(JOther[2 * i + JT], dpB);

        // this is the same from both sides
        float frictionBound = abs(lambda()[2 * i + JN]) * getFriction();
        fmax()[2 * i + JT] =  frictionBound;
        fmin()[2 * i + JT] = -frictionBound;

        // TODO do stick later
    }
}   

void Manifold::computeDerivatives(Rigid* body) {
    // both manifolds are already stored on their respective forces
    // nothing needs to be done
}

}