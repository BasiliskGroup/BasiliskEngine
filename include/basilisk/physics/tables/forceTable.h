#ifndef FORCE_TABLE_H
#define FORCE_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>
#include <basilisk/util/constants.h>

namespace bsk::internal {

using ::bsk::internal::MAX_ROWS;

class Force;

class ForceTable : public VirtualTable {
private:
    // compute variables
    std::vector<Force*> forces;
    std::vector<bool> toDelete;
    std::vector<std::array<glm::vec3, MAX_ROWS>> JA;
    std::vector<std::array<glm::vec3, MAX_ROWS>> JB;
    std::vector<std::array<glm::mat3, MAX_ROWS>> HA;
    std::vector<std::array<glm::mat3, MAX_ROWS>> HB;
    std::vector<std::array<float, MAX_ROWS>> C;
    std::vector<std::array<float, MAX_ROWS>> fmin;
    std::vector<std::array<float, MAX_ROWS>> fmax;
    std::vector<std::array<float, MAX_ROWS>> stiffness;
    std::vector<std::array<float, MAX_ROWS>> fracture;
    std::vector<std::array<float, MAX_ROWS>> penalty;
    std::vector<std::array<float, MAX_ROWS>> lambda;

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
    Force* getForces(std::size_t index) { return forces[index]; }
    bool getToDelete(std::size_t index) { return toDelete[index]; }
    int getRows(std::size_t index) { return rows[index]; }

    // index specific
    glm::vec3& getJA(std::size_t forceIndex, int row) { return JA[forceIndex][row]; }
    glm::vec3& getJB(std::size_t forceIndex, int row) { return JB[forceIndex][row]; }
    glm::mat3& getHA(std::size_t forceIndex, int row) { return HA[forceIndex][row]; }
    glm::mat3& getHB(std::size_t forceIndex, int row) { return HB[forceIndex][row]; }
    float getC(std::size_t forceIndex, int row) { return C[forceIndex][row]; }
    float getFmin(std::size_t forceIndex, int row) { return fmin[forceIndex][row]; }
    float getFmax(std::size_t forceIndex, int row) { return fmax[forceIndex][row]; }
    float getStiffness(std::size_t forceIndex, int row) { return stiffness[forceIndex][row]; }
    float getFracture(std::size_t forceIndex, int row) { return fracture[forceIndex][row]; }
    float getPenalty(std::size_t forceIndex, int row) { return penalty[forceIndex][row]; }
    float getLambda(std::size_t forceIndex, int row) { return lambda[forceIndex][row]; }

    // getters full row
    std::array<glm::vec3, MAX_ROWS>& getJA(std::size_t forceIndex) { return JA[forceIndex]; }
    std::array<glm::vec3, MAX_ROWS>& getJB(std::size_t forceIndex) { return JB[forceIndex]; }
    std::array<glm::mat3x3, MAX_ROWS>& getHA(std::size_t forceIndex) { return HA[forceIndex]; }
    std::array<glm::mat3x3, MAX_ROWS>& getHB(std::size_t forceIndex) { return HB[forceIndex]; }
    std::array<float, MAX_ROWS>& getC(std::size_t forceIndex) { return C[forceIndex]; }
    std::array<float, MAX_ROWS>& getFmin(std::size_t forceIndex) { return fmin[forceIndex]; }
    std::array<float, MAX_ROWS>& getFmax(std::size_t forceIndex) { return fmax[forceIndex]; }
    std::array<float, MAX_ROWS>& getStiffness(std::size_t forceIndex) { return stiffness[forceIndex]; }
    std::array<float, MAX_ROWS>& getFracture(std::size_t forceIndex) { return fracture[forceIndex]; }
    std::array<float, MAX_ROWS>& getPenalty(std::size_t forceIndex) { return penalty[forceIndex]; }
    std::array<float, MAX_ROWS>& getLambda(std::size_t forceIndex) { return lambda[forceIndex]; }

    // setters
    void setForces(std::size_t index, Force* value) { forces[index] = value; }
    void setToDelete(std::size_t index, bool value) { toDelete[index] = value; }
    void setRows(std::size_t index, int value) { rows[index] = value; }

    // index specific
    void setJA(std::size_t forceIndex, int row, const glm::vec3& value) { JA[forceIndex][row] = value; }
    void setJB(std::size_t forceIndex, int row, const glm::vec3& value) { JB[forceIndex][row] = value; }
    void setHA(std::size_t forceIndex, int row, const glm::mat3& value) { HA[forceIndex][row] = value; }
    void setHB(std::size_t forceIndex, int row, const glm::mat3& value) { HB[forceIndex][row] = value; }
    void setC(std::size_t forceIndex, int row, float value) { C[forceIndex][row] = value; }
    void setFmin(std::size_t forceIndex, int row, float value) { fmin[forceIndex][row] = value; }
    void setFmax(std::size_t forceIndex, int row, float value) { fmax[forceIndex][row] = value; }
    void setStiffness(std::size_t forceIndex, int row, float value) { stiffness[forceIndex][row] = value; }
    void setFracture(std::size_t forceIndex, int row, float value) { fracture[forceIndex][row] = value; }
    void setPenalty(std::size_t forceIndex, int row, float value) { penalty[forceIndex][row] = value; }
    void setLambda(std::size_t forceIndex, int row, float value) { lambda[forceIndex][row] = value; }
    
    // full row
    void setJA(std::size_t forceIndex, const std::array<glm::vec3, MAX_ROWS>& value) { JA[forceIndex] = value; }
    void setJB(std::size_t forceIndex, const std::array<glm::vec3, MAX_ROWS>& value) { JB[forceIndex] = value; }
    void setHA(std::size_t forceIndex, const std::array<glm::mat3x3, MAX_ROWS>& value) { HA[forceIndex] = value; }
    void setHB(std::size_t forceIndex, const std::array<glm::mat3x3, MAX_ROWS>& value) { HB[forceIndex] = value; }
    void setC(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { C[forceIndex] = value; }
    void setFmin(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { fmin[forceIndex] = value; }
    void setFmax(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { fmax[forceIndex] = value; }
    void setStiffness(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { stiffness[forceIndex] = value; }
    void setFracture(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { fracture[forceIndex] = value; }
    void setPenalty(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { penalty[forceIndex] = value; }
    void setLambda(std::size_t forceIndex, const std::array<float, MAX_ROWS>& value) { lambda[forceIndex] = value; }
};

}

#endif