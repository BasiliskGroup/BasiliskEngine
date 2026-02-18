#ifndef BSK_PHYSICS_FORCES_JOINT_H
#define BSK_PHYSICS_FORCES_JOINT_H

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Table Struct
// ------------------------------------------------------------
struct JointStruct {
    glm::vec2 rA = glm::vec2(0.0f);
    glm::vec2 rB = glm::vec2(0.0f);
    bsk::vec3 C0 = bsk::vec3(0.0f);
    float torqueArm = 0.0f;
    float restAngle = 0.0f;

    // padding to 16 bytes
    char _padding[8] = { 0 };
};

// Revolute joint + angle constraint between two rigid bodies, with optional fracture
class Joint : public Force {
public:
    Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness = glm::vec3{ INFINITY, INFINITY, INFINITY },
        float fracture = INFINITY);
    ~Joint();

    static int rows(ForceTable* forceTable, std::size_t index) { return 3; }
    int rows() override { return 3; }
    bool initialize() override;
    static void computeConstraint(ForceTable* forceTable, std::size_t index, float alpha);
    static void computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body);
    
    // Getters
    glm::vec2 getRA() const;
    glm::vec2 getRB() const;
    glm::vec3 getC0() const;
    float getTorqueArm() const;
    float getRestAngle() const;
    JointStruct& getData();
    const JointStruct& getData() const;
    
    // Setters
    void setRA(const glm::vec2& value);
    void setRB(const glm::vec2& value);
    void setC0(const glm::vec3& value);
    void setTorqueArm(float value);
    void setRestAngle(float value);
    
    // Mutable references for direct access (for performance-critical code)
    glm::vec2& getRARef();
    glm::vec2& getRBRef();
    glm::vec3& getC0Ref();
    void setData(const JointStruct& value);
};

}

#endif