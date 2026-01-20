#include <pybind11/pybind11.h>
#include <basilisk/light/ambientLight.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

void bind_ambientLight(py::module_& m) {
    py::class_<AmbientLight, Light>(m, "AmbientLight")
        .def(py::init<glm::vec3, float>(), py::arg("color") = glm::vec3(1.0, 1.0, 1.0), py::arg("intensity") = 1.0f);
}