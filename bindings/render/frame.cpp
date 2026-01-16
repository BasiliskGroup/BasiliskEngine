#include <pybind11/pybind11.h>
#include <basilisk/render/frame.h>
#include <basilisk/engine/engine.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_frame(py::module_& m) {
    py::class_<Frame>(m, "Frame")
        .def(py::init<Engine*, unsigned int, unsigned int>(), py::arg("engine"), py::arg("width"), py::arg("height"))
        .def("use", &Frame::use)
        .def("clear", &Frame::clear, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a"))
        .def("render", py::overload_cast<>(&Frame::render))
        .def("render", py::overload_cast<int, int, int, int>(&Frame::render), py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"))
        .def("getShader", &Frame::getShader)
        .def("getVBO", &Frame::getVBO)
        .def("getEBO", &Frame::getEBO)
        .def("getVAO", &Frame::getVAO)
        .def("getFBO", &Frame::getFBO)
        .def("getWidth", &Frame::getWidth)
        .def("getHeight", &Frame::getHeight)
        .def("getAspectRatio", &Frame::getAspectRatio)
        .def("getRenderWidth", &Frame::getRenderWidth)
        .def("getRenderHeight", &Frame::getRenderHeight);
}