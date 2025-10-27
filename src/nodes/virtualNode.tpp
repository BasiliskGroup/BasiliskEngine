#include "scene/sceneRoute.h"

/**
 * @brief Construct a new Node object
 * 
 * @param shader 
 * @param mesh 
 * @param texture 
 * @param position 
 * @param rotation
 * @param scale 
 */
template<typename derived, typename position_type, typename rotation_type, typename scale_type>
VirtualNode<derived, position_type, rotation_type, scale_type>::VirtualNode(VirtualScene<NodeType, position_type, rotation_type, scale_type>* scene, Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale):
    scene(scene), parent(scene->getRoot()), shader(shader), mesh(mesh), texture(texture), position(position), rotation(rotation), scale(scale) {

    parent->children.push_back(asNode());

    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);
}

template<typename derived, typename position_type, typename rotation_type, typename scale_type>
VirtualNode<derived, position_type, rotation_type, scale_type>::VirtualNode(NodeType* parent, Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale):
    scene(parent->getScene()), parent(parent), shader(shader), mesh(mesh), texture(texture), position(position), rotation(rotation), scale(scale) {

    parent->children.push_back(asNode());

    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);
}

template<typename derived, typename position_type, typename rotation_type, typename scale_type>
VirtualNode<derived, position_type, rotation_type, scale_type>::VirtualNode(VirtualScene<NodeType, position_type, rotation_type, scale_type>* scene, NodeType* parent):
    scene(scene), parent(parent), shader(nullptr), mesh(nullptr), texture(nullptr), position(), rotation(), scale(), vbo(nullptr), ebo(nullptr), vao(nullptr) {

        if (parent != nullptr) {
            parent->children.push_back(asNode());
        }
    }

/**
 * @brief Destroy the Node object. Release the vbo, ebo, and vao
 * 
 */
template<typename derived, typename position_type, typename rotation_type, typename scale_type>
VirtualNode<derived, position_type, rotation_type, scale_type>::~VirtualNode() {
    if (vao) delete vao;
    if (vbo) delete vbo;
    if (ebo) delete ebo;

    // delete tree
    while (children.size() > 0) {
        NodeType* child = children.back();
        delete child;
    }

    if (parent != nullptr) {
        parent->remove(asNode());
    }
}

/**
 * @brief Render the vao on this node
 * 
 */
template<typename derived, typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<derived, position_type, rotation_type, scale_type>::render() {
    shader->bind("uTexture", texture, 0);
    shader->setUniform("uModel", model);
    vao->render();
}

/**
 * @brief 
 * 
 * @tparam position_type 
 * @tparam rotation_type 
 * @tparam scale_type 
 * @param child 
 */
template<typename derived, typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<derived, position_type, rotation_type, scale_type>::add(NodeType* child) {
    child->parent->remove(child);
    child->parent = this;
    children.push_back(child);
}

/**
 * @brief 
 * 
 * @tparam position_type 
 * @tparam rotation_type 
 * @tparam scale_type 
 * @param child 
 */
template<typename derived, typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<derived, position_type, rotation_type, scale_type>::remove(NodeType* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
    }
}