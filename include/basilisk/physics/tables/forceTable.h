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
#include <basilisk/physics/tables/adjacency.h>
#include <basilisk/compute/gpuTypes.hpp>

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
    std::vector<std::size_t> indexMap;

    // force specific tables
    ForceTypeTable<ManifoldData>* manifoldTable;
    ForceTypeTable<JointStruct>* jointTable;
    ForceTypeTable<SpringStruct>* springTable;
    ForceTypeTable<MotorStruct>* motorTable;

    // structure variables
    std::vector<int> rows;

public:
    ForceTable(std::size_t capacity);
    ~ForceTable();

    void markAsDeleted(std::size_t index);
    void resize(std::size_t newCapacity);
    void compact();
    void insert(Force* force);

    void remapBodyIndices();

    // getters 
    Force* getForce(std::size_t index) { return forces[index]; }
    bool getToDelete(std::size_t index) { return toDelete[index]; }
    int getRows(std::size_t index) { return rows[index]; }
    ForceType getForceType(std::size_t index) { return forceTypes[index]; }
    BodyStruct& getBodies(std::size_t index) { return bodies[index]; }
    glm::vec3 getPosA(std::size_t index);
    glm::vec3 getPosB(std::size_t index);
    glm::vec3 getInitialA(std::size_t index);
    glm::vec3 getInitialB(std::size_t index);

    Solver* getSolver() { return solver; }
    ForceTypeTable<ManifoldData>* getManifoldTable() { return manifoldTable; }
    ForceTypeTable<JointStruct>* getJointTable() { return jointTable; }
    ForceTypeTable<SpringStruct>* getSpringTable() { return springTable; }
    ForceTypeTable<MotorStruct>* getMotorTable() { return motorTable; }

    // index specific
    bsk::vec3& getJ(std::size_t forceIndex, int row) { return derivatives[forceIndex][row].J; }
    bsk::mat3x3& getH(std::size_t forceIndex, int row) { return derivatives[forceIndex][row].H; }
    float getC(std::size_t forceIndex, int row) { return parameters[forceIndex][row].C; }
    float getFmin(std::size_t forceIndex, int row) { return parameters[forceIndex][row].fmin; }
    float getFmax(std::size_t forceIndex, int row) { return parameters[forceIndex][row].fmax; }
    float getStiffness(std::size_t forceIndex, int row) { return parameters[forceIndex][row].stiffness; }
    float getFracture(std::size_t forceIndex, int row) { return parameters[forceIndex][row].fracture; }
    float getPenalty(std::size_t forceIndex, int row) { return parameters[forceIndex][row].penalty; }
    float getLambda(std::size_t forceIndex, int row) { return parameters[forceIndex][row].lambda; }
    bsk::vec3& getRhs(std::size_t forceIndex, int row) { return solverSides[forceIndex][row].rhs; }
    bsk::mat3x3& getLhs(std::size_t forceIndex, int row) { return solverSides[forceIndex][row].lhs; }

    ParameterStruct& getParameter(std::size_t forceIndex, int row) { return parameters[forceIndex][row]; }
    DerivativeStruct& getDerivative(std::size_t forceIndex, int row) { return derivatives[forceIndex][row]; }

    std::size_t& getMappedIndex(std::size_t forceIndex) { return indexMap[forceIndex]; }

    // full row
    Parameters& getParameters(std::size_t forceIndex) { return parameters[forceIndex]; }
    Derivatives& getDerivatives(std::size_t forceIndex) { return derivatives[forceIndex]; }

    // setters
    void setForces(std::size_t index, Force* value) { forces[index] = value; }
    void setToDelete(std::size_t index, bool value) { toDelete[index] = value; }
    void setRows(std::size_t index, int value) { rows[index] = value; }
    void setForceType(std::size_t index, ForceType value);
    void setBodies(std::size_t index, const BodyStruct& value) { bodies[index] = value; }
    void setPosA(std::size_t index, const glm::vec3& value);
    void setPosB(std::size_t index, const glm::vec3& value);
    void setInitialA(std::size_t index, const glm::vec3& value);
    void setInitialB(std::size_t index, const glm::vec3& value);
    void setSolver(Solver* value) { solver = value; }
    void setRhs(std::size_t forceIndex, int row, const bsk::vec3& value) { solverSides[forceIndex][row].rhs = value; }
    void setLhs(std::size_t forceIndex, int row, const bsk::mat3x3& value) { solverSides[forceIndex][row].lhs = value; }

    // index specific
    void setJ(std::size_t forceIndex, int row, const glm::vec3& value) { derivatives[forceIndex][row].J = value; }
    void setH(std::size_t forceIndex, int row, const glm::mat3& value) { derivatives[forceIndex][row].H = value; }
    void setC(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].C = value; }
    void setFmin(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].fmin = value; }
    void setFmax(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].fmax = value; }
    void setStiffness(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].stiffness = value; }
    void setFracture(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].fracture = value; }
    void setPenalty(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].penalty = value; }
    void setLambda(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].lambda = value; }

    void setParameter(std::size_t forceIndex, int row, const ParameterStruct& value) { parameters[forceIndex][row] = value; }
    void setDerivative(std::size_t forceIndex, int row, const DerivativeStruct& value) { derivatives[forceIndex][row] = value; }

    // full row
    void setParameters(std::size_t forceIndex, const Parameters& value) { parameters[forceIndex] = value; }
    void setDerivatives(std::size_t forceIndex, const Derivatives& value) { derivatives[forceIndex] = value; }

    // debug
    void printIndices() const;
};

}

#endif