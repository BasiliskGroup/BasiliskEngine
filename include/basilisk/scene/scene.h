#ifndef BSK_SCENE_H
#define BSK_SCENE_H

#include <basilisk/util/includes.h>
#include <memory>
#include <basilisk/engine/engine.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/scene/virtualScene.h>
#include <basilisk/camera/camera.h>
#include <basilisk/render/shader.h>
#include <basilisk/resource/lightServer.h>
#include <basilisk/light/light.h>
#include <basilisk/render/skybox.h>
#include <basilisk/scene/raycast.h>

namespace bsk::internal {

class Node;

class Scene : public VirtualScene<Node, glm::vec3, glm::quat, glm::vec3> {
    private:
        StaticCamera* camera;
        StaticCamera* internalCamera;
        std::shared_ptr<StaticCamera> cameraPython;
        Shader* shader;
        LightServer* lightServer;
        Skybox* skybox = nullptr;

    public:
        Scene(Engine* engine, bool addSkybox = true, bool addLight = true, bool addCube = false);
        Scene(Engine* engine, Shader* shader, bool addSkybox = true, bool addLight = true, bool addCube = false);
        ~Scene();

        void update();
        void render();
        
        void add(Light* light);
        void add(std::shared_ptr<Light> light);
        void addDefaults(bool addSkybox = true, bool addLight = true, bool addCube = true);
        // TODO add remove light

        void add(Node* node) override;
        void add(std::shared_ptr<Node> node) override;
        void remove(Node* node) override;
        void remove(std::shared_ptr<Node> node) override;

        inline void setCamera(StaticCamera* camera) { this->camera = camera; }
        inline void setCamera(std::shared_ptr<StaticCamera> cameraSp) { cameraPython = std::move(cameraSp); camera = cameraPython.get(); }
        inline void setSkybox(Skybox* skybox) { this->skybox = skybox; }
        
        inline Shader* getShader() override { return shader; }
        inline StaticCamera* getCamera() { return camera; }

        // mouse interaction
        RayCastResult pick(glm::vec2 mousePosition);
        RayCastResult raycast(glm::vec3 origin, glm::vec3 direction);
};

}

#endif