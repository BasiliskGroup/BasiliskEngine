#include <pybind11/pybind11.h>

#include <basilisk/render/material.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_material(py::module_& m) {
    py::class_<Material>(m, "Material")
        .def(py::init([](const glm::vec3& color, Image* albedo, Image* normal) {
            return new Material(color, albedo, normal);
        }),
        py::arg("color") = glm::vec3{1.0f, 1.0f, 1.0f},
        py::arg("albedo") = nullptr,
        py::arg("normal") = nullptr)

        // Getters
        .def("getColor", &Material::getColor)
        .def("getAlbedo", &Material::getAlbedo)
        .def("getNormal", &Material::getNormal);
}