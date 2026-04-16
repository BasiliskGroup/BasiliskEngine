#ifndef BODY_TABLE_H
#define BODY_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>       
#include <basilisk/compute/gpuWrapper.hpp>
#include <basilisk/compute/gpuTypes.hpp>

namespace bsk::internal {

class Rigid;
class Collider;
class BVH;
class ColliderTable;
class Solver;

class BodyTable : public VirtualTable {
private:
    Solver* solver;

    // CPU side data
    std::vector<Rigid*> bodies;
    std::vector<bool> toDelete;
    std::vector<bool> sleeping; // TODO, currently unused
    std::vector<bsk::vec3> pos;
    std::vector<bsk::vec3> initial;
    std::vector<bsk::vec3> inertial;
    std::vector<bsk::vec3> vel;
    std::vector<bsk::vec3> prevVel;
    std::vector<glm::vec2> scale;
    std::vector<float> friction;
    std::vector<float> mass;
    std::vector<float> moment;
    std::vector<float> radius;
    std::vector<uint32_t> collider;
    std::vector<glm::mat2x2> mat;
    std::vector<glm::mat2x2> imat;
    std::vector<glm::mat2x2> rmat;
    std::vector<bool> updated;
    std::vector<int> color;
    std::vector<glm::vec3> jacobianMask; // TODO create buffer later
    std::vector<uint32_t> indexMap;

    BVH* bvh;

    // GPU side data
    GpuBuffer<bsk::vec3>* posBuffer;
    GpuBuffer<bsk::vec3>* initialBuffer;
    GpuBuffer<bsk::vec3>* inertialBuffer;
    GpuBuffer<bsk::vec3>* velBuffer;
    GpuBuffer<bsk::vec3>* prevVelBuffer;
    GpuBuffer<float>* frictionBuffer;
    GpuBuffer<float>* massBuffer;
    GpuBuffer<float>* momentBuffer;
    StagingBuffer<bsk::vec3>* velStagingBuffer;
    StagingBuffer<bsk::vec3>* prevVelStagingBuffer;

    ComputeShader*& velocityShader;

public:
    BodyTable(Solver* solver, uint32_t capacity, ComputeShader*& velocityShader);
    ~BodyTable();

    void computeTransforms(); // TODO, determine if this would be better per-object
    void warmstartBodies(const float dt, const std::optional<glm::vec3>& gravity);
    void updateVelocities(float dt);
    glm::vec3 getGravity(uint32_t index) const;
    glm::vec3 getGravity(Rigid* body) const;

    void markAsDeleted(uint32_t index);

    void resize(uint32_t newCapacity) override;
    void compact() override;
    void insert(Rigid* body, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity, Collider* collider);

    void writeToNodes();
    void writeToGpu();

    // getters
    Rigid* getBodies(uint32_t index) { return bodies[index]; }
    bool getToDelete(uint32_t index) { return toDelete[index]; }
    glm::vec3& getPos(uint32_t index) { return pos[index]; }
    glm::vec3& getInitial(uint32_t index) { return initial[index]; }
    glm::vec3& getInertial(uint32_t index) { return inertial[index]; }
    glm::vec3& getVel(uint32_t index) { return vel[index]; }
    glm::vec3& getPrevVel(uint32_t index) { return prevVel[index]; }
    glm::vec2 getScale(uint32_t index) { return scale[index]; }
    float getFriction(uint32_t index) { return friction[index]; }
    float getMass(uint32_t index) { return mass[index]; }
    float getMoment(uint32_t index) { return moment[index]; }
    float getRadius(uint32_t index) { return radius[index]; }
    uint32_t getCollider(uint32_t index) { return collider[index]; }
    glm::mat2x2& getMat(uint32_t index) { return mat[index]; }
    glm::mat2x2& getImat(uint32_t index) { return imat[index]; }
    glm::mat2x2& getRmat(uint32_t index) { return rmat[index]; }
    bool getUpdated(uint32_t index) { return updated[index]; }
    int getColor(uint32_t index) { return color[index]; }
    BVH* getBVH() { return bvh; }
    float getDensity(uint32_t index);
    glm::vec3& getJacobianMask(uint32_t index) { return jacobianMask[index]; }
    uint32_t getMappedIndex(uint32_t index) { return indexMap[index]; }

    GpuBuffer<bsk::vec3>* getPosBuffer() { return posBuffer; }
    GpuBuffer<bsk::vec3>* getInitialBuffer() { return initialBuffer; }
    GpuBuffer<bsk::vec3>* getInertialBuffer() { return inertialBuffer; }
    GpuBuffer<bsk::vec3>* getVelBuffer() { return velBuffer; }
    GpuBuffer<bsk::vec3>* getPrevVelBuffer() { return prevVelBuffer; }
    GpuBuffer<float>* getFrictionBuffer() { return frictionBuffer; }
    GpuBuffer<float>* getMassBuffer() { return massBuffer; }
    GpuBuffer<float>* getMomentBuffer() { return momentBuffer; }

    // setters
    void setBodies(uint32_t index, Rigid* value) { bodies[index] = value; }
    void setToDelete(uint32_t index, bool value) { toDelete[index] = value; }
    void setPos(uint32_t index, const glm::vec3& value) { pos[index] = value; }
    void setInitial(uint32_t index, const glm::vec3& value) { initial[index] = value; }
    void setInertial(uint32_t index, const glm::vec3& value) { inertial[index] = value; }
    void setVel(uint32_t index, const glm::vec3& value) { vel[index] = value; }
    void setPrevVel(uint32_t index, const glm::vec3& value) { prevVel[index] = value; }
    void setScale(uint32_t index, const glm::vec2& value) { scale[index] = value; }
    void setFriction(uint32_t index, float value) { friction[index] = value; }
    void setMass(uint32_t index, float value) { mass[index] = value; }
    void setMoment(uint32_t index, float value) { moment[index] = value; }
    void setRadius(uint32_t index, float value) { radius[index] = value; }
    void setCollider(uint32_t index, uint32_t value) { collider[index] = value; }
    void setMat(uint32_t index, const glm::mat2x2& value) { mat[index] = value; }
    void setImat(uint32_t index, const glm::mat2x2& value) { imat[index] = value; }
    void setRmat(uint32_t index, const glm::mat2x2& value) { rmat[index] = value; }
    void setUpdated(uint32_t index, bool value) { updated[index] = value; }
    void setColor(uint32_t index, int value) { color[index] = value; }
    void setDensity(uint32_t index, float value);
    void setJacobianMask(uint32_t index, const glm::vec3& value) { jacobianMask[index] = value; }


};

}

#endif