#include <basilisk/camera/camera2d.h>

/**
 * @brief Construct a new Camera 2D object
 * 
 * @param position Starting position of the camera
 */
Camera2D::Camera2D(Engine* engine, glm::vec2 position): engine(engine), position(position) {
    viewScale = glm::vec2(10.0f, 10.0f);
    updateProjection();
    updateView();
}

/**
 * @brief Write the view and projection matrices to the given shader.
 *        Assumes uniform names are 'uView' and 'uProjection'
 * 
 * @param shader 
 */
void Camera2D::use(Shader* shader) {
    shader->setUniform("uView", view);
    shader->setUniform("uProjection", projection);
}

/**
 * @brief Handle inputs to update the camera
 * 
 * @param mouse 
 * @param keys 
 */
void Camera2D::update() {
    
    // Get mouse and keyboard from engine
    Mouse* mouse = engine->getMouse();
    Keyboard* keys = engine->getKeyboard();
    
    // Movement
    float speed = 3.0;
    float dt = 0.005;
    float velocity = (speed * dt);

    position.x += (keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A)) * velocity;
    position.y += (keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)) * velocity;

    updateView();
}

/**
 * @brief Creates an orthigraphic projection fo the camera
 * 
 */
void Camera2D::updateProjection() {
    projection = glm::ortho(0.0f, viewScale.x, viewScale.y, 0.0f, -1.0f, 1.0f);
}

/**
 * @brief Updates the view matrix based on the current position
 * 
 */
void Camera2D::updateView() {
    view = glm::mat4(1.0f);
    glm::vec2 translation(viewScale.x / 2.0 - position.x, viewScale.y / 2.0 + position.y);
    view = glm::translate(view, glm::vec3(translation, 0.0f));
}