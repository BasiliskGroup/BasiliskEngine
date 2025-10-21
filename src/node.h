#ifndef NODE_H
#define NODE_H

#include "includes.h"
#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "vbo.h"
#include "ebo.h"
#include "vao.h"
#include "mat.h"

class Node {
    private:
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        Matrix modelMatrix;

        Shader* shader;
        Mesh* mesh;
        Texture* texture;

        VBO* vbo;
        EBO* ebo;
        VAO* vao;

    public:
        Node(Shader* shader, Mesh* mesh, Texture* texture);
        ~Node();

        void update();
        void render();
};

#endif