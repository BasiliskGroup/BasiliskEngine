#include <basilisk/engine/engine.h>


Engine::Engine(int width, int height, const char* title) {
    window = new Window(width, height, title);
    mouse = new Mouse(window);
    keyboard = new Keyboard(window);
    resourceServer = new ResourceServer();

    mouse->setGrab();
}

Engine::~Engine() {
    delete mouse;
    delete keyboard;
    delete resourceServer;
    delete window;
}


void Engine::update() {
    window->clear(0.2, 0.3, 0.3, 1.0);

    // Mouse Updates
    mouse->update();
    if (keyboard->getPressed(GLFW_KEY_ESCAPE)) {
        mouse->setVisible();
    }
    if (mouse->getClicked()) {
        mouse->setGrab();
    }
}


void Engine::render() {
    window->render();
}