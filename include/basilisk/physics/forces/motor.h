#pragma once

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Table Struct
// ------------------------------------------------------------
struct MotorStruct {
    float speed = 0.0f;

    // padding to 16 bytes
    char _padding[12] = { 0 };
};

// Motor force which applies a torque to two rigid bodies to achieve a desired angular speed
class Motor : public Force {
public:
    Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque);

    static int rows(ForceTable* forceTable, std::size_t index) { return 1; }
    int rows() override { return 1; }
    bool initialize() override { return true; }
    static void computeConstraint(ForceTable* forceTable, std::size_t index, float alpha);
    static void computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body);
    
    // Getters
    float getSpeed() const;
    MotorStruct& getData();
    const MotorStruct& getData() const;
    
    // Setters
    void setSpeed(float value);
    void setData(const MotorStruct& value);
};

}

