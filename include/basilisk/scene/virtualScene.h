#ifndef BSK_VIRTUAL_SCENE_H
#define BSK_VIRTUAL_SCENE_H

#include <memory>
#include <basilisk/render/shader.h>
#include <basilisk/resource/resourceServer.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

template<typename NodeType, typename position_type, typename rotation_type, typename scale_type>
class VirtualScene {
protected:
    Engine* engine = nullptr;
    NodeType* root = nullptr;

    // store references to children as shared pointers so they are not destroyed when dereferenced from Python
    std::unordered_map<NodeType*, std::shared_ptr<NodeType>> childrenPythonMap;
    
public:
    VirtualScene(Engine* engine) : engine(engine) {
        root = new NodeType(this); // parent = nullptr
    }

    virtual ~VirtualScene() {
        clear();
    }

    inline Engine* getEngine() { return engine; }
    inline NodeType* getRoot() const { return root; }

    virtual void add(NodeType* node) = 0;
    virtual void add(std::shared_ptr<NodeType> node) = 0;
    virtual void remove(NodeType* node) = 0;
    virtual void remove(std::shared_ptr<NodeType> node) = 0;
    
    virtual Shader* getShader() = 0;

protected:
    void clear() {
        if (root != nullptr) {
            // Remove Python-owned nodes from the tree so root's destructor won't delete them.
            // They are owned by shared_ptrs in childrenPythonMap and will be destroyed when the map is destroyed.
            for (auto& [node_ptr, node_sp] : childrenPythonMap) {
                root->remove(node_ptr);
            }
            delete root;
            root = nullptr;
        }
    }
};

}

#endif