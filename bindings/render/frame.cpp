#include <pybind11/pybind11.h>
#include <basilisk/render/frame.h>
#include <basilisk/engine/engine.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_frame(py::module_& m) {
    py::class_<Frame>(m, "Frame")
        .def(py::init<Engine*, unsigned int, unsigned int>(), py::arg("engine"), py::arg("width")=800, py::arg("height")=800)
        .def(py::init<Engine*, Shader*, unsigned int, unsigned int>(), py::arg("engine"), py::arg("shader"), py::arg("width")=800, py::arg("height")=800)
        .def("use", &Frame::use)
        .def("clear", &Frame::clear, py::arg("r") = 0.0f, py::arg("g") = 0.0f, py::arg("b") = 0.0f, py::arg("a") = 1.0f)
        .def("render", py::overload_cast<>(&Frame::render))
        .def("render", py::overload_cast<int, int, int, int>(&Frame::render), py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"))
        .def("get_shader", &Frame::getShader, py::return_value_policy::reference_internal)
        .def("get_vbo", &Frame::getVBO, py::return_value_policy::reference_internal)
        .def("get_ebo", &Frame::getEBO, py::return_value_policy::reference_internal)
        .def("get_vao", &Frame::getVAO, py::return_value_policy::reference_internal)
        .def("get_fbo", &Frame::getFBO, py::return_value_policy::reference_internal)
        .def("get_width", &Frame::getWidth)
        .def("get_height", &Frame::getHeight)
        .def("get_aspect_ratio", &Frame::getAspectRatio)
        .def("get_render_width", &Frame::getRenderWidth)
        .def("get_render_height", &Frame::getRenderHeight)
        .def("set_filter_linear", &Frame::setFilterLinear)
        .def("set_filter_nearest", &Frame::setFilterNearest)
        .def("get_image", &Frame::getImage)
        .def("get_texture", &Frame::getTexture, py::return_value_policy::take_ownership);
}