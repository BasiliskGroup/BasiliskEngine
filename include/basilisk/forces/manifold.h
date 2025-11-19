#ifndef BSK_MANIFOLD_H
#define BSK_MANIFOLD_H

#include <basilisk/forces/force.h>

namespace bsk::internal {

class Manifold : public Force {
private:

    // tracking the SoA
    uint contactIndex;

public:
    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB, uint index);
    ~Manifold();

    BskVec2Pair& getC0();
    BskVec2Pair& getRA();
    BskVec2Pair& getRB();
    glm::vec2& getNormal();
    float& getFriction();
    bool getStick();
    BskVec2Triplet& getSimplex();
    uint& getForceIndex();

    glm::vec2& getTangent();
    glm::mat2x2& getBasis();
    BskVec2Pair& getRAW();
    BskVec2Pair& getRBW();
    BskFloatROWS& getCdA();
    BskFloatROWS& getCdB();

    int rows() const override { return MANIFOLD_ROWS; }
    void draw() const override;

    void initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
};

}

#endif