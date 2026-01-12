#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/physics/collision/bvh.h>
#include <basilisk/physics/maths.h>

namespace bsk::internal {

Rigid::Rigid(Solver* solver, Node2D* node, Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity)
    : solver(solver), node(node), forces(nullptr), next(nullptr), prev(nullptr), collider(collider), degree(0), satur(0) {
    // Add to linked list
    solver->insert(this);
    this->solver->getBodyTable()->insert(this, position, size, density, friction, velocity, collider);
}


Rigid::~Rigid() {
    // Remove from linked list
    solver->remove(this);

    // remove from bvh and bodytable
    this->solver->getBodyTable()->getBVH()->remove(this);
    this->solver->getBodyTable()->markAsDeleted(this->index);

    // Delete all forces
    Force* curForce = forces;
    while (curForce) {
        Force* nextForce = (curForce->getBodyA() == this) ? curForce->getNextA() : curForce->getNextB();
        delete curForce;
        curForce = nextForce;
    }
}

bool Rigid::constrainedTo(Rigid* other) const {
    // Check if this body is constrained to the other body
    for (Force* f = forces; f != nullptr; f = (f->getBodyA() == this) ? f->getNextA() : f->getNextB())
        if ((f->getBodyA() == this && f->getBodyB() == other) || (f->getBodyA() == other && f->getBodyB() == this))
            return true;
    return false;
}

void Rigid::resetColoring() {
    setColor(-1);
    setDegree(0);
    setSatur(0);
    this->usedColors.clear();
}

bool Rigid::isColorUsed(int color) const {
    if (color < 0 || color >= usedColors.size()) {
        return false;
    }
    return usedColors[color];
}

void Rigid::reserveColors(int count) {
    usedColors.resize(count, false);
}

bool Rigid::isColored() const {
    return getColor() != -1;
}

void Rigid::useColor(int color) {
    reserveColors(color + 1);
    usedColors[color] = true;
}

int Rigid::getNextUnusedColor() const {
    int candidate = 0;
    while (isColorUsed(candidate)) {
        candidate++;
    }
    return candidate;
}

bool Rigid::verifyColoring() const {
    int myColor = getColor();
    // If not colored, verification passes trivially
    if (myColor == -1) {
        return true;
    }
    
    // Check all adjacent rigid bodies connected through forces
    for (Force* force = forces; force != nullptr; force = (force->getBodyA() == this) ? force->getNextA() : force->getNextB()) {
        Rigid* other = (force->getBodyA() == this) ? force->getBodyB() : force->getBodyA();
        
        // If adjacent rigid has the same color, coloring is invalid
        if (other != nullptr && other->getColor() == myColor) {
            return false;
        }
    }
    
    return true;
}

void Rigid::insert(Force* force) {
    if (force == nullptr) {
        return;
    }

    // Determine if this body is bodyA or bodyB
    if (force->getBodyA() == this) {
        // This is bodyA
        force->setNextA(forces);
        force->setPrevA(nullptr);

        if (forces) {
            // Update the prev pointer of the old head
            if (forces->getBodyA() == this) {
                forces->setPrevA(force);
            } else {
                forces->setPrevB(force);
            }
        }
    } else {
        // This is bodyB
        force->setNextB(forces);
        force->setPrevB(nullptr);

        if (forces) {
            // Update the prev pointer of the old head
            if (forces->getBodyA() == this) {
                forces->setPrevA(force);
            } else {
                forces->setPrevB(force);
            }
        }
    }

    forces = force;
    degree++;
}

void Rigid::remove(Force* force) {
    if (force == nullptr) {
        return;
    }

    // Determine if this body is bodyA or bodyB
    bool isBodyA = (force->getBodyA() == this);

    Force* prev = isBodyA ? force->getPrevA() : force->getPrevB();
    Force* next = isBodyA ? force->getNextA() : force->getNextB();

    if (prev) {
        // Update prev's next pointer
        if (prev->getBodyA() == this) {
            prev->setNextA(next);
        } else {
            prev->setNextB(next);
        }
    } else {
        // This was the head of the list
        forces = next;
    }

    if (next) {
        // Update next's prev pointer
        if (next->getBodyA() == this) {
            next->setPrevA(prev);
        } else {
            next->setPrevB(prev);
        }
    }

    // Clear this force's pointers
    if (isBodyA) {
        force->setPrevA(nullptr);
        force->setNextA(nullptr);
    } else {
        force->setPrevB(nullptr);
        force->setNextB(nullptr);
    }

    degree--;
}

void Rigid::setPosition(const glm::vec3& pos) {
    this->solver->getBodyTable()->setPos(this->index, pos);
}

void Rigid::setScale(const glm::vec2& scale) {
    this->solver->getBodyTable()->setScale(this->index, scale);
}

void Rigid::setVelocity(const glm::vec3& vel) {
    this->solver->getBodyTable()->setVel(this->index, vel);
}

glm::vec3 Rigid::getVelocity() const {
    return this->solver->getBodyTable()->getVel(this->index);
}

float Rigid::getDensity() const {
    float mass = this->solver->getBodyTable()->getMass(this->index);
    glm::vec2 size = this->solver->getBodyTable()->getScale(this->index);
    return mass / (size.x * size.y);
}

float Rigid::getFriction() const {
    return this->solver->getBodyTable()->getFriction(this->index);
}

glm::vec3 Rigid::getVel() const {
    return this->solver->getBodyTable()->getVel(this->index);
}

void Rigid::setInitial(const glm::vec3& initial) {
    this->solver->getBodyTable()->setInitial(this->index, initial);
}

void Rigid::setInertial(const glm::vec3& inertial) {
    this->solver->getBodyTable()->setInertial(this->index, inertial);
}

void Rigid::setPrevVelocity(const glm::vec3& prevVelocity) {
    this->solver->getBodyTable()->setPrevVel(this->index, prevVelocity);
}

void Rigid::setMass(float mass) {
    this->solver->getBodyTable()->setMass(this->index, mass);
}

void Rigid::setMoment(float moment) {
    this->solver->getBodyTable()->setMoment(this->index, moment);
}

void Rigid::setFriction(float friction) {
    this->solver->getBodyTable()->setFriction(this->index, friction);
}

void Rigid::setRadius(float radius) {
    this->solver->getBodyTable()->setRadius(this->index, radius);
}

void Rigid::setColor(int color) {
    this->solver->getBodyTable()->setColor(this->index, color);
}

void Rigid::setDegree(int degree) {
    this->degree = degree;
}

void Rigid::setSatur(int satur) {
    this->satur = satur;
}

void Rigid::setCollider(Collider* collider) {
    this->collider = collider;
}

void Rigid::setForces(Force* forces) {
    this->forces = forces;
}

void Rigid::setNext(Rigid* next) {
    this->next = next;
}

void Rigid::setPrev(Rigid* prev) {
    this->prev = prev;
}

void Rigid::setNode(Node2D* node) {
    this->node = node;
}

void Rigid::setIndex(std::size_t index) {
    this->index = index;
}

glm::vec3 Rigid::getPosition() const {
    return this->solver->getBodyTable()->getPos(this->index);
}

glm::vec3 Rigid::getInitial() const {
    return this->solver->getBodyTable()->getInitial(this->index);
}

glm::vec3 Rigid::getInertial() const {
    return this->solver->getBodyTable()->getInertial(this->index);
}

glm::vec3 Rigid::getPrevVelocity() const {
    return this->solver->getBodyTable()->getPrevVel(this->index);
}

glm::vec2 Rigid::getSize() const {
    return this->solver->getBodyTable()->getScale(this->index);
}

float Rigid::getMass() const {
    return this->solver->getBodyTable()->getMass(this->index);
}

float Rigid::getMoment() const {
    return this->solver->getBodyTable()->getMoment(this->index);
}

float Rigid::getRadius() const {
    return this->solver->getBodyTable()->getRadius(this->index);
}

int Rigid::getColor() const {
    return this->solver->getBodyTable()->getColor(this->index);
}

int Rigid::getDegree() const {
    return this->degree;
}

int Rigid::getSatur() const {
    return this->satur;
}

void Rigid::getAABB(glm::vec2& bl, glm::vec2& tr) const {
    glm::vec3 pos = getPosition();
    glm::vec2 size = getSize();
    glm::vec2 halfDim = collider->getHalfDim();
    float angle = pos.z;  // Rotation angle
    
    // Compute the four corners of the unrotated rectangle in local space
    glm::vec2 localHalfDim = size * halfDim;
    glm::vec2 corners[4] = {
        glm::vec2(-localHalfDim.x, -localHalfDim.y),  // bottom-left
        glm::vec2( localHalfDim.x, -localHalfDim.y),  // bottom-right
        glm::vec2( localHalfDim.x,  localHalfDim.y),   // top-right
        glm::vec2(-localHalfDim.x,  localHalfDim.y)   // top-left
    };
    
    // Rotate and translate each corner, then find the bounding AABB
    glm::mat2 rot = rotation(angle);
    glm::vec2 worldPos = glm::vec2(pos.x, pos.y);
    
    // Initialize with first transformed corner
    glm::vec2 transformed = rot * corners[0] + worldPos;
    bl = transformed;
    tr = transformed;
    
    // Find min/max of all transformed corners
    for (int i = 1; i < 4; i++) {
        transformed = rot * corners[i] + worldPos;
        bl = glm::min(bl, transformed);
        tr = glm::max(tr, transformed);
    }
}

}