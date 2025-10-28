#include "scene/sceneRoute.h"

Node2D::Node2D(Scene2D* scene, Mesh* mesh, Texture* texture, vec2 position, float rotation, vec2 scale)
    : VirtualNode(scene, mesh, texture, position, rotation, scale) {
    updateModel();
}

Node2D::Node2D(Node2D* parent, Mesh* mesh, Texture* texture, vec2 position, float rotation, vec2 scale)
    : VirtualNode(parent, mesh, texture, position, rotation, scale) {
    updateModel();
}

Node2D::Node2D(Scene2D* scene, Node2D* parent) : VirtualNode(scene, parent) {}

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