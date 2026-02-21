#ifndef BSK_NODE2D_H
#define BSK_NODE2D_H

#include <basilisk/util/includes.h>
#include <basilisk/nodes/virtualNode.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/scene/raycast.h>

namespace bsk::internal {

class Scene2D;  // Forward declaration
class Node2D;

struct CollisionData {
    Node2D* other = nullptr;
    glm::vec2 normal = glm::vec2(0.0f, 0.0f);
    float depth = 0.0f;
};

class Node2D : public VirtualNode<Node2D, glm::vec2, float, glm::vec2> {
private:
    using VirtualScene2D = VirtualScene<Node2D, glm::vec2, float, glm::vec2>;

    Rigid* rigid;
    float layer=0.0;

    // save data here so that we can adopt it when the node is adopted
    struct PhysicsData {
        Collider* collider = nullptr;
        float density = 1.0f;
        float friction = 0.5f;
        glm::vec3 velocity = glm::vec3(0.0f);
    } physicsData;

public:
    Node2D(VirtualScene2D* scene);
    Node2D(Scene2D* scene, Mesh* mesh = nullptr, Material* material = nullptr, glm::vec2 position = {0.0, 0.0}, float rotation = 0.0, glm::vec2 scale = {1.0, 1.0}, glm::vec3 velocity = {0.0, 0.0, 0.0}, Collider* collider = nullptr, float density = 1.0, float friction = 0.5);
    Node2D(Node2D* parent, Mesh* mesh = nullptr, Material* material = nullptr, glm::vec2 position = {0.0, 0.0}, float rotation = 0.0, glm::vec2 scale = {1.0, 1.0}, glm::vec3 velocity = {0.0, 0.0, 0.0}, Collider* collider = nullptr, float density = 1.0, float friction = 0.5);
    Node2D(Mesh* mesh = nullptr, Material* material = nullptr, glm::vec2 position = {0.0, 0.0}, float rotation = 0.0, glm::vec2 scale = {1.0, 1.0}, glm::vec3 velocity = {0.0, 0.0, 0.0}, Collider* collider = nullptr, float density = 1.0, float friction = 0.5);
    Node2D(const Node2D& other) noexcept;
    Node2D(Node2D&& other) noexcept;
    
    ~Node2D();
    
    Node2D& operator=(const Node2D& other) noexcept;
    Node2D& operator=(Node2D&& other) noexcept;

    void setPosition(glm::vec2 position) override;
    void setPosition(glm::vec3 position);
    void setRotation(float rotation) override;
    void setScale(glm::vec2 scale) override;
    void setVelocity(glm::vec3 velocity);
    void setLayer(float layer) { this->layer = layer; updateModel(); }
    void setCollider(Collider* collider);
    void setDensity(float density);
    void setFriction(float friction);
    // void setManifoldMask(float x, float y, float z) { rigid->setManifoldMask(x, y, z); }

    Scene2D* getScene() { return (Scene2D*) scene; }
    Rigid* getRigid() { return rigid; }
    glm::vec3 getVelocity();
    glm::vec2& getPositionRef();
    glm::vec2& getScaleRef();
    glm::vec3& getVelocityRef();
    float getLayer() { return layer; }
    float getDensity();
    float getFriction();
    Collider* getCollider();
    // glm::vec3 getManifoldMask() { return rigid->getManifoldMask(); }

    std::vector<CollisionData> getCollisions();

    void onSceneChange(VirtualScene2D* oldScene, VirtualScene2D* newScene);
    void add(Node2D* child);
    void remove(Node2D* child);

    // collision exposure
    ForceType constrainedTo(Node2D* other);
    bool justCollided(Node2D* other);
    bool isTouching(Node2D* other);

    // raycasting
    RayCastResult2D raycast(glm::vec2 origin, glm::vec2 direction);
    bool pointIsInside(glm::vec2 position);



private:
    void updateModel();
    void bindRigid(Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction);
    void clear();
    void setRigid(const Node2D& other);
    void setRigid(Node2D&& other);
};

}

#endif