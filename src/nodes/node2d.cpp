#include <basilisk/scene/sceneRoute.h>
#include <basilisk/physics/rigid.h>

namespace bsk::internal {

Node2D::Node2D(VirtualScene2D* scene, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(scene, mesh, material, position, rotation, scale), rigid(nullptr) {
    updateModel();
    bindRigid(mesh, material, position, rotation, scale, velocity, collider, density, friction);
    Engine::getResourceServer()->getMaterialServer()->add(material);
}

Node2D::Node2D(Node2D* parent, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(parent, mesh, material, position, rotation, scale), rigid(nullptr) {
    updateModel();
    bindRigid(mesh, material, position, rotation, scale, velocity, collider, density, friction);
    Engine::getResourceServer()->getMaterialServer()->add(material);
}

Node2D::Node2D(VirtualScene2D* scene) : VirtualNode(scene), rigid(nullptr) {}

Node2D::Node2D(Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(mesh, material, position, rotation, scale), rigid(nullptr) {
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(material);

    // save data here so that we can adopt it when the node is adopted
    physicsData.collider = collider;
    physicsData.density = density;
    physicsData.friction = friction;
    physicsData.velocity = velocity;
}

Node2D::Node2D(const Node2D& other) noexcept : VirtualNode(other), rigid(nullptr) {
    if (this == &other) return;
    
    // Copy physics data (needed for orphaned nodes that will be adopted later)
    physicsData = other.physicsData;
    
    setRigid(other);
}

// Static flag to prevent rigid destruction during move operations
static thread_local bool g_inMoveOperation = false;

Node2D::Node2D(Node2D&& other) noexcept : VirtualNode([&other]() -> VirtualNode<Node2D, glm::vec2, float, glm::vec2>&& {
    // Set flag before base constructor runs
    g_inMoveOperation = true;
    return std::move(other);
}()), rigid(nullptr) {
    g_inMoveOperation = false;  // Clear flag
    
    if (this == &other) return;
    
    // Move physics data
    physicsData = std::move(other.physicsData);
    
    // Transfer rigid body ownership
    setRigid(std::move(other));
}

Node2D::~Node2D() {
    clear();
}

Node2D& Node2D::operator=(const Node2D& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(other);

    clear();
    
    // Copy physics data
    physicsData = other.physicsData;
    
    setRigid(other);

    return *this;
}

Node2D& Node2D::operator=(Node2D&& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(std::move(other));

    clear();
    
    // Move physics data
    physicsData = std::move(other.physicsData);
    
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

    // Always save data so it can be used when node is adopted into a scene
    physicsData.collider = collider;
    physicsData.density = density;
    physicsData.friction = friction;
    physicsData.velocity = velocity;

    if (scene == nullptr) return;

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

    // recursively clear rigids from children
    for (auto child : children) {
        child->clear();
    }
}

// -------------------------------------------------------------------
// used in copy constructors, rigids already have same stats as nodes
// -------------------------------------------------------------------
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

void Node2D::onSceneChange(VirtualScene2D* oldScene, VirtualScene2D* newScene) {
    // Save rigid state before destroying
    if (rigid != nullptr) {
        physicsData.velocity = rigid->getVelocity();
        delete rigid;
        rigid = nullptr;
    }

    // Recreate rigid if we're moving into a scene (not being orphaned)
    if (newScene != nullptr) {
        bindRigid(getMesh(), getMaterial(), getPosition(), getRotation(), getScale(), 
                  physicsData.velocity, physicsData.collider, physicsData.density, physicsData.friction);
    }

    // Recursively update all children
    for (auto child : children) {
        child->onSceneChange(oldScene, newScene);
    }
}

void Node2D::add(Node2D* child) {
    VirtualScene2D* oldScene = child->scene;
    
    // For same-scene reparenting, we need to preserve rigid bodies for the entire subtree
    // because VirtualNode::remove() calls orphanRecursive() which would destroy them all
    std::vector<std::pair<Node2D*, Rigid*>> savedRigids;
    
    if (oldScene != nullptr && oldScene == this->scene) {
        // Same-scene reparenting - save all rigid bodies in the subtree
        for (auto it = child->begin(); it != child->end(); ++it) {
            Node2D* node = *it;
            if (node->rigid != nullptr) {
                savedRigids.push_back({node, node->rigid});
                node->rigid = nullptr;  // Prevent destruction
            }
        }
    }
    
    VirtualNode::add(child);  // This may call remove() on old parent
    VirtualScene2D* newScene = child->scene;

    // Restore all rigids if this was same-scene reparenting
    if (!savedRigids.empty()) {
        for (auto& [node, rigid] : savedRigids) {
            node->rigid = rigid;
            node->rigid->setNode(node);  // Update the rigid's node pointer
        }
    }
    // Otherwise, handle scene change if one occurred
    else if (oldScene != newScene) {
        child->onSceneChange(oldScene, newScene);
    }
}

void Node2D::remove(Node2D* child) {
    VirtualScene2D* oldScene = child->scene;
    
    VirtualNode::remove(child);  // Orphans the child (sets scene to nullptr)

    // VirtualNode::remove always orphans (sets scene to nullptr)
    // Don't destroy rigid if this is part of a move operation
    if (oldScene != nullptr && !g_inMoveOperation) {
        child->onSceneChange(oldScene, nullptr);
    }
}

}