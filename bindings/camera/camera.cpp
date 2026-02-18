#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/camera/staticCamera2d.h>
#include <basilisk/camera/camera2d.h>
#include <basilisk/engine/engine.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

namespace {

const glm::vec2 default_position(0.0f, 0.0f);
const float default_scale = 10.0f;

// Proxy for mutable position property (camera.position.x += 1, etc.)
class CameraPositionProxy {
public:
    StaticCamera2D* camera;
    float get_x() const { return static_cast<float>(camera->getX()); }
    void set_x(float v) { camera->setX(static_cast<double>(v)); }
    float get_y() const { return static_cast<float>(camera->getY()); }
    void set_y(float v) { camera->setY(static_cast<double>(v)); }
};

// Proxy for mutable view_scale property (camera.view_scale.x, etc.)
class CameraViewScaleProxy {
public:
    StaticCamera2D* camera;
    float get_x() const { return static_cast<float>(camera->getViewWidth()); }
    void set_x(float v) {
        float y = static_cast<float>(camera->getViewHeight());
        camera->setScale(v, y);
    }
    float get_y() const { return static_cast<float>(camera->getViewHeight()); }
    void set_y(float v) {
        float x = static_cast<float>(camera->getViewWidth());
        camera->setScale(x, v);
    }
};

}  // namespace

void bind_camera(py::module_& m) {
    // Bind proxy: in-place modification + tuple/PyGLM compat (glm.vec2(*pos), glm.mix(...))
    py::class_<CameraPositionProxy>(m, "_CameraPositionProxy", py::module_local())
        .def_property("x", &CameraPositionProxy::get_x, &CameraPositionProxy::set_x)
        .def_property("y", &CameraPositionProxy::get_y, &CameraPositionProxy::set_y)
        .def("__iter__", [](const CameraPositionProxy& p) {
            return py::iter(py::make_tuple(p.get_x(), p.get_y()));
        })
        .def("__len__", [](const CameraPositionProxy&) { return 2; })
        .def("__getitem__", [](const CameraPositionProxy& p, py::ssize_t i) {
            if (i == 0) return p.get_x();
            if (i == 1) return p.get_y();
            throw py::index_error("index out of range");
        });

    py::class_<CameraViewScaleProxy>(m, "_CameraViewScaleProxy", py::module_local())
        .def_property("x", &CameraViewScaleProxy::get_x, &CameraViewScaleProxy::set_x)
        .def_property("y", &CameraViewScaleProxy::get_y, &CameraViewScaleProxy::set_y);

    py::class_<StaticCamera2D>(m, "StaticCamera2D")
        .def(py::init<Engine*, glm::vec2, float>(),
             py::arg("engine"),
             py::arg("position") = default_position,
             py::arg("scale") = default_scale)
        .def("update", &StaticCamera2D::update)
        .def("use", &StaticCamera2D::use, py::arg("shader"))
        // Setters
        .def("set_position", &StaticCamera2D::setPosition, py::arg("position"))
        .def("set_x", &StaticCamera2D::setX, py::arg("x"))
        .def("set_y", &StaticCamera2D::setY, py::arg("y"))
        .def("set_scale", py::overload_cast<float>(&StaticCamera2D::setScale), py::arg("scale"))
        .def("set_scale", py::overload_cast<float, float>(&StaticCamera2D::setScale), py::arg("x_scale"), py::arg("y_scale"))
        .def("set_scale", py::overload_cast<glm::vec2>(&StaticCamera2D::setScale), py::arg("view_scale"))
        // Getters
        .def("get_position", &StaticCamera2D::getPosition)
        .def("get_x", &StaticCamera2D::getX)
        .def("get_y", &StaticCamera2D::getY)
        .def("get_view_scale", &StaticCamera2D::getViewScale)
        .def("get_view_width", &StaticCamera2D::getViewWidth)
        .def("get_view_height", &StaticCamera2D::getViewHeight)
        // Properties (mutable: camera.position.x += 1, camera.position = (x,y), etc.)
        .def_property("position",
            [](StaticCamera2D& c) {
                auto p = std::make_unique<CameraPositionProxy>();
                p->camera = &c;
                return p.release();
            },
            [](StaticCamera2D& c, const glm::vec2& v) { c.setPosition(v); },
            py::return_value_policy::take_ownership)
        .def_property_readonly("pos", [](StaticCamera2D& c) { return c.getPosition(); })
        .def_property_readonly("view_scale",
            [](StaticCamera2D& c) {
                auto p = std::make_unique<CameraViewScaleProxy>();
                p->camera = &c;
                return p.release();
            },
            py::return_value_policy::take_ownership)
        .def_property("scale",
            [](StaticCamera2D& c) { return static_cast<float>(c.getViewWidth()); },
            [](StaticCamera2D& c, float v) { c.setScale(v); });

    py::class_<Camera2D, StaticCamera2D>(m, "Camera2D")
        .def(py::init<Engine*, glm::vec2, float>(),
             py::arg("engine"),
             py::arg("position") = default_position,
             py::arg("scale") = default_scale)
        .def("update", &Camera2D::update)
        .def_property("speed",
            [](const Camera2D& c) { return c.getSpeed(); },
            [](Camera2D& c, float v) { c.setSpeed(v); });
}
