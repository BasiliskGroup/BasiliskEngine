#include <pybind11/pybind11.h>
#include <basilisk/IO/keyboard.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_keyboard(py::module_& m) {
    py::class_<Keyboard>(m, "Keyboard")
        .def("update", &Keyboard::update)
        .def("get_down", &Keyboard::getDown, py::arg("keyCode"))
        .def("get_pressed", &Keyboard::getPressed, py::arg("keyCode"))
        .def("get_released", &Keyboard::getReleased, py::arg("keyCode"));
}