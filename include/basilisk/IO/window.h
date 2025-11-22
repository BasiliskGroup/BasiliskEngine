#ifndef BSK_WINDOW_H
#define BSK_WINDOW_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Window {
    private:
        GLFWwindow* window;
        int width;
        int height;
        double deltaTime;
        double previousTime;
        float windowScaleX, windowScaleY;

    public:
        Window(int width, int height, const char* title);
        ~Window();

        static void windowResize(GLFWwindow* window, int width, int height);

        bool isRunning();
        void render();
        void clear(float r=0.0, float g=0.0, float b=0.0, float a=1.0);
        void use();

        GLFWwindow* getWindow() { return window; }
        int getWidth() { return width; }
        int getHeight() { return height; }
        float getWindowScaleX() { return windowScaleX; }
        float getWindowScaleY() { return windowScaleY; }
        double getDeltaTime() { return deltaTime; }
};

}

#endif