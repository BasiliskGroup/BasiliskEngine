#include <basilisk/camera/camera.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

/**
 * @brief 
 * 
 */
void Camera::update() {

    // Get mouse and keyboard from engine
    Mouse* mouse = engine->getMouse();
    Keyboard* keys = engine->getKeyboard();

    // Looking
    float yOffset = mouse->getRelativeY() * sensitivity / 5;
    float xOffset = mouse->getRelativeX() * sensitivity / 5;

    yaw += xOffset;
    pitch -= yOffset;
    pitch = std::max(-89.0f, std::min(89.0f, pitch));

    // Movement
    float dt = 0.005;
    float velocity = (speed * dt) * (keys->getPressed(GLFW_KEY_CAPS_LOCK) * 3 + 1);

    moveForward((keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)) * velocity);
    moveSide((keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A)) * velocity);
    moveUp((keys->getPressed(GLFW_KEY_SPACE) - keys->getPressed(GLFW_KEY_LEFT_SHIFT)) * velocity);

    updateProjection();
    updateView();
}

void Camera::moveSide(float distance) {
    position -= right * distance;
}

void Camera::moveForward(float distance) {
    position += glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))) * distance;
}

void Camera::moveUp(float distance) {
    position.y += distance;
}

};