#ifndef BSK_KEYBOARD_H
#define BSK_KEYBOARD_H

#include <basilisk/util/includes.h>
#include <basilisk/IO/window.h>

namespace bsk::internal {

// GLFW Keycodes to Basilisk Keycodes
enum KeyCode {
    K_A = GLFW_KEY_A,
    K_B = GLFW_KEY_B,
    K_C = GLFW_KEY_C,
    K_D = GLFW_KEY_D,
    K_E = GLFW_KEY_E,
    K_F = GLFW_KEY_F,
    K_G = GLFW_KEY_G,
    K_H = GLFW_KEY_H,
    K_I = GLFW_KEY_I,
    K_J = GLFW_KEY_J,
    K_K = GLFW_KEY_K,
    K_L = GLFW_KEY_L,
    K_M = GLFW_KEY_M,
    K_N = GLFW_KEY_N,
    K_O = GLFW_KEY_O,
    K_P = GLFW_KEY_P,
    K_Q = GLFW_KEY_Q,
    K_R = GLFW_KEY_R,
    K_S = GLFW_KEY_S,
    K_T = GLFW_KEY_T,
    K_U = GLFW_KEY_U,
    K_V = GLFW_KEY_V,
    K_W = GLFW_KEY_W,
    K_X = GLFW_KEY_X,
    K_Y = GLFW_KEY_Y,
    K_Z = GLFW_KEY_Z,
    K_0 = GLFW_KEY_0,
    K_1 = GLFW_KEY_1,
    K_2 = GLFW_KEY_2,
    K_3 = GLFW_KEY_3,
    K_4 = GLFW_KEY_4,
    K_5 = GLFW_KEY_5,
    K_6 = GLFW_KEY_6,
    K_7 = GLFW_KEY_7,
    K_8 = GLFW_KEY_8,
    K_9 = GLFW_KEY_9,
    K_F1 = GLFW_KEY_F1,
    K_F2 = GLFW_KEY_F2,
    K_F3 = GLFW_KEY_F3,
    K_F4 = GLFW_KEY_F4,
    K_F5 = GLFW_KEY_F5,
    K_F6 = GLFW_KEY_F6,
    K_F7 = GLFW_KEY_F7,
    K_F8 = GLFW_KEY_F8,
    K_F9 = GLFW_KEY_F9,
    K_F10 = GLFW_KEY_F10,
    K_F11 = GLFW_KEY_F11,
    K_F12 = GLFW_KEY_F12,
    K_BACKSPACE = GLFW_KEY_BACKSPACE,
    K_TAB = GLFW_KEY_TAB,
    K_ENTER = GLFW_KEY_ENTER,
    K_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
    K_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
    K_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
    K_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
    K_LEFT_ALT = GLFW_KEY_LEFT_ALT,
    K_RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
    K_CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
    K_ESCAPE = GLFW_KEY_ESCAPE,
    K_SPACE = GLFW_KEY_SPACE
};

class Keyboard {
    private:
        Window* window;
        bool keys[GLFW_KEY_LAST];
        bool previousKeys[GLFW_KEY_LAST];

    public:
        Keyboard(Window* window): window(window) {}

        void update();

        bool getDown(KeyCode keyCode);
        bool getPressed(KeyCode keyCode);
        bool getReleased(KeyCode keyCode);
};

}

#endif