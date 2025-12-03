#include <basilisk/engine/engine.h>

namespace bsk::internal {

Engine::Engine(int width, int height, const char* title, bool autoMouseGrab) {
    window = new Window(width, height, title);
    mouse = new Mouse(this);
    keyboard = new Keyboard(window);
    frame = new Frame(this, width, height);
    resourceServer = new ResourceServer();

    this->autoMouseGrab = autoMouseGrab;

    if (autoMouseGrab) {
        mouse->setGrab();
    }
}

Engine::~Engine() {
    delete mouse;
    delete keyboard;
    delete resourceServer;
    delete frame;
    delete window;
}


void Engine::update() {
    frame->use();
    frame->clear();

    // Mouse Updates
    mouse->update();
    // Auto mouse grab if enabled
    if (autoMouseGrab) {
        if (keyboard->getPressed(GLFW_KEY_ESCAPE)) {
            mouse->setVisible();
        }
        if (mouse->getClicked()) {
            mouse->setGrab();
        }
    }
}


void Engine::render() {
    window->use();
    window->clear(0.1, 0.1, 0.1, 1.0);
    frame->render();
    window->render();
}

void Engine::useContext() {
    window->use();
}

void Engine::setResolution(unsigned int width, unsigned int height) {
    delete frame;
    frame = new Frame(this, width, height);
}

}