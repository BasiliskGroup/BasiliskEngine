#include <pybind11/pybind11.h>
#include <basilisk/render/vao.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_vao(py::module_& m) {
    py::class_<VAO>(m, "VAO")
        .def(py::init<Shader*, VBO*, EBO*>(), py::arg("shader"), py::arg("vertices"), py::arg("indices"))
        .def("render", &VAO::render, py::arg("instanceCount"))
        .def("bind", &VAO::bind)
        .def("bindAttribute", &VAO::bindAttribute, py::arg("location"), py::arg("count"), py::arg("dataType"), py::arg("stride"), py::arg("offset"), py::arg("divisor"))
        .def("bindAttributes", &VAO::bindAttributes, py::arg("attribs"), py::arg("divisor"))
        .def("bindBuffer", py::overload_cast<VBO*, std::vector<std::string>, unsigned int>(&VAO::bindBuffer), py::arg("buffer"), py::arg("attribs"), py::arg("divisor"))
        .def("bindBuffer", py::overload_cast<VBO*, EBO*, std::vector<std::string>, unsigned int>(&VAO::bindBuffer), py::arg("buffer"), py::arg("indices"), py::arg("attribs"), py::arg("divisor"));
}