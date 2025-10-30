#ifndef VIRTUAL_SCENE_H
#define VIRTUAL_SCENE_H

#include "render/shader.h"
#include "resource/resourceServer.h"
#include "engine/engine.h"

template<typename NodeType, typename position_type, typename rotation_type, typename scale_type>
class VirtualScene {
protected:
    NodeType* root = nullptr;
    
public:
    VirtualScene() {
        root = new NodeType(this, nullptr); // parent = nullptr
    }

    virtual ~VirtualScene() {
        delete root;
    }

    inline NodeType* getRoot() const { return root; }
    virtual Shader* getShader() = 0;
};

#endif