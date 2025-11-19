#ifndef BSK_MANIFOLD_H
#define BSK_MANIFOLD_H

#include <basilisk/forces/force.h>

namespace bsk::internal {

class Manifold : public Force {
private:

    // tracking the SoA
    uint contactIndex;

    // TEMPORARY
    BskVec2Pair C0;
    BskVec2Pair rA;
    BskVec2Pair rB;
    BskVec3Pair JAn, JBn, JAt, JBt;
    glm::vec2 normal;
    float friction;
    bool stick;
    BskVec2Triplet simplex;
    glm::vec2 tangent;
    glm::mat2x2 basis;
    BskVec2Pair rAW;
    BskVec2Pair rBW;
    BskFloatROWS cdA;
    BskFloatROWS cdB;

public:
    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB, uint index);
    ~Manifold();

    BskVec2Pair& getC0() { return C0; }
    BskVec2Pair& getRA() { return rA; }
    BskVec2Pair& getRB() { return rB; }
    glm::vec2& getNormal() { return normal; }
    float& getFriction() { return friction; }
    bool& getStick() { return stick; }
    BskVec2Triplet& getSimplex() { return simplex; }
    glm::vec2& getTangent() { return tangent; }
    glm::mat2x2& getBasis() { return basis; }
    BskVec2Pair& getRAW() { return rAW; }
    BskVec2Pair& getRBW() { return rBW; }
    BskFloatROWS& getCdA() { return cdA; }
    BskFloatROWS& getCdB() { return cdB; }

    int rows() const override { return MANIFOLD_ROWS; }
    void draw() const override;

    void initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
};

}

#endif