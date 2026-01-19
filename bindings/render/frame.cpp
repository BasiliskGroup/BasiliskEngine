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
        .def("get_shader", &Frame::getShader)
        .def("get_vbo", &Frame::getVBO)
        .def("get_ebo", &Frame::getEBO)
        .def("get_vao", &Frame::getVAO)
        .def("get_fbo", &Frame::getFBO)
        .def("get_width", &Frame::getWidth)
        .def("get_height", &Frame::getHeight)
        .def("get_aspect_ratio", &Frame::getAspectRatio)
        .def("get_render_width", &Frame::getRenderWidth)
        .def("get_render_height", &Frame::getRenderHeight);
}