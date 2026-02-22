#include <basilisk/engine/engine.h>
#include <basilisk/compute/gpuWrapper.hpp>

namespace bsk::internal {

std::unique_ptr<ResourceServer> Engine::resourceServer(nullptr);

Engine::Engine(int width, int height, const char* title, bool autoMouseGrab, bool showSplash) {
    window = new Window(width, height, title);
    mouse = new Mouse(this);
    keyboard = new Keyboard(window);
    frame = new Frame(this, width, height);

    if (!resourceServer) {
        resourceServer = std::make_unique<ResourceServer>();

        // only initialize the GPU once, we'll use the fact that resourceServer is a singleton to ensure this
        initGpu();
    } else {
        // CRITICAL: Reset MaterialServer state when creating a new Engine instance
        // The C++ singleton persists between Python runs, so when Python objects are destroyed
        // and recreated, the C++ side still has raw pointers to the old (destroyed) materials.
        // This causes crashes when those pointers are accessed. Resetting clears stale pointers.
        // This ensures consistent, reliable behavior between runs.
        resourceServer->getMaterialServer()->reset();
    }

    this->autoMouseGrab = autoMouseGrab;

    if (autoMouseGrab) {
        mouse->setGrab();
    }

    if (showSplash) {
        ::bsk::internal::showSplash(this);
    }
}

Engine::~Engine() {
    delete mouse;
    delete keyboard;
    delete frame;
    delete window;
}


void Engine::update() {
    // CRITICAL: Process deferred material updates/additions at START of update, BEFORE frame operations
    // This ensures texture array operations (including resizing) happen before any frame is active.
    // 
    // IMPORTANT: Materials swapped during scene->update() will be processed in the NEXT frame.
    // This is acceptable because:
    // 1. Materials should be added immediately when node.set_material() is called (if context is current)
    // 2. If they can't be added immediately, they'll use default material (ID 0) for 1 frame (safe)
    // 3. Processing here ensures all deferred materials are ready before rendering begins
    if (resourceServer) {
        resourceServer->getMaterialServer()->processPendingUpdates();
    }
    
    frame->use();
    frame->clear();

    // Input Updates
    keyboard->update();
    mouse->update();

    // Auto mouse grab if enabled
    if (autoMouseGrab) {
        if (keyboard->getPressed(KeyCode::K_ESCAPE)) {
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

void Engine::showSplash() {
    ::bsk::internal::showSplash(this);
}

}