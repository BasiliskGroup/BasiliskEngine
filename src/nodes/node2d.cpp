#include <basilisk/scene/sceneRoute.h>
#include <basilisk/physics/rigid.h>

namespace bsk::internal {

Node2D::Node2D(VirtualScene2D* scene, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(scene, mesh, material, position, rotation, scale), rigid(nullptr) {
    updateModel();
    bindRigid(mesh, material, position, rotation, scale, velocity, collider, density, friction);
    getScene()->getEngine()->getResourceServer()->getMaterialServer()->add(material);
}

Node2D::Node2D(Node2D* parent, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(parent, mesh, material, position, rotation, scale), rigid(nullptr) {
    updateModel();
    bindRigid(mesh, material, position, rotation, scale, velocity, collider, density, friction);
    getScene()->getEngine()->getResourceServer()->getMaterialServer()->add(material);
}

Node2D::Node2D(VirtualScene2D* scene, Node2D* parent) : VirtualNode(scene, parent), rigid(nullptr) {}

Node2D::Node2D(const Node2D& other) noexcept : VirtualNode(other), rigid(nullptr) {
    if (this == &other) return;
    setRigid(other);
}

Node2D::Node2D(Node2D&& other) noexcept : VirtualNode(std::move(other)), rigid(nullptr) {
    if (this == &other) return;
    setRigid(std::move(other));
}

Node2D::~Node2D() {
    clear();
}

Node2D& Node2D::operator=(const Node2D& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(other);

    clear();
    setRigid(other);

    return *this;
}

Node2D& Node2D::operator=(Node2D&& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(std::move(other));

    clear();
    setRigid(std::move(other));

    return *this;    
}

/**
 * @brief Helper to update the model matrix when node is updated. 
 * 
 */
void Node2D::updateModel() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, -position.y, layer));
    model = glm::rotate(model, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
}

void Node2D::setPosition(glm::vec2 position) {
    if (this->rigid) this->rigid->setPosition({position.x, position.y, this->rotation});
    this->position = position;
    updateModel();
}

void Node2D::setPosition(glm::vec3 position) {
    if (this->rigid) this->rigid->setPosition(position);
    this->position = {position.x , position.y};
    this->rotation = position.z;
    updateModel();
}

void Node2D::setRotation(float rotation) {
    if (this->rigid) this->rigid->setPosition({this->position.x, this->position.y, rotation});
    this->rotation = rotation;
    updateModel();
}

void Node2D::setScale(glm::vec2 scale) {
    if (this->rigid) this->rigid->setScale(scale);
    this->scale = scale;
    updateModel();
}

void Node2D::setVelocity(glm::vec3 velocity) {
    if (this->rigid) this->rigid->setVelocity(velocity);
}

void Node2D::bindRigid(Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction) {
    if (rigid) delete rigid;
    rigid = nullptr;

    if (collider != nullptr) {
        Scene2D* scene2d = static_cast<Scene2D*>(scene);
        rigid = new Rigid(scene2d->getSolver(), this, collider, { this->position, this->rotation }, this->scale, density, friction, velocity);
    }
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
        other.rigid->getCollider(),
        { other.position, other.rotation }, 
        other.scale, 
        other.rigid->getDensity(), 
        other.rigid->getFriction(), 
        other.rigid->getVel()
    );
}

void Node2D::setRigid(Node2D&& other) {
    clear();
    if (other.rigid == nullptr) return;

    rigid = other.rigid;
    this->rigid->setNode(this);
    other.rigid = nullptr;
}

ForceType Node2D::constrainedTo(Node2D* other){
    if (this->rigid == nullptr || other == nullptr || other->rigid == nullptr) {
        return NULL_FORCE;
    }

    return this->rigid->constrainedTo(other->rigid) ? NULL_FORCE : MANIFOLD;
}

bool Node2D::isTouching(Node2D* other){
    if (other == nullptr || other->rigid == nullptr) {
        return false;
    }

    return constrainedTo(other) == MANIFOLD;
}

glm::vec3 Node2D::getVelocity() {
    if (rigid == nullptr) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return rigid->getVelocity();
}

}