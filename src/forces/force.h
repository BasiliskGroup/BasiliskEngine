#ifndef FORCE_H
#define FORCE_H


class ForceTable;
class Solver;
class Rigid;

class Force {
private:
    Solver* solver;
    Force* next; // next in solver list
    Force* prev; // prev in solver list
    Force* nextA; // next in body list
    Force* prevA; // prev in body list

    Force* twin; // points to twin force

    Rigid* bodyA;
    Rigid* bodyB;

protected:

    // for table access
    uint index;

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
    
    Vec3ROWS& J();
    Mat3x3ROWS& H();
    FloatROWS& C();
    FloatROWS& motor();
    FloatROWS& stiffness();
    FloatROWS& fracture();
    FloatROWS& fmax();
    FloatROWS& fmin();
    FloatROWS& penalty();
    FloatROWS& lambda();

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
};

// --------------------------------------------- //
//         TODO add springs and joints           //
// --------------------------------------------- //

#endif