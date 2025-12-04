#ifndef BSK_STATIC_CAMERA_H
#define BSK_STATIC_CAMERA_H


#include <basilisk/util/includes.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/render/shader.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

class StaticCamera : public VirtualCamera {
    protected:
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
        StaticCamera(Engine* engine, glm::vec3 position = {0.0f, 0.0f, 0.0f}, float pitch = 0.0, float yaw = 0.0);

        void update();
        void use(Shader* shader);

        glm::vec3 getPosition() { return position; }
        float getX() { return position.x; }
        float getY() { return position.y; }
        float getZ() { return position.z; }
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

}

#endif