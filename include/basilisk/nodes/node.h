#ifndef ENTITY_H
#define ENTITY_H

#include <basilisk/util/includes.h>
#include <basilisk/nodes/virtualNode.h>
#include <basilisk/scene/scene.h>

namespace bsk::internal {

class Node : public VirtualNode<Node, glm::vec3, glm::quat, glm::vec3> {
    private:
        using VirtualScene3D = VirtualScene<Node, glm::vec3, glm::quat, glm::vec3>;

    public:
        struct Params {
            Mesh* mesh = nullptr;
            Material* material = nullptr;
            glm::vec3 position = { 0, 0, 0 };
            glm::quat rotation = { 1, 0, 0, 0 };
            glm::vec3 scale = { 1, 1, 1 };
        };

        Node(VirtualScene3D* scene, Params params);
        Node(Node* parent, Params params);
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