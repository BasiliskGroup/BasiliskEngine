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

// BskVec2Pair& Manifold::getC0() { return this->getTable()->getManifoldTable()->getC0()[getSpecialIndex()]; }
// BskVec2Pair& Manifold::getRA() { return this->getTable()->getManifoldTable()->getRA()[getSpecialIndex()]; } // explicit body
// BskVec2Pair& Manifold::getRB() { return this->getTable()->getManifoldTable()->getRB()[getSpecialIndex()]; }
// glm::vec2& Manifold::getNormal() { return this->getTable()->getManifoldTable()->getNormal()[getSpecialIndex()]; }
// float& Manifold::getFriction() { return this->getTable()->getManifoldTable()->getFriction()[getSpecialIndex()]; }
// bool Manifold::getStick() { return this->getTable()->getManifoldTable()->getStick()[getSpecialIndex()]; }
// BskVec2Triplet& Manifold::getSimplex() { return this->getTable()->getManifoldTable()->getSimplex()[getSpecialIndex()]; }
// uint& Manifold::getForceIndex() { return this->getTable()->getManifoldTable()->getForceIndex()[getSpecialIndex()]; }

// glm::vec2& Manifold::getTangent() { return this->getTable()->getManifoldTable()->getTangent()[getSpecialIndex()]; }
// glm::mat2x2& Manifold::getBasis() { return this->getTable()->getManifoldTable()->getBasis()[getSpecialIndex()]; }

// BskVec2Pair& Manifold::getRAW() { return this->getTable()->getManifoldTable()->getRAW()[getSpecialIndex()]; } // explicit body
// BskVec2Pair& Manifold::getRBW() { return this->getTable()->getManifoldTable()->getRBW()[getSpecialIndex()]; }
// BskFloatROWS& Manifold::getCdA() { return this->getTable()->getManifoldTable()->getCdA()[getSpecialIndex()]; } // explicit body
// BskFloatROWS& Manifold::getCdB() { return this->getTable()->getManifoldTable()->getCdB()[getSpecialIndex()]; }

void Manifold::initialize() {
    std::cout << "\n=== MANIFOLD INITIALIZE ===\n";
    std::cout << "isA: " << (getIsA() ? "true" : "false") << "\n";
    std::cout << "bodyA: " << bodyA->getIndex() << ", bodyB: " << bodyB->getIndex() << "\n";
    
    getFriction() = sqrt(bodyA->getFriction() * bodyB->getFriction());
    std::cout << "Friction: " << getFriction() << " (bodyA: " << bodyA->getFriction() << ", bodyB: " << bodyB->getFriction() << ")\n";

    Rigid* realBodyA = getIsA() ? bodyA : bodyB;
    Rigid* realBodyB = getIsA() ? bodyB : bodyA;
    auto& realJA = getIsA() ? getJ() : twin->getJ();
    auto& realJB = getIsA() ? twin->getJ() : getJ();

    std::cout << "realBodyA: " << realBodyA->getIndex() << ", realBodyB: " << realBodyB->getIndex() << "\n";

    for (uint i = 0; i < 2; i++) {
        getTangent() = { getNormal().y, -getNormal().x };
        getBasis() = {
            getNormal().x, getNormal().y,
            getTangent().x, getTangent().y
        };

        getRAW()[i] = realBodyA->getRMat() * getRA()[i];
        getRBW()[i] = realBodyB->getRMat() * getRB()[i];

        realJA[2 * i + JN] = glm::vec3{ getBasis()[0][0], getBasis()[0][1], cross(getRAW()[i], getNormal()) };
        realJA[2 * i + JT] = glm::vec3{ getBasis()[1][0], getBasis()[1][1], cross(getRAW()[i], getTangent()) };
        realJB[2 * i + JN] = glm::vec3{ -getBasis()[0][0], -getBasis()[0][1], -cross(getRBW()[i], getNormal()) };
        realJB[2 * i + JT] = glm::vec3{ -getBasis()[1][0], -getBasis()[1][1], -cross(getRBW()[i], getTangent()) };

        glm::vec2 posA = glm::vec2(realBodyA->getPos());
        glm::vec2 posB = glm::vec2(realBodyB->getPos());
        glm::vec2 diff = posA + getRAW()[i] - posB - getRBW()[i];
        getC0()[i] = getBasis() * diff;

        std::cout << "--- Contact " << i << " ---\n";
        std::cout << "  Normal: (" << getNormal().x << ", " << getNormal().y << ")\n";
        std::cout << "  Tangent: (" << getTangent().x << ", " << getTangent().y << ")\n";
        std::cout << "  Basis: [" << getBasis()[0][0] << ", " << getBasis()[0][1] 
                << "; " << getBasis()[1][0] << ", " << getBasis()[1][1] << "]\n";
        std::cout << "  rA (local): (" << getRA()[i].x << ", " << getRA()[i].y << ")\n";
        std::cout << "  rB (local): (" << getRB()[i].x << ", " << getRB()[i].y << ")\n";
        std::cout << "  RAW: (" << getRAW()[i].x << ", " << getRAW()[i].y << ")\n";
        std::cout << "  RBW: (" << getRBW()[i].x << ", " << getRBW()[i].y << ")\n";
        std::cout << "  posA: (" << posA.x << ", " << posA.y << ")\n";
        std::cout << "  posB: (" << posB.x << ", " << posB.y << ")\n";
        std::cout << "  diff: (" << diff.x << ", " << diff.y << ")\n";
        std::cout << "  realJA[" << (2*i+JN) << "] (normal): (" << realJA[2*i+JN].x << ", " << realJA[2*i+JN].y << ", " << realJA[2*i+JN].z << ")\n";
        std::cout << "  realJB[" << (2*i+JN) << "] (normal): (" << realJB[2*i+JN].x << ", " << realJB[2*i+JN].y << ", " << realJB[2*i+JN].z << ")\n";
        std::cout << "  realJA[" << (2*i+JT) << "] (tangent): (" << realJA[2*i+JT].x << ", " << realJA[2*i+JT].y << ", " << realJA[2*i+JT].z << ")\n";
        std::cout << "  realJB[" << (2*i+JT) << "] (tangent): (" << realJB[2*i+JT].x << ", " << realJB[2*i+JT].y << ", " << realJB[2*i+JT].z << ")\n";
        std::cout << "  C0: (" << getC0()[i].x << ", " << getC0()[i].y << ")\n";
    }
}

void Manifold::computeConstraint(float alpha) {
    std::cout << "\n=== MANIFOLD COMPUTE CONSTRAINT ===\n";
    std::cout << "alpha: " << alpha << "\n";
    std::cout << "bodyA: " << bodyA->getIndex() << ", bodyB: " << bodyB->getIndex() << "\n";
    
    auto& JOther = twin->getJ();
    auto& COther = twin->getC();

    for (uint i = 0; i < 2; i++) {
        glm::vec3 dpA = bodyA->getPos() - bodyA->getInitial();
        glm::vec3 dpB = bodyB->getPos() - bodyB->getInitial();
        
        std::cout << "Contact " << i << ":\n";
        std::cout << "  bodyA pos: (" << bodyA->getPos().x << ", " << bodyA->getPos().y << ", " << bodyA->getPos().z << ")\n";
        std::cout << "  bodyA initial: (" << bodyA->getInitial().x << ", " << bodyA->getInitial().y << ", " << bodyA->getInitial().z << ")\n";
        std::cout << "  dpA: (" << dpA.x << ", " << dpA.y << ", " << dpA.z << ")\n";
        std::cout << "  bodyB pos: (" << bodyB->getPos().x << ", " << bodyB->getPos().y << ", " << bodyB->getPos().z << ")\n";
        std::cout << "  bodyB initial: (" << bodyB->getInitial().x << ", " << bodyB->getInitial().y << ", " << bodyB->getInitial().z << ")\n";
        std::cout << "  dpB: (" << dpB.x << ", " << dpB.y << ", " << dpB.z << ")\n";
        
        float dotJdpA_N = glm::dot(getJ()[2 * i + JN], dpA);
        float dotJOtherdpB_N = glm::dot(JOther[2 * i + JN], dpB);

        std::cout << "  J_N: (" << getJ()[2*i+JN].x << ", " << getJ()[2*i+JN].y << ", " << getJ()[2*i+JN].z << ")\n";
        std::cout << "  JOther_N: (" << JOther[2*i+JN].x << ", " << JOther[2*i+JN].y << ", " << JOther[2*i+JN].z << ")\n";
        std::cout << "  dot(J_N, dpA): " << dotJdpA_N << "\n";
        std::cout << "  dot(JOther_N, dpB): " << dotJOtherdpB_N << "\n";
        std::cout << "  C0.x: " << getC0()[i].x << "\n";

        getC()[2 * i + JN] = COther[2 * i + JN] = getC0()[i].x * (1 - alpha) + dotJdpA_N + dotJOtherdpB_N;
        
        std::cout << "  C_normal [" << (2*i+JN) << "]: " << getC()[2*i+JN] << "\n";
        
        float dotJdpA_T = glm::dot(getJ()[2 * i + JT], dpA);
        float dotJOtherdpB_T = glm::dot(JOther[2 * i + JT], dpB);
        
        std::cout << "  J_T: (" << getJ()[2*i+JT].x << ", " << getJ()[2*i+JT].y << ", " << getJ()[2*i+JT].z << ")\n";
        std::cout << "  JOther_T: (" << JOther[2*i+JT].x << ", " << JOther[2*i+JT].y << ", " << JOther[2*i+JT].z << ")\n";
        std::cout << "  dot(J_T, dpA): " << dotJdpA_T << "\n";
        std::cout << "  dot(JOther_T, dpB): " << dotJOtherdpB_T << "\n";
        std::cout << "  C0.y: " << getC0()[i].y << "\n";
        
        getC()[2 * i + JT] = COther[2 * i + JT] = getC0()[i].y * (1 - alpha) + dotJdpA_T + dotJOtherdpB_T;
        
        std::cout << "  C_tangent [" << (2*i+JT) << "]: " << getC()[2*i+JT] << "\n";

        float frictionBound = abs(getLambda()[2 * i + JN]) * getFriction();
        getFmax()[2 * i + JT] =  frictionBound;
        getFmin()[2 * i + JT] = -frictionBound;
        
        std::cout << "  lambda_normal: " << getLambda()[2*i+JN] << "\n";
        std::cout << "  friction: " << getFriction() << "\n";
        std::cout << "  frictionBound: " << frictionBound << "\n";
        std::cout << "  fmin[tangent]: " << getFmin()[2*i+JT] << ", fmax[tangent]: " << getFmax()[2*i+JT] << "\n";
    }
}

void Manifold::computeDerivatives(Rigid* body) {
    std::cout << "\n=== MANIFOLD COMPUTE DERIVATIVES ===\n";
    std::cout << "Force index: " << index << ", for body: " << body->getIndex() << "\n";
    std::cout << "Derivatives already stored in J arrays, no action needed\n";
    // both manifolds are already stored on their respective forces
    // nothing needs to be done
}

}