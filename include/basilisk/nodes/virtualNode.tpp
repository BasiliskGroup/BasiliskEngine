#include <basilisk/scene/sceneRoute.h>

namespace bsk::internal {

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
 * @param material 
 * @param position 
 * @param rotation 
 * @param scale 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::VirtualNode(VirtualScene<Derived, P, R, S>* scene, Mesh* mesh, Material* material, P position, R rotation, S scale) : 
    scene(scene), 
    parent(scene->getRoot()), 
    shader(scene->getShader()), 
    mesh(mesh), 
    material(material), 
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
 * @param material 
 * @param position 
 * @param rotation 
 * @param scale 
 */
template<typename Derived, typename P, typename R, typename S>
VirtualNode<Derived, P, R, S>::VirtualNode(Derived* parent, Mesh* mesh, Material* material, P position, R rotation, S scale) : 
    scene(parent->getScene()), 
    parent(parent), 
    shader(parent->getShader()), 
    mesh(mesh), 
    material(material), 
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
    material(nullptr), 
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
      material(other.material),
      position(other.position),
      rotation(other.rotation),
      scale(other.scale),
      model(other.model),
      vbo(nullptr),
      ebo(nullptr),
      vao(nullptr)
{
    std::vector<Derived*> childrenCopy = other.children;
    
    // Deep copy children
    for (auto* child : childrenCopy) {
        auto clone = new Derived(*child);
        // Save parent pointer BEFORE any vector operations
        Derived* cloneParent = clone->parent;
        
        // clone's constructor added it to other.parent - remove it
        if (cloneParent && cloneParent != asNode()) {
            cloneParent->remove(clone);
        }
        clone->parent = asNode();
        children.push_back(clone);
    }

    createBuffers();
    
    // Add ourselves to parent
    if (parent) {
        parent->children.push_back(asNode());
    }
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
      material(other.material),
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

    if (parent) {
        parent->remove(other.asNode());
        parent->children.push_back(asNode());
    }

    // make other safe to destroy
    other.vbo = nullptr;
    other.ebo = nullptr;
    other.vao = nullptr;
    other.parent = nullptr;
    other.children.clear();
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
    shader = other.shader;
    mesh = other.mesh;
    material = other.material;
    position = other.position;
    rotation = other.rotation;
    scale = other.scale;
    model = other.model;

    // Make a copy of the children vector to avoid iterator invalidation
    std::vector<Derived*> childrenCopy = other.children;
    
    // copy children
    for (auto* child : childrenCopy) {
        auto clone = new Derived(*child);
        // Save parent pointer before any vector operations
        Derived* cloneParent = clone->parent;
        
        // Remove clone from wherever it was added
        if (cloneParent && cloneParent != asNode()) {
            cloneParent->remove(clone);
        }
        clone->parent = asNode();
        children.push_back(clone);
    }

    // initialize
    createBuffers();
    
    // Set parent and add to parent's children
    parent = other.parent;
    if (parent) {  
        parent->children.push_back(asNode());
    }

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
    material = other.material;
    position = std::move(other.position);
    rotation = std::move(other.rotation);
    scale = std::move(other.scale);
    model = std::move(other.model);

    // move children
    children = std::move(other.children);
    for (auto* child : children)
        child->parent = asNode();

    // initialize
    if (parent) {
        parent->remove(other.asNode());
        parent->children.push_back(asNode());
    }

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
    shader->setUniform("uModel", model);
    if (material) {
        int materialID = getEngine()->getResourceServer()->getMaterialServer()->get(material);
        shader->setUniform("uMaterialID", materialID);
    }
    else {
        shader->setUniform("uMaterialID", 0);
    }
    vao->render();
}

/**
 * @brief 
 * 
 * @tparam Derived 
 * @tparam P 
 * @tparam R 
 * @tparam S 
 * @return Engine* 
 */
template<typename Derived, typename P, typename R, typename S>
Engine* VirtualNode<Derived, P, R, S>::getEngine() {
    return scene->getEngine();
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
    child->parent = asNode();
    children.push_back(child);
}

/**
 * @brief Set the Material object
 * 
 * @param material 
 */
template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::setMaterial(Material* material) { 
    this->material = material; 
    getEngine()->getResourceServer()->getMaterialServer()->add(material); 
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
    auto toDelete = std::move(children);
    children.clear(); // vector is now empty before we delete

    // delete tree
    for (Derived* child : toDelete) {
        child->parent = nullptr;
        delete child;
    }

    // delete buffers
    deleteBuffers();

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
    if (mesh->getIndices().empty()) {
        vbo = new VBO(mesh->getVertices());
        ebo = nullptr;
        vao = new VAO(shader, vbo);
    } else {
        vbo = new VBO(mesh->getVertices());
        ebo = new EBO(mesh->getIndices());
        vao = new VAO(shader, vbo, ebo);
    }
}

template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::deleteBuffers() {
    if (vao) { delete vao; vao = nullptr; }
    if (vbo) { delete vbo; vbo = nullptr; }
    if (ebo) { delete ebo; ebo = nullptr; }
}

template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::setMesh(Mesh* mesh) {
    this->mesh = mesh;
    deleteBuffers();
    createBuffers();
}

template<typename Derived, typename P, typename R, typename S>
void VirtualNode<Derived, P, R, S>::setScene(VirtualScene<Derived, P, R, S>* scene) {
    this->scene = scene;
    scene->getRoot()->add(asNode());
    this->deleteBuffers();
    this->createBuffers();
}

}