#include <pybind11/pybind11.h>

#include <basilisk/nodes/node2d.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_node2d(py::module_& m) {
    py::class_<Node2D>(m, "Node2D")

        // Node2D with scene
        .def(py::init<
            Scene2D*,
            Mesh*,
            Material*,
            glm::vec2,
            float,
            glm::vec2,
            glm::vec3,
            Collider*,
            float,
            float
        >(),
        py::arg("scene"),
        py::arg("mesh"),
        py::arg("material"),
        py::arg("position"),
        py::arg("rotation"),
        py::arg("scale"),
        py::arg("velocity"),
        py::arg("collider"),
        py::arg("density"),
        py::arg("friction"))

        // Node2D with parent
        .def(py::init<
            Node2D*,
            Mesh*,
            Material*,
            glm::vec2,
            float,
            glm::vec2,
            glm::vec3,
            Collider*,
            float,
            float
        >(),
        py::arg("parent"),
        py::arg("mesh"),
        py::arg("material"),
        py::arg("position"),
        py::arg("rotation"),
        py::arg("scale"),
        py::arg("velocity"),
        py::arg("collider"),
        py::arg("density"),
        py::arg("friction"))

        // Other constructors
        .def(py::init<Scene2D*, Node2D*>())

        // Setters (casters apply automatically)
        .def("setPosition", py::overload_cast<glm::vec2>(&Node2D::setPosition))
        .def("setPosition", py::overload_cast<glm::vec3>(&Node2D::setPosition))
        .def("setRotation", &Node2D::setRotation)
        .def("setScale", &Node2D::setScale)
        .def("setVelocity", &Node2D::setVelocity);
}