#ifndef BSK_NODE2D_H
#define BSK_NODE2D_H

#include <basilisk/util/includes.h>
#include <basilisk/nodes/virtualNode.h>
#include <basilisk/shapes/collider.h>
#include <basilisk/shapes/rigid.h>
#include <basilisk/scene/scene2d.h>

namespace bsk::internal {

class Node2D : public VirtualNode<Node2D, glm::vec2, float, glm::vec2> {
private:
    using VirtualScene2D = VirtualScene<Node2D, glm::vec2, float, glm::vec2>;

    Rigid* rigid;
    float layer=0.0;
    glm::vec2 colliderScale;

public:
    struct Params {
        // graphics
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        glm::vec2 position = { 0, 0 };
        float rotation = 0;
        glm::vec2 scale = { 1, 1 };
        glm::vec2 colliderScale = { 1, 1 };
        
        // both?
        glm::vec3 velocity = { 0, 0, 0 };

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

    glm::vec3 getVelocity();

    void setPosition(glm::vec2 position);
    void setPosition(glm::vec3 position);
    void setRotation(float rotation);
    void setScale(glm::vec2 scale);
    void setVelocity(glm::vec3 velocity);
    void setLayer(float layer) { this->layer = layer; updateModel(); }
    void setManifoldMask(bool x, bool y, bool z) { rigid->setManifoldMask(x, y, z); }

    Scene2D* getScene() { return (Scene2D*) scene; }
    glm::bvec3 getManifoldMask() { return rigid->getManifoldMask(); }

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

}

#endif