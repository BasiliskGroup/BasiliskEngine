#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/solver.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_collider(py::module_& m) {
    py::class_<Collider>(m, "Collider")
        .def(py::init<Solver*, std::vector<glm::vec2>>(),
             py::arg("solver"),
             py::arg("vertices"))
        
        // Getters
        .def("getVertices", &Collider::getVertices, py::return_value_policy::reference_internal)
        .def("getMass", &Collider::getMass)
        .def("getMoment", &Collider::getMoment)
        .def("getRadius", &Collider::getRadius)
        .def("getCOM", &Collider::getCOM)
        .def("getGC", &Collider::getGC)
        .def("getHalfDim", &Collider::getHalfDim)
        .def("getArea", &Collider::getArea)
        .def("getBaseMoment", &Collider::getBaseMoment)
        .def("getBaseRadius", &Collider::getBaseRadius)
        
        // Setters
        .def("setVertices", &Collider::setVertices)
        .def("setCOM", &Collider::setCOM)
        .def("setGC", &Collider::setGC)
        .def("setHalfDim", &Collider::setHalfDim)
        .def("setArea", &Collider::setArea)
        .def("setBaseMoment", &Collider::setBaseMoment);
}