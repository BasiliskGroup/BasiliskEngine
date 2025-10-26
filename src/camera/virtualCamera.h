#ifndef VIRTUAL_CAMERA_H
#define VIRTUAL_CAMERA_H

#include "util/includes.h"
#include "engine/engine.h"
#include "render/shader.h"

class VirtualCamera {
    protected:
        glm::mat4 view;
        glm::mat4 projection;

        virtual void updateProjection() = 0;
        virtual void updateView() = 0;

    public:
        virtual void update(Engine* engine) = 0;
        virtual void use(Shader* shader) = 0;
};

#endif