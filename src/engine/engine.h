#ifndef ENGINE_H
#define ENGINE_H

#include "util/includes.h"
#include "IO/window.h"
#include "IO/keyboard.h"
#include "IO/mouse.h"


class Engine {
    private:
        Window* window;
        Keyboard* keyboard;
        Mouse* mouse;

        double deltaTime;

    public:
        Engine(int width, int height, const char* title);
        ~Engine();

        bool isRunning() { return window->isRunning(); }
        void update();
        void render();
        
        GLFWwindow* getWindow() { return window->getWindow(); }
        Mouse* getMouse() { return mouse; }
        Keyboard* getKeyboard() { return keyboard; }
};

#endif