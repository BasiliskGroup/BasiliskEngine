#ifndef SCENE_H
#define SCENE_H

#include "util/includes.h"
#include "engine/engine.h"
#include "camera/virtualCamera.h"
#include "scene/virtualScene.h"
#include "camera/camera.h"
#include "render/shader.h"

class Node;

class Scene : public VirtualScene<Node, vec3, quat, vec3> {
    private:
        Camera* camera;
        Shader* shader;
        Engine* engine;

    public:
        Scene(Engine* engine);
        ~Scene();

        void update();
        void render();

        void setCamera(Camera* camera) { this->camera = camera; }

        inline Shader* getShader() { return shader; }
        inline Camera* getCamera() { return camera; }
        inline Engine* getEngine() { return engine; }
};

#endif