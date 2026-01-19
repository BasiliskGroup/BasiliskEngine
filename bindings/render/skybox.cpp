#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/skybox.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_skybox(py::module_& m) {
    py::class_<Skybox>(m, "Skybox")
        .def(py::init<Cubemap*, bool>(), py::arg("cubemap"), py::arg("ownsCubemap") = false)
        .def(py::init<const std::vector<Image*>&>(), py::arg("faces"))
        .def(py::init<const std::vector<std::string>&>(), py::arg("faces"))
        .def("render", &Skybox::render, py::arg("camera"))
        .def("getCubemap", &Skybox::getCubemap)
        .def("getShader", &Skybox::getShader)
        .def("getVBO", &Skybox::getVBO)
        .def("getVAO", &Skybox::getVAO);
}