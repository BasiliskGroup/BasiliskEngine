#include <pybind11/pybind11.h>
#include <basilisk/IO/mouse.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_mouse(py::module_& m) {
    py::class_<Mouse>(m, "Mouse")
        .def("update", &Mouse::update)
        .def("getX", &Mouse::getX)
        .def("getY", &Mouse::getY)
        .def("getRelativeX", &Mouse::getRelativeX)
        .def("getRelativeY", &Mouse::getRelativeY)
        .def("getWorldX", &Mouse::getWorldX)
        .def("getWorldY", &Mouse::getWorldY)
        .def("getClicked", &Mouse::getClicked)
        .def("getLeftClicked", &Mouse::getLeftClicked)
        .def("getMiddleClicked", &Mouse::getMiddleClicked)
        .def("getRightClicked", &Mouse::getRightClicked)
        .def("getLeftReleased", &Mouse::getLeftReleased)
        .def("getMiddleReleased", &Mouse::getMiddleReleased)
        .def("getRightReleased", &Mouse::getRightReleased)
        .def("getLeftDown", &Mouse::getLeftDown)
        .def("getMiddleDown", &Mouse::getMiddleDown)
        .def("getRightDown", &Mouse::getRightDown)
        .def("setGrab", &Mouse::setGrab)
        .def("setVisible", &Mouse::setVisible)
        .def("setHidden", &Mouse::setHidden);
}