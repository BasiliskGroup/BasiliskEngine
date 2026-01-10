#include <basilisk/camera/camera2d.h>
#include <basilisk/engine/engine.h>

namespace bsk::internal {

/**
 * @brief 
 * 
 */
void Camera2D::update() {
    
    // Get mouse and keyboard from engine
    Mouse* mouse = engine->getMouse();
    Keyboard* keys = engine->getKeyboard();
    
    // Movement
    float dt = 0.005;
    float velocity = (speed * dt);

    position.x += (keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A)) * velocity;
    position.y += (keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)) * velocity;

    updateView();
}

};