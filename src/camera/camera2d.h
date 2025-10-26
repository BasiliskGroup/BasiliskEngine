#ifndef CAMERA_2D_H
#define CAMERA_2D_H


#include "util/includes.h"
#include "camera/virtualCamera.h"
#include "render/shader.h"
#include "engine/engine.h"


class Camera2D : public VirtualCamera{
    private:
        glm::vec2 position;

        void updateProjection();
        void updateView();

    public:
        Camera2D(glm::vec2 position={0.0f, 0.0f});

        void update(Engine* engine);
        void use(Shader* shader);

        void setPosition(glm::vec2 position) { this->position = position; }
};


#endif