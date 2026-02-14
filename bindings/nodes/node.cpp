#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/nodes/node.h>
#include <basilisk/scene/scene.h>
#include <basilisk/render/mesh.h>
#include <basilisk/render/material.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

namespace {
const glm::vec3 default_position(0.0f, 0.0f, 0.0f);
const glm::vec3 default_scale(1.0f, 1.0f, 1.0f);
const glm::quat default_rotation(0.0f, 0.0f, 0.0f, 1.0f);
}  // namespace

void bind_node(py::module_& m) {
    py::class_<Node, std::shared_ptr<Node>>(m, "Node")

        // Node with scene - use lambda to add to childrenPythonMap
        .def(py::init([](Scene* scene, Mesh* mesh, Material* material,
                         glm::vec3 position, glm::quat rotation, glm::vec3 scale) {
            auto node = std::make_shared<Node>(scene, mesh, material, position, rotation, scale);
            scene->add(node);
            return node;
        }),
        py::arg("scene"),
        py::arg("mesh") = nullptr,
        py::arg("material") = nullptr,
        py::arg("position") = default_position,
        py::arg("rotation") = default_rotation,
        py::arg("scale") = default_scale)

        // Node with parent - use lambda to add to childrenPythonMap
        .def(py::init([](Node* parent, Mesh* mesh, Material* material,
                         glm::vec3 position, glm::quat rotation, glm::vec3 scale) {
            auto node = std::make_shared<Node>(parent, mesh, material, position, rotation, scale);
            if (parent && parent->getScene()) {
                parent->getScene()->add(node);
            }
            return node;
        }),
        py::arg("parent"),
        py::arg("mesh") = nullptr,
        py::arg("material") = nullptr,
        py::arg("position") = default_position,
        py::arg("rotation") = default_rotation,
        py::arg("scale") = default_scale)

        // Empty Node with scene - use lambda to add to childrenPythonMap
        .def(py::init([](Scene* scene) {
            auto node = std::make_shared<Node>(scene);
            scene->add(node);
            return node;
        }),
        py::arg("scene"))

        // Orphan Node (mesh, material, ...) - no scene, so no need to add
        .def(py::init<Mesh*, Material*, glm::vec3, glm::quat, glm::vec3>(),
             py::arg("mesh"),
             py::arg("material"),
             py::arg("position") = default_position,
             py::arg("rotation") = default_rotation,
             py::arg("scale") = default_scale)

        // Setters
        .def("set_position", &Node::setPosition, py::arg("position"))
        .def("set_rotation", &Node::setRotation, py::arg("rotation"))
        .def("set_scale", &Node::setScale, py::arg("scale"))

        // Getters (Node)
        .def("get_scene", &Node::getScene)

        // Getters (VirtualNode)
        .def("get_position", &Node::getPosition)
        .def("get_rotation", &Node::getRotation)
        .def("get_scale", &Node::getScale)
        .def("get_parent", &Node::getParent)
        .def("get_shader", &Node::getShader)
        .def("get_material", &Node::getMaterial)
        .def("get_mesh", &Node::getMesh)
        .def("get_engine", &Node::getEngine)
        .def("get_children", &Node::getChildren, py::return_value_policy::reference_internal)

        // Hierarchy
        .def("add", &Node::add, py::arg("child"))
        .def("remove", &Node::remove, py::arg("child"));
}
