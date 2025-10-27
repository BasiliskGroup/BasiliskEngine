#ifndef ENTITY2D_H
#define ENTITY2D_H

#include "util/includes.h"
#include "nodes/virtualNode.h"
#include "scene/virtualScene.h"

class Node2D : public VirtualNode<Node2D, vec2, float, vec2> {
    private:
        using Scene2D = VirtualScene<Node2D, vec2, float, vec2>;

        float layer=0.0;

    public:
        Node2D(Scene2D* scene, Shader* shader, Mesh* mesh, Texture* texture, vec2 position={0, 0}, float rotation=0, vec2 scale={100, 100});
        Node2D(Node2D* parent, Shader* shader, Mesh* mesh, Texture* texture, vec2 position={0, 0}, float rotation=0, vec2 scale={100, 100});
        Node2D(Scene2D* scene, Node2D* parent);

        void setPosition(vec2 position);
        void setRotation(float rotation);
        void setScale(vec2 scale);

    private:
        void updateModel();
    
};

#endif