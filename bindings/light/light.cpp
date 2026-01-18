#include <pybind11/pybind11.h>
#include <basilisk/light/light.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

void bind_light(py::module_& m) {
    py::class_<Light>(m, "Light")
        .def("getColor", &Light::getColor)
        .def("getIntensity", &Light::getIntensity)
        .def("setColor", &Light::setColor, py::arg("color"))
        .def("setIntensity", &Light::setIntensity, py::arg("intensity"));
}

