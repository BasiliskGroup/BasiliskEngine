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
    py::class_<Contact>(m, "Contact")
        .def_readwrite("feature", &Contact::feature)
        .def_readwrite("rA", &Contact::rA)
        .def_readwrite("rB", &Contact::rB)
        .def_readwrite("normal", &Contact::normal)
        .def_readwrite("C0", &Contact::C0)
        .def_readwrite("stick", &Contact::stick);
    
    // Bind the FeaturePair union
    py::class_<FeaturePair>(m, "FeaturePair")
        .def_readwrite("e", &FeaturePair::e)
        .def_readwrite("value", &FeaturePair::value);
    
    // Bind the Edges struct
    py::class_<FeaturePair::Edges>(m, "Edges")
        .def_readwrite("inEdge1", &FeaturePair::Edges::inEdge1)
        .def_readwrite("outEdge1", &FeaturePair::Edges::outEdge1)
        .def_readwrite("inEdge2", &FeaturePair::Edges::inEdge2)
        .def_readwrite("outEdge2", &FeaturePair::Edges::outEdge2);
    
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