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
    float dt = engine->getDeltaTime();
    float velocity = (speed * dt);

    position.x += (keys->getDown(KeyCode::K_D) - keys->getDown(KeyCode::K_A)) * velocity;
    position.y += (keys->getDown(KeyCode::K_W) - keys->getDown(KeyCode::K_S)) * velocity;

    updateView();
}

};