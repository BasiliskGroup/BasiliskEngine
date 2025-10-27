#include "scene/sceneRoute.h"

/**
 * @brief Construct a new Virtual Node< Derived,  P,  R,  S>:: Virtual Node object
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param scene 
 * @param shader 
 * @param mesh 
 * @param texture 
 * @param position 
 * @param rotation 
 * @param scale 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::VirtualNode(VirtualScene<Derived, P, R, S>* scene, Shader* shader, Mesh* mesh, Texture* texture, P position, R rotation, S scale) : 
    scene(scene), 
    parent(scene->getRoot()), 
    shader(shader), mesh(mesh), 
    texture(texture), 
    position(position), 
    rotation(rotation), 
    scale(scale) 
{
    parent->children.push_back(asNode());
    createBuffers();
}

/**
 * @brief Construct a new Virtual Node< Derived,  P,  R,  S>:: Virtual Node object
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param parent 
 * @param shader 
 * @param mesh 
 * @param texture 
 * @param position 
 * @param rotation 
 * @param scale 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::VirtualNode(Derived* parent, Shader* shader, Mesh* mesh, Texture* texture, P position, R rotation, S scale) : 
    scene(parent->getScene()), 
    parent(parent), 
    shader(shader), 
    mesh(mesh), 
    texture(texture), 
    position(position), 
    rotation(rotation), 
    scale(scale) 
{
    parent->children.push_back(asNode());
    createBuffers();
}

/**
 * @brief Construct a new Virtual Node< Derived,  P,  R,  S>:: Virtual Node object
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param scene 
 * @param parent 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::VirtualNode(VirtualScene<Derived, P, R, S>* scene, Derived* parent) : 
    scene(scene), 
    parent(parent), 
    shader(nullptr), 
    mesh(nullptr), 
    texture(nullptr), 
    position(), // default
    rotation(), // default
    scale(), // default
    vbo(nullptr), 
    ebo(nullptr), 
    vao(nullptr) 
{
    if (parent != nullptr) {
        parent->children.push_back(asNode());
    }
}

/**
 * @brief Construct a new Virtual Node< Derived, P, R, S>:: Virtual Node object
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param other 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived,P,R,S>::VirtualNode(const VirtualNode& other) noexcept
    : scene(other.scene),
      parent(other.parent),
      shader(other.shader),
      mesh(other.mesh),
      texture(other.texture),
      position(other.position),
      rotation(other.rotation),
      scale(other.scale),
      model(other.model),
      vbo(nullptr),
      ebo(nullptr),
      vao(nullptr)
{
    // copy children
    for (auto* child : other.children) {
        auto clone = new Derived(*child);
        clone->parent = asNode();
        children.push_back(clone);
    }

    // initialize
    createBuffers();
    parent->children.push_back(asNode());
}

/**
 * @brief Construct a new Virtual Node< Derived,  P,  R,  S>:: Virtual Node object
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param other 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::VirtualNode(VirtualNode&& other) noexcept
    : scene(other.scene),
      parent(other.parent),
      shader(other.shader),
      mesh(other.mesh),
      texture(other.texture),
      vbo(other.vbo),
      ebo(other.ebo),
      vao(other.vao),
      position(std::move(other.position)),
      rotation(std::move(other.rotation)),
      scale(std::move(other.scale)),
      model(std::move(other.model)),
      children(std::move(other.children))
{
    // rebind children to new parent
    for (auto* child : children)
        child->parent = asNode();

    // make other safe to destroy
    other.vbo = nullptr;
    other.ebo = nullptr;
    other.vao = nullptr;
    other.parent = nullptr;
    other.children.clear();

    // initialize self
    parent->children.push_back(asNode());
}

/**
 * @brief Destroy the Virtual Node< Derived,  P,  R,  S>:: Virtual Node object
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::~VirtualNode() {
    clear();
}

/**
 * @brief Copies a VirtualNode and all of its children
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param other 
 * @return VirtualNode<Derived, P, R, S>& 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>& VirtualNode<Derived, P, R, S>::operator=(const VirtualNode& other) noexcept {
    if (this == &other) return *this;
    clear();

    // copy values
    scene = other.scene;
    parent = other.parent;
    shader = other.shader;
    mesh = other.mesh;
    texture = other.texture;
    position = other.position;
    rotation = other.rotation;
    scale = other.scale;
    model = other.model;

    // copy children
    for (auto* child : other.children) {
        auto clone = new Derived(*child);
        clone->parent = asNode();
        children.push_back(clone);
    }

    // initialize
    createBuffers();
    parent->children.push_back(asNode());

    return *this;
}

/**
 * @brief Moves a VirualNode to this node
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param other 
 * @return VirtualNode<Derived, P, R, S>& 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>& VirtualNode<Derived, P, R, S>::operator=(VirtualNode&& other) noexcept {
    if (this == &other) return *this;
    clear();
    
    // move values
    scene = other.scene;
    parent = other.parent;
    shader = other.shader;
    mesh = other.mesh;
    texture = other.texture;
    position = std::move(other.position);
    rotation = std::move(other.rotation);
    scale = std::move(other.scale);
    model = std::move(other.model);

    // move children
    children = std::move(other.children);
    for (auto* child : children)
        child->parent = asNode();

    // initialize
    parent->children.push_back(asNode());

    // unbind other so it doesn't delete our stuff
    other.vbo = nullptr;
    other.ebo = nullptr;
    other.vao = nullptr;
    other.children.clear();
    return *this;
}

/**
 * @brief Render the vao on this node
 * 
 */
template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::render() {
    shader->bind("uTexture", texture, 0);
    shader->setUniform("uModel", model);
    vao->render();
}

/**
 * @brief Safely adds a VirtualNode to this VirtualNode's subtree
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param child 
 */
template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::add(Derived* child) {
    child->parent->remove(child);
    child->parent = this;
    children.push_back(child);
}

/**
 * @brief Safely removes a Vurtual from the children subtree
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @param child 
 */
template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::remove(Derived* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
    }
}

/**
 * @brief Detaches the VirtualNode and deletes all children
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 */
template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::clear() {
    // delete tree
    for (auto* child : children) {
        delete child;
    }
    children.clear();

    // delete buffers
    if (vao) { delete vao; vao = nullptr; }
    if (vbo) { delete vbo; vbo = nullptr; }
    if (ebo) { delete ebo; ebo = nullptr; }

    // remove from parent
    if (parent != nullptr) {
        parent->remove(asNode());
        parent = nullptr;
    }
}

/**
 * @brief Creates the VBO, EBO, and VAO for the VirtualNode
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 */
template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::createBuffers() {
    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);
}