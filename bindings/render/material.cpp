#include <memory>
#include <pybind11/pybind11.h>

#include <basilisk/render/material.h>
#include "glm/glmCasters.hpp"
#include "vector_proxy.h"

namespace py = pybind11;
using namespace bsk::internal;

void bind_material(py::module_& m) {
    py::class_<Material, std::shared_ptr<Material>>(m, "Material")
        .def(py::init([](py::object color_obj, Image* albedo, Image* normal, float alpha, float subsurface, float sheen, float sheenTint, float anisotropic, float specular, float metallicness, float clearcoat, float clearcoatGloss) {
            glm::vec3 color = color_obj.is_none() ? glm::vec3{1.0f, 1.0f, 1.0f} : vec3_from_pyobject(color_obj);
            return new Material(color, albedo, normal, alpha, subsurface, sheen, sheenTint, anisotropic, specular, metallicness, clearcoat, clearcoatGloss);
        }),
        py::arg("color") = py::none(),
        py::arg("albedo") = nullptr,
        py::arg("normal") = nullptr,
        py::arg("alpha") = 1.0f,
        py::arg("subsurface") = 0.0f,
        py::arg("sheen") = 0.0f,
        py::arg("sheen_tint") = 0.0f,
        py::arg("anisotropic") = 0.0f,
        py::arg("specular") = 0.75f,
        py::arg("metallicness") = 0.0f,
        py::arg("clearcoat") = 0.0f,
        py::arg("clearcoat_gloss") = 0.0f)

        // Getters
        .def("get_color", &Material::getColor)
        .def("get_albedo", &Material::getAlbedo)
        .def("get_normal", &Material::getNormal)
        .def("get_alpha", &Material::getAlpha)
        .def("get_roughness", &Material::getRoughness)
        .def("get_subsurface", &Material::getSubsurface)
        .def("get_sheen", &Material::getSheen)
        .def("get_sheen_tint", &Material::getSheenTint)
        .def("get_anisotropic", &Material::getAnisotropic)
        .def("get_specular", &Material::getSpecular)
        .def("get_metallicness", &Material::getMetallicness)
        .def("get_clearcoat", &Material::getClearcoat)
        .def("get_clearcoat_gloss", &Material::getClearcoatGloss)

        // Setters
        .def("set_color", &Material::setColor, py::arg("color"))
        .def("set_albedo", py::overload_cast<Image*>(&Material::setAlbedo), py::arg("albedo"))
        .def("set_albedo", py::overload_cast<std::shared_ptr<Image>>(&Material::setAlbedo), py::arg("albedo"))
        .def("set_normal", py::overload_cast<Image*>(&Material::setNormal), py::arg("normal"))
        .def("set_normal", py::overload_cast<std::shared_ptr<Image>>(&Material::setNormal), py::arg("normal"))
        .def("set_alpha", &Material::setAlpha, py::arg("alpha"))
        .def("set_roughness", &Material::setRoughness, py::arg("roughness"))
        .def("set_subsurface", &Material::setSubsurface, py::arg("subsurface"))
        .def("set_sheen", &Material::setSheen, py::arg("sheen"))
        .def("set_sheen_tint", &Material::setSheenTint, py::arg("sheen_tint"))
        .def("set_anisotropic", &Material::setAnisotropic, py::arg("anisotropic"))
        .def("set_specular", &Material::setSpecular, py::arg("specular"))
        .def("set_metallicness", &Material::setMetallicness, py::arg("metallicness"))
        .def("set_clearcoat", &Material::setClearcoat, py::arg("clearcoat"))
        .def("set_clearcoat_gloss", &Material::setClearcoatGloss, py::arg("clearcoat_gloss"));
}