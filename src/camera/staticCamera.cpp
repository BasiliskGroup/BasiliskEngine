#include <basilisk/camera/staticCamera.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

/**
 * @brief Construct a new Camera object
 * 
 * @param position 
 * @param pitch 
 * @param yaw 
 */
StaticCamera::StaticCamera(Engine* engine, glm::vec3 position, float pitch, float yaw): 
    engine(engine), position(position), pitch(pitch), yaw(yaw) 
{
    updateView();
    updateProjection();
}

/**
 * @brief 
 * 
 */
void StaticCamera::update() {
    updateProjection();
    updateView();
}

/**
 * @brief 
 * 
 */
void StaticCamera::updateView() {
    forward = {
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };
    right = glm::normalize(glm::cross(worldUp, forward));
    up = glm::cross(forward, right);

    view = glm::lookAt(position, position + forward, up);
}

/**
 * @brief Update the projection matrix based on current parameters
 * 
 */
void StaticCamera::updateProjection() {
    projection = glm::perspective(fov, aspect, near, far);
}

/**
 * @brief Write the view and projection matrices to the given shader.
 *        Assumes uniform names are 'uView' and 'uProjection'
 * 
 * @param shader 
 */
void StaticCamera::use(Shader* shader) {
    shader->setUniform("uView", view);
    shader->setUniform("uProjection", projection);
    shader->setUniform("uCameraPosition", position);
    shader->setUniform("uViewDirection", forward);
}

}