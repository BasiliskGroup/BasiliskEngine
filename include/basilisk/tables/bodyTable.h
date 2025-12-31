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
    std::vector<std::size_t> collider;
    std::vector<glm::mat2x2> mat;
    std::vector<glm::mat2x2> imat;
    std::vector<glm::mat2x2> rmat;
    std::vector<bool> updated;
    std::vector<ushort> color;
    std::vector<ushort> degree;
    std::vector<ushort> satur;

    // updating forces
    std::vector<std::size_t> oldIndex;
    std::vector<std::size_t> inverseForceMap;

    // solving
    std::vector<glm::vec3> rhs;
    std::vector<glm::mat3x3> lhs;

public:
    BodyTable(std::size_t capacity);
    ~BodyTable() = default;

    void computeTransforms();
    void warmstartBodies(const float dt, const float gravity);
    void updateVelocities(float dt);

    void markAsDeleted(std::size_t index);

    inline Rigid* getBody(std::size_t index) const { return bodies[index]; }
    inline glm::vec3& getPos(std::size_t index) { return pos[index]; }
    inline glm::vec3& getInitial(std::size_t index) { return initial[index]; }
    inline glm::vec3& getInertial(std::size_t index) { return inertial[index]; }
    inline glm::vec3& getVel(std::size_t index) { return vel[index]; }
    inline glm::vec3& getPrevVel(std::size_t index) { return prevVel[index]; }
    inline glm::vec2& getScale(std::size_t index) { return scale[index]; }
    inline float getFriction(std::size_t index) const { return friction[index]; }
    inline float getMass(std::size_t index) const { return mass[index]; }
    inline float getMoment(std::size_t index) const { return moment[index]; }
    inline float getRadius(std::size_t index) const { return radius[index]; }
    inline std::size_t getCollider(std::size_t index) const { return collider[index]; }
    inline glm::mat2x2& getMat(std::size_t index) { return mat[index]; }
    inline glm::mat2x2& getIMat(std::size_t index) { return imat[index]; }
    inline glm::mat2x2& getRMat(std::size_t index) { return rmat[index]; }
    inline bool getUpdated(std::size_t index) const { return updated[index]; }
    inline unsigned short getColor(std::size_t index) { return color[index]; }
    inline unsigned short getDegree(std::size_t index) const { return degree[index]; }
    inline unsigned short getSatur(std::size_t index) const { return satur[index]; }
    inline glm::vec3& getRHS(std::size_t index) { return rhs[index]; }
    inline glm::mat3x3& getLHS(std::size_t index) { return lhs[index]; }
    inline std::size_t getInverseForceMap(std::size_t index) const { return inverseForceMap[index]; }

    inline void setBody(std::size_t index, Rigid* body) { bodies[index] = body; }
    inline void setPos(std::size_t index, const glm::vec3& pos) { this->pos[index] = pos; }
    inline void setInitial(std::size_t index, const glm::vec3& initial) { this->initial[index] = initial; }
    inline void setInertial(std::size_t index, const glm::vec3& inertial) { this->inertial[index] = inertial; }
    inline void setVel(std::size_t index, const glm::vec3& vel) { this->vel[index] = vel; }
    inline void setPrevVel(std::size_t index, const glm::vec3& prevVel) { this->prevVel[index] = prevVel; }
    inline void setScale(std::size_t index, const glm::vec2& scale) { this->scale[index] = scale; }
    inline void setFriction(std::size_t index, float friction) { this->friction[index] = friction; }
    inline void setMass(std::size_t index, float mass) { this->mass[index] = mass; }
    inline void setMoment(std::size_t index, float moment) { this->moment[index] = moment; }
    inline void setRadius(std::size_t index, float radius) { this->radius[index] = radius; }
    inline void setCollider(std::size_t index, std::size_t collider) { this->collider[index] = collider; }
    inline void setMat(std::size_t index, const glm::mat2x2& mat) { this->mat[index] = mat; }
    inline void setIMat(std::size_t index, const glm::mat2x2& imat) { this->imat[index] = imat; }
    inline void setRMat(std::size_t index, const glm::mat2x2& rmat) { this->rmat[index] = rmat; }
    inline void setUpdated(std::size_t index, bool updated) { this->updated[index] = updated; }
    inline void setColor(std::size_t index, unsigned short color) { this->color[index] = color; }
    inline void setDegree(std::size_t index, unsigned short degree) { this->degree[index] = degree; }
    inline void setSatur(std::size_t index, unsigned short satur) { this->satur[index] = satur; }
    inline void setRHS(std::size_t index, const glm::vec3& rhs) { this->rhs[index] = rhs; }
    inline void setLHS(std::size_t index, const glm::mat3x3& lhs) { this->lhs[index] = lhs; }
    inline void setInverseForceMap(std::size_t index, std::size_t inverseForceMap) { this->inverseForceMap[index] = inverseForceMap; }

    void resize(std::size_t newCapacity) override;
    void compact() override;
    std::size_t insert(Rigid* body, glm::vec3 pos, glm::vec3 vel, glm::vec2 scale, float friction, float mass, float moment, std::size_t collider, float radius);

    void writeToNodes();
};

}

#endif
