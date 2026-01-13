#include <pybind11/pybind11.h>

#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_spring(py::module_& m) {
    py::class_<Spring, Force>(m, "Spring")
        .def(py::init<Solver*, Rigid*, Rigid*, glm::vec2, glm::vec2, float, float>(),
             py::arg("solver"),
             py::arg("bodyA"),
             py::arg("bodyB"),
             py::arg("rA"),
             py::arg("rB"),
             py::arg("stiffness"),
             py::arg("rest") = -1.0f)
        
        // Getters
        .def("getRA", &Spring::getRA)
        .def("getRB", &Spring::getRB)
        .def("getRest", &Spring::getRest)
        
        // Setters
        .def("setRA", &Spring::setRA)
        .def("setRB", &Spring::setRB)
        .def("setRest", &Spring::setRest);
}