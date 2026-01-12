#include <basilisk/util/maths.h>

namespace bsk::internal {

void tripleProduct(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, glm::vec2& o) {
    o = glm::dot(a, c) * b - glm::dot(a, b) * c;
}

void perpTowards(const glm::vec2& v, const glm::vec2& to, glm::vec2& perp) {
    // Two possible perpendiculars
    glm::vec2 left  = glm::vec2(-v.y,  v.x);
    glm::vec2 right = glm::vec2( v.y, -v.x);

    // Pick whichever points more toward 'to'
    perp = (glm::dot(left, to) > glm::dot(right, to)) ? left : right;
}


/**
 * @brief Transforms the vector v using the position vector and scale/rotation matrix
 * 
 * @param pos 
 * @param mat 
 * @param v 
 */
void transform(const glm::vec2& pos, const glm::mat2x2& mat, glm::vec2& v) {
    v = mat * v + pos;
}

float cross(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.y - a.y * b.x;
}

float triangleArea2(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

std::pair<glm::vec3, glm::vec2> connectSquare(const glm::vec2& a, const glm::vec2& b, float width) {
    glm::vec2 delta = b - a;
    float len = glm::length(delta);
    glm::vec2 mid = (a + b) * 0.5f;
    float angle = std::atan2(delta.y, delta.x);
    glm::vec3 pos(mid.x, mid.y, angle);
    glm::vec2 scale(len, width);

    return {pos, scale};
}

}