#ifndef BSK_INCLUDES_H
#define BSK_INCLUDES_H

// Standard library
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <stack>
#include <queue>
#include <set>
#include <optional>
#include <memory>
#include <utility>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <limits>
#include <fstream>
#include <sstream>
#include <functional>
#include <type_traits>

// Third-party libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <basilisk/util/constants.h>

namespace bsk::internal {

// Type aliases for cross-platform compatibility
using uint = unsigned int;
using ushort = unsigned short;

// AoS types
using BskVec2Triplet = std::array<glm::vec2, 3>;
using BskVec2Pair = std::array<glm::vec2, 2>;
using BskFloatPair = std::array<float, 2>;
using BskVec3ROWS = std::array<glm::vec3, ROWS>;
using BskVec3Pair = std::array<glm::vec3, 2>;
using BskMat3x3ROWS = std::array<glm::mat3x3, ROWS>;
using BskFloatROWS = std::array<float, ROWS>;
using BskBoolPair = std::array<bool, 2>;

// Mini structs
enum JType {
    JN = 0,
    JT = 1,
};

enum ForceType {
    NULL_FORCE,
    MANIFOLD,
    JOINT,
    SPRING,
    IGNORE_COLLISION
};

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

}

#endif
