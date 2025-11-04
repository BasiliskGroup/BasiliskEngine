#ifndef MATHS_H
#define MATHS_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

void tripleProduct(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, glm::vec2& o);
void perpTowards(const glm::vec2& v, const glm::vec2& to, glm::vec2& perp);
void transform(const glm::vec2& pos, const glm::mat2x2& mat, glm::vec2& v);
float cross(const glm::vec2& a, const glm::vec2& b);

inline glm::vec2 xy(const glm::vec3& v) noexcept {
    return glm::vec2(v.x, v.y);
}

float triangleArea2(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

}

#endif