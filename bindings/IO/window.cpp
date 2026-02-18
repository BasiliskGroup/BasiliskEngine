#include <pybind11/pybind11.h>
#include <basilisk/IO/window.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_window(py::module_& m) {
    py::class_<Window>(m, "Window")
        .def("is_running", &Window::isRunning)
        .def("render", &Window::render)
        .def("clear", &Window::clear,
             py::arg("r") = 0.0f, py::arg("g") = 0.0f, py::arg("b") = 0.0f, py::arg("a") = 1.0f)
        .def("use", &Window::use)
        .def("get_width", &Window::getWidth)
        .def("get_height", &Window::getHeight)
        .def("get_window_scale_x", &Window::getWindowScaleX)
        .def("get_window_scale_y", &Window::getWindowScaleY)
        .def("get_delta_time", &Window::getDeltaTime)
        .def("get_time", &Window::getTime)
        .def("get_fps", &Window::getFPS)
        .def("enable_depth_test", &Window::enableDepthTest)
        .def("enable_cull_face", &Window::enableCullFace)
        .def("enable_multisample", &Window::enableMultisample)
        .def("enable_blend", &Window::enableBlend)
        .def("enable_v_sync", &Window::enableVSync)
        .def("disable_depth_test", &Window::disableDepthTest)
        .def("disable_cull_face", &Window::disableCullFace)
        .def("disable_multisample", &Window::disableMultisample)
        .def("disable_blend", &Window::disableBlend)
        .def("disable_v_sync", &Window::disableVSync);
}
