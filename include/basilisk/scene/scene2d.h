#ifndef BSK_SCENE2D_H
#define BSK_SCENE2D_H

#include <basilisk/util/includes.h>
#include <basilisk/engine/engine.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/scene/virtualScene.h>
#include <basilisk/camera/camera2d.h>
#include <basilisk/render/shader.h>
#include <basilisk/solver/solver.h>

namespace bsk::internal {

class Node2D;

class Scene2D : public VirtualScene<Node2D, glm::vec2, float, glm::vec2> {
    private:
        Camera2D* camera;
        Shader* shader;
        Solver* solver;

    public:
        Scene2D(Engine* engine);
        ~Scene2D();

        void update(float dt);
        void render();

        void setCamera(Camera2D* camera) { this->camera = camera; }

        inline Shader* getShader() { return shader; }
        inline Camera2D* getCamera() { return camera; }
        inline Solver* getSolver() { return solver; }
};

}

#endif