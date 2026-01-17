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

    std::size_t index;

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
    std::size_t getIndex() const { return index; }

    // index specific
    glm::vec3& getJ(int index, Rigid* body) const;
    glm::mat3x3& getH(int index, Rigid* body) const;
    float getC(int index) const;
    float getFmin(int index) const;
    float getFmax(int index) const;
    float getStiffness(int index) const;
    float getFracture(int index) const;
    float getPenalty(int index) const;
    float getLambda(int index) const;

    // full row
    std::array<glm::vec3, MAX_ROWS>& getJ(Rigid* body) const;
    std::array<glm::mat3x3, MAX_ROWS>& getH(Rigid* body) const;
    std::array<float, MAX_ROWS>& getC() const;
    std::array<float, MAX_ROWS>& getFmin() const;
    std::array<float, MAX_ROWS>& getFmax() const;
    std::array<float, MAX_ROWS>& getStiffness() const;
    std::array<float, MAX_ROWS>& getFracture() const;
    std::array<float, MAX_ROWS>& getPenalty() const;
    std::array<float, MAX_ROWS>& getLambda() const;
    
    // Setters
    void setNext(Force* value) { next = value; }
    void setPrev(Force* value) { prev = value; }
    void setNextA(Force* value) { nextA = value; }
    void setNextB(Force* value) { nextB = value; }
    void setPrevA(Force* value) { prevA = value; }
    void setPrevB(Force* value) { prevB = value; }
    void setIndex(std::size_t index) { this->index = index; }

    // index specific
    void setJ(int index, Rigid* body, const glm::vec3& value);
    void setH(int index, Rigid* body, const glm::mat3& value);
    void setC(int index, float value);
    void setFmin(int index, float value);
    void setFmax(int index, float value);
    void setStiffness(int index, float value);
    void setFracture(int index, float value);
    void setPenalty(int index, float value);
    void setLambda(int index, float value);

    // full row
    void setJ(Rigid* body, const std::array<glm::vec3, MAX_ROWS>& value);
    void setH(Rigid* body, const std::array<glm::mat3x3, MAX_ROWS>& value);
    void setC(const std::array<float, MAX_ROWS>& value);
    void setFmin(const std::array<float, MAX_ROWS>& value);
    void setFmax(const std::array<float, MAX_ROWS>& value);
    void setStiffness(const std::array<float, MAX_ROWS>& value);
    void setFracture(const std::array<float, MAX_ROWS>& value);
    void setPenalty(const std::array<float, MAX_ROWS>& value);
    void setLambda(const std::array<float, MAX_ROWS>& value);
};

}

