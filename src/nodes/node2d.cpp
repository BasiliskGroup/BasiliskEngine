#include <basilisk/scene/sceneRoute.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/util/maths.h>

namespace {

// Extract vertex position from flat vertex buffer (position is first 3 floats per vertex).
glm::vec3 meshVertexPosition(const std::vector<float>& vertices,
                             const std::vector<unsigned int>& indices,
                             unsigned int vertexIndex) {
    if (vertices.empty() || indices.empty())
        return glm::vec3(0.0f);
    unsigned int maxIndex = 0;
    for (unsigned int idx : indices)
        if (idx > maxIndex) maxIndex = idx;
    unsigned int numVertices = maxIndex + 1;
    unsigned int stride = static_cast<unsigned int>(vertices.size()) / numVertices;
    if (stride < 3) stride = 3;
    unsigned int offset = vertexIndex * stride;
    return glm::vec3(vertices[offset], vertices[offset + 1], vertices[offset + 2]);
}

// Ray (origin + t*direction, t>=0) vs segment (a,b). Returns true if hit; t and hitPoint are set.
bool raySegmentIntersect2(const glm::vec2& origin, const glm::vec2& direction,
                          const glm::vec2& a, const glm::vec2& b,
                          float& t, glm::vec2& hitPoint) {
    glm::vec2 edge = b - a;
    glm::vec2 va = a - origin;
    float denom = direction.x * edge.y - direction.y * edge.x;
    const float eps = 1e-9f;
    if (std::abs(denom) < eps) return false;
    float inv = 1.0f / denom;
    t = (va.x * edge.y - va.y * edge.x) * inv;
    float s = (va.x * direction.y - va.y * direction.x) * inv;
    if (t < 0.0f || s < 0.0f || s > 1.0f) return false;
    hitPoint = origin + direction * t;
    return true;
}

}  // namespace

namespace bsk::internal {

Node2D::Node2D(Scene2D* scene, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(scene, mesh ? mesh : Engine::getResourceServer()->defaultQuad, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale), rigid(nullptr) {
    updateModel();
    bindRigid(mesh, material, position, rotation, scale, velocity, collider, density, friction);
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

Node2D::Node2D(Node2D* parent, Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(parent, mesh ? mesh : Engine::getResourceServer()->defaultQuad, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale), rigid(nullptr) {
    updateModel();
    bindRigid(mesh, material, position, rotation, scale, velocity, collider, density, friction);
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

Node2D::Node2D(VirtualScene2D* scene) : VirtualNode(scene), rigid(nullptr) {
    // Root node needs identity model so child hierarchy composition is valid.
    model = glm::mat4(1.0f);
}

Node2D::Node2D(Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction)
    : VirtualNode(mesh ? mesh : Engine::getResourceServer()->defaultQuad, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale), rigid(nullptr) {
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());

    // save data here so that we can adopt it when the node is adopted
    physicsData.collider = collider;
    physicsData.density = density;
    physicsData.friction = friction;
    physicsData.velocity = velocity;
}

Node2D::Node2D(const Node2D& other) noexcept : VirtualNode(other), rigid(nullptr) {
    if (this == &other) return;
    
    // Copy physics data (needed for orphaned nodes that will be adopted later)
    physicsData = other.physicsData;
    
    setRigid(other);
}

// Static flag to prevent rigid destruction during move operations
static thread_local bool g_inMoveOperation = false;

Node2D::Node2D(Node2D&& other) noexcept : VirtualNode([&other]() -> VirtualNode<Node2D, glm::vec2, float, glm::vec2>&& {
    // Set flag before base constructor runs
    g_inMoveOperation = true;
    return std::move(other);
}()), rigid(nullptr) {
    g_inMoveOperation = false;  // Clear flag
    
    if (this == &other) return;
    
    // Move physics data
    physicsData = std::move(other.physicsData);
    
    // Transfer rigid body ownership
    setRigid(std::move(other));
}

Node2D::~Node2D() {
    clear();
}

Node2D& Node2D::operator=(const Node2D& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(other);

    clear();
    
    // Copy physics data
    physicsData = other.physicsData;
    
    setRigid(other);

    return *this;
}

Node2D& Node2D::operator=(Node2D&& other) noexcept {
    if (this == &other) return *this;
    VirtualNode::operator=(std::move(other));

    clear();
    
    // Move physics data
    physicsData = std::move(other.physicsData);
    
    setRigid(std::move(other));

    return *this;    
}

/**
 * @brief Helper to update the model matrix when node is updated. 
 * 
 */
void Node2D::updateModel() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, -position.y, layer));
    model = glm::rotate(model, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));

    // Compose with parent transform so child nodes inherit hierarchy transforms.
    if (parent) {
        model = parent->getModel() * model;
    }

    for (auto child : children) {
        child->updateModel();
    }
}

void Node2D::setPosition(glm::vec2 position) {
    if (this->rigid) this->rigid->setPosition({position.x, position.y, this->rotation});
    this->position = position;
    updateModel();
}

void Node2D::setPosition(glm::vec3 position) {
    if (this->rigid) this->rigid->setPosition(position);
    this->position = {position.x , position.y};
    this->rotation = position.z;
    updateModel();
}

void Node2D::setRotation(float rotation) {
    if (this->rigid) this->rigid->setPosition({this->position.x, this->position.y, rotation});
    this->rotation = rotation;
    updateModel();
}

void Node2D::setScale(glm::vec2 scale) {
    if (this->rigid) this->rigid->setScale(glm::abs(scale));
    this->scale = scale;
    updateModel();
}

void Node2D::setResolvesCollisions(bool resolvesCollisions) {
    physicsData.resolvesCollisions = resolvesCollisions;
    if (this->rigid) this->rigid->setResolvesCollisions(resolvesCollisions);
}

void Node2D::setCollisionGroup(int group) {
    physicsData.collisionGroup = group;
    if (this->rigid) this->rigid->setCollisionGroup(group);
}

void Node2D::setVelocity(glm::vec3 velocity) {
    if (this->rigid) this->rigid->setVelocity(velocity);
}

void Node2D::bindRigid(Mesh* mesh, Material* material, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 velocity, Collider* collider, float density, float friction) {
    if (rigid) delete rigid;
    rigid = nullptr;

    // Always save data so it can be used when node is adopted into a scene
    physicsData.collider = collider;
    physicsData.density = density;
    physicsData.friction = friction;
    physicsData.velocity = velocity;

    if (scene == nullptr) return;

    if (collider != nullptr) {
        Scene2D* scene2d = static_cast<Scene2D*>(scene);
        rigid = new Rigid(scene2d->getSolver(), this, collider, { this->position, this->rotation }, this->scale, density, friction, velocity);
        rigid->setResolvesCollisions(physicsData.resolvesCollisions);
        rigid->setCollisionGroup(physicsData.collisionGroup);
    }
}

void Node2D::clear() {
    if (rigid != nullptr) {
        delete rigid;
        rigid = nullptr;
    }

    // recursively clear rigids from children
    for (auto child : children) {
        child->clear();
    }
}

// -------------------------------------------------------------------
// used in copy constructors, rigids already have same stats as nodes
// -------------------------------------------------------------------
void Node2D::setRigid(const Node2D& other) {
    clear();
    if (other.rigid == nullptr) return;

    Solver* solver = other.rigid->getSolver();

    this->rigid = new Rigid(
        solver, 
        this, 
        other.rigid->getCollider(),
        { other.position, other.rotation }, 
        other.scale, 
        other.rigid->getDensity(), 
        other.rigid->getFriction(), 
        other.rigid->getVel()
    );
    this->rigid->setResolvesCollisions(physicsData.resolvesCollisions);
    this->rigid->setCollisionGroup(physicsData.collisionGroup);
}

void Node2D::setRigid(Node2D&& other) {
    clear();
    if (other.rigid == nullptr) return;

    rigid = other.rigid;
    this->rigid->setNode(this);
    other.rigid = nullptr;
}

void Node2D::setCollider(Collider* collider) {
    if (this->rigid) this->rigid->setCollider(collider);
    physicsData.collider = collider;
}

void Node2D::setDensity(float density) {
    if (this->rigid) this->rigid->setDensity(density);
    physicsData.density = density;
}

void Node2D::setFriction(float friction) {
    if (this->rigid) this->rigid->setFriction(friction);
    physicsData.friction = friction;
}

float Node2D::getDensity() {
    assert(this->rigid != nullptr);
    return this->rigid->getDensity();
}

float Node2D::getFriction() {
    assert(this->rigid != nullptr);
    return this->rigid->getFriction();
}

Collider* Node2D::getCollider() {
    assert(this->rigid != nullptr);
    return this->rigid->getCollider();
}

ForceType Node2D::constrainedTo(Node2D* other){
    if (this->rigid == nullptr || other == nullptr || other->rigid == nullptr) {
        return NULL_FORCE;
    }

    return this->rigid->constrainedTo(other->rigid) ? NULL_FORCE : MANIFOLD;
}

bool Node2D::isTouching(Node2D* other){
    if (other == nullptr || other->rigid == nullptr) {
        return false;
    }

    return constrainedTo(other) == MANIFOLD;
}

bool Node2D::justCollided(Node2D* other) {
    // True if we are currently in contact with other. Per-frame "contact just began"
    // would require solver contact history; for now matches isTouching.
    return isTouching(other);
}

glm::vec3 Node2D::getVelocity() {
    if (rigid == nullptr) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return rigid->getVelocity();
}

glm::vec2& Node2D::getPositionRef() {
    return this->position;
}

glm::vec2& Node2D::getScaleRef() {
    return this->scale;
}

glm::vec3& Node2D::getVelocityRef() {
    if (rigid != nullptr) {
        return rigid->getVelocityRef();
    }
    return physicsData.velocity;
}

void Node2D::onSceneChange(VirtualScene2D* oldScene, VirtualScene2D* newScene) {
    // Save rigid state before destroying
    if (rigid != nullptr) {
        physicsData.velocity = rigid->getVelocity();
        delete rigid;
        rigid = nullptr;
    }

    // Recreate rigid if we're moving into a scene (not being orphaned)
    if (newScene != nullptr) {
        bindRigid(getMesh(), getMaterial(), getPosition(), getRotation(), getScale(), 
                  physicsData.velocity, physicsData.collider, physicsData.density, physicsData.friction);
    }

    // Recursively update all children
    for (auto child : children) {
        child->onSceneChange(oldScene, newScene);
    }
}

void Node2D::add(Node2D* child) {
    VirtualScene2D* oldScene = child->scene;
    
    // For same-scene reparenting, we need to preserve rigid bodies for the entire subtree
    // because VirtualNode::remove() calls orphanRecursive() which would destroy them all
    std::vector<std::pair<Node2D*, Rigid*>> savedRigids;
    
    if (oldScene != nullptr && oldScene == this->scene) {
        // Same-scene reparenting - save all rigid bodies in the subtree
        for (auto it = child->begin(); it != child->end(); ++it) {
            Node2D* node = *it;
            if (node->rigid != nullptr) {
                savedRigids.push_back({node, node->rigid});
                node->rigid = nullptr;  // Prevent destruction
            }
        }
    }
    
    VirtualNode::add(child);  // This may call remove() on old parent
    VirtualScene2D* newScene = child->scene;

    // Restore all rigids if this was same-scene reparenting
    if (!savedRigids.empty()) {
        for (auto& [node, rigid] : savedRigids) {
            node->rigid = rigid;
            node->rigid->setNode(node);  // Update the rigid's node pointer
        }
    }
    // Otherwise, handle scene change if one occurred
    else if (oldScene != newScene) {
        child->onSceneChange(oldScene, newScene);
    }
}

void Node2D::remove(Node2D* child) {
    VirtualScene2D* oldScene = child->scene;
    
    VirtualNode::remove(child);  // Orphans the child (sets scene to nullptr)

    // VirtualNode::remove always orphans (sets scene to nullptr)
    // Don't destroy rigid if this is part of a move operation
    if (oldScene != nullptr && !g_inMoveOperation) {
        child->onSceneChange(oldScene, nullptr);
    }
}

bool Node2D::pointIsInside(glm::vec2 position) {
    Mesh* m = getMesh();
    if (!m) return false;

    const auto& vertices = m->getVertices();
    const auto& indices = m->getIndices();
    if (indices.size() < 3) return false;

    // Convert world position to model space
    glm::vec4 modelPos = glm::inverse(getModel()) * glm::vec4(position.x, -position.y, 0.0f, 1.0f);
    glm::vec2 modelPoint(modelPos.x, -modelPos.y);

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        glm::vec3 v0 = meshVertexPosition(vertices, indices, indices[i]);
        glm::vec3 v1 = meshVertexPosition(vertices, indices, indices[i + 1]);
        glm::vec3 v2 = meshVertexPosition(vertices, indices, indices[i + 2]);

        glm::vec2 a(v0.x, v0.y);
        glm::vec2 b(v1.x, v1.y);
        glm::vec2 c(v2.x, v2.y);

        if (pointInTriangle2(modelPoint, a, b, c))
            return true;
    }
    return false;
}

RayCastResult2D Node2D::raycast(glm::vec2 origin, glm::vec2 direction) {
    RayCastResult2D result;

    Mesh* m = getMesh();
    if (!m) return result;

    const auto& vertices = m->getVertices();
    const auto& indices = m->getIndices();
    if (indices.size() < 3) return result;

    glm::vec2 dirNorm = glm::normalize(direction);

    // Convert to model space
    glm::mat4 invModel = glm::inverse(getModel());
    glm::vec2 originModel = glm::vec2(invModel * glm::vec4(origin.x, -origin.y, 0.0f, 1.0f));
    originModel.y = -originModel.y;
    glm::vec2 dirModel = glm::vec2(invModel * glm::vec4(dirNorm.x, -dirNorm.y, 0.0f, 0.0f));
    dirModel.y = -dirModel.y;
    dirModel = glm::normalize(dirModel);

    float closestT = result.distance;

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        glm::vec2 a = xy(meshVertexPosition(vertices, indices, indices[i]));
        glm::vec2 b = xy(meshVertexPosition(vertices, indices, indices[i + 1]));
        glm::vec2 c = xy(meshVertexPosition(vertices, indices, indices[i + 2]));

        glm::vec2 edges[3][2] = {{a, b}, {b, c}, {c, a}};
        for (int e = 0; e < 3; ++e) {
            float t;
            glm::vec2 hitModel;
            if (!raySegmentIntersect2(originModel, dirModel, edges[e][0], edges[e][1], t, hitModel))
                continue;
            if (t < closestT && t > 1e-9f) {
                closestT = t;
                glm::vec2 edgeDirModel = edges[e][1] - edges[e][0];
                glm::vec2 normalModel = glm::normalize(glm::vec2(-edgeDirModel.y, edgeDirModel.x));

                glm::vec4 hitWorld4 = getModel() * glm::vec4(hitModel.x, -hitModel.y, 0.0f, 1.0f);
                glm::vec2 hitWorld(hitWorld4.x, -hitWorld4.y);
                glm::vec4 normalWorld4 = getModel() * glm::vec4(normalModel.x, -normalModel.y, 0.0f, 0.0f);
                glm::vec2 normalWorld = glm::normalize(glm::vec2(normalWorld4.x, -normalWorld4.y));

                // Ensure normal points toward the ray origin.
                if (glm::dot(normalWorld, origin - hitWorld) < 0.0f) {
                    normalWorld = -normalWorld;
                }

                result.node = this;
                result.intersection = hitWorld;
                result.normal = normalWorld;
                result.distance = glm::length(hitWorld - origin);
            }
        }
    }

    return result;
}

std::vector<CollisionData> Node2D::getCollisions() {
    std::vector<CollisionData> collisions;
    if (rigid == nullptr) return collisions;
    return rigid->getCollisions();
}

void Node2D::setJacobianMask(const glm::vec3& jacobianMask) {
    if (rigid) rigid->setJacobianMask(jacobianMask);
}

glm::vec3 Node2D::getJacobianMask() {
    if (rigid) return rigid->getJacobianMask();
    return glm::vec3(1.0f, 1.0f, 1.0f);
}

std::shared_ptr<Node2D> Node2D::orphanCopy() const {
    glm::vec3 vel = (rigid != nullptr) ? rigid->getVelocity() : physicsData.velocity;
    auto copy = std::make_shared<Node2D>(
        getMesh(), getMaterial(),
        getPosition(), getRotation(), getScale(),
        vel, physicsData.collider, physicsData.density, physicsData.friction
    );
    auto meshShared = getMeshShared();
    if (meshShared) copy->setMesh(meshShared);
    auto materialShared = getMaterialShared();
    if (materialShared) copy->setMaterial(materialShared);
    copy->setLayer(layer);
    copy->setResolvesCollisions(physicsData.resolvesCollisions);
    copy->setCollisionGroup(physicsData.collisionGroup);
    return copy;
}

}