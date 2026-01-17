#include <pybind11/pybind11.h>
#include <basilisk/render/shader.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

void bind_shader(py::module_& m) {
    py::class_<Shader>(m, "Shader")
        .def(py::init<const char*, const char*>(), py::arg("vertexPath"), py::arg("fragmentPath"))
        .def("use", &Shader::use)
        .def("bind", py::overload_cast<const char*, Texture*, unsigned int>(&Shader::bind), py::arg("name"), py::arg("texture"), py::arg("slot"))
        .def("bind", py::overload_cast<const char*, TextureArray*, unsigned int>(&Shader::bind), py::arg("name"), py::arg("textureArray"), py::arg("slot"))
        .def("bind", py::overload_cast<const char*, TBO*, unsigned int>(&Shader::bind), py::arg("name"), py::arg("tbo"), py::arg("slot"))
        .def("bind", py::overload_cast<const char*, FBO*, unsigned int>(&Shader::bind), py::arg("name"), py::arg("fbo"), py::arg("slot"))
        .def("getUniformLocation", &Shader::getUniformLocation, py::arg("name"))
        .def("getStride", &Shader::getStride)
        .def("getAttributes", &Shader::getAttributes)
        .def("setUniform", py::overload_cast<const char*, float>(&Shader::setUniform), py::arg("name"), py::arg("value"))
        .def("setUniform", py::overload_cast<const char*, double>(&Shader::setUniform), py::arg("name"), py::arg("value"))
        .def("setUniform", py::overload_cast<const char*, int>(&Shader::setUniform), py::arg("name"), py::arg("value"))
        .def("setUniform", py::overload_cast<const char*, glm::vec3>(&Shader::setUniform), py::arg("name"), py::arg("value"))
        .def("setUniform", py::overload_cast<const char*, glm::mat4>(&Shader::setUniform), py::arg("name"), py::arg("value"));
}