#include <basilisk/IO/keyboard.h>

namespace bsk::internal {

bool Keyboard::getPressed(unsigned int keyCode) {
    return (glfwGetKey(window->getWindow(), keyCode) == GLFW_PRESS);
}

}