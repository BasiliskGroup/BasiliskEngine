#ifndef BSK_NODE2D_H
#define BSK_NODE2D_H

#include <basilisk/util/includes.h>
#include <basilisk/nodes/virtualNode.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/collision/collider.h>

namespace bsk::internal {

class Scene2D;  // Forward declaration

class Node2D : public VirtualNode<Node2D, glm::vec2, float, glm::vec2> {
private:
    using VirtualScene2D = VirtualScene<Node2D, glm::vec2, float, glm::vec2>;

    Rigid* rigid;
    float layer=0.0;

public:
    Node2D(VirtualScene2D* scene, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction);
    Node2D(Node2D* parent, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction);
    Node2D(VirtualScene2D* scene, Node2D* parent);
    Node2D(const Node2D& other) noexcept;
    Node2D(Node2D&& other) noexcept;
    
    ~Node2D();
    
    Node2D& operator=(const Node2D& other) noexcept;
    Node2D& operator=(Node2D&& other) noexcept;

    void setPosition(glm::vec2 position);
    void setPosition(glm::vec3 position);
    void setRotation(float rotation);
    void setScale(glm::vec2 scale);
    void setVelocity(glm::vec3 velocity);
    void setLayer(float layer) { this->layer = layer; updateModel(); }
    // void setManifoldMask(float x, float y, float z) { rigid->setManifoldMask(x, y, z); }

    Scene2D* getScene() { return (Scene2D*) scene; }
    // glm::vec3 getManifoldMask() { return rigid->getManifoldMask(); }
    Rigid* getRigid() { return rigid; }
    glm::vec3 getVelocity();
    // float getDensity() { return rigid != nullptr ? rigid->getDensity() : -1; }
    float getLayer() { return layer; }

    // collision exposure
    ForceType constrainedTo(Node2D* other);
    bool justCollided(Node2D* other);
    bool isTouching(Node2D* other);

private:
    void updateModel();
    void bindRigid(Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction);
    void clear();
    void setRigid(const Node2D& other);
    void setRigid(Node2D&& other);
};

}

#endif