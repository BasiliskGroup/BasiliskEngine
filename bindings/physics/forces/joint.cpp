#include <pybind11/pybind11.h>
#include <cmath>

#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_joint(py::module_& m) {
    py::class_<Joint, Force>(m, "Joint")
        .def(py::init<Solver*, Rigid*, Rigid*, glm::vec2, glm::vec2, glm::vec3, float>(),
             py::arg("solver"),
             py::arg("bodyA"),
             py::arg("bodyB"),
             py::arg("rA"),
             py::arg("rB"),
             py::arg("stiffness") = glm::vec3{INFINITY, INFINITY, INFINITY},
             py::arg("fracture") = INFINITY)
        
        // Getters
        .def("getRA", &Joint::getRA)
        .def("getRB", &Joint::getRB)
        .def("getC0", &Joint::getC0)
        .def("getTorqueArm", &Joint::getTorqueArm)
        .def("getRestAngle", &Joint::getRestAngle)
        
        // Setters
        .def("setRA", &Joint::setRA)
        .def("setRB", &Joint::setRB)
        .def("setC0", &Joint::setC0)
        .def("setTorqueArm", &Joint::setTorqueArm)
        .def("setRestAngle", &Joint::setRestAngle);
}