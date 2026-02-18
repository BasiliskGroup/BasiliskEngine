#ifndef BSK_QUAT_PROXY_H
#define BSK_QUAT_PROXY_H

#include <functional>
#include <pybind11/pybind11.h>
#include <glm/gtc/quaternion.hpp>

// Quaternion proxy — separate class due to quat-specific memory layout and operations.
// glm::quat stores as (w, x, y, z) in memory but components are named .x .y .z .w.
// Supports: component access (x,y,z,w), quat*quat, quat*scalar, conjugate, inverse, normalize.
// Does NOT support +, -, / between quats (not meaningful for unit quaternions).
class QuatProxy {
    public:
        using GetterFunc = std::function<glm::quat()>;
        using SetterFunc = std::function<void(const glm::quat&)>;
    
        GetterFunc getter;
        SetterFunc setter;
        static constexpr int size = 4;
    
        QuatProxy(GetterFunc get, SetterFunc set) : getter(get), setter(set) {}
    
        glm::quat get() const { return getter(); }
        void set(const glm::quat& q) { setter(q); }
    
        // Component accessors (x, y, z, w)
        float get_x() const { return getter().x; }
        void set_x(float v) { modify([&](glm::quat& q) { q.x = v; }); }
    
        float get_y() const { return getter().y; }
        void set_y(float v) { modify([&](glm::quat& q) { q.y = v; }); }
    
        float get_z() const { return getter().z; }
        void set_z(float v) { modify([&](glm::quat& q) { q.z = v; }); }
    
        float get_w() const { return getter().w; }
        void set_w(float v) { modify([&](glm::quat& q) { q.w = v; }); }
    
        // Assignment
        QuatProxy& operator=(const glm::quat& other) { setter(other); return *this; }
        QuatProxy& operator=(py::object other) { setter(from_iterable(other)); return *this; }
    
        // Quaternion multiplication (composition)
        glm::quat operator*(const glm::quat& o) const { return getter() * o; }
        glm::quat operator*(float s)            const { return getter() * s; }
    
        QuatProxy& operator*=(const glm::quat& o) { return modify_op([&](glm::quat& q) { q *= o; }); }
        QuatProxy& operator*=(float s)            { return modify_op([&](glm::quat& q) { q *= s; }); }
    
        // Quaternion-specific operations
        glm::quat conjugate() const { return glm::conjugate(getter()); }
        glm::quat inverse()   const { return glm::inverse(getter()); }
        glm::quat normalize() const { return glm::normalize(getter()); }
        float     length()    const { return glm::length(getter()); }
        float     dot(const glm::quat& o) const { return glm::dot(getter(), o); }
    
        // Euler angle conversion (returns vec3 in radians: pitch, yaw, roll)
        glm::vec3 euler_angles() const { return glm::eulerAngles(getter()); }
    
    private:
        // Iterable must be length 4, ordered as (x, y, z, w)
        glm::quat from_iterable(py::object obj) const {
            if (py::len(obj) != 4)
                throw py::value_error("Iterable length must be 4 for quat (x, y, z, w)");
            return glm::quat(
                py::cast<float>(obj[py::int_(3)]),  // w
                py::cast<float>(obj[py::int_(0)]),  // x
                py::cast<float>(obj[py::int_(1)]),  // y
                py::cast<float>(obj[py::int_(2)])   // z
            );
        }
    
        template<typename F>
        void modify(F&& fn) {
            auto q = getter();
            fn(q);
            setter(q);
        }
    
        template<typename F>
        QuatProxy& modify_op(F&& fn) {
            modify(std::forward<F>(fn));
            return *this;
        }
    };
    
    // Helper to convert py::object (4-tuple as x,y,z,w or native glm::quat) to glm::quat
    inline glm::quat quat_from_pyobject(py::object obj) {
        if (py::isinstance<glm::quat>(obj)) return py::cast<glm::quat>(obj);
        if (py::len(obj) != 4) throw py::value_error("Iterable length must be 4 for quat (x, y, z, w)");
        return glm::quat(
            py::cast<float>(obj[py::int_(3)]),  // w
            py::cast<float>(obj[py::int_(0)]),  // x
            py::cast<float>(obj[py::int_(1)]),  // y
            py::cast<float>(obj[py::int_(2)])   // z
        );
    }
    
    // Bind function for QuatProxy
    inline void bind_quat_proxy(py::module_& m, const char* name) {
        py::class_<QuatProxy>(m, name, py::module_local())
            .def_property("x", &QuatProxy::get_x, &QuatProxy::set_x)
            .def_property("y", &QuatProxy::get_y, &QuatProxy::set_y)
            .def_property("z", &QuatProxy::get_z, &QuatProxy::set_z)
            .def_property("w", &QuatProxy::get_w, &QuatProxy::set_w)
            .def("__len__",     [](const QuatProxy&) { return 4; })
            // __iter__ and __getitem__ expose components as (x, y, z, w)
            .def("__iter__", [](const QuatProxy& p) {
                auto q = p.get();
                return py::iter(py::make_tuple(q.x, q.y, q.z, q.w));
            })
            .def("__getitem__", [](const QuatProxy& p, py::ssize_t i) -> float {
                auto q = p.get();
                if (i == 0) return q.x;
                if (i == 1) return q.y;
                if (i == 2) return q.z;
                if (i == 3) return q.w;
                throw py::index_error("index out of range");
            })
            .def("__setitem__", [](QuatProxy& p, py::ssize_t i, float v) {
                auto q = p.get();
                if      (i == 0) q.x = v;
                else if (i == 1) q.y = v;
                else if (i == 2) q.z = v;
                else if (i == 3) q.w = v;
                else throw py::index_error("index out of range");
                p.set(q);
            })
            .def("__setattr__", [](QuatProxy& p, const std::string& attr, py::object value) {
                auto q = p.get();
                if      (attr == "x") q.x = py::cast<float>(value);
                else if (attr == "y") q.y = py::cast<float>(value);
                else if (attr == "z") q.z = py::cast<float>(value);
                else if (attr == "w") q.w = py::cast<float>(value);
                else throw py::attribute_error("Unknown attribute");
                p.set(q);
            })
            // Multiplication (quat composition and scalar scaling)
            .def("__mul__",  [](const QuatProxy& p, const glm::quat& o) { return p * o; })
            .def("__mul__",  [](const QuatProxy& p, float s)             { return p * s; })
            .def("__rmul__", [](const QuatProxy& p, float s)             { return p * s; })
            .def("__imul__", [](QuatProxy& p, const glm::quat& o)        { return p *= o; })
            .def("__imul__", [](QuatProxy& p, float s)                   { return p *= s; })
            // Quaternion-specific methods
            .def("conjugate",    &QuatProxy::conjugate)
            .def("inverse",      &QuatProxy::inverse)
            .def("normalize",    &QuatProxy::normalize)
            .def("length",       &QuatProxy::length)
            .def("dot",          &QuatProxy::dot,          py::arg("other"))
            .def("euler_angles", &QuatProxy::euler_angles,
                 "Returns euler angles as a vec3 (pitch, yaw, roll) in radians");
    }

#endif  // BSK_QUAT_PROXY_H
