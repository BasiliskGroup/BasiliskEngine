#ifndef BSK_SCENE2D_H
#define BSK_SCENE2D_H

#include <basilisk/util/includes.h>
#include <basilisk/engine/engine.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/scene/virtualScene.h>
#include <basilisk/camera/camera2d.h>
#include <basilisk/render/shader.h>
#include <basilisk/physics/solver.h>

namespace bsk::internal {

class Node2D;

class Scene2D : public VirtualScene<Node2D, glm::vec2, float, glm::vec2> {
    private:
        StaticCamera2D* camera;
        StaticCamera2D* internalCamera;
        Shader* shader;
        Solver* solver;

    public:
        Scene2D(Engine* engine);
        ~Scene2D();

        void update();
        void render();

        void setCamera(StaticCamera2D* camera) { this->camera = camera; }

        inline Shader* getShader() { return shader; }
        inline StaticCamera2D* getCamera() { return camera; }
        inline Solver* getSolver() { return solver; }
};

}

#endif