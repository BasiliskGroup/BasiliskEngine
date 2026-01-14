#include <pybind11/pybind11.h>

#include <basilisk/nodes/node.h>
#include <basilisk/scene/scene.h>
#include <basilisk/render/mesh.h>
#include <basilisk/render/material.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

void bind_node(py::module_& m) {
    py::class_<Node>(m, "Node")

        // Node with scene
        .def(py::init<
            Scene*,
            Mesh*,
            Material*,
            glm::vec3,
            glm::quat,
            glm::vec3
        >(),
        py::arg("scene"),
        py::arg("mesh") = nullptr,
        py::arg("material") = nullptr,
        py::arg("position") = glm::vec3(0.0f, 0.0f, 0.0f),
        py::arg("rotation") = glm::quat(0.0f, 0.0f, 0.0f, 1.0f),
        py::arg("scale") = glm::vec3(1.0f, 1.0f, 1.0f))

        // Node with parent
        .def(py::init<
            Node*,
            Mesh*,
            Material*,
            glm::vec3,
            glm::quat,
            glm::vec3
        >(),
        py::arg("parent"),
        py::arg("mesh") = nullptr,
        py::arg("material") = nullptr,
        py::arg("position") = glm::vec3(0.0f, 0.0f, 0.0f),
        py::arg("rotation") = glm::quat(0.0f, 0.0f, 0.0f, 1.0f),
        py::arg("scale") = glm::vec3(1.0f, 1.0f, 1.0f))

        // Other constructors
        .def(py::init<Scene*, Node*>(), py::arg("scene"), py::arg("parent"))

        // Setters (casters apply automatically)
        .def("setPosition", &Node::setPosition, py::arg("position"))
        .def("setRotation", &Node::setRotation, py::arg("rotation"))
        .def("setScale", &Node::setScale, py::arg("scale"));
}
