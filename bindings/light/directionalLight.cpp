#include <pybind11/pybind11.h>
#include <basilisk/light/directionalLight.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

void bind_directionalLight(py::module_& m) {
    py::class_<DirectionalLight, Light>(m, "DirectionalLight")
        .def(py::init<glm::vec3, float, glm::vec3>(), py::arg("color") = glm::vec3(1.0, 1.0, 1.0), py::arg("intensity") = 1.0f, py::arg("direction") = glm::vec3(0.0, -1.0, 0.2))
        .def("get_direction", &DirectionalLight::getDirection)
        .def("set_direction", &DirectionalLight::setDirection, py::arg("direction"));
}