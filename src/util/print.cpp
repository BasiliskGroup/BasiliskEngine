#include <basilisk/util/print.h>

namespace bsk::internal {

void print(std::string str) {
    std::cout << str << std::endl;
}

void print(char* str) {
    std::cout << str << std::endl;
}

void print(int n) {
    std::cout << n << std::endl;
}

void print(long l) {
    std::cout << l << std::endl;
}

void print(uint n) {
    std::cout << n << std::endl;
}

void print(float f) {
    std::cout << f << std::endl;
}

void print(const glm::vec2& vec) {
    std::cout << "<" << vec.x << "\t" << vec.y << ">" << std::endl;
}

void print(const glm::vec3& vec) {
    std::cout << "<" << vec.x << "\t" << vec.y << "\t" << vec.z << ">" << std::endl;
}

void print(const glm::quat& quat) {
    std::cout << "<" << quat.w << "\t" << quat.x << "\t" << quat.y << "\t" << quat.z << ">" << std::endl;
}

void print(const glm::mat2x2& mat) {
    for (int i = 0; i < 2; i++) print(mat[i]);
}

void print(const glm::mat3x3& mat) {
    for (int i = 0; i < 3; i++) print(mat[i]);
}

}