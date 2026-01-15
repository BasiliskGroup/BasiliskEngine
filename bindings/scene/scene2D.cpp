#include <pybind11/pybind11.h>
#include <basilisk/scene/scene2d.h>
#include <basilisk/engine/engine.h>

namespace py = pybind11;

void bind_scene2d(py::module_& m) {
    py::class_<bsk::internal::Scene2D>(m, "Scene2D")
        .def(py::init<bsk::internal::Engine*>(), py::arg("engine"))
        .def("update", &bsk::internal::Scene2D::update)
        .def("render", &bsk::internal::Scene2D::render)
        .def("setCamera", &bsk::internal::Scene2D::setCamera, py::arg("camera"))
        .def("getShader", &bsk::internal::Scene2D::getShader)
        .def("getCamera", &bsk::internal::Scene2D::getCamera);
}