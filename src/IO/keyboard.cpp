#include <basilisk/IO/keyboard.h>

namespace bsk::internal {

/**
 * @brief Update the keyboard state
 * 
 */
void Keyboard::update() {
    for (int i = 0; i < GLFW_KEY_LAST; i++) {
        previousKeys[i] = keys[i];
        keys[i] = glfwGetKey(window->getWindow(), i) == GLFW_PRESS;
    }
}

/**
 * @brief Get if the key was pressed this frame (excluding held keys)
 * 
 * @param keyCode 
 * @return true 
 * @return false 
 */
bool Keyboard::getPressed(KeyCode keyCode) {
    return keys[keyCode] && !previousKeys[keyCode];
}

/**
 * @brief Get if the key is down (held keys)
 * 
 * @param keyCode 
 * @return true 
 * @return false 
 */
bool Keyboard::getDown(KeyCode keyCode) {
    return keys[keyCode];
}

/**
 * @brief Get if the key was released this frame
 * 
 * @param keyCode 
 * @return true 
 * @return false 
 */
bool Keyboard::getReleased(KeyCode keyCode) {
    return !keys[keyCode] && previousKeys[keyCode];
}

}