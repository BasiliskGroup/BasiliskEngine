#include "camera.h"


Camera::Camera(Engine* engine): 
    pitch(0), yaw(-90), speed(3), sensitivity(1), fov(50.0), engine(engine) {
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    direction = glm::vec3(0.0f, 0.0f, -1.0f);

    updatePerspective();
}

void Camera::update() {

    float dt = engine->getDeltaTime();
    Mouse* mouse = engine->getMouse();
    Keys* keys = engine->getKeys();
    
    float yOffset = mouse->getRelativeY() * sensitivity / 5;
    float xOffset = mouse->getRelativeX() * sensitivity / 5;

    yaw += xOffset;
    pitch -= yOffset;
    pitch = std::max(-89.0f, std::min(89.0f, pitch));

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(UP, direction));
    glm::vec3 cameraUp = glm::cross(direction, cameraRight);
    view = glm::lookAt(position, position + direction, cameraUp);

    float velocity = (speed * dt) * (keys->isPressed(GLFW_KEY_CAPS_LOCK) * 3 + 1);
    if (keys->isPressed(GLFW_KEY_W))
        position += glm::normalize(glm::vec3(direction.x, 0, direction.z)) * velocity;
    if (keys->isPressed(GLFW_KEY_S))
        position -= glm::normalize(glm::vec3(direction.x, 0, direction.z)) * velocity;
    if (keys->isPressed(GLFW_KEY_D))
        position -= cameraRight * velocity;
    if (keys->isPressed(GLFW_KEY_A))
        position += cameraRight * velocity;
    
    position.y += (keys->isPressed(GLFW_KEY_SPACE) - keys->isPressed(GLFW_KEY_LEFT_SHIFT)) * velocity;
}

void Camera::write(Shader* shader) {
    shader->setUniform("view", view);
    shader->setUniform("projection", projection);
}

void Camera::setFOV(float fov) {
    this->fov = fov;
    updatePerspective();
}

void Camera::updatePerspective() {
    Window* window = engine->getWindow();
    projection = glm::perspective(glm::radians(fov), (float)window->getWidth()/window->getHeight(), 0.1f, 100.0f);

}