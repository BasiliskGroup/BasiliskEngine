#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/forces/force.h>

namespace py = pybind11;

void bind_rigid(py::module_& m) {
    py::class_<bsk::internal::Rigid>(m, "Rigid")
        // Constructor without velocity (uses default velocity of {0, 0, 0})
        .def(py::init([](bsk::internal::Solver* solver, bsk::internal::Node2D* node, bsk::internal::Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction) {
            return new bsk::internal::Rigid(solver, node, collider, position, size, density, friction, glm::vec3{0, 0, 0});
        }), py::arg("solver"), py::arg("node"), py::arg("collider"), py::arg("position"), 
            py::arg("size"), py::arg("density"), py::arg("friction"))
        // Constructor with explicit velocity
        .def(py::init([](bsk::internal::Solver* solver, bsk::internal::Node2D* node, bsk::internal::Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity) {
            return new bsk::internal::Rigid(solver, node, collider, position, size, density, friction, velocity);
        }), py::arg("solver"), py::arg("node"), py::arg("collider"), py::arg("position"), 
            py::arg("size"), py::arg("density"), py::arg("friction"), py::arg("velocity"))
        
        // Constraint methods
        .def("constrainedTo", &bsk::internal::Rigid::constrainedTo)
        
        // Coloring methods
        .def("resetColoring", &bsk::internal::Rigid::resetColoring)
        .def("isColored", &bsk::internal::Rigid::isColored)
        .def("isColorUsed", &bsk::internal::Rigid::isColorUsed)
        .def("getNextUnusedColor", &bsk::internal::Rigid::getNextUnusedColor)
        .def("reserveColors", &bsk::internal::Rigid::reserveColors)
        .def("useColor", &bsk::internal::Rigid::useColor)
        .def("incrSatur", &bsk::internal::Rigid::incrSatur)
        .def("verifyColoring", &bsk::internal::Rigid::verifyColoring)
        
        // Linked list management
        .def("insert", &bsk::internal::Rigid::insert)
        .def("remove", &bsk::internal::Rigid::remove)
        
        // Setters
        .def("setPosition", &bsk::internal::Rigid::setPosition)
        .def("setScale", &bsk::internal::Rigid::setScale)
        .def("setVelocity", &bsk::internal::Rigid::setVelocity)
        .def("setInitial", &bsk::internal::Rigid::setInitial)
        .def("setInertial", &bsk::internal::Rigid::setInertial)
        .def("setPrevVelocity", &bsk::internal::Rigid::setPrevVelocity)
        .def("setMass", &bsk::internal::Rigid::setMass)
        .def("setMoment", &bsk::internal::Rigid::setMoment)
        .def("setFriction", &bsk::internal::Rigid::setFriction)
        .def("setRadius", &bsk::internal::Rigid::setRadius)
        .def("setColor", &bsk::internal::Rigid::setColor)
        .def("setDegree", &bsk::internal::Rigid::setDegree)
        .def("setSatur", &bsk::internal::Rigid::setSatur)
        .def("setCollider", &bsk::internal::Rigid::setCollider)
        .def("setForces", &bsk::internal::Rigid::setForces)
        .def("setNext", &bsk::internal::Rigid::setNext)
        .def("setPrev", &bsk::internal::Rigid::setPrev)
        .def("setNode", &bsk::internal::Rigid::setNode)
        .def("setIndex", &bsk::internal::Rigid::setIndex)
        
        // Getters
        .def("getPosition", &bsk::internal::Rigid::getPosition)
        .def("getInitial", &bsk::internal::Rigid::getInitial)
        .def("getInertial", &bsk::internal::Rigid::getInertial)
        .def("getVelocity", &bsk::internal::Rigid::getVelocity)
        .def("getPrevVelocity", &bsk::internal::Rigid::getPrevVelocity)
        .def("getSize", &bsk::internal::Rigid::getSize)
        .def("getMass", &bsk::internal::Rigid::getMass)
        .def("getMoment", &bsk::internal::Rigid::getMoment)
        .def("getFriction", &bsk::internal::Rigid::getFriction)
        .def("getRadius", &bsk::internal::Rigid::getRadius)
        .def("getColor", &bsk::internal::Rigid::getColor)
        .def("getDegree", &bsk::internal::Rigid::getDegree)
        .def("getSatur", &bsk::internal::Rigid::getSatur)
        .def("getCollider", &bsk::internal::Rigid::getCollider)
        .def("getForces", &bsk::internal::Rigid::getForces)
        .def("getNext", &bsk::internal::Rigid::getNext)
        .def("getPrev", &bsk::internal::Rigid::getPrev)
        .def("getNode", &bsk::internal::Rigid::getNode)
        .def("getSolver", &bsk::internal::Rigid::getSolver)
        .def("getDensity", &bsk::internal::Rigid::getDensity)
        .def("getVel", &bsk::internal::Rigid::getVel)
        .def("getIndex", &bsk::internal::Rigid::getIndex)
        .def("getAABB", [](const bsk::internal::Rigid& self) {
            glm::vec2 bl, tr;
            self.getAABB(bl, tr);
            return std::make_pair(bl, tr);
        });
}