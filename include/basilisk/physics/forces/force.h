#pragma once

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/adjacency.h>

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

    std::size_t index;

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
    std::size_t getIndex() const { return index; }
    glm::vec3& getPosA() const;
    glm::vec3& getPosB() const;
    glm::vec3& getInitialA() const;
    glm::vec3& getInitialB() const;
    ForceType getForceType() const;

    glm::vec3& getJ(int index, Rigid* body) const;
    glm::mat3x3& getH(int index, Rigid* body) const;
    glm::vec3& getJA(int index) const;
    glm::vec3& getJB(int index) const;
    glm::mat3x3& getHA(int index) const;
    glm::mat3x3& getHB(int index) const;
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
    void setIndex(std::size_t index) { this->index = index; }
    void setPosA(const glm::vec3& value);
    void setPosB(const glm::vec3& value);
    void setInitialA(const glm::vec3& value);
    void setInitialB(const glm::vec3& value);
    void setForceType(ForceType value);

    void setJ(int index, Rigid* body, const glm::vec3& value);
    void setH(int index, Rigid* body, const glm::mat3& value);
    void setJA(int index, const glm::vec3& value);
    void setJB(int index, const glm::vec3& value);
    void setHA(int index, const glm::mat3& value);
    void setHB(int index, const glm::mat3& value);
    void setC(int index, float value);
    void setFmin(int index, float value);
    void setFmax(int index, float value);
    void setStiffness(int index, float value);
    void setFracture(int index, float value);
    void setPenalty(int index, float value);
    void setLambda(int index, float value);
};

}

