#ifndef BSK_PHYSICS_FORCES_FORCE_H
#define BSK_PHYSICS_FORCES_FORCE_H

#include <basilisk/util/includes.h>
#include <basilisk/compute/gpuTypes.hpp>

namespace bsk::internal {

class Solver;
class Rigid;
class ForceTable;

// Holds all user defined and derived constraint parameters, and provides a common interface for all forces.
class Force {
protected:
    Solver* solver;
    ForceTable* forceTable;
    Rigid* bodyA;
    Rigid* bodyB;
    Force* nextA;
    Force* nextB;
    Force* next;
    Force* prev;
    Force* prevA;
    Force* prevB;

    uint32_t index;
    uint32_t specialIndex = -1; // default to invalid index

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    virtual ~Force();

    void disable();

    virtual int rows() = 0;
    virtual bool initialize() = 0;
    
    // Getters
    Solver* getSolver() const { return solver; }
    Rigid* getBodyA() const { return bodyA; }
    Rigid* getBodyB() const { return bodyB; }
    Force* getNext() const { return next; }
    Force* getNextA() const { return nextA; }
    Force* getNextB() const { return nextB; }
    Force* getPrev() const { return prev; }
    Force* getPrevA() const { return prevA; }
    Force* getPrevB() const { return prevB; }
    uint32_t getIndex() const { return index; }
    uint32_t getSpecialIndex() const { return specialIndex; }
    uint32_t getForceIndex() const { return index; }
    glm::vec3 getPosA() const;
    glm::vec3 getPosB() const;
    glm::vec3 getInitialA() const;
    glm::vec3 getInitialB() const;
    ForceType getForceType() const;

    bsk::vec3& getJ(int index) const;
    bsk::mat3x3& getH(int index) const;
    float getC(int index) const;
    float getFmin(int index) const;
    float getFmax(int index) const;
    float getStiffness(int index) const;
    float getFracture(int index) const;
    float getPenalty(int index) const;
    float getLambda(int index) const;

    // Setters
    void setNext(Force* value) { next = value; }
    void setPrev(Force* value) { prev = value; }
    void setNextA(Force* value) { nextA = value; }
    void setNextB(Force* value) { nextB = value; }
    void setPrevA(Force* value) { prevA = value; }
    void setPrevB(Force* value) { prevB = value; }
    void setIndex(uint32_t index) { this->index = index; }
    void setSpecialIndex(uint32_t index) { this->specialIndex = index; }
    void setForceIndex(uint32_t index) { this->index = index; }
    void setPosA(const glm::vec3& value);
    void setPosB(const glm::vec3& value);
    void setInitialA(const glm::vec3& value);
    void setInitialB(const glm::vec3& value);
    void setForceType(ForceType value);

    void setJ(int index, const glm::vec3& value);
    void setH(int index, const glm::mat3& value);
    void setC(int index, float value);
    void setFmin(int index, float value);
    void setFmax(int index, float value);
    void setStiffness(int index, float value);
    void setFracture(int index, float value);
    void setPenalty(int index, float value);
    void setLambda(int index, float value);
};

}

#endif