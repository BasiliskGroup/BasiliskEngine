#ifndef BSK_ENGINE_H
#define BSK_ENGINE_H

#include <basilisk/util/includes.h>
#include <basilisk/IO/window.h>
#include <basilisk/IO/keyboard.h>
#include <basilisk/IO/mouse.h>
#include <basilisk/resource/resourceServer.h>
#include <basilisk/render/frame.h>

namespace bsk::internal {

class Engine {
    private:
        Window* window;
        Keyboard* keyboard;
        Mouse* mouse;

        Frame* frame;
        ResourceServer* resourceServer;

    public:
        Engine(int width, int height, const char* title);
        ~Engine();

        bool isRunning() { return window->isRunning(); }
        void update();
        void render();
        void useContext();
        
        inline Window* getWindow() const { return window; }
        inline Mouse* getMouse() const { return mouse; }
        inline Keyboard* getKeyboard() const { return keyboard; }
        inline Frame* getFrame() const { return frame; }
        inline ResourceServer* getResourceServer() const { return resourceServer; }
        inline double getDeltaTime() { return window->getDeltaTime(); }
};

}

#endif