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

    int rows() const override { return 4; }
    void draw() const override;
};

}

#endif