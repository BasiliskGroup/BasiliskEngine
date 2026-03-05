#ifndef BSK_PHYSICS_FORCES_MOTOR_H
#define BSK_PHYSICS_FORCES_MOTOR_H

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
static_assert(sizeof(MotorStruct) % 16 == 0, "MotorStruct must be 16-byte aligned");

// Motor force which applies a torque to two rigid bodies to achieve a desired angular speed
class Motor : public Force {
public:
    Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque);
    ~Motor();

    static int rows(ForceTable* forceTable, uint32_t specialIndex) { return 1; }
    int rows() override { return 1; }
    bool initialize() override { return true; }
    static void computeConstraint(ForceTable* forceTable, uint32_t specialIndex, float alpha);
    static void computeDerivatives(ForceTable* forceTable, uint32_t specialIndex, uint32_t bodyIndex, const glm::vec3& jacobianMask);
    
    // Getters
    float getSpeed() const;
    MotorStruct& getData();
    const MotorStruct& getData() const;
    
    // Setters
    void setSpeed(float value);
    void setData(const MotorStruct& value);
};

}

#endif