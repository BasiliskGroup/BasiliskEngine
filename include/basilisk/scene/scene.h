#ifndef SCENE_H
#define SCENE_H

#include <basilisk/util/includes.h>
#include <basilisk/engine/engine.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/scene/virtualScene.h>
#include <basilisk/camera/camera.h>
#include <basilisk/render/shader.h>

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