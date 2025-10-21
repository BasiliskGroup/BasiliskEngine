#ifndef MESH_H
#define MESH_H

#include "includes.h"

class Mesh {
    private:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

    public:
        Mesh(const std::string modelPath);

        std::vector<Vertex>& getVertices() { return vertices; }
        std::vector<unsigned int>& getIndices() { return indices; }
};

#endif