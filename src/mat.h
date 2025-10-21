#ifndef MAT_H
#define MAT_H

#include "includes.h"

class Matrix {
    private:
        glm::mat4 matrix;

    public:
        Matrix();

        void makeIdentity();
        void makePerspective(float FOV, float aspect, float near, float far);
        void makeOrographic(float left, float right, float bottom, float top, float near, float far);
        void makeView(glm::vec3 position, glm::vec3 target, glm::vec3 up);

        void invert();
        void translate(glm::vec3 translation);
        void rotate(glm::vec3 axis, float rotation);
        void scale(glm::vec3 scale);
        
        glm::mat4 getMatrix() { return matrix; }
};

#endif