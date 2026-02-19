#include <basilisk/scene/sceneRoute.h>

namespace bsk::internal {

// Root node constructor (used by VirtualScene to create root node)
Node::Node(VirtualScene3D* scene)
    : VirtualNode(scene) {
    // Root node doesn't need material
    model = glm::mat4(1.0f);
}

Node::Node(Scene* scene, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(scene, mesh ? mesh : Engine::getResourceServer()->defaultCube, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale) {
    // Only update model and add material if this is not the root node (root node has parent == nullptr)
    if (parent == nullptr) { return; }
    
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

Node::Node(Node* parent, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(parent, mesh ? mesh : Engine::getResourceServer()->defaultCube, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale) {
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

Node::Node(Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(mesh ? mesh : Engine::getResourceServer()->defaultCube, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale) {
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

void Node::updateModel() {
    model = glm::translate(glm::mat4(1.0f), position)
          * glm::mat4_cast(rotation)
          * glm::scale(glm::mat4(1.0f), scale);

    // Apply the parent model to this node's model
    if (parent) {
        std::cout << "Parent pointer: " << parent << std::endl;
        model = parent->getModel() * model;
    }
          
    for (auto child : children) {
        child->updateModel();
    }
}

void Node::setPosition(glm::vec3 position) {
    this->position = position;
    updateModel();
}

void Node::setRotation(glm::quat rotation) {
    this->rotation = rotation;
    updateModel();
}

void Node::setScale(glm::vec3 scale) {
    this->scale = scale;
    updateModel();
}

}