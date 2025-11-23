#ifndef BSK_CAMERA_2D_H
#define BSK_CAMERA_2D_H

#include <basilisk/camera/staticCamera2d.h>

namespace bsk::internal {

class Camera2D : public StaticCamera2D {
    public:
        Camera2D(Engine* engine, glm::vec2 position = {0.0f, 0.0f}, float scale=10.0f) : StaticCamera2D(engine, position, scale) {}

        void update() override;
};

};

#endif