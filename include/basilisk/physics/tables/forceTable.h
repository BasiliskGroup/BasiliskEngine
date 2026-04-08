#ifndef FORCE_TABLE_H
#define FORCE_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>
#include <basilisk/util/constants.h>

#include <basilisk/physics/forces/ignoreCollision.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/compute/gpuTypes.hpp>
#include <basilisk/compute/gpuWrapper.hpp>


namespace bsk::internal {

template<typename T>
class ForceTypeTable;

using ::bsk::internal::MAX_ROWS;

// ------------------------------------------------------------
// Force Structs
// ------------------------------------------------------------
struct ParameterStruct {
    float C;
    float fmin;
    float fmax;
    float stiffness;
    float fracture;
    float penalty;
    float lambda;

    uint32_t _padding;
};
static_assert(sizeof(ParameterStruct) % 16 == 0, "ParameterStruct must be 16-byte aligned");

struct DerivativeStruct {
    bsk::vec3 J;
    bsk::mat3x3 H;
};
static_assert(sizeof(DerivativeStruct) % 16 == 0, "DerivativeStruct must be 16-byte aligned");

struct SolverSidesStruct {
    bsk::vec3 rhs;
    bsk::mat3x3 lhs;
};
static_assert(sizeof(SolverSidesStruct) % 16 == 0, "SolverSides must be 16-byte aligned");

struct BodyStruct {
    uint32_t a;
    uint32_t b;

    uint32_t _padding[2];
};
static_assert(sizeof(BodyStruct) % 16 == 0, "BodyStruct must be 16-byte aligned");

using Parameters = std::array<ParameterStruct, MAX_ROWS>;
using Derivatives = std::array<DerivativeStruct, MAX_ROWS>;
using SolverSides = std::array<SolverSidesStruct, MAX_ROWS>;

class Force;

// ------------------------------------------------------------
// Force Table Class
// ------------------------------------------------------------
class ForceTable : public VirtualTable {
private:
    Solver* solver;

    // compute variables
    std::vector<Force*> forces;
    std::vector<bool> toDelete;
    std::vector<Parameters> parameters;
    std::vector<Derivatives> derivatives;
    std::vector<ForceType> forceTypes;
    std::vector<BodyStruct> bodies;
    std::vector<SolverSides> solverSides;

    // used to map row vectors to new indices
    std::vector<uint32_t> indexMap;

    // force specific tables
    ForceTypeTable<ManifoldData>* manifoldTable;
    ForceTypeTable<JointStruct>* jointTable;
    ForceTypeTable<SpringStruct>* springTable;
    ForceTypeTable<MotorStruct>* motorTable;

    // structure variables
    std::vector<int> rows;

    // GPU side data
    GpuBuffer<Parameters>* parameterBuffer;
    GpuBuffer<Derivatives>* derivativeBuffer;
    GpuBuffer<SolverSides>* solverSidesBuffer;
    GpuBuffer<ForceType>* forceTypeBuffer;
    GpuBuffer<BodyStruct>* bodyBuffer;

public:
    ForceTable(uint32_t capacity);
    ~ForceTable();

    void markAsDeleted(uint32_t index);
    void resize(uint32_t newCapacity);
    void compact();
    void insert(Force* force);

    void remapBodyIndices();
    void writeToGPU();

    // getters 
    Force* getForce(uint32_t index) { return forces[index]; }
    bool getToDelete(uint32_t index) { return toDelete[index]; }
    int getRows(uint32_t index) { return rows[index]; }
    ForceType getForceType(uint32_t index) { return forceTypes[index]; }
    BodyStruct& getBodies(uint32_t index) { return bodies[index]; }
    glm::vec3 getPosA(uint32_t index);
    glm::vec3 getPosB(uint32_t index);
    glm::vec3 getInitialA(uint32_t index);
    glm::vec3 getInitialB(uint32_t index);

    Solver* getSolver() { return solver; }
    ForceTypeTable<ManifoldData>* getManifoldTable() { return manifoldTable; }
    ForceTypeTable<JointStruct>* getJointTable() { return jointTable; }
    ForceTypeTable<SpringStruct>* getSpringTable() { return springTable; }
    ForceTypeTable<MotorStruct>* getMotorTable() { return motorTable; }

    // index specific
    bsk::vec3& getJ(uint32_t forceIndex, int row) { return derivatives[forceIndex][row].J; }
    bsk::mat3x3& getH(uint32_t forceIndex, int row) { return derivatives[forceIndex][row].H; }
    float getC(uint32_t forceIndex, int row) { return parameters[forceIndex][row].C; }
    float getFmin(uint32_t forceIndex, int row) { return parameters[forceIndex][row].fmin; }
    float getFmax(uint32_t forceIndex, int row) { return parameters[forceIndex][row].fmax; }
    float getStiffness(uint32_t forceIndex, int row) { return parameters[forceIndex][row].stiffness; }
    float getFracture(uint32_t forceIndex, int row) { return parameters[forceIndex][row].fracture; }
    float getPenalty(uint32_t forceIndex, int row) { return parameters[forceIndex][row].penalty; }
    float getLambda(uint32_t forceIndex, int row) { return parameters[forceIndex][row].lambda; }
    bsk::vec3& getRhs(uint32_t forceIndex, int row) { return solverSides[forceIndex][row].rhs; }
    bsk::mat3x3& getLhs(uint32_t forceIndex, int row) { return solverSides[forceIndex][row].lhs; }

    ParameterStruct& getParameter(uint32_t forceIndex, int row) { return parameters[forceIndex][row]; }
    DerivativeStruct& getDerivative(uint32_t forceIndex, int row) { return derivatives[forceIndex][row]; }

    uint32_t& getMappedIndex(uint32_t forceIndex) { return indexMap[forceIndex]; }

    // full row
    Parameters& getParameters(uint32_t forceIndex) { return parameters[forceIndex]; }
    Derivatives& getDerivatives(uint32_t forceIndex) { return derivatives[forceIndex]; }

    // setters
    void setForces(uint32_t index, Force* value) { forces[index] = value; }
    void setToDelete(uint32_t index, bool value) { toDelete[index] = value; }
    void setRows(uint32_t index, int value) { rows[index] = value; }
    void setForceType(uint32_t index, ForceType value);
    void setBodies(uint32_t index, const BodyStruct& value) { bodies[index] = value; }
    void setPosA(uint32_t index, const glm::vec3& value);
    void setPosB(uint32_t index, const glm::vec3& value);
    void setInitialA(uint32_t index, const glm::vec3& value);
    void setInitialB(uint32_t index, const glm::vec3& value);
    void setSolver(Solver* value) { solver = value; }
    void setRhs(uint32_t forceIndex, int row, const bsk::vec3& value) { solverSides[forceIndex][row].rhs = value; }
    void setLhs(uint32_t forceIndex, int row, const bsk::mat3x3& value) { solverSides[forceIndex][row].lhs = value; }

    // index specific
    void setJ(uint32_t forceIndex, int row, const glm::vec3& value) { derivatives[forceIndex][row].J = value; }
    void setH(uint32_t forceIndex, int row, const glm::mat3& value) { derivatives[forceIndex][row].H = value; }
    void setC(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].C = value; }
    void setFmin(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].fmin = value; }
    void setFmax(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].fmax = value; }
    void setStiffness(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].stiffness = value; }
    void setFracture(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].fracture = value; }
    void setPenalty(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].penalty = value; }
    void setLambda(uint32_t forceIndex, int row, float value) { parameters[forceIndex][row].lambda = value; }

    void setParameter(uint32_t forceIndex, int row, const ParameterStruct& value) { parameters[forceIndex][row] = value; }
    void setDerivative(uint32_t forceIndex, int row, const DerivativeStruct& value) { derivatives[forceIndex][row] = value; }

    // full row
    void setParameters(uint32_t forceIndex, const Parameters& value) { parameters[forceIndex] = value; }
    void setDerivatives(uint32_t forceIndex, const Derivatives& value) { derivatives[forceIndex] = value; }

    // debug
    void printIndices() const;
};

}

#endif