#include "scene/sceneRoute.h"

Node2D::Node2D(VirtualScene2D* scene, Params params)
    : VirtualNode(scene, params.mesh, params.material, params.position, params.rotation, params.scale), rigid(nullptr) {
    updateModel();
    bindRigid(params);
    getScene()->getEngine()->getResourceServer()->getMaterialServer()->add(params.material);
}

Node2D::Node2D(Node2D* parent, Params params)
    : VirtualNode(parent, params.mesh, params.material, params.position, params.rotation, params.scale), rigid(nullptr) {
    updateModel();
    bindRigid(params);
    getScene()->getEngine()->getResourceServer()->getMaterialServer()->add(params.material);
}

Node2D::Node2D(VirtualScene2D* scene, Node2D* parent) : VirtualNode(scene, parent), rigid(nullptr) {}

Node2D::Node2D(const Node2D& other) noexcept : VirtualNode(other), rigid(nullptr) {
    if (this == &other) return;
    // setRigid(other);
}

Node2D::Node2D(Node2D&& other) noexcept : VirtualNode(std::move(other)), rigid(nullptr) {
    if (this == &other) return;
    // setRigid(std::move(other));
}

Node2D::~Node2D() {
    clear();
}

Node2D& Node2D::operator=(const Node2D& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(other);

    clear();
    // setRigid(other);

    return *this;
}

Node2D& Node2D::operator=(Node2D&& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(std::move(other));

    clear();
    // setRigid(std::move(other));

    return *this;    
}

/**
 * @brief Helper to update the model matrix when node is updated. 
 * 
 */
void Node2D::updateModel() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, -position.y, layer));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
}

void Node2D::setPosition(vec2 position) {
    this->position = position;
    updateModel();
}

void Node2D::setRotation(float rotation) {
    this->rotation = rotation;
    updateModel();
}

void Node2D::setScale(vec2 scale) {
    this->scale = scale;
    updateModel();
}

void Node2D::bindRigid(Params& params) {
    if (params.collider == nullptr) return;
    if (rigid) delete rigid;

    rigid = new Rigid(getScene()->getSolver(), this, { this->position, this->rotation }, scale, params.density, params.friction, params.velocity, params.collider);
}

void Node2D::clear() {
    if (rigid != nullptr) {
        delete rigid;
        rigid = nullptr;
    }
}

// -------------------
// used in copy constructors, rigids already have same stats as nodes
// -------------------
void Node2D::setRigid(const Node2D& other) {
    clear();
    if (other.rigid == nullptr) return;

    Solver* solver = other.rigid->getSolver();

    this->rigid = new Rigid(
        solver, 
        this, 
        { other.position, other.rotation }, 
        other.scale, other.rigid->getDensity(), 
        other.rigid->getFriction(), 
        other.rigid->getVel(), 
        other.rigid->getColliderIndex()
    );
}

void Node2D::setRigid(Node2D&& other) {
    clear();
    if (other.rigid == nullptr) return;

    rigid = other.rigid;
    this->rigid->setNode(this);
    other.rigid = nullptr;
}