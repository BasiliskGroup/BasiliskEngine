#ifndef BSK_ENGINE_H
#define BSK_ENGINE_H

#include <basilisk/util/includes.h>
#include <basilisk/IO/window.h>
#include <basilisk/IO/keyboard.h>
#include <basilisk/IO/mouse.h>
#include <basilisk/resource/resourceServer.h>

namespace bsk::internal {

class Engine {
    private:
        Window* window;
        Keyboard* keyboard;
        Mouse* mouse;
        ResourceServer* resourceServer;

    public:
        Engine(int width, int height, const char* title);
        ~Engine();

        bool isRunning() { return window->isRunning(); }
        void update();
        void render();
        void useContext();
        
        inline GLFWwindow* getWindow() const { return window->getWindow(); }
        inline Mouse* getMouse() const { return mouse; }
        inline Keyboard* getKeyboard() const { return keyboard; }
        inline ResourceServer* getResourceServer() const { return resourceServer; }
        inline double getDeltaTime() { return window->getDeltaTime(); }
};

}

#endif