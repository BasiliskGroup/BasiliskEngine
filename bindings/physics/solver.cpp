#include <pybind11/pybind11.h>

#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_solver(py::module_& m) {
    py::class_<Solver>(m, "Solver")
        .def(py::init<>())
        
        // Linked list management - Rigid overloads
        .def("insert", py::overload_cast<Rigid*>(&Solver::insert))
        .def("remove", py::overload_cast<Rigid*>(&Solver::remove))
        
        // Linked list management - Force overloads
        .def("insert", py::overload_cast<Force*>(&Solver::insert))
        .def("remove", py::overload_cast<Force*>(&Solver::remove))
        
        // Getters
        .def("getNumRigids", &Solver::getNumRigids)
        .def("getNumForces", &Solver::getNumForces)
        .def("getGravity", &Solver::getGravity)
        .def("getIterations", &Solver::getIterations)
        .def("getDt", &Solver::getDt)
        .def("getAlpha", &Solver::getAlpha)
        .def("getBeta", &Solver::getBeta)
        .def("getGamma", &Solver::getGamma)
        .def("getPostStabilize", &Solver::getPostStabilize)

        // Setters
        .def("setGravity", &Solver::setGravity)
        .def("setIterations", &Solver::setIterations)
        .def("setDt", &Solver::setDt)
        .def("setAlpha", &Solver::setAlpha)
        .def("setBeta", &Solver::setBeta)
        .def("setGamma", &Solver::setGamma)
        .def("setPostStabilize", &Solver::setPostStabilize);
}