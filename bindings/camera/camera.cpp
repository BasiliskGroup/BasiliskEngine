#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/camera/staticCamera2d.h>
#include <basilisk/camera/camera2d.h>
#include <basilisk/camera/staticCamera.h>
#include <basilisk/camera/camera.h>
#include <basilisk/engine/engine.h>

// IMPORTANT: include GLM casters (for glm::vec2/vec3, matrices, etc.)
#include "glm/glmCasters.hpp"
#include "vector_proxy.h"

namespace py = pybind11;
using namespace bsk::internal;

namespace {

const glm::vec2 default_position_2d(0.0f, 0.0f);
const float default_scale_2d = 10.0f;

const glm::vec3 default_position_3d(0.0f, 0.0f, 0.0f);
const float default_pitch_3d = 0.0f;
const float default_yaw_3d = 0.0f;

}  // namespace

void bind_camera(py::module_& m) {
    // 2D cameras ---------------------------------------------------------
    py::class_<StaticCamera2D, std::shared_ptr<StaticCamera2D>>(m, "StaticCamera2D")
        .def(py::init<Engine*, glm::vec2, float>(),
             py::arg("engine"),
             py::arg("position") = default_position_2d,
             py::arg("scale") = default_scale_2d)
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
                return new Vec2Proxy(
                    [&c]() { return c.getPosition(); },
                    [&c](const glm::vec2& v) { c.setPosition(v); }
                );
            },
            [](StaticCamera2D& c, py::object value) { c.setPosition(vec2_from_pyobject(value)); },
            py::return_value_policy::take_ownership)
        .def_property_readonly("pos", [](StaticCamera2D& c) { return c.getPosition(); })
        .def_property("view_scale",
            [](StaticCamera2D& c) {
                return new Vec2Proxy(
                    [&c]() { return c.getViewScale(); },
                    [&c](const glm::vec2& v) { c.setScale(v); }
                );
            },
            [](StaticCamera2D& c, py::object value) { c.setScale(vec2_from_pyobject(value)); },
            py::return_value_policy::take_ownership)
        .def_property("scale",
            [](StaticCamera2D& c) { return static_cast<float>(c.getViewWidth()); },
            [](StaticCamera2D& c, float v) { c.setScale(v); });

    py::class_<Camera2D, StaticCamera2D, std::shared_ptr<Camera2D>>(m, "Camera2D")
        .def(py::init<Engine*, glm::vec2, float>(),
             py::arg("engine"),
             py::arg("position") = default_position_2d,
             py::arg("scale") = default_scale_2d)
        .def("update", &Camera2D::update)
        .def_property("speed",
            [](const Camera2D& c) { return c.getSpeed(); },
            [](Camera2D& c, float v) { c.setSpeed(v); });

    // 3D cameras ---------------------------------------------------------
    py::class_<StaticCamera, std::shared_ptr<StaticCamera>>(m, "StaticCamera")
        .def(py::init<Engine*, glm::vec3, float, float>(),
             py::arg("engine"),
             py::arg("position") = default_position_3d,
             py::arg("pitch") = default_pitch_3d,
             py::arg("yaw") = default_yaw_3d)
        .def("update", &StaticCamera::update)
        .def("use", &StaticCamera::use, py::arg("shader"))
        // Setters
        .def("set_position", &StaticCamera::setPosition, py::arg("position"))
        .def("set_x", &StaticCamera::setX, py::arg("x"))
        .def("set_y", &StaticCamera::setY, py::arg("y"))
        .def("set_z", &StaticCamera::setZ, py::arg("z"))
        .def("set_yaw", &StaticCamera::setYaw, py::arg("yaw"))
        .def("set_pitch", &StaticCamera::setPitch, py::arg("pitch"))
        .def("set_fov", &StaticCamera::setFOV, py::arg("fov"))
        .def("set_aspect", &StaticCamera::setAspect, py::arg("aspect"))
        .def("set_near", &StaticCamera::setNear, py::arg("near"))
        .def("set_far", &StaticCamera::setFar, py::arg("far"))
        .def("look_at", &StaticCamera::lookAt, py::arg("target"))
        // Getters
        .def("get_position", &StaticCamera::getPosition)
        .def("get_x", &StaticCamera::getX)
        .def("get_y", &StaticCamera::getY)
        .def("get_z", &StaticCamera::getZ)
        .def("get_yaw", &StaticCamera::getYaw)
        .def("get_pitch", &StaticCamera::getPitch)
        .def("get_fov", &StaticCamera::getFOV)
        .def("get_aspect", &StaticCamera::getAspect)
        .def("get_near", &StaticCamera::getNear)
        .def("get_far", &StaticCamera::getFar);

    py::class_<Camera, StaticCamera, std::shared_ptr<Camera>>(m, "Camera")
        .def(py::init<Engine*, glm::vec3, float, float>(),
             py::arg("engine"),
             py::arg("position") = default_position_3d,
             py::arg("pitch") = default_pitch_3d,
             py::arg("yaw") = default_yaw_3d)
        .def("update", &Camera::update)
        .def_property("speed",
            [](const Camera& c) { return c.getSpeed(); },
            [](Camera& c, float v) { c.setSpeed(v); });
}
