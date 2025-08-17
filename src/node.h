#ifndef NODE_H
#define NODE_H

#include "includes.h"
#include "mesh.h"
#include "buffer.h"
#include "vao.h"


class Node {        
    static unsigned int maxID;
    static std::vector<unsigned int> freeIDs;

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

        // Batching Information
        unsigned int id;

    public:
        Node(Shader* shader, Mesh* mesh, Texture* texture, vec3 position = vec3(0, 0, 0), quat rotation = quat(1, 0, 0, 0), vec3 scale = vec3(1, 1, 1));
        ~Node() { freeIDs.push_back(id); }
        
        void render();
        void update(float deltaTime);

        inline vec3& getPosition() { return position; }
        inline quat& getRotation() { return rotation; }
        inline vec3& getScale()    { return scale; }

        inline void setPosition(vec3 position) { this->position = position; }
        inline void setRotation(quat rotation) { this->rotation = rotation; }
        inline void setScale(vec3 scale)       { this->scale = scale; }

        inline VAO* getVAO() { return &vao; }
};


#endif