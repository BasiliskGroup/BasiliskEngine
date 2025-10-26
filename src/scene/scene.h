#ifndef SCENE_H
#define SCENE_H

#include "util/includes.h"
#include "engine/engine.h"
#include "camera/virtualCamera.h"
#include "camera/camera.h"
#include "nodes/nodeHandler.h"
#include "nodes/node.h"

class Scene {
    private:
        Engine* engine;
        Camera* camera;
        NodeHandler* nodeHandler;
        Shader* shader;

    public:
        Scene(Engine* engine);
        ~Scene();

        void update();
        void render();

        void add(Node* node);

        void setCamera(Camera* camera) { this->camera = camera; }

        inline Shader* getShader() { return shader; }
        inline Camera* getCamera() { return camera; }
};

#endif