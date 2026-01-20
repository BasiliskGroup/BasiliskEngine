#include <pybind11/pybind11.h>
#include <basilisk/light/light.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

void bind_light(py::module_& m) {
    py::class_<Light>(m, "Light")
        .def("get_color", &Light::getColor)
        .def("get_intensity", &Light::getIntensity)
        .def("set_color", &Light::setColor, py::arg("color"))
        .def("set_intensity", &Light::setIntensity, py::arg("intensity"));
}

