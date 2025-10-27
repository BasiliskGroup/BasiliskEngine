#ifndef VIRTUAL_SCENE_H
#define VIRTUAL_SCENE_H

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

    NodeType* getRoot() const { return root; }
};

#endif