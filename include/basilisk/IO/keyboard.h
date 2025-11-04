#ifndef BSK_KEYBOARD_H
#define BSK_KEYBOARD_H

#include <basilisk/util/includes.h>
#include <basilisk/IO/window.h>

namespace bsk::internal {

class Keyboard {
    private:
        Window* window;

    public:
        Keyboard(Window* window): window(window) {}

        bool getPressed(unsigned int keyCode);
};

}

#endif