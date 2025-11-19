#ifndef BSK_BODY_TABLE_H
#define BSK_BODY_TABLE_H

#include <basilisk/tables/virtualTable.h>
#include <basilisk/util/print.h>
#include <basilisk/shapes/rigid.h>

namespace bsk::internal {

class Rigid;

// NOTE we do not need copy or move constructor as we will only have one of these
class BodyTable : public VirtualTable {
private: 
    // columns
    std::vector<Rigid*> bodies;
    std::vector<bool> toDelete;
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> initial;
    std::vector<glm::vec3> inertial;
    std::vector<glm::vec3> vel;
    std::vector<glm::vec3> prevVel;
    std::vector<glm::vec2> scale;
    std::vector<float> friction;
    std::vector<float> mass;
    std::vector<float> moment;
    std::vector<float> radius;
    std::vector<uint> collider;
    std::vector<glm::mat2x2> mat;
    std::vector<glm::mat2x2> imat;
    std::vector<glm::mat2x2> rmat;
    std::vector<bool> updated;
    std::vector<ushort> color;
    std::vector<ushort> degree;
    std::vector<ushort> satur;

    // updating forces
    std::vector<uint> oldIndex;
    std::vector<uint> inverseForceMap;

    // solving
    std::vector<glm::vec3> rhs;
    std::vector<glm::mat3x3> lhs;

public:
    BodyTable(uint capacity);
    ~BodyTable() = default;

    void computeTransforms();
    void warmstartBodies(const float dt, const float gravity);
    void updateVelocities(float dt);

    void markAsDeleted(uint index);

    inline auto& getBodies() { return bodies; }
    inline auto& getPos() { return pos; }
    inline auto& getInitial() { return initial; }
    inline auto& getInertial() { return inertial; }
    inline auto& getVel() { return vel; }
    inline auto& getPrevVel() { return prevVel; }
    inline auto& getScale() { return scale; }
    inline auto& getFriction() { return friction; }
    inline auto& getMass() { return mass; }
    inline auto& getMoment() { return moment; }
    inline auto& getRadius() { return radius; }
    inline auto& getCollider() { return collider; }
    inline auto& getMat() { return mat; }
    inline auto& getIMat() { return imat; }
    inline auto& getRMat() { return rmat; }
    inline auto& getUpdated() { return updated; }
    inline auto& getColor() { return color; }
    inline auto& getDegree() { return degree; }
    inline auto& getSatur() { return satur; }
    inline auto& getRHS() { return rhs; }
    inline auto& getLHS() { return lhs; }
    inline auto& getInverseForceMap() { return inverseForceMap; }

    void resize(uint newCapacity) override;
    void compact() override;
    uint insert(Rigid* body, glm::vec3 pos, glm::vec3 vel, glm::vec2 scale, float friction, float mass, float moment, uint collider, float radius);

    void writeToNodes();
};

}

#endif
