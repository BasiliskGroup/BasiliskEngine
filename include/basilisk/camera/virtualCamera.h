#ifndef BSK_VIRTUAL_CAMERA_H
#define BSK_VIRTUAL_CAMERA_H

#include <basilisk/util/includes.h>
#include <basilisk/render/shader.h>

namespace bsk::internal {

class VirtualCamera {
    protected:
        glm::mat4 view;
        glm::mat4 projection;

        virtual void updateProjection() = 0;
        virtual void updateView() = 0;

    public:
        virtual void update() = 0;
        virtual void use(Shader* shader) = 0;
};

}

#endif