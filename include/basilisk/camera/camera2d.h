#ifndef CAMERA_2D_H
#define CAMERA_2D_H


#include <basilisk/util/includes.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/render/shader.h>
#include <basilisk/engine/engine.h>


class Camera2D : public VirtualCamera{
    private:
        Engine* engine;

        glm::vec2 position;
        glm::vec2 viewScale;

        void updateProjection();
        void updateView();

    public:
        Camera2D(Engine* engine, glm::vec2 position = {0.0f, 0.0f});

        void update();
        void use(Shader* shader);

        void setPosition(glm::vec2 position) { this->position = position; }
};


#endif