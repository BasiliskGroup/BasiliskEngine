#ifndef BSK_VECTOR_PROXY_H
#define BSK_VECTOR_PROXY_H

#include <functional>
#include <pybind11/pybind11.h>
#include <glm/glm.hpp>

namespace py = pybind11;

template<typename VecType>
class VectorPropertyProxy {
public:
    using GetterFunc = std::function<VecType()>;
    using SetterFunc = std::function<void(const VecType&)>;

    GetterFunc getter;
    SetterFunc setter;
    static constexpr int size = sizeof(VecType) / sizeof(float);

    VectorPropertyProxy(GetterFunc get, SetterFunc set) : getter(get), setter(set) {}

    VecType get() const { return getter(); }
    void set(const VecType& v) { setter(v); }

    // Component accessors
    float get_x() const { return getter().x; }
    void set_x(float v) { modify([&](VecType& vec) { vec.x = v; }); }

    float get_y() const { return getter().y; }
    void set_y(float v) { modify([&](VecType& vec) { vec.y = v; }); }

    // z component only available for vec3
    float get_z() const requires (size == 3) { return getter().z; }
    void set_z(float v) requires (size == 3) { modify([&](VecType& vec) { vec.z = v; }); }

    // Assignment
    VectorPropertyProxy& operator=(const VecType& other) { setter(other); return *this; }
    VectorPropertyProxy& operator=(py::object other) { setter(from_iterable(other)); return *this; }

    // Arithmetic (binary)
    VecType operator+(const VecType& o) const { return getter() + o; }
    VecType operator+(py::object o)     const { return getter() + from_iterable(o); }
    VecType operator-(const VecType& o) const { return getter() - o; }
    VecType operator-(py::object o)     const { return getter() - from_iterable(o); }
    VecType operator*(float s)          const { return getter() * s; }
    VecType operator*(const VecType& o) const { return getter() * o; }
    VecType operator*(py::object o)     const { return is_scalar(o) ? getter() * py::cast<float>(o) : getter() * from_iterable(o); }
    VecType operator/(float s)          const { return getter() / s; }
    VecType operator/(const VecType& o) const { return getter() / o; }
    VecType operator/(py::object o)     const { return is_scalar(o) ? getter() / py::cast<float>(o) : getter() / from_iterable(o); }

    // Compound assignment
    VectorPropertyProxy& operator+=(const VecType& o) { return modify_op([&](VecType& v) { v += o; }); }
    VectorPropertyProxy& operator+=(py::object o)     { return modify_op([&](VecType& v) { v += from_iterable(o); }); }
    VectorPropertyProxy& operator-=(const VecType& o) { return modify_op([&](VecType& v) { v -= o; }); }
    VectorPropertyProxy& operator-=(py::object o)     { return modify_op([&](VecType& v) { v -= from_iterable(o); }); }
    VectorPropertyProxy& operator*=(float s)          { return modify_op([&](VecType& v) { v *= s; }); }
    VectorPropertyProxy& operator*=(const VecType& o) { return modify_op([&](VecType& v) { v *= o; }); }
    VectorPropertyProxy& operator*=(py::object o)     { return is_scalar(o) ? modify_op([&](VecType& v) { v *= py::cast<float>(o); }) : modify_op([&](VecType& v) { v *= from_iterable(o); }); }
    VectorPropertyProxy& operator/=(float s)          { return modify_op([&](VecType& v) { v /= s; }); }
    VectorPropertyProxy& operator/=(const VecType& o) { return modify_op([&](VecType& v) { v /= o; }); }
    VectorPropertyProxy& operator/=(py::object o)     { return is_scalar(o) ? modify_op([&](VecType& v) { v /= py::cast<float>(o); }) : modify_op([&](VecType& v) { v /= from_iterable(o); }); }

private:
    static bool is_scalar(py::object o) {
        return py::isinstance<py::float_>(o) || py::isinstance<py::int_>(o);
    }

    VecType from_iterable(py::object obj) const {
        if (py::len(obj) != size)
            throw py::value_error("Iterable length must match vector size");
        VecType result;
        for (int i = 0; i < size; ++i)
            reinterpret_cast<float*>(&result)[i] = py::cast<float>(obj[py::int_(i)]);
        return result;
    }

    template<typename F>
    void modify(F&& fn) {
        auto vec = getter();
        fn(vec);
        setter(vec);
    }

    template<typename F>
    VectorPropertyProxy& modify_op(F&& fn) {
        modify(std::forward<F>(fn));
        return *this;
    }
};

using Vec2Proxy = VectorPropertyProxy<glm::vec2>;
using Vec3Proxy = VectorPropertyProxy<glm::vec3>;

// Helpers to convert py::object to glm vectors
inline glm::vec2 vec2_from_pyobject(py::object obj) {
    if (py::isinstance<glm::vec2>(obj)) return py::cast<glm::vec2>(obj);
    if (py::len(obj) != 2) throw py::value_error("Iterable length must be 2 for vec2");
    return {py::cast<float>(obj[py::int_(0)]), py::cast<float>(obj[py::int_(1)])};
}

inline glm::vec3 vec3_from_pyobject(py::object obj) {
    if (py::isinstance<glm::vec3>(obj)) return py::cast<glm::vec3>(obj);
    if (py::len(obj) != 3) throw py::value_error("Iterable length must be 3 for vec3");
    return {py::cast<float>(obj[py::int_(0)]), py::cast<float>(obj[py::int_(1)]), py::cast<float>(obj[py::int_(2)])};
}

// Generic bind function for both vec2 and vec3 proxies
template<typename VecType>
void bind_vector_proxy(py::module_& m, const char* name) {
    using Proxy = VectorPropertyProxy<VecType>;
    constexpr int N = Proxy::size;

    auto proxy = py::class_<Proxy>(m, name, py::module_local())
        .def_property("x", &Proxy::get_x, &Proxy::set_x)
        .def_property("y", &Proxy::get_y, &Proxy::set_y)
        .def("__len__",     [](const Proxy&) { return N; })
        .def("__getitem__", [](const Proxy& p, py::ssize_t i) {
            if (i >= 0 && i < N) {
                auto vec = p.get();
                return reinterpret_cast<const float*>(&vec)[i];
            }
            throw py::index_error("index out of range");
        })
        .def("__setitem__", [](Proxy& p, py::ssize_t i, float v) {
            if (i < 0 || i >= N) throw py::index_error("index out of range");
            auto vec = p.get();
            reinterpret_cast<float*>(&vec)[i] = v;
            p.set(vec);
        })
        .def("__iter__", [N](const Proxy& p) {
            py::list items;
            auto vec = p.get();
            for (int i = 0; i < N; ++i) items.append(reinterpret_cast<const float*>(&vec)[i]);
            return py::iter(items);
        })
        .def("__setattr__", [](Proxy& p, const std::string& attr, py::object value) {
            auto vec = p.get();
            float* data = reinterpret_cast<float*>(&vec);
            if      (attr == "x") data[0] = py::cast<float>(value);
            else if (attr == "y") data[1] = py::cast<float>(value);
            else if (N == 3 && attr == "z") data[2] = py::cast<float>(value);
            else throw py::attribute_error("Unknown attribute");
            p.set(vec);
        })
        .def("__add__",      [](const Proxy& p, const VecType& o) { return p + o; })
        .def("__add__",      [](const Proxy& p, py::object o)     { return p + o; })
        .def("__sub__",      [](const Proxy& p, const VecType& o) { return p - o; })
        .def("__sub__",      [](const Proxy& p, py::object o)     { return p - o; })
        .def("__mul__",      [](const Proxy& p, float s)          { return p * s; })
        .def("__mul__",      [](const Proxy& p, const VecType& o) { return p * o; })
        .def("__mul__",      [](const Proxy& p, py::object o)     { return p * o; })
        .def("__truediv__",  [](const Proxy& p, float s)          { return p / s; })
        .def("__truediv__",  [](const Proxy& p, const VecType& o) { return p / o; })
        .def("__truediv__",  [](const Proxy& p, py::object o)     { return p / o; })
        .def("__radd__",     [](const Proxy& p, const VecType& o) { return o + p.get(); })
        .def("__radd__",     [](const Proxy& p, py::object o)     {
            if (py::len(o) == N) {
                VecType ov;
                for (int i = 0; i < N; ++i) reinterpret_cast<float*>(&ov)[i] = py::cast<float>(o[py::int_(i)]);
                return ov + p.get();
            }
            throw py::type_error("Unsupported type for reverse addition");
        })
        .def("__rsub__",     [](const Proxy& p, const VecType& o) { return o - p.get(); })
        .def("__rsub__",     [](const Proxy& p, py::object o)     {
            if (py::len(o) == N) {
                VecType ov; for (int i=0;i<N;++i) reinterpret_cast<float*>(&ov)[i]=py::cast<float>(o[py::int_(i)]);
                return ov - p.get();
            }
            throw py::type_error("Unsupported type for reverse subtraction");
        })
        .def("__rmul__",     [](const Proxy& p, float s)          { return p * s; })
        .def("__rmul__",     [](const Proxy& p, py::object s)     {
            if (py::isinstance<py::float_>(s) || py::isinstance<py::int_>(s)) return p * py::cast<float>(s);
            throw py::type_error("Unsupported type for reverse multiplication");
        })
        .def("__iadd__",     [](Proxy& p, const VecType& o) { return p += o; })
        .def("__iadd__",     [](Proxy& p, py::object o)     { return p += o; })
        .def("__isub__",     [](Proxy& p, const VecType& o) { return p -= o; })
        .def("__isub__",     [](Proxy& p, py::object o)     { return p -= o; })
        .def("__imul__",     [](Proxy& p, float s)          { return p *= s; })
        .def("__imul__",     [](Proxy& p, const VecType& o) { return p *= o; })
        .def("__imul__",     [](Proxy& p, py::object o)     { return p *= o; })
        .def("__itruediv__", [](Proxy& p, float s)          { return p /= s; })
        .def("__itruediv__", [](Proxy& p, const VecType& o) { return p /= o; })
        .def("__itruediv__", [](Proxy& p, py::object o)     { return p /= o; });

    // Add z property only for vec3
    if constexpr (N == 3) {
        proxy.def_property("z", &Proxy::get_z, &Proxy::set_z);
    }
}



#endif // BSK_VECTOR_PROXY_H