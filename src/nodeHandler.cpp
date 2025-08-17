#include "nodeHandler.h"


NodeHandler::NodeHandler() {
    chunkHandler = new ChunkHandler();
}

NodeHandler::~NodeHandler() {
    //
    delete chunkHandler;
}

void NodeHandler::update(float deltaTime) {
    for (Node* node : nodes) {
        node->update(deltaTime);
    }
}

void NodeHandler::render(Camera* camera) {
    for (Node* node : nodes) {
        camera->write(node->getVAO()->getShader());
    }
    
    chunkHandler->render();
}

