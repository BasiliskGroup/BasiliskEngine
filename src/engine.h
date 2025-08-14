#ifndef ENGINE_H
#define ENGINE_H

#include "includes.h"
#include "window.h"
#include "keys.h"
#include "mouse.h"


class Engine {
    private:
        Window window;
        Keys keys;
        Mouse mouse;
        float lastFrame;
        float deltaTime;
        float windowTitleTimer;
    public:
        Engine(unsigned int width, unsigned int height, std::string title);
        
        void update();
        void render();

        bool isRunning() { return window.isRunning(); }
        void close() { window.close(); }
        
        float getDeltaTime() { return deltaTime; }
        Keys* getKeys() { return &keys; }
        Mouse* getMouse() { return &mouse; }
        Window* getWindow() { return &window; }
};

#endif