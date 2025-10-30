#ifndef SCENE2D_H
#define SCENE2D_H

#include "util/includes.h"
#include "engine/engine.h"
#include "camera/virtualCamera.h"
#include "scene/virtualScene.h"
#include "camera/camera2d.h"
#include "render/shader.h"
#include "solver/solver.h"

class Node2D;

class Scene2D : public VirtualScene<Node2D, vec2, float, vec2> {
    private:
        Engine* engine;
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

#endif