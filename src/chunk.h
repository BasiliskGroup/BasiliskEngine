#ifndef CHUNK_H
#define CHUNK_H

#include "includes.h"
#include "node.h"
#include "buffer.h"
#include "vao.h"


class Chunk {
    private:
        std::vector<Node*> nodes;
        std::vector<Vertex> batchData;

        

        void batchAllNodes();
    
    public:
        Chunk();

        void render();

        void add(Node* node);
        void remove(Node* node);
};

#endif
