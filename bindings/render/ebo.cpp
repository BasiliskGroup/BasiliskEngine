#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/ebo.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_ebo(py::module_& m) {
    py::class_<EBO>(m, "EBO")
        .def(py::init([](const std::vector<unsigned int>& data, unsigned int drawType) {
            return new EBO(data, drawType);
        }), py::arg("data"), py::arg("drawType") = GL_STATIC_DRAW)
        .def("bind", &EBO::bind)
        .def("unbind", &EBO::unbind)
        .def("getSize", &EBO::getSize)
        .def("write", static_cast<void (EBO::*)(const void*, unsigned int, unsigned int)>(&EBO::write), py::arg("data"), py::arg("size"), py::arg("offset"));
}