#ifndef BSK_NODE_H
#define BSK_NODE_H

#include <basilisk/util/includes.h>
#include <basilisk/nodes/virtualNode.h>
#include <basilisk/scene/scene.h>
#include <basilisk/scene/raycast.h>

namespace bsk::internal {

class Node : public VirtualNode<Node, glm::vec3, glm::quat, glm::vec3> {
    private:
        using VirtualScene3D = VirtualScene<Node, glm::vec3, glm::quat, glm::vec3>;

    public:
        Node(VirtualScene3D* scene);
        Node(Scene* scene, Mesh* mesh = nullptr, Material* material = nullptr, glm::vec3 position = {0.0, 0.0, 0.0}, glm::quat rotation = {1.0, 0.0, 0.0, 0.0}, glm::vec3 scale = {1.0, 1.0, 1.0});
        Node(Node* parent, Mesh* mesh = nullptr, Material* material = nullptr, glm::vec3 position = {0.0, 0.0, 0.0}, glm::quat rotation = {1.0, 0.0, 0.0, 0.0}, glm::vec3 scale = {1.0, 1.0, 1.0});
        Node(Mesh* mesh = nullptr, Material* material = nullptr, glm::vec3 position = {0.0, 0.0, 0.0}, glm::quat rotation = {1.0, 0.0, 0.0, 0.0}, glm::vec3 scale = {1.0, 1.0, 1.0});

        // already defined in VirtualNode
        Node(const Node& other) noexcept = default;
        Node(Node&& other) noexcept = default;
        ~Node() = default;
        Node& operator=(const Node& other) noexcept = default;
        Node& operator=(Node&& other) noexcept = default;

        void setPosition(glm::vec3 position) override;
        void setRotation(glm::quat rotation) override;
        void setScale(glm::vec3 scale) override;

        Scene* getScene() { return (Scene*) scene; }
        glm::mat4 getModel() { return model; }

        // raycasting
        RayCastResult raycast(glm::vec3 origin, glm::vec3 direction);

        /** Returns an orphaned copy of this node. */
        std::shared_ptr<Node> orphanCopy() const;

    private:
        void updateModel();
};

}

#endif