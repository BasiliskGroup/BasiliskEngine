#include <pybind11/pybind11.h>
#include <basilisk/scene/scene2d.h>
#include <basilisk/engine/engine.h>
#include <basilisk/nodes/node2d.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_scene2d(py::module_& m) {
    py::class_<Scene2D>(m, "Scene2D")
        .def(py::init<Engine*>(), py::arg("engine"))
        .def("update", &Scene2D::update)
        .def("render", &Scene2D::render)
        .def("add", &Scene2D::add, py::arg("node"))
        .def("remove", &Scene2D::remove, py::arg("node"))
        .def("set_camera", &Scene2D::setCamera, py::arg("camera"))
        .def("get_camera", &Scene2D::getCamera)
        .def("get_shader", &Scene2D::getShader)
        .def("get_engine", &Scene2D::getEngine)
        .def("get_root", &Scene2D::getRoot)
        .def("get_solver", &Scene2D::getSolver);
}