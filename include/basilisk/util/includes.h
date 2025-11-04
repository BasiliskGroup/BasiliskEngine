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

// AoS types
using Vec2Triplet = std::array<glm::vec2, 3>;
using Vec2Pair = std::array<glm::vec2, 2>;
using FloatPair = std::array<float, 2>;
using Vec3ROWS = std::array<glm::vec3, ROWS>;
using Mat3x3ROWS = std::array<glm::mat3x3, ROWS>;
using FloatROWS = std::array<float, ROWS>;

// Mini structs
enum JType {
    JN,
    JT,
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
