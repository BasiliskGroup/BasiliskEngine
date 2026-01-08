#ifndef BODY_TABLE_H
#define BODY_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>

namespace bsk::internal {

class Rigid;
class Collider;
class BVH;

class BodyTable : public VirtualTable {
private:
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
    std::vector<int> color;

    // updating forces
    std::vector<std::size_t> oldIndex;
    std::vector<std::size_t> inverseForceMap;

    // solving
    std::vector<glm::vec3> rhs;
    std::vector<glm::mat3x3> lhs;

    BVH* bvh;
    
public:
    BodyTable(std::size_t capacity);
    ~BodyTable();

    void computeTransforms(); // TODO, determine if this would be better per-object
    void warmstartBodies(const float dt, const std::optional<glm::vec3>& gravity);
    void updateVelocities(float dt);
    glm::vec3 getGravity(std::size_t index) const;
    glm::vec3 getGravity(Rigid* body) const;

    void markAsDeleted(std::size_t index);

    void resize(std::size_t newCapacity) override;
    void compact() override;
    void insert(Rigid* body, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity, Collider* collider);

    void writeToNodes();

    // getters
    Rigid* getBodies(std::size_t index) { return bodies[index]; }
    bool getToDelete(std::size_t index) { return toDelete[index]; }
    glm::vec3& getPos(std::size_t index) { return pos[index]; }
    glm::vec3& getInitial(std::size_t index) { return initial[index]; }
    glm::vec3& getInertial(std::size_t index) { return inertial[index]; }
    glm::vec3& getVel(std::size_t index) { return vel[index]; }
    glm::vec3& getPrevVel(std::size_t index) { return prevVel[index]; }
    glm::vec2 getScale(std::size_t index) { return scale[index]; }
    float getFriction(std::size_t index) { return friction[index]; }
    float getMass(std::size_t index) { return mass[index]; }
    float getMoment(std::size_t index) { return moment[index]; }
    float getRadius(std::size_t index) { return radius[index]; }
    std::size_t getCollider(std::size_t index) { return collider[index]; }
    glm::mat2x2& getMat(std::size_t index) { return mat[index]; }
    glm::mat2x2& getImat(std::size_t index) { return imat[index]; }
    glm::mat2x2& getRmat(std::size_t index) { return rmat[index]; }
    bool getUpdated(std::size_t index) { return updated[index]; }
    int getColor(std::size_t index) { return color[index]; }
    std::size_t getOldIndex(std::size_t index) { return oldIndex[index]; }
    std::size_t getInverseForceMap(std::size_t index) { return inverseForceMap[index]; }
    glm::vec3& getRhs(std::size_t index) { return rhs[index]; }
    glm::mat3x3& getLhs(std::size_t index) { return lhs[index]; }
    BVH* getBVH() { return bvh; }

    // setters
    void setBodies(std::size_t index, Rigid* value) { bodies[index] = value; }
    void setToDelete(std::size_t index, bool value) { toDelete[index] = value; }
    void setPos(std::size_t index, const glm::vec3& value) { pos[index] = value; }
    void setInitial(std::size_t index, const glm::vec3& value) { initial[index] = value; }
    void setInertial(std::size_t index, const glm::vec3& value) { inertial[index] = value; }
    void setVel(std::size_t index, const glm::vec3& value) { vel[index] = value; }
    void setPrevVel(std::size_t index, const glm::vec3& value) { prevVel[index] = value; }
    void setScale(std::size_t index, const glm::vec2& value) { scale[index] = value; }
    void setFriction(std::size_t index, float value) { friction[index] = value; }
    void setMass(std::size_t index, float value) { mass[index] = value; }
    void setMoment(std::size_t index, float value) { moment[index] = value; }
    void setRadius(std::size_t index, float value) { radius[index] = value; }
    void setCollider(std::size_t index, std::size_t value) { collider[index] = value; }
    void setMat(std::size_t index, const glm::mat2x2& value) { mat[index] = value; }
    void setImat(std::size_t index, const glm::mat2x2& value) { imat[index] = value; }
    void setRmat(std::size_t index, const glm::mat2x2& value) { rmat[index] = value; }
    void setUpdated(std::size_t index, bool value) { updated[index] = value; }
    void setColor(std::size_t index, int value) { color[index] = value; }
    void setOldIndex(std::size_t index, std::size_t value) { oldIndex[index] = value; }
    void setInverseForceMap(std::size_t index, std::size_t value) { inverseForceMap[index] = value; }
    void setRhs(std::size_t index, const glm::vec3& value) { rhs[index] = value; }
    void setLhs(std::size_t index, const glm::mat3x3& value) { lhs[index] = value; }
};

}

#endif