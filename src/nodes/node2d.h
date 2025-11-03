#ifndef ENTITY2D_H
#define ENTITY2D_H

#include "util/includes.h"
#include "nodes/virtualNode.h"
#include "shapes/collider.h"
#include "shapes/rigid.h"
#include "scene/scene2d.h"

class Node2D : public VirtualNode<Node2D, vec2, float, vec2> {
private:
    using VirtualScene2D = VirtualScene<Node2D, vec2, float, vec2>;

    Rigid* rigid;
    float layer=0.0;

public:
    struct Params {
        // graphics
        Mesh* mesh = nullptr;
        Texture* texture = nullptr;
        vec2 position = { 0, 0 };
        float rotation = 0;
        vec2 scale = { 1, 1 };
        
        // both?
        vec3 velocity = { 0, 0, 0 };

        // physics
        Collider* collider = nullptr;
        float density = 1;
        float friction = 0.4;
    };

    Node2D(VirtualScene2D* scene, Params params);
    Node2D(Node2D* parent, Params params);
    Node2D(VirtualScene2D* scene, Node2D* parent);
    Node2D(const Node2D& other) noexcept;
    Node2D(Node2D&& other) noexcept;
    
    ~Node2D();
    
    Node2D& operator=(const Node2D& other) noexcept;
    Node2D& operator=(Node2D&& other) noexcept;

    void setPosition(vec2 position);
    void setPosition(vec3 position);
    void setRotation(float rotation);
    void setScale(vec2 scale);
    void setVelocity(vec3 velocity);

    Scene2D* getScene() { return (Scene2D*) scene; }

    // collision exposure
    ForceType constrainedTo(Node2D* other);
    bool justCollided(Node2D* other);
    bool isTouching(Node2D* other);

private:
    void updateModel();
    void bindRigid(Params& params);
    void clear();
    void setRigid(const Node2D& other);
    void setRigid(Node2D&& other);
    
};

#endif