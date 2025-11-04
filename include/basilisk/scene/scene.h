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
        Camera* camera;
        Shader* shader;

    public:
        Scene(Engine* engine);
        ~Scene();

        void update();
        void render();

        void setCamera(Camera* camera) { this->camera = camera; }

        inline Shader* getShader() { return shader; }
        inline Camera* getCamera() { return camera; }
};

}

#endif