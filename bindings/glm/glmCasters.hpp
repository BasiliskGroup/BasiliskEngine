#ifndef BSK_GLM_CASTERS_HPP
#define BSK_GLM_CASTERS_HPP

#include <pybind11/pybind11.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace pybind11::detail {

inline bool load_vec(pybind11::handle src, float* out, size_t n) {
    // PyGLM-style attributes
    static const char* names[] = {"x", "y", "z", "w"};

    bool has_attrs = true;
    for (size_t i = 0; i < n; ++i)
        has_attrs &= pybind11::hasattr(src, names[i]);

    if (has_attrs) {
        for (size_t i = 0; i < n; ++i)
            out[i] = src.attr(names[i]).cast<float>();
        return true;
    }

    // Generic sequence
    if (pybind11::isinstance<pybind11::sequence>(src)) {
        pybind11::sequence seq = pybind11::reinterpret_borrow<pybind11::sequence>(src);
        if (seq.size() != static_cast<pybind11::ssize_t>(n))
            return false;

        for (size_t i = 0; i < n; ++i)
            out[i] = seq[i].cast<float>();
        return true;
    }

    return false;
}

// -------------------------
// glm::vec2
// -------------------------
template <>
struct type_caster<glm::vec2> {
public:
    PYBIND11_TYPE_CASTER(glm::vec2, _("glm.vec2"));

    bool load(pybind11::handle src, bool) {
        return load_vec(src, &value.x, 2);
    }

    static pybind11::handle cast(const glm::vec2& v,
                           pybind11::return_value_policy,
                           pybind11::handle) {
        pybind11::object glm = pybind11::module_::import("glm");
        return glm.attr("vec2")(v.x, v.y).release();
    }
};

// -------------------------
// glm::vec3
// -------------------------
template <>
struct type_caster<glm::vec3> {
public:
    PYBIND11_TYPE_CASTER(glm::vec3, _("glm.vec3"));

    bool load(pybind11::handle src, bool) {
        return load_vec(src, &value.x, 3);
    }

    static pybind11::handle cast(const glm::vec3& v,
                           pybind11::return_value_policy,
                           pybind11::handle) {
        pybind11::object glm = pybind11::module_::import("glm");
        return glm.attr("vec3")(v.x, v.y, v.z).release();
    }
};


// -------------------------
// glm::quat
// -------------------------
template <>
struct type_caster<glm::quat> {
public:
    PYBIND11_TYPE_CASTER(glm::quat, _("glm.quat"));

    bool load(pybind11::handle src, bool) {
        float tmp[4];
        if (!load_vec(src, tmp, 4))
            return false;

        value = glm::quat(tmp[3], tmp[0], tmp[1], tmp[2]); // (w, x, y, z)
        return true;
    }

    static pybind11::handle cast(const glm::quat& q,
                           pybind11::return_value_policy,
                           pybind11::handle) {
        pybind11::object glm = pybind11::module_::import("glm");
        return glm.attr("quat")(q.w, q.x, q.y, q.z).release();
    }
};


} // namespace pybind11::detail

#endif // BSK_GLM_CASTERS_HPP