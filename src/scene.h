#ifndef SCENE_H
#define SCENE_H

#include "includes.h"
#include "engine.h"
#include "camera.h"
#include "nodeHandler.h"    
#include "node.h"

class Scene {
    private:
        Engine* engine;
        Camera camera;
        NodeHandler nodeHanlder;
    public:
        Scene(Engine* engine);

        void update();
        void render();

        void add(Node* node) { nodeHanlder.add(node); }
};

#endif