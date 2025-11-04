#ifndef VIRTUAL_SCENE_H
#define VIRTUAL_SCENE_H

#include <basilisk/render/shader.h>
#include <basilisk/resource/resourceServer.h>
#include <basilisk/engine/engine.h>

template<typename NodeType, typename position_type, typename rotation_type, typename scale_type>
class VirtualScene {
protected:
    NodeType* root = nullptr;
    
public:
    VirtualScene() {
        root = new NodeType(this, nullptr); // parent = nullptr
    }

    virtual ~VirtualScene() {
        clear();
    }

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

#endif