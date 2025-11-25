#include <basilisk/solver/physics.h>

namespace bsk::internal {

Rigid::Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, Collider* collider) : 
    solver(solver), 
    node(node), 
    forces(nullptr), 
    next(nullptr), 
    prev(nullptr), 
    density(density),
    relations(),
    index(-1),
    pos(pos),
    initial(pos),
    inertial(pos),
    vel(vel),
    prevVel(vel),
    scale(scale),
    friction(friction),
    collider(collider->getIndex())
{
    // Add to linked list
    solver->insert(this);

    // compute intermediate values
    float volume = scale.x * scale.y; // TODO replace this with actual collider volume
    mass = scale.x * scale.y * density * volume; 
    moment = mass * glm::dot(scale, scale) / 12.0f; // TODO replace with collider moment
    radius = glm::length(scale * 0.5f);

    computeTransforms();
}   

Rigid::Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, uint collider) : 
    solver(solver), 
    node(node), 
    forces(nullptr), 
    next(nullptr), 
    prev(nullptr), 
    density(density),
    relations(),
    index(-1),
    pos(pos),
    initial(pos),
    inertial(pos),
    vel(vel),
    prevVel(vel),
    scale(scale),
    friction(friction),
    collider(collider)
{
    // Add to linked list
    solver->insert(this);

    // compute intermediate values
    float volume = scale.x * scale.y; // TODO replace this with actual collider volume
    mass = scale.x * scale.y * density * volume; 
    moment = mass * glm::dot(scale, scale) / 12.0f; // TODO replace with collider moment
    radius = glm::length(scale * 0.5f);

    computeTransforms();
}   

Rigid::~Rigid() {
    // remove from Table
    solver->remove(this);

    // delete all forces
    Force* curForce = forces;
    while (curForce) {
        Force* nextForce = (curForce->getBodyA() == this) ? curForce->getNextA() : curForce->getNextB();
        delete curForce;
        curForce = nextForce;
    }
}

BodyTable* Rigid::getBodyTable() {
    return solver->getBodyTable();
}

uint Rigid::getColliderIndex() {
    return getBodyTable()->getCollider()[index];
}

// ----------------------
// Linked list management
// ----------------------

void Rigid::insert(Force* force){
    if (force == nullptr) {
        return;
    }

    // Determine if this body is bodyA or bodyB
    if (force->getBodyA() == this) {
        // This is bodyA
        force->getNextA() = forces;
        force->getPrevA() = nullptr;

        if (forces) {
            // Update the prev pointer of the old head
            if (forces->getBodyA() == this) {
                forces->getPrevA() = force;
            } else {
                forces->getPrevB() = force;
            }
        }
    } else {
        // This is bodyB
        force->getNextB() = forces;
        force->getPrevB() = nullptr;

        if (forces) {
            // Update the prev pointer of the old head
            if (forces->getBodyA() == this) {
                forces->getPrevA() = force;
            } else {
                forces->getPrevB() = force;
            }
        }
    }

    forces = force;
}

void Rigid::remove(Force* force){
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
            prev->getNextA() = next;
        } else {
            prev->getNextB() = next;
        }
    } else {
        // This was the head of the list
        forces = next;
    }

    if (next) {
        // Update next's prev pointer
        if (next->getBodyA() == this) {
            next->getPrevA() = prev;
        } else {
            next->getPrevB() = prev;
        }
    }

    // Clear this force's pointers
    if (isBodyA) {
        force->getPrevA() = nullptr;
        force->getNextA() = nullptr;
    } else {
        force->getPrevB() = nullptr;
        force->getNextB() = nullptr;
    }
}

// ----------------------
// Graph
// ----------------------

void Rigid::precomputeRelations() {
    relations.clear();

    uint i = 0;
    for (Force* f = forces; f != nullptr; f = (f->getBodyA() == this) ? f->getNextA() : f->getNextB()) {
        Rigid* other = (f->getBodyA() == this) ? f->getBodyB() : f->getBodyA();
        if (other == nullptr) {
            continue;
        }

        relations.emplace_back(other->getIndex(), f, f->getType());
    }
}

// ----------------------
// Broad Collision
// ----------------------

ForceType Rigid::constrainedTo(uint other, Force*& force) const {
    // check if this body is constrained to the other body
    for (const auto& rel : relations) {
        if (rel.bodyB == other) {
            force = rel.force;
            return rel.type;
        }
    }
    return NULL_FORCE;
}

void Rigid::draw() {

}

void Rigid::clear() {
    if (node != nullptr) {
        delete node;
        node = nullptr;
    }
}

// --------------------
// setters
// --------------------
void Rigid::setPosition(const glm::vec3& pos) {
    this->pos = pos;
}

void Rigid::setVelocity(const glm::vec3& vel) {
    this->vel = vel;
}

void Rigid::computeTransforms() {
    float angle = pos.z;
    float sx = scale.x, sy = scale.y;
    float isx = 1 / sx, isy = 1 / sy;

    float c = cos(angle);
    float s = -sin(angle); // to align with weird screen space coordinate systems. 

    rMat = { c, -s, s, c };
    mat = { c * sx, -s * sy, s * sx, c * sy };
    iMat = {
        c * isx,   s * isy,
    -s * isx,   c * isy
    };

    updated = false;
}

}

