#ifndef BINDINGS_GLM_HELPERS_H
#define BINDINGS_GLM_HELPERS_H

#include <pybind11/pybind11.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stdexcept>

namespace py = pybind11;

namespace bsk::bindings::helpers {

/**
 * Convert a Python tuple of 2 floats to glm::vec2
 * @param tuple Python tuple with 2 elements
 * @return glm::vec2
 */
inline glm::vec2 tuple_to_vec2(py::tuple tuple) {
    if (py::len(tuple) != 2) {
        throw std::runtime_error("Tuple must have exactly 2 elements (x, y)");
    }
    return glm::vec2(
        py::cast<float>(tuple[0]),
        py::cast<float>(tuple[1])
    );
}

/**
 * Convert a Python tuple of 3 floats to glm::vec3
 * @param tuple Python tuple with 3 elements
 * @return glm::vec3
 */
inline glm::vec3 tuple_to_vec3(py::tuple tuple) {
    if (py::len(tuple) != 3) {
        throw std::runtime_error("Tuple must have exactly 3 elements (x, y, z)");
    }
    return glm::vec3(
        py::cast<float>(tuple[0]),
        py::cast<float>(tuple[1]),
        py::cast<float>(tuple[2])
    );
}

/**
 * Convert a Python tuple of 4 floats to glm::quat
 * @param tuple Python tuple with 4 elements (w, x, y, z)
 * @return glm::quat
 */
inline glm::quat tuple_to_quat(py::tuple tuple) {
    if (py::len(tuple) != 4) {
        throw std::runtime_error("Tuple must have exactly 4 elements (w, x, y, z)");
    }
    return glm::quat(
        py::cast<float>(tuple[0]),  // w
        py::cast<float>(tuple[1]),  // x
        py::cast<float>(tuple[2]),  // y
        py::cast<float>(tuple[3])   // z
    );
}

/**
 * Convert glm::vec2 to a Python tuple
 * @param vec glm::vec2
 * @return Python tuple (x, y)
 */
inline py::tuple vec2_to_tuple(const glm::vec2& vec) {
    return py::make_tuple(vec.x, vec.y);
}

/**
 * Convert glm::vec3 to a Python tuple
 * @param vec glm::vec3
 * @return Python tuple (x, y, z)
 */
inline py::tuple vec3_to_tuple(const glm::vec3& vec) {
    return py::make_tuple(vec.x, vec.y, vec.z);
}

/**
 * Convert glm::quat to a Python tuple
 * @param quat glm::quat
 * @return Python tuple (w, x, y, z)
 */
inline py::tuple quat_to_tuple(const glm::quat& quat) {
    return py::make_tuple(quat.w, quat.x, quat.y, quat.z);
}

} // namespace bsk::bindings::helpers

#endif // BINDINGS_GLM_HELPERS_H
