#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/vbo.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_vbo(py::module_& m) {
    py::class_<VBO>(m, "VBO")
        .def(py::init<const void*, unsigned int, unsigned int>(), py::arg("data"), py::arg("size"), py::arg("drawType"))
        .def(py::init([](const std::vector<float>& data, unsigned int drawType) {
            return new VBO(data, drawType);
        }), py::arg("data"), py::arg("drawType") = GL_STATIC_DRAW)
        .def("bind", &VBO::bind)
        .def("unbind", &VBO::unbind)
        .def("getSize", &VBO::getSize);
}