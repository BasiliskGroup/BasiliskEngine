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

    // TODO determine if we need padding (vec4 aligned)
    // float _padding;
};

struct DerivativeStruct {
    bsk::vec3 J;
    bsk::mat3x3 H;
};

union SpecialParameters {
    ManifoldData manifold;
    JointStruct joint;
    SpringStruct spring;
    MotorStruct motor;

    SpecialParameters() : manifold() {}
};

struct Positional {
    std::array<bsk::vec3, 2> pos;
    std::array<bsk::vec3, 2> initial;
};

// ------------------------------------------------------------
// Force Table Types
// ------------------------------------------------------------
using Derivatives = std::array<DerivativeStruct, MAX_ROWS>;
using Parameters = std::array<ParameterStruct, MAX_ROWS>;

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
    std::vector<Derivatives> derivativesA;
    std::vector<Derivatives> derivativesB;
    std::vector<ForceType> forceTypes;
    std::vector<SpecialParameters> specialParameters;
    std::vector<Positional> positional;

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

    // getters 
    Force* getForce(std::size_t index) { return forces[index]; }
    bool getToDelete(std::size_t index) { return toDelete[index]; }
    int getRows(std::size_t index) { return rows[index]; }
    ForceType getForceType(std::size_t index) { return forceTypes[index]; }
    Positional& getPositional(std::size_t index) { return positional[index]; }
    bsk::vec3& getPosA(std::size_t index) { return positional[index].pos[0]; }
    bsk::vec3& getPosB(std::size_t index) { return positional[index].pos[1]; }
    bsk::vec3& getInitialA(std::size_t index) { return positional[index].initial[0]; }
    bsk::vec3& getInitialB(std::size_t index) { return positional[index].initial[1]; }

    Solver* getSolver() { return solver; }
    ForceTypeTable<ManifoldData>* getManifoldTable() { return manifoldTable; }
    ForceTypeTable<JointStruct>* getJointTable() { return jointTable; }
    ForceTypeTable<SpringStruct>* getSpringTable() { return springTable; }
    ForceTypeTable<MotorStruct>* getMotorTable() { return motorTable; }

    // index specific
    bsk::vec3& getJA(std::size_t forceIndex, int row) { return derivativesA[forceIndex][row].J; }
    bsk::vec3& getJB(std::size_t forceIndex, int row) { return derivativesB[forceIndex][row].J; }
    bsk::mat3x3& getHA(std::size_t forceIndex, int row) { return derivativesA[forceIndex][row].H; }
    bsk::mat3x3& getHB(std::size_t forceIndex, int row) { return derivativesB[forceIndex][row].H; }
    float getC(std::size_t forceIndex, int row) { return parameters[forceIndex][row].C; }
    float getFmin(std::size_t forceIndex, int row) { return parameters[forceIndex][row].fmin; }
    float getFmax(std::size_t forceIndex, int row) { return parameters[forceIndex][row].fmax; }
    float getStiffness(std::size_t forceIndex, int row) { return parameters[forceIndex][row].stiffness; }
    float getFracture(std::size_t forceIndex, int row) { return parameters[forceIndex][row].fracture; }
    float getPenalty(std::size_t forceIndex, int row) { return parameters[forceIndex][row].penalty; }
    float getLambda(std::size_t forceIndex, int row) { return parameters[forceIndex][row].lambda; }

    ParameterStruct& getParameter(std::size_t forceIndex, int row) { return parameters[forceIndex][row]; }
    DerivativeStruct& getDerivativeA(std::size_t forceIndex, int row) { return derivativesA[forceIndex][row]; }
    DerivativeStruct& getDerivativeB(std::size_t forceIndex, int row) { return derivativesB[forceIndex][row]; }

    std::size_t& getMappedIndex(std::size_t forceIndex) { return indexMap[forceIndex]; }

    // full row
    Parameters& getParameters(std::size_t forceIndex) { return parameters[forceIndex]; }
    Derivatives& getDerivativesA(std::size_t forceIndex) { return derivativesA[forceIndex]; }
    Derivatives& getDerivativesB(std::size_t forceIndex) { return derivativesB[forceIndex]; }

    SpecialParameters& getSpecialParameters(std::size_t forceIndex) { return specialParameters[forceIndex]; }
    ManifoldData& getManifolds(std::size_t forceIndex) { return specialParameters[forceIndex].manifold; }
    JointStruct& getJoints(std::size_t forceIndex) { return specialParameters[forceIndex].joint; }
    SpringStruct& getSprings(std::size_t forceIndex) { return specialParameters[forceIndex].spring; }
    MotorStruct& getMotors(std::size_t forceIndex) { return specialParameters[forceIndex].motor; }

    // setters
    void setForces(std::size_t index, Force* value) { forces[index] = value; }
    void setToDelete(std::size_t index, bool value) { toDelete[index] = value; }
    void setRows(std::size_t index, int value) { rows[index] = value; }
    void setForceType(std::size_t index, ForceType value);
    void setPositional(std::size_t index, const Positional& value) { positional[index] = value; }
    void setPosA(std::size_t index, const glm::vec3& value) { positional[index].pos[static_cast<std::size_t>(ForceBodyOffset::A)] = value; }
    void setPosB(std::size_t index, const glm::vec3& value) { positional[index].pos[static_cast<std::size_t>(ForceBodyOffset::B)] = value; }
    void setInitialA(std::size_t index, const glm::vec3& value) { positional[index].initial[static_cast<std::size_t>(ForceBodyOffset::A)] = value; }
    void setInitialB(std::size_t index, const glm::vec3& value) { positional[index].initial[static_cast<std::size_t>(ForceBodyOffset::B)] = value; }
    void setSolver(Solver* value) { solver = value; }
    
    // index specific
    void setJA(std::size_t forceIndex, int row, const glm::vec3& value) { derivativesA[forceIndex][row].J = value; }
    void setJB(std::size_t forceIndex, int row, const glm::vec3& value) { derivativesB[forceIndex][row].J = value; }
    void setHA(std::size_t forceIndex, int row, const glm::mat3& value) { derivativesA[forceIndex][row].H = value; }
    void setHB(std::size_t forceIndex, int row, const glm::mat3& value) { derivativesB[forceIndex][row].H = value; }
    void setC(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].C = value; }
    void setFmin(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].fmin = value; }
    void setFmax(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].fmax = value; }
    void setStiffness(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].stiffness = value; }
    void setFracture(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].fracture = value; }
    void setPenalty(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].penalty = value; }
    void setLambda(std::size_t forceIndex, int row, float value) { parameters[forceIndex][row].lambda = value; }

    void setParameter(std::size_t forceIndex, int row, const ParameterStruct& value) { parameters[forceIndex][row] = value; }
    void setDerivativeA(std::size_t forceIndex, int row, const DerivativeStruct& value) { derivativesA[forceIndex][row] = value; }
    void setDerivativeB(std::size_t forceIndex, int row, const DerivativeStruct& value) { derivativesB[forceIndex][row] = value; }

    // full row
    void setParameters(std::size_t forceIndex, const Parameters& value) { parameters[forceIndex] = value; }
    void setDerivativesA(std::size_t forceIndex, const Derivatives& value) { derivativesA[forceIndex] = value; }
    void setDerivativesB(std::size_t forceIndex, const Derivatives& value) { derivativesB[forceIndex] = value; }

    void setSpecialParameters(std::size_t forceIndex, const SpecialParameters& value) { specialParameters[forceIndex] = value; }
    void setManifolds(std::size_t forceIndex, const ManifoldData& value) { specialParameters[forceIndex].manifold = value; }
    void setJoints(std::size_t forceIndex, const JointStruct& value) { specialParameters[forceIndex].joint = value; }
    void setSprings(std::size_t forceIndex, const SpringStruct& value) { specialParameters[forceIndex].spring = value; }
    void setMotors(std::size_t forceIndex, const MotorStruct& value) { specialParameters[forceIndex].motor = value; }
};

}

#endif