#ifndef MESH_H
#define MESH_H

#include "includes.h"
#include "buffer.h"


class Mesh {
    private:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        VBO* vbo;
        EBO* ebo;

    public:
        Mesh(const std::string modelPath);

        std::vector<Vertex> getVertices() { return vertices; }
        std::vector<unsigned int> getIndices() { return indices; }

        VBO* getVBO() { return vbo; }
        EBO* getEBO() { return ebo; }
};

#endif