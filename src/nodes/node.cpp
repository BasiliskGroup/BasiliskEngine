#include <basilisk/scene/sceneRoute.h>

namespace bsk::internal {

Node::Node(VirtualScene3D* scene, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(scene, mesh, material, position, rotation, scale) {
    updateModel();
    getScene()->getEngine()->getResourceServer()->getMaterialServer()->add(material);
}

Node::Node(Node* parent, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(parent, mesh, material, position, rotation, scale) {
    updateModel();
    getScene()->getEngine()->getResourceServer()->getMaterialServer()->add(material);
}

Node::Node(VirtualScene3D* scene, Node* parent) : VirtualNode(scene, parent) {}

void Node::updateModel() {
    model = glm::translate(glm::mat4(1.0f), position)
          * glm::mat4_cast(rotation)
          * glm::scale(glm::mat4(1.0f), scale);
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