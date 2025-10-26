#ifndef SCENE_H
#define SCENE_H

#include "util/includes.h"
#include "engine/engine.h"
#include "camera/virtualCamera.h"
#include "nodes/nodeHandler.h"
#include "virtualNode.h"

class Scene {
    private:
        Engine* engine;
        VirtualCamera camera;
        NodeHandler nodeHandler;

    public:
        Scene(Engine* engine);
        ~Scene();

        void update();
        void render();

        void add(VirtualNode node);

        void setCamera(VirtualCamera camera) { this->camera = camera; }
        VirtualCamera getCamera() { return camera; }
};

#endif