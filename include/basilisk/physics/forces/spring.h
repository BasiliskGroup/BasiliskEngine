#pragma once

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Table Struct
// ------------------------------------------------------------
struct SpringStruct {
    glm::vec2 rA;
    glm::vec2 rB;
    float rest;
};

// Standard spring force
class Spring : public Force
{
public:
    Spring(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, float stiffness, float rest = -1);

    int rows() const override { return 1; }

    bool initialize() override { return true; }
    static void computeConstraint(ForceTable* forceTable, std::size_t index, float alpha);
    static void computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body);
    
    // Getters
    glm::vec2 getRA() const;
    glm::vec2 getRB() const;
    float getRest() const;
    SpringStruct& getData();
    const SpringStruct& getData() const;
    
    // Setters
    void setRA(const glm::vec2& value);
    void setRB(const glm::vec2& value);
    void setRest(float value);
    void setData(const SpringStruct& value);

    // Mutable references
    glm::vec2& getRARef();
    glm::vec2& getRBRef();
    float& getRestRef();
};

}

