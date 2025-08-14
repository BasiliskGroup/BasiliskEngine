#ifndef WINDOW_H
#define WINDOW_H

#include "includes.h"

void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

class Window {
    private:
        GLFWwindow* window;
        bool running;
        unsigned int width;
        unsigned int height;

    public:
        Window(unsigned int width, unsigned int height, const std::string title);
        ~Window();

        void update();
        void show();
        void close();
        
        bool isRunning();
        GLFWwindow* getWindow() { return window; }
        unsigned int getWidth() { return width; }
        unsigned int getHeight() { return height; }

        void clear();
        void clear(float r, float g, float b);
        void clear(float r, float g, float b, float a);
        
        
};

#endif