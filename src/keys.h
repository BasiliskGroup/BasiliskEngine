#ifndef KEYS_H
#define KEYS_H

#include "includes.h"
#include "window.h"

class Keys {
    private:
        GLFWwindow* window;
    public:
        Keys(GLFWwindow* window): window(window) {}

        void update();

        bool isPressed(unsigned int key) { return (glfwGetKey(window, key) == GLFW_PRESS); }
};

#endif