#include <pybind11/pybind11.h>

#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_motor(py::module_& m) {
    py::class_<Motor, Force>(m, "Motor")
        .def(py::init<Solver*, Rigid*, Rigid*, float, float>(),
             py::arg("solver"),
             py::arg("bodyA"),
             py::arg("bodyB"),
             py::arg("speed"),
             py::arg("maxTorque"))
        
        // Getters
        .def("getSpeed", &Motor::getSpeed)
        
        // Setters
        .def("setSpeed", &Motor::setSpeed);
}