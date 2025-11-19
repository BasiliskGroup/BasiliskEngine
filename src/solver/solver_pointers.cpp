#include <basilisk/solver/physics.h>

namespace bsk::internal {

void Solver::insert(Rigid* rigid) {
    if (rigid == nullptr) return;
    
    rigid->getNext() = bodies;
    rigid->getPrev() = nullptr;
    
    if (bodies) {
        bodies->getPrev() = rigid;
    }
    
    bodies = rigid;
}

void Solver::insert(Force* force) {
    if (force == nullptr) return;
    
    force->getNext() = forces;
    force->getPrev() = nullptr;
    
    if (forces) {
        forces->getPrev() = force;
    }
    
    forces = force;
}

void Solver::remove(Rigid* rigid) {
    if (rigid == nullptr) return;
    
    if (rigid->getPrev()) {
        rigid->getPrev()->getNext() = rigid->getNext();
    } else {
        bodies = rigid->getNext();
    }
    
    if (rigid->getNext()) {
        rigid->getNext()->getPrev() = rigid->getPrev();
    }
    
    rigid->getPrev() = nullptr;
    rigid->getNext() = nullptr;
}

void Solver::remove(Force* force) {
    if (force == nullptr) return;
    
    if (force->getPrev()) {
        force->getPrev()->getNext() = force->getNext();
    } else {
        forces = force->getNext();
    }
    
    if (force->getNext()) {
        force->getNext()->getPrev() = force->getPrev();
    }
    
    force->getPrev() = nullptr;
    force->getNext() = nullptr;
}

void Solver::clear() {
    // delete bodies
    Rigid* curRigid = bodies;
    while (curRigid) {
        Rigid* nextRigid = curRigid->getNext();
        delete curRigid;
        curRigid = nextRigid;
    }

    // delete forces
    Force* curForce = forces;
    while (curForce) {
        Force* nextForce = curForce->getNext();

        // set to nullptr to prevent bad removes()
        curForce->getSolver() = nullptr;
        delete curForce;
        curForce = nextForce;
    }

    delete bodyTable;
    delete forceTable;
    delete colliderTable;

    // set everything to nullptr for clean practice
    bodies = nullptr;
    forces = nullptr;
    bodyTable = nullptr;
    forceTable = nullptr;
    colliderTable = nullptr;
}

}