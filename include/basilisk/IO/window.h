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

        inline GLFWwindow* getWindow() { return window; }
        inline int getWidth() { return width; }
        inline int getHeight() { return height; }
        inline float getWindowScaleX() { return windowScaleX; }
        inline float getWindowScaleY() { return windowScaleY; }
        inline double getDeltaTime() { return deltaTime; }
        inline double getTime() { return glfwGetTime(); }
        inline int getFPS() { return (int)round(1.0 / deltaTime); }

        inline void enableDepthTest() { glEnable(GL_DEPTH_TEST);  }
        inline void enableCullFace() { glEnable(GL_CULL_FACE); }
        inline void enableMultisample() { glEnable(GL_MULTISAMPLE); }
        inline void enableBlend() { glEnable(GL_BLEND); }
        inline void enableVSync() { glfwSwapInterval(1); }
        inline void disableDepthTest() { glDisable(GL_DEPTH_TEST); }
        inline void disableCullFace() { glDisable(GL_CULL_FACE); }  
        inline void disableMultisample() { glDisable(GL_MULTISAMPLE); }
        inline void disableBlend() { glDisable(GL_BLEND); }
        inline void disableVSync() { glfwSwapInterval(0); }
};

}

#endif