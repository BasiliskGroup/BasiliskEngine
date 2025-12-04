#ifndef BSK_MOUSE_H
#define BSK_MOUSE_H

#include <basilisk/util/includes.h>
#include <basilisk/IO/window.h>
#include <basilisk/camera/staticCamera2d.h>

namespace bsk::internal {

class Engine;

class Mouse {
    private:
        Engine* engine;
        Window* window;
        double x, y;
        double previousX, previousY;

        bool left, middle, right;
        bool previousLeft;
        bool previousMiddle;
        bool previousRight;
    
    public:
        Mouse(Engine* engine);
        
        void update();

        bool getClicked()       { return left && !previousLeft; }
        bool getLeftClicked()   { return left && !previousLeft; }
        bool getMiddleClicked() { return middle && !previousMiddle; }
        bool getRightClicked()  { return right && !previousRight; }

        bool getLeftReleased()   { return !left && previousLeft; }
        bool getMiddleReleased() { return !middle && previousMiddle; }
        bool getRightReleased()  { return !right && previousRight; }

        bool getLeftDown()   { return left; }
        bool getMiddleDown() { return middle; }
        bool getRightDown()  { return right; }

        double getX();
        double getY();

        double getRelativeX() { return x - previousX; }
        double getRelativeY() { return y - previousY; }

        double getWorldX(StaticCamera2D* camera);
        double getWorldY(StaticCamera2D* camera);

        void setGrab();
        void setVisible();
        void setHidden();
};

};

#endif