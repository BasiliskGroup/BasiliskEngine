#ifndef BSK_CAMERA_H
#define BSK_CAMERA_H

#include <basilisk/camera/staticCamera.h>

namespace bsk::internal {

class Camera : public StaticCamera {
    private:
        float speed = 3.0f;  // Default movement speed
        void moveSide(float distance);
        void moveForward(float distance);
        void moveUp(float distance);

    public:
        Camera(Engine* engine, glm::vec3 position = {0.0f, 0.0f, 0.0f}, float pitch = 0.0, float yaw = 0.0) : StaticCamera(engine, position, pitch, yaw) {}

        void update() override;
        
        float getSpeed() const { return speed; }
        void setSpeed(float speed) { this->speed = speed; }
};

};

#endif