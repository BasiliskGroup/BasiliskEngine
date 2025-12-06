#ifndef BSK_VIRTUAL_NODE_H
#define BSK_VIRTUAL_NODE_H

#include <basilisk/util/includes.h>
#include <basilisk/render/vbo.h>
#include <basilisk/render/ebo.h>
#include <basilisk/render/vao.h>
#include <basilisk/render/mesh.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/material.h>

namespace bsk::internal {

class Engine;

template<typename NodeType, typename P, typename R, typename S>
class VirtualScene;

template<typename Derived, typename P, typename R, typename S>
class VirtualNode {
private:
    Shader* shader;
    Mesh* mesh;
    Material* material;

protected:
    VirtualScene<Derived, P, R, S>* scene;
    Derived* parent;
    std::vector<Derived*> children;

    P position;
    R rotation;
    S scale;
    glm::mat4 model;

    VBO* vbo;
    EBO* ebo;
    VAO* vao;

public:
    class iterator {
    private:
        std::stack<Derived*> nodes;
    
    public:
        iterator(Derived* root) {
            if (root) nodes.push(root);
        }

        bool operator!=(const iterator& other) const {
            return nodes != other.nodes;
        }

        Derived* operator*() const {
            return nodes.top();
        }

        iterator& operator++() {
            Derived* current = nodes.top();
            nodes.pop();

            for (auto it = current->children.rbegin(); it != current->children.rend(); ++it) {
                nodes.push(*it);
            }
            return *this;
        }
    };

    // we don't want a default constructor, every node must be a part of a tree
    VirtualNode(VirtualScene<Derived, P, R, S>* scene, Derived* parent); // used to create root nodes
    VirtualNode(VirtualScene<Derived, P, R, S>* scene, Mesh* mesh, Material* material, P position, R rotation, S scale);
    VirtualNode(Derived* parent, Mesh* mesh, Material* material, P position, R rotation, S scale);
    VirtualNode(const VirtualNode& other) noexcept;
    VirtualNode(VirtualNode&& other) noexcept;
    virtual ~VirtualNode();

    VirtualNode& operator=(const VirtualNode& other) noexcept;
    VirtualNode& operator=(VirtualNode&& other) noexcept;

    void render();

    virtual void setPosition(P position) {};
    virtual void setRotation(R rotation) {};
    virtual void setScale(S scale) {};
    void setMesh(Mesh* mesh);
    void setScene(VirtualScene<Derived, P, R, S>* scene);
    void setMaterial(Material* material);

    P getPosition() const { return position; }
    R getRotation() const { return rotation; }
    S getScale() const { return scale; }
    VirtualScene<Derived, P, R, S>* getScene() const { return scene; }
    Derived* getParent() const { return parent; }
    Shader* getShader() { return shader; }
    Material* getMaterial() { return material; }
    Mesh* getMesh() { return mesh; }
    Engine* getEngine();

    // node hierarchy
    const std::vector<Derived*>& getChildren() { return children; }
    void add(Derived* child);
    void remove(Derived* child);

    iterator begin() { return iterator(asNode()); }
    iterator end() { return iterator(nullptr); }

private:
    Derived* asNode() { return static_cast<Derived*>(this); }

    // helper functions to avoid copying code
    void clear();
    void createBuffers();
    void deleteBuffers();
};

}

#include <basilisk/nodes/virtualNode.tpp>

#endif