#include "chunk.h"


Chunk::Chunk() {
    
}


void Chunk::render() {
    for (Node* node : nodes) {
        node->render();
    }
}


void Chunk::add(Node* node) {
    nodes.push_back(node);
}


void Chunk::batchAllNodes() {

}