#include <pybind11/pybind11.h>
#include <basilisk/render/material.h>
#include <glm/glm.hpp>

namespace py = pybind11;

void bind_material(py::module_& m) {
    py::class_<bsk::internal::Material>(m, "Material")
        .def(py::init([](py::tuple color, bsk::internal::Image* albedo, bsk::internal::Image* normal) {
            if (py::len(color) != 3) {
                throw std::runtime_error("Color tuple must have exactly 3 elements (r, g, b)");
            }
            glm::vec3 colorVec(
                py::cast<float>(color[0]),
                py::cast<float>(color[1]),
                py::cast<float>(color[2])
            );
            return new bsk::internal::Material(colorVec, albedo, normal);
        }))
        .def("getColor", &bsk::internal::Material::getColor)
        .def("getAlbedo", &bsk::internal::Material::getAlbedo)
        .def("getNormal", &bsk::internal::Material::getNormal);
}