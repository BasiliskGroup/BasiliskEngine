#include <basilisk/camera/staticCamera2d.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

/**
 * @brief Construct a new Camera 2D object
 * 
 * @param position Starting position of the camera
 */
StaticCamera2D::StaticCamera2D(Engine* engine, glm::vec2 position, float scale): engine(engine), position(position) {
    setScale(scale);
    updateProjection();
    updateView();
}

/**
 * @brief Write the view and projection matrices to the given shader.
 *        Assumes uniform names are 'uView' and 'uProjection'
 * 
 * @param shader 
 */
void StaticCamera2D::use(Shader* shader) {
    shader->setUniform("uView", view);
    shader->setUniform("uProjection", projection);
}

/**
 * @brief 
 * 
 */
void StaticCamera2D::update() {
    updateView();
}

void StaticCamera2D::setScale(float scale) {
    float xScale, yScale;
    if (engine->getFrame()->getHeight() > engine->getFrame()->getWidth()) {
        xScale = scale;
        yScale = scale * engine->getFrame()->getHeight() / engine->getFrame()->getWidth();
    }
    else {
        xScale = scale * engine->getFrame()->getWidth() / engine->getFrame()->getHeight();
        yScale = scale;
    }
    viewScale = glm::vec2(xScale, yScale);
    updateProjection();
}

/**
 * @brief Creates an orthigraphic projection fo the camera
 * 
 */
void StaticCamera2D::updateProjection() {
    projection = glm::ortho(0.0f, viewScale.x, viewScale.y, 0.0f, -1.0f, 1.0f);
}

/**
 * @brief Updates the view matrix based on the current position
 * 
 */
void StaticCamera2D::updateView() {
    view = glm::mat4(1.0f);
    glm::vec2 translation(viewScale.x / 2.0 - position.x, viewScale.y / 2.0 + position.y);
    view = glm::translate(view, glm::vec3(translation, 0.0f));
}

}