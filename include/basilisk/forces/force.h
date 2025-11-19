#ifndef BSK_FORCE_H
#define BSK_FORCE_H

namespace bsk::internal {

class ForceTable;
class Solver;
class Rigid;

class Force {
protected:
    Solver* solver;
    Force* next; // next in solver list
    Force* prev; // prev in solver list
    Force* nextA; // next in body list
    Force* prevA; // prev in body list

    Force* twin; // points to twin force

    Rigid* bodyA;
    Rigid* bodyB;

    // for table access
    uint index;

    // TEMP, restore tables
    BskVec3ROWS J;
    BskMat3x3ROWS H;
    BskFloatROWS C;
    BskFloatROWS motor;
    BskFloatROWS stiffness;
    BskFloatROWS fracture;
    BskFloatROWS fmax;
    BskFloatROWS fmin;
    BskFloatROWS penalty;
    BskFloatROWS lambda;
    ForceType type;
    bool isA;

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    Force() = default;
    virtual ~Force();

    // getters 
    // pointers
    inline Solver*& getSolver() { return solver; }
    inline Force*& getNext()    { return next; }
    inline Force*& getPrev()    { return prev; }
    inline Force*& getNextA()   { return nextA; }
    inline Force*& getPrevA()   { return prevA; }
    inline Force*& getTwin()    { return twin; }
    inline Rigid*& getBodyA()   { return bodyA; }
    inline Rigid*& getBodyB()   { return bodyB; }

    ForceTable* getTable();
    
    BskVec3ROWS& getJ() { return J; }
    BskMat3x3ROWS& getH() { return H; }
    BskFloatROWS& getC() { return C; }
    BskFloatROWS& getMotor() { return motor; }
    BskFloatROWS& getStiffness() { return stiffness; }
    BskFloatROWS& getFracture() { return fracture; }
    BskFloatROWS& getFmax() { return fmax; }
    BskFloatROWS& getFmin() { return fmin; }
    BskFloatROWS& getPenalty() { return penalty; }
    BskFloatROWS& getLambda() { return lambda; }
    bool& getIsA() { return isA; }

    //table
    ForceType& getType();
    inline uint getIndex() { return index; }

    void markAsDeleted();
    void disable();

    // setters
    void setIndex(uint index) { this->index = index; }

    // number of jacobian rows (max = 4)
    virtual int rows() const = 0;
    virtual void draw() const {};

    // NOTE initialization and computations will be done in the ForceTable for bulk operations
    virtual void initialize() = 0;
    virtual void computeConstraint(float alpha) = 0;
    virtual void computeDerivatives(Rigid* body) = 0;
};

}

// --------------------------------------------- //
//         TODO add springs and joints           //
// --------------------------------------------- //

#endif