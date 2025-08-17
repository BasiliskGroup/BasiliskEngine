#ifndef NODE_HANDLER_H
#define NODE_HANDLER_H

#include "includes.h"
#include "chunkHandler.h"
#include "node.h"
#include "camera.h"

class NodeHandler {
    private:
        std::vector<Node*> nodes;
        ChunkHandler* chunkHandler;
    
    public:
        NodeHandler();
        ~NodeHandler();

        void update(float deltaTime);
        void render(Camera* camera);
        
        void add(Node* node) { nodes.push_back(node); chunkHandler->add(node); }
};

#endif