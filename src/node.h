#ifndef NODE_H
#define NODE_H

#include "includes.h"
#include "mesh.h"
#include "vbo.h"
#include "ebo.h"
#include "vao.h"


class Node {        
    private:
        // Transformations
        vec3 position;
        quat rotation;
        vec3 scale;
        mat4x4 model;

        // Rendering
        Mesh* mesh;
        Texture* texture;
        VAO vao;

        // Physics
        vec6 veclocity;

    public:
        Node(Shader* shader, Mesh* mesh, Texture* texture, vec3 position = vec3(0, 0, 0), quat rotation = quat(1, 0, 0, 0), vec3 scale = vec3(1, 1, 1));
        void render();
        void update(float deltaTime);

        void setPosition(vec3 position) { this->position = position; }
        void setRotation(quat rotation) { this->rotation = rotation; }
        void setScale(vec3 scale)       { this->scale = scale; }

        VAO* getVAO() { return &vao; }
};


#endif