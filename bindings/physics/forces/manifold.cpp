#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_manifold(py::module_& m) {
    // Bind the Contact struct
    py::class_<Manifold::Contact>(m, "Contact")
        .def_readwrite("feature", &Manifold::Contact::feature)
        .def_readwrite("rA", &Manifold::Contact::rA)
        .def_readwrite("rB", &Manifold::Contact::rB)
        .def_readwrite("normal", &Manifold::Contact::normal)
        .def_readwrite("C0", &Manifold::Contact::C0)
        .def_readwrite("stick", &Manifold::Contact::stick);
    
    // Bind the FeaturePair union
    py::class_<Manifold::FeaturePair>(m, "FeaturePair")
        .def_readwrite("e", &Manifold::FeaturePair::e)
        .def_readwrite("value", &Manifold::FeaturePair::value);
    
    // Bind the Edges struct
    py::class_<Manifold::FeaturePair::Edges>(m, "Edges")
        .def_readwrite("inEdge1", &Manifold::FeaturePair::Edges::inEdge1)
        .def_readwrite("outEdge1", &Manifold::FeaturePair::Edges::outEdge1)
        .def_readwrite("inEdge2", &Manifold::FeaturePair::Edges::inEdge2)
        .def_readwrite("outEdge2", &Manifold::FeaturePair::Edges::outEdge2);
    
    py::class_<Manifold, Force>(m, "Manifold")
        .def(py::init<Solver*, Rigid*, Rigid*>(),
             py::arg("solver"),
             py::arg("bodyA"),
             py::arg("bodyB"))
        
        .def_static("collide", &Manifold::collide,
                    py::arg("bodyA"),
                    py::arg("bodyB"),
                    py::arg("contacts"))
        
        // Getters
        .def("getContact", &Manifold::getContact)
        .def("getContactRef", &Manifold::getContactRef)
        .def("getNumContacts", &Manifold::getNumContacts)
        .def("getFriction", &Manifold::getFriction);
}