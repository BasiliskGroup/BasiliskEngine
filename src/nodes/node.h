#ifndef ENTITY_H
#define ENTITY_H

#include "util/includes.h"
#include "nodes/virtualNode.h"
#include "scene/virtualScene.h"

class Node : public VirtualNode<Node, vec3, quat, vec3> {
    private:
        using Scene = VirtualScene<Node, vec3, quat, vec3>;

    public:
        struct Params {
            Mesh* mesh;
            Texture* texture;
            vec3 position = { 0, 0, 0 };
            quat rotation = { 1, 0, 0, 0 };
            vec3 scale = { 1, 1, 1 };
        };

        Node(Scene* scene, Params params);
        Node(Node* parent, Params params);
        Node(Scene* scene, Node* parent);

        // already defined in VirtualNode
        Node(const Node& other) noexcept = default;
        Node(Node&& other) noexcept = default;
        ~Node() = default;
        Node& operator=(const Node& other) noexcept = default;
        Node& operator=(Node&& other) noexcept = default;

        void setPosition(vec3 position);
        void setRotation(quat rotation);
        void setScale(vec3 scale);

    private:
        void updateModel();
};

#endif