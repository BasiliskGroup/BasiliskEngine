#ifndef ENGINE_H
#define ENGINE_H

#include "util/includes.h"
#include "IO/window.h"
#include "IO/keyboard.h"
#include "IO/mouse.h"
#include "resource/resourceServer.h"


class Engine {
    private:
        Window* window;
        Keyboard* keyboard;
        Mouse* mouse;
        ResourceServer* resourceServer;

        double deltaTime;

    public:
        Engine(int width, int height, const char* title);
        ~Engine();

        bool isRunning() { return window->isRunning(); }
        void update();
        void render();
        
        inline GLFWwindow* getWindow() const { return window->getWindow(); }
        inline Mouse* getMouse() const { return mouse; }
        inline Keyboard* getKeyboard() const { return keyboard; }
        inline ResourceServer* getResourceServer() const { return resourceServer; }
};

#endif