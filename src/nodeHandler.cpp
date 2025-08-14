#include "nodeHandler.h"

void NodeHandler::update(float deltaTime) {
    for (Node* node : nodes) {
        node->update(deltaTime);
    }
}

void NodeHandler::render(Camera* camera) {
    for (Node* node : nodes) {
        camera->write(node->getVAO()->getShader());
        node->render();
    }
}