#include <basilisk/IO/mouse.h>
#include <basilisk/engine/engine.h>


namespace bsk::internal {

Mouse::Mouse(Engine* engine): engine(engine), window(engine->getWindow()), x(0), y(0), previousX(0), previousY(0) {}

/**
 * @brief Update the state of the mouse and store previous state
 * 
 */
void Mouse::update() {
    // Store the previous frame's mouse state
    previousX = x;
    previousY = y;
    
    previousLeft   = left;
    previousMiddle = middle;
    previousRight  = right;
    
    // Update the position and click states
    glfwGetCursorPos(window->getWindow(), &x, &y);

    left = glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    middle = glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    right = glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

double Mouse::getX() { return x * engine->getWindow()->getWindowScaleX(); }
double Mouse::getY() { return y * engine->getWindow()->getWindowScaleY(); }

/**
 * @brief Grabs the mouse and hides it
 * 
 */
void Mouse::setGrab() {
    glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

/**S
 * @brief Shows the mouse and ungrabs it
 * 
 */
void Mouse::setVisible() {
    glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

/**
 * @brief Hides the mouse but does not grab it
 * 
 */
void Mouse::setHidden() {
    glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}


double Mouse::getWorldX(StaticCamera2D* camera) {
    double tileWidth = (double)engine->getFrame()->getRenderWidth() / camera->getViewWidth();
    return camera->getX() + (getX() - window->getWidth() * engine->getWindow()->getWindowScaleX() / 2) / tileWidth;
}


double Mouse::getWorldY(StaticCamera2D* camera) {
    double tileHeight = (double)engine->getFrame()->getRenderHeight() / camera->getViewHeight();
    return camera->getY() + (window->getHeight() * engine->getWindow()->getWindowScaleY() / 2 - getY()) / tileHeight;
}

};
