#ifndef BSK_NODE_H
#define BSK_NODE_H

#include <basilisk/util/includes.h>
#include <basilisk/nodes/virtualNode.h>
#include <basilisk/scene/scene.h>

namespace bsk::internal {

class Node : public VirtualNode<Node, glm::vec3, glm::quat, glm::vec3> {
    private:
        using VirtualScene3D = VirtualScene<Node, glm::vec3, glm::quat, glm::vec3>;

    public:
        Node(VirtualScene3D* scene, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale);
        Node(Node* parent, Mesh* mesh, Material* material, glm::vec3 position, glm::quat rotation, glm::vec3 scale);
        Node(VirtualScene3D* scene, Node* parent);

        // already defined in VirtualNode
        Node(const Node& other) noexcept = default;
        Node(Node&& other) noexcept = default;
        ~Node() = default;
        Node& operator=(const Node& other) noexcept = default;
        Node& operator=(Node&& other) noexcept = default;

        void setPosition(glm::vec3 position);
        void setRotation(glm::quat rotation);
        void setScale(glm::vec3 scale);

        Scene* getScene() { return (Scene*) scene; }

    private:
        void updateModel();
};

}

#endif