#ifndef VIRTUAL_ENTITY_H
#define VIRTUAL_ENTITY_H

#include "util/includes.h"
#include "render/vbo.h"
#include "render/ebo.h"
#include "render/vao.h"
#include "render/mesh.h"
#include "render/shader.h"

template<typename node_type, typename position_type, typename rotation_type, typename scale_type>
class VirtualScene;

template<typename derived, typename position_type, typename rotation_type, typename scale_type>
class VirtualNode {
public: 
    using NodeType = derived;

protected:
    VirtualScene<NodeType, position_type, rotation_type, scale_type>* scene;
    NodeType* parent;
    std::vector<NodeType*> children;

private:
    Shader* shader;
    Mesh* mesh;
    Texture* texture;
    
    VBO* vbo;
    EBO* ebo;
    VAO* vao;

public:
    class iterator {
    private:
        std::stack<NodeType*> nodes;
    
    public:
        iterator(NodeType* root) {
            if (root) nodes.push(root);
        }

        bool operator!=(const iterator& other) const {
            return nodes != other.nodes;
        }

        NodeType* operator*() const {
            return nodes.top();
        }

        iterator& operator++() {
            NodeType* current = nodes.top();
            nodes.pop();

            for (auto it = current->children.rbegin(); it != current->children.rend(); ++it) {
                nodes.push(*it);
            }
            return *this;
        }
    };
    
protected:
    position_type position;
    rotation_type rotation;
    scale_type scale;
    glm::mat4 model;

    NodeType* asNode() { return static_cast<NodeType*>(this); }
    
public:
    VirtualNode(VirtualScene<NodeType, position_type, rotation_type, scale_type>* scene, NodeType* parent); // used to create root nodes
    VirtualNode(VirtualScene<NodeType, position_type, rotation_type, scale_type>* scene, Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale);
    VirtualNode(NodeType* parent, Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale);
    ~VirtualNode();

    void render();

    virtual void setPosition(position_type position) {};
    virtual void setRotation(rotation_type rotation) {};
    virtual void setScale(scale_type scale) {};

    position_type getPosition() const { return position; }
    rotation_type getRotation() const { return rotation; }
    scale_type getScale() const { return scale; }
    VirtualScene<NodeType, position_type, rotation_type, scale_type>* getScene() const { return scene; }
    NodeType* getParent() const { return parent; }

    // node hierarchy
    const std::vector<NodeType*>& getChildren() { return children; }
    void add(NodeType* child);
    void remove(NodeType* child);

    iterator begin() { return iterator(asNode()); }
    iterator end() { return iterator(nullptr); }
};

#include "nodes/virtualNode.tpp"

#endif