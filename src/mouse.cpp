#include "mouse.h"


void Mouse::update() {
    previousX = x;
    previousY = y;
    glfwGetCursorPos(window, &x, &y);
}

bool Mouse::clicked() {
    return false;
}

bool Mouse::middleClicked() {
    return false;
}

bool Mouse::rightClicked() {
    return false;
}

bool Mouse::leftDown() { return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS; }
bool Mouse::middleDown() { return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS; }
bool Mouse::rightDown() { return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS; }

double Mouse::getX() { return x; }
double Mouse::getY() { return y; }
double Mouse::getRelativeX() { return x - previousX; }
double Mouse::getRelativeY() { return y - previousY; }

void Mouse::setGrab() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
void Mouse::setVisible() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
void Mouse::setHidden() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }

