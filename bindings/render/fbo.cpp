#include <pybind11/pybind11.h>
#include <basilisk/render/fbo.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_fbo(py::module_& m) {
    py::class_<FBO>(m, "FBO")
        .def(py::init<unsigned int, unsigned int, unsigned int>(), py::arg("width"), py::arg("height"), py::arg("components"))
        .def("bind", &FBO::bind)
        .def("unbind", &FBO::unbind)
        .def("clear", &FBO::clear, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a"))
        .def("getID", &FBO::getID)
        .def("getTextureID", &FBO::getTextureID)
        .def("getDepthID", &FBO::getDepthID)
        .def("getWidth", &FBO::getWidth)
        .def("getHeight", &FBO::getHeight);
}