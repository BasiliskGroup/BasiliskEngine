#include <pybind11/pybind11.h>

#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_joint(py::module_&);
void bind_spring(py::module_&);
void bind_motor(py::module_&);
void bind_manifold(py::module_&);

void bind_force(py::module_& m) {
    // Force is an abstract base class - cannot be instantiated directly
    // Only derived classes (Joint, Spring, Motor, Manifold) can be instantiated
    py::class_<Force>(m, "Force")
        .def("disable", &Force::disable);

    // bind derived classes to the same submodule
    bind_joint(m);
    bind_spring(m);
    bind_motor(m);
    bind_manifold(m);
}