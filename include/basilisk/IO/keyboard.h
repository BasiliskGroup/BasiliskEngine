#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <basilisk/util/includes.h>
#include <basilisk/IO/window.h>

class Keyboard {
    private:
        Window* window;

    public:
        Keyboard(Window* window): window(window) {}

        bool getPressed(unsigned int keyCode);
};

#endif