#ifndef CAMERA_H
#define CAMERA_H


#include <basilisk/util/includes.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/render/shader.h>
#include <basilisk/engine/engine.h>


class Camera : public VirtualCamera {
    private:
        Engine* engine;

        glm::vec3 position;
        glm::vec3 worldUp {0.0f, 1.0f, 0.0};
        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;
    
        float pitch;
        float yaw;

        float fov = 45.0f;
        float aspect = 1.0f;
        float near = 0.1f;
        float far = 100.0f;
        float sensitivity = 1.0;

        void updateProjection();
        void updateView();

    public:
        Camera(Engine* engine, glm::vec3 position = {0.0f, 0.0f, 0.0f}, float pitch = 0.0, float yaw = 0.0);

        void update();
        void use(Shader* shader);

        void moveSide(float distance);
        void moveForward(float distance);
        void moveUp(float distance);

        glm::vec3 getPosition() { return position; }
        float getX() { return position.x; }
        float getZ() { return position.y; }
        float getY() { return position.z; }
        float getYaw() { return yaw; }
        float getPitch() { return pitch; }

        void setX(float x) { position.x = x; updateView(); }
        void setY(float y) { position.y = y; updateView(); }
        void setZ(float z) { position.z = z; updateView(); }
        void setYaw(float yaw) { this->yaw = yaw; updateView(); }
        void setPitch(float pitch) { this->pitch = pitch; updateView(); }
        void setFOV(float fov) { this->fov = fov; updateProjection();}
        void setAspect(float aspect) { this->aspect = aspect; updateProjection();}
        void setNear(float near) { this->near = near; updateProjection();}
        void setFar(float far) { this->far = far; updateProjection();}
};

#endif