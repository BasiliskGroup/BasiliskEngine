#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/forces/force.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_rigid(py::module_& m) {
    py::class_<Rigid>(m, "Rigid")
        // Constructor with explicit velocity
        .def(py::init<
            Solver*,
            Node2D*,
            Collider*,
            glm::vec3,
            glm::vec2,
            float,
            float,
            glm::vec3
        >(),
        py::arg("solver"),
        py::arg("node"),
        py::arg("collider"),
        py::arg("position"),
        py::arg("size"),
        py::arg("density"),
        py::arg("friction"),
        py::arg("velocity"))
        
        // Constraint methods
        .def("constrainedTo", &Rigid::constrainedTo)
        
        // Coloring methods
        .def("resetColoring", &Rigid::resetColoring)
        .def("isColored", &Rigid::isColored)
        .def("isColorUsed", &Rigid::isColorUsed)
        .def("getNextUnusedColor", &Rigid::getNextUnusedColor)
        .def("reserveColors", &Rigid::reserveColors)
        .def("useColor", &Rigid::useColor)
        .def("incrSatur", &Rigid::incrSatur)
        .def("verifyColoring", &Rigid::verifyColoring)
        
        // Linked list management
        .def("insert", &Rigid::insert)
        .def("remove", &Rigid::remove)
        
        // Setters
        .def("setPosition", &Rigid::setPosition)
        .def("setScale", &Rigid::setScale)
        .def("setVelocity", &Rigid::setVelocity)
        .def("setInitial", &Rigid::setInitial)
        .def("setInertial", &Rigid::setInertial)
        .def("setPrevVelocity", &Rigid::setPrevVelocity)
        .def("setMass", &Rigid::setMass)
        .def("setMoment", &Rigid::setMoment)
        .def("setFriction", &Rigid::setFriction)
        .def("setRadius", &Rigid::setRadius)
        .def("setCollider", &Rigid::setCollider)
        .def("setNode", &Rigid::setNode)
        
        // Getters
        .def("getPosition", &Rigid::getPosition)
        .def("getInitial", &Rigid::getInitial)
        .def("getInertial", &Rigid::getInertial)
        .def("getVelocity", &Rigid::getVelocity)
        .def("getPrevVelocity", &Rigid::getPrevVelocity)
        .def("getSize", &Rigid::getSize)
        .def("getMass", &Rigid::getMass)
        .def("getMoment", &Rigid::getMoment)
        .def("getFriction", &Rigid::getFriction)
        .def("getRadius", &Rigid::getRadius)
        .def("getColor", &Rigid::getColor)
        .def("getDegree", &Rigid::getDegree)
        .def("getSatur", &Rigid::getSatur)
        .def("getCollider", &Rigid::getCollider)
        .def("getForces", &Rigid::getForces)
        .def("getNext", &Rigid::getNext)
        .def("getPrev", &Rigid::getPrev)
        .def("getNode", &Rigid::getNode)
        .def("getSolver", &Rigid::getSolver)
        .def("getDensity", &Rigid::getDensity)
        .def("getVel", &Rigid::getVel)
        .def("getIndex", &Rigid::getIndex)
        .def("getAABB", [](const Rigid& self) {
            glm::vec2 bl, tr;
            self.getAABB(bl, tr);
            return std::make_pair(bl, tr);
        });
}