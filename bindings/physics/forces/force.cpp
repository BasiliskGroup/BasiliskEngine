#include <pybind11/pybind11.h>

#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_force(py::module_& m) {
    // Force is an abstract base class - cannot be instantiated directly
    // Only derived classes (Joint, Spring, Motor, Manifold) can be instantiated
    py::class_<Force>(m, "Force")
        .def("disable", &Force::disable)
        
        // Getters
        .def("getSolver", &Force::getSolver)
        .def("getBodyA", &Force::getBodyA)
        .def("getBodyB", &Force::getBodyB)
        .def("getNext", &Force::getNext)
        .def("getNextA", &Force::getNextA)
        .def("getNextB", &Force::getNextB)
        .def("getPrev", &Force::getPrev)
        .def("getPrevA", &Force::getPrevA)
        .def("getPrevB", &Force::getPrevB)
        .def("getJ", &Force::getJ)
        .def("getH", &Force::getH)
        .def("getC", &Force::getC)
        .def("getFmin", &Force::getFmin)
        .def("getFmax", &Force::getFmax)
        .def("getStiffness", &Force::getStiffness)
        .def("getFracture", &Force::getFracture)
        .def("getPenalty", &Force::getPenalty)
        .def("getLambda", &Force::getLambda);
}