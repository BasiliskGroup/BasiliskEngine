#include "engine.h"

Engine::Engine(unsigned int width, unsigned int height, std::string title): 
    window(Window(width, height, title)),
    keys(Keys(window.getWindow())), 
    mouse(Mouse(window.getWindow())),
    lastFrame(0), deltaTime(0) 
{
    mouse.setGrab();
    update();
    update();
}
    

void Engine::update() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  

    windowTitleTimer += deltaTime;
    if (windowTitleTimer > 1.0) {
        glfwSetWindowTitle(window.getWindow(), std::to_string(1.0 / deltaTime).c_str());
        windowTitleTimer = 0;
    }
    

    keys.update();
    mouse.update();

    if (keys.isPressed(GLFW_KEY_ESCAPE)) {
        mouse.setVisible();
    }
    if (mouse.leftDown()) {
        mouse.setGrab();
    }

    window.update();
    window.clear(0.2, 0.3, 0.3, 1.0);
}

void Engine::render() {
    window.show();
}