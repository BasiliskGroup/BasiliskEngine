#include <pybind11/pybind11.h>
#include <basilisk/IO/mouse.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_mouse(py::module_& m) {
    py::class_<Mouse>(m, "Mouse")
        .def("update", &Mouse::update)
        .def("get_x", &Mouse::getX)
        .def("get_y", &Mouse::getY)
        .def("get_relative_x", &Mouse::getRelativeX)
        .def("get_relative_y", &Mouse::getRelativeY)
        .def("get_world_x", &Mouse::getWorldX)
        .def("get_world_y", &Mouse::getWorldY)
        .def("get_clicked", &Mouse::getClicked)
        .def("get_left_clicked", &Mouse::getLeftClicked)
        .def("get_middle_clicked", &Mouse::getMiddleClicked)
        .def("get_right_clicked", &Mouse::getRightClicked)
        .def("get_left_released", &Mouse::getLeftReleased)
        .def("get_middle_released", &Mouse::getMiddleReleased)
        .def("get_right_released", &Mouse::getRightReleased)
        .def("get_left_down", &Mouse::getLeftDown)
        .def("get_middle_down", &Mouse::getMiddleDown)
        .def("get_right_down", &Mouse::getRightDown)
        .def("set_grab", &Mouse::setGrab)
        .def("set_visible", &Mouse::setVisible)
        .def("set_hidden", &Mouse::setHidden);
}