#include <pybind11/pybind11.h>
#include <basilisk/light/pointLight.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

void bind_pointLight(py::module_& m) {
    py::class_<PointLight, Light>(m, "PointLight")
        .def(py::init<glm::vec3, float, glm::vec3, float>(), py::arg("color") = glm::vec3(1.0, 1.0, 1.0), py::arg("intensity") = 1.0f, py::arg("position") = glm::vec3(0.0, 0.0, 0.0), py::arg("range") = 15.0f)
        .def("get_position", &PointLight::getPosition)
        .def("get_range", &PointLight::getRange)
        .def("set_position", &PointLight::setPosition, py::arg("position"))
        .def("set_range", &PointLight::setRange, py::arg("range"));
}