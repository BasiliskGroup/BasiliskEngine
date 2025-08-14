#ifndef MOUSE_H
#define MOUSE_H

#include "includes.h"
#include "window.h"

class Mouse {
    private:
        GLFWwindow* window;
        double x, y;
        double previousX, previousY;
    public:
        Mouse(GLFWwindow* window): window(window), x(400), y(300), previousX(400), previousY(300) {}
        
        void update();

        bool clicked();
        bool middleClicked();
        bool rightClicked();
        bool leftDown();
        bool middleDown();
        bool rightDown();

        double getX();
        double getY();
        double getRelativeX();
        double getRelativeY();

        void setGrab();
        void setVisible();
        void setHidden();
};

#endif