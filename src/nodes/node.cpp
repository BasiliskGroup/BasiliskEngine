#include "scene/sceneRoute.h"

Node::Node(Scene* scene, Params params)
    : VirtualNode(scene, params.mesh, params.material, params.position, params.rotation, params.scale) {
    updateModel();
}

Node::Node(Node* parent, Params params)
    : VirtualNode(parent, params.mesh, params.material, params.position, params.rotation, params.scale) {
    updateModel();
}

Node::Node(Scene* scene, Node* parent) : VirtualNode(scene, parent) {}

void Node::updateModel() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::scale(model, scale);
}

void Node::setPosition(vec3 position) {
    this->position = position;
    updateModel();
}

void Node::setRotation(quat rotation) {
    this->rotation = rotation;
    updateModel();
}

void Node::setScale(vec3 scale) {
    this->scale = scale;
    updateModel();
}