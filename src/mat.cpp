#include "mat.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 * 
 */
Matrix::Matrix() {
    makeIdentity();
}

/**
 * @brief Convert this matrix to an identity matrix
 * 
 */
void Matrix::makeIdentity() {
    matrix = glm::mat4(1.0f);
}

/**
 * @brief Convert this matrix to a perspective projection matrix
 * 
 * @param FOV Feild of view in degrees
 * @param aspect Aspect ratio of the screen (width / height)
 * @param near Distance of the near plane
 * @param far Distance of the far plane
 */
void Matrix::makePerspective(float FOV, float aspect, float near, float far) {
    matrix = glm::perspective(glm::radians(FOV), aspect, near, far);
}

/**
 * @brief Convert this matrix to an orthographic projection Matrix
 * 
 * @param left 
 * @param right 
 * @param bottom 
 * @param top 
 * @param near 
 * @param far 
 */
void Matrix::makeOrographic(float left, float right, float bottom, float top, float near, float far) {
    matrix = glm::ortho(left, right, bottom, top, near, far);
}

void Matrix::makeView(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    matrix = glm::lookAt(position, target, up);
}

/**
 * @brief Inverts this matrix
 * 
 */
void Matrix::invert() {
    matrix = glm::inverse(matrix);
}

/**
 * @brief Translates this matrix by the given vector
 * 
 * @param translation Vector of translation (x, y, z)
 */
void Matrix::translate(glm::vec3 translation) {
    matrix = glm::translate(matrix, translation);
}

/**
 * @brief Rotates this matrix along the given axis for 'rotation' radians
 * 
 * @param axis Axis about which to rotate
 * @param rotation Magnitude of rotation in radians
 */
void Matrix::rotate(glm::vec3 axis, float rotation) {
    matrix = glm::rotate(matrix, rotation, axis);
}

/**
 * @brief Scales this matrix by the given vector
 * 
 * @param scale Vector of scale (x, y, z)
 */
void Matrix::scale(glm::vec3 scale) {
    matrix = glm::scale(matrix, scale);
}