#ifndef BSK_VIRTUAL_SCENE_H
#define BSK_VIRTUAL_SCENE_H

#include <basilisk/render/shader.h>
#include <basilisk/resource/resourceServer.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

template<typename NodeType, typename position_type, typename rotation_type, typename scale_type>
class VirtualScene {
protected:
    Engine* engine = nullptr;
    NodeType* root = nullptr;
    
public:
    VirtualScene(Engine* engine) : engine(engine) {
        root = new NodeType(this, nullptr); // parent = nullptr
    }

    virtual ~VirtualScene() {
        clear();
    }

    inline Engine* getEngine() { return engine; }
    inline NodeType* getRoot() const { return root; }
    
    virtual Shader* getShader() = 0;

protected:
    void clear() {
        if (root != nullptr) {
            delete root;
            root = nullptr;
        }
    }
};

}

#endif