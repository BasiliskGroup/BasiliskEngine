#ifndef BSK_SCENE_H
#define BSK_SCENE_H

#include <basilisk/util/includes.h>
#include <basilisk/engine/engine.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/scene/virtualScene.h>
#include <basilisk/camera/camera.h>
#include <basilisk/render/shader.h>

namespace bsk::internal {

class Node;

class Scene : public VirtualScene<Node, glm::vec3, glm::quat, glm::vec3> {
    private:
        StaticCamera* camera;
        StaticCamera* internalCamera;
        Shader* shader;

    public:
        Scene(Engine* engine);
        Scene(Engine* engine, Shader* shader);
        ~Scene();

        void update();
        void render();

        void setCamera(StaticCamera* camera) { this->camera = camera; }

        inline Shader* getShader() { return shader; }
        inline StaticCamera* getCamera() { return camera; }
};

}

#endif