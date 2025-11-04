#ifndef BSK_PRINT_H
#define BSK_PRINT_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

void print(std::string str);
void print(char* str);
void print(int n);
void print(long l);
void print(uint n);
void print(float f);
void print(const glm::vec2& vec);
void print(const glm::vec3& vec);
void print(const glm::quat& quat);
void print(const glm::mat2x2& mat);
void print(const glm::mat3x3& mat);

}

#endif