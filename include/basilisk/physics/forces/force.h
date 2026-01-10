/*
* Copyright (c) 2025 Chris Giles
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Chris Giles makes no representations about the suitability
* of this software for any purpose.
* It is provided "as is" without express or implied warranty.
*/

#pragma once

#include <basilisk/util/includes.h>
#include <stdexcept>

#define MAX_ROWS 4                    // Most number of rows an individual constraint can have

namespace bsk::internal {
    class Solver;
    class Rigid;
}

namespace bsk::internal {

// Holds all user defined and derived constraint parameters, and provides a common interface for all forces.
class Force
{
protected:
    Solver* solver;
    Rigid* bodyA;
    Rigid* bodyB;
    Force* nextA;
    Force* nextB;
    Force* next;
    Force* prev;
    Force* prevA;
    Force* prevB;

    glm::vec3 JA[MAX_ROWS];
    glm::vec3 JB[MAX_ROWS];
    glm::mat3 H[MAX_ROWS];
    float C[MAX_ROWS];
    float fmin[MAX_ROWS];
    float fmax[MAX_ROWS];
    float stiffness[MAX_ROWS];
    float fracture[MAX_ROWS];
    float penalty[MAX_ROWS];
    float lambda[MAX_ROWS];

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    virtual ~Force();

    void disable();

    virtual int rows() const = 0;
    virtual bool initialize() = 0;
    virtual void computeConstraint(float alpha) = 0;
    virtual void computeDerivatives(Rigid* body) = 0;
    
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
    const glm::vec3& getJ(int index, Rigid* body) const { 
        if (body == bodyA) return JA[index];
        if (body == bodyB) return JB[index];
        throw std::runtime_error("Force::getJ: body must be either bodyA or bodyB");
    }
    const glm::mat3& getH(int index) const { return H[index]; }
    float getC(int index) const { return C[index]; }
    float getFmin(int index) const { return fmin[index]; }
    float getFmax(int index) const { return fmax[index]; }
    float getStiffness(int index) const { return stiffness[index]; }
    float getFracture(int index) const { return fracture[index]; }
    float getPenalty(int index) const { return penalty[index]; }
    float getLambda(int index) const { return lambda[index]; }
    
    // Setters
    void setJ(int index, Rigid* body, const glm::vec3& value) { 
        if (body == bodyA) {
            JA[index] = value;
            return;
        }
        if (body == bodyB) {
            JB[index] = value;
            return;
        }
        throw std::runtime_error("Force::getJ: body must be either bodyA or bodyB");
    }
    void setH(int index, const glm::mat3& value) { H[index] = value; }
    void setC(int index, float value) { C[index] = value; }
    void setFmin(int index, float value) { fmin[index] = value; }
    void setFmax(int index, float value) { fmax[index] = value; }
    void setStiffness(int index, float value) { stiffness[index] = value; }
    void setFracture(int index, float value) { fracture[index] = value; }
    void setPenalty(int index, float value) { penalty[index] = value; }
    void setLambda(int index, float value) { lambda[index] = value; }
    void setNext(Force* value) { next = value; }
    void setPrev(Force* value) { prev = value; }
    void setNextA(Force* value) { nextA = value; }
    void setNextB(Force* value) { nextB = value; }
    void setPrevA(Force* value) { prevA = value; }
    void setPrevB(Force* value) { prevB = value; }
    
    // // Get mutable references for direct access (for performance-critical code)
    // glm::vec3& getJRef(int index, Rigid* body) { return (body == bodyA) ? JA[index] : JB[index]; }
    // glm::mat3& getHRef(int index) { return H[index]; }
    // float& getCRef(int index) { return C[index]; }
    // float& getStiffnessRef(int index) { return stiffness[index]; }
    // float& getPenaltyRef(int index) { return penalty[index]; }
    // float& getLambdaRef(int index) { return lambda[index]; }
    // float& getFminRef(int index) { return fmin[index]; }
    // float& getFmaxRef(int index) { return fmax[index]; }
    // float& getFractureRef(int index) { return fracture[index]; }
};

}

