#include <basilisk/scene/sceneRoute.h>
#include <basilisk/util/maths.h>

namespace bsk::internal {

// Root node constructor (used by VirtualScene to create root node)
Node::Node(VirtualScene3D* scene)
    : VirtualNode(scene) {
    // Root node doesn't need material
    model = glm::mat4(1.0f);
}

Node::Node(Scene* scene, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(scene, mesh ? mesh : Engine::getResourceServer()->defaultCube, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale) {
    // Only update model and add material if this is not the root node (root node has parent == nullptr)
    if (parent == nullptr) { return; }
    
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

Node::Node(Node* parent, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(parent, mesh ? mesh : Engine::getResourceServer()->defaultCube, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale) {
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

Node::Node(Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : VirtualNode(mesh ? mesh : Engine::getResourceServer()->defaultCube, material ? material : Engine::getResourceServer()->defaultMaterial, position, rotation, scale) {
    updateModel();
    Engine::getResourceServer()->getMaterialServer()->add(getMaterial());
}

void Node::updateModel() {
    model = glm::translate(glm::mat4(1.0f), position)
          * glm::mat4_cast(rotation)
          * glm::scale(glm::mat4(1.0f), scale);

    // Apply the parent model to this node's model
    if (parent) {
        model = parent->getModel() * model;
    }
    
    for (auto child : children) {
        child->updateModel();
    }
}

void Node::setPosition(glm::vec3 position) {
    this->position = position;
    updateModel();
}

void Node::setRotation(glm::quat rotation) {
    this->rotation = rotation;
    updateModel();
}

void Node::setScale(glm::vec3 scale) {
    this->scale = scale;
    updateModel();
}

// raycasting - takes world-space ray, converts to model space for intersection test
RayCastResult Node::raycast(glm::vec3 worldOrigin, glm::vec3 worldDirection) {
    RayCastResult result;

    Mesh* m = getMesh();
    if (!m) return result;

    const auto& vertices = m->getVertices();
    const auto& indices = m->getIndices();
    if (indices.size() < 3) return result;

    // Convert world ray to model space
    glm::mat4 invModel = glm::inverse(getModel());
    glm::vec3 origin = glm::vec3(invModel * glm::vec4(worldOrigin, 1.0f));
    glm::vec3 direction = glm::normalize(glm::vec3(invModel * glm::vec4(worldDirection, 0.0f)));

    float closestT = result.distance;

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        glm::vec3 v0 = getMeshVertexPosition(vertices, indices, indices[i]);
        glm::vec3 v1 = getMeshVertexPosition(vertices, indices, indices[i + 1]);
        glm::vec3 v2 = getMeshVertexPosition(vertices, indices, indices[i + 2]);

        float t, u, v;
        if (!rayTriangleIntersect(origin, direction, v0, v1, v2, t, u, v))
            continue;

        if (t < closestT && t > 0.0f) {
            closestT = t;
            result.node = this;
            glm::vec3 modelIntersection = origin + direction * t;
            glm::vec3 modelNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            // Transform back to world space
            result.intersection = glm::vec3(getModel() * glm::vec4(modelIntersection, 1.0f));
            result.normal = glm::normalize(glm::vec3(glm::transpose(invModel) * glm::vec4(modelNormal, 0.0f)));
            result.distance = glm::length(result.intersection - worldOrigin);
        }
    }

    return result;
}

std::shared_ptr<Node> Node::orphanCopy() const {
    auto copy = std::make_shared<Node>(getMesh(), getMaterial(), getPosition(), getRotation(), getScale());
    auto meshShared = getMeshShared();
    if (meshShared) copy->setMesh(meshShared);
    auto materialShared = getMaterialShared();
    if (materialShared) copy->setMaterial(materialShared);
    return copy;
}

}