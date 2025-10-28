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
    struct Params {
        Mesh* mesh;
        Texture* texture;
        vec2 position = { 0, 0 };
        float rotation = 0;
        vec2 scale = { 1, 1 };
    };

    Node2D(Scene2D* scene, Params params);
    Node2D(Node2D* parent, Params params);
    Node2D(Scene2D* scene, Node2D* parent);

    // already defined in VirtualNode
    Node2D(const Node2D& other) noexcept = default;
    Node2D(Node2D&& other) noexcept = default;
    ~Node2D() = default;
    Node2D& operator=(const Node2D& other) noexcept = default;
    Node2D& operator=(Node2D&& other) noexcept = default;

    void setPosition(vec2 position);
    void setRotation(float rotation);
    void setScale(vec2 scale);

private:
    void updateModel();
    
};

#endif