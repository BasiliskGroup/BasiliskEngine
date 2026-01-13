#include <pybind11/pybind11.h>
#include <basilisk/render/material.h>
#include "helpers/glm_helpers.h"

namespace py = pybind11;
using namespace bsk::bindings::helpers;

void bind_material(py::module_& m) {
    py::class_<bsk::internal::Material>(m, "Material")
        .def(py::init([](py::tuple color, bsk::internal::Image* albedo, bsk::internal::Image* normal) {
            return new bsk::internal::Material(tuple_to_vec3(color), albedo, normal);
        }))
        .def("getColor", &bsk::internal::Material::getColor)
        .def("getAlbedo", &bsk::internal::Material::getAlbedo)
        .def("getNormal", &bsk::internal::Material::getNormal);
}