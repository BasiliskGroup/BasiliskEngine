#ifndef CAMERA_H
#define CAMERA_H

#include "includes.h"
#include "shader.h"
#include "engine.h"
#include "keys.h"
#include <algorithm>


class Camera {
    private:
        Engine* engine;

        float pitch, yaw;
        float fov;
        float speed, sensitivity;

        glm::vec3 position;
        glm::vec3 direction;

        glm::mat4 view;
        glm::mat4 projection;
    public:
        Camera(Engine* engine);
        void update();
        void write(Shader* shader);

        glm::vec3 getPosition() { return position; }
        glm::vec3 getDirection() { return direction; }

        void setPosition(glm::vec3 position) { this->position = position; }
        void setDirection(glm::vec3 direction) { this->direction = direction; }
        void setFOV(float fov);
        void updatePerspective();
};

#endif