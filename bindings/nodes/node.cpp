#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "glm/glmCasters.hpp"  // DO NOT REMOVE - must be before vector_proxy.h (uses py::cast<glm::vec3>)
#include <basilisk/nodes/node.h>
#include <basilisk/scene/scene.h>
#include <basilisk/render/mesh.h>
#include <basilisk/render/material.h>
#include "vector_proxy.h"
#include "quat_proxy.h"

namespace py = pybind11;
using namespace bsk::internal;

namespace {
    const glm::vec3 default_position(0.0f, 0.0f, 0.0f);
    const glm::vec3 default_scale(1.0f, 1.0f, 1.0f);
    const glm::quat default_rotation(1.0f, 0.0f, 0.0f, 0.0f);
}  // namespace

void bind_node(py::module_& m) {
    // Bind vector and quat proxies (Node runs before Node2D, so we bind Vec3 here; Node2D binds Vec2)
    bind_vector_proxy<glm::vec3>(m, "_Vec3PropertyProxy");
    bind_quat_proxy(m, "_QuatPropertyProxy");

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
            // Node is already added to parent's children by the constructor
            // Register it in the Python map (Scene::add will check if it needs to be added to root)
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

        // Setters (use _from_pyobject for set_rotation so PyGLM quat, tuple (w,x,y,z), etc. all work like the property)
        .def("set_position", &Node::setPosition, py::arg("position"))
        .def("set_rotation", [](Node& n, py::object value) { n.setRotation(quat_from_pyobject(value)); },
             py::arg("rotation"))
        .def("set_scale", &Node::setScale, py::arg("scale"))
        .def("set_mesh", py::overload_cast<Mesh*>(&Node::setMesh), py::arg("mesh"))
        .def("set_mesh", py::overload_cast<std::shared_ptr<Mesh>>(&Node::setMesh), py::arg("mesh"))
        .def("set_material", py::overload_cast<Material*>(&Node::setMaterial), py::arg("material"))
        .def("set_material", py::overload_cast<std::shared_ptr<Material>>(&Node::setMaterial), py::arg("material"))

        // Getters (Node)
        .def("get_scene", &Node::getScene)

        // Getters (VirtualNode)
        .def("get_position", &Node::getPosition)
        .def("get_rotation", &Node::getRotation)
        .def("get_scale", &Node::getScale)
        .def("get_parent", &Node::getParent, py::return_value_policy::reference_internal)
        .def("get_shader", &Node::getShader, py::return_value_policy::reference_internal)
        .def("get_material", &Node::getMaterial, py::return_value_policy::reference_internal)
        .def("get_mesh", &Node::getMesh, py::return_value_policy::reference_internal)
        .def("get_engine", &Node::getEngine, py::return_value_policy::reference_internal)
        .def("get_children", &Node::getChildren, py::return_value_policy::reference_internal)

        // Proxies: in-place (node.position.y += 1) + tuple/PyGLM
        .def_property("position",
            [](Node& n) {
                return new Vec3Proxy(
                    [&n]() { return n.getPosition(); },
                    [&n](const glm::vec3& v) { n.setPosition(v); }
                );
            },
            [](Node& n, py::object value) { n.setPosition(vec3_from_pyobject(value)); },
            py::return_value_policy::take_ownership)
        .def_property("scale",
            [](Node& n) {
                return new Vec3Proxy(
                    [&n]() { return n.getScale(); },
                    [&n](const glm::vec3& v) { n.setScale(v); }
                );
            },
            [](Node& n, py::object value) { n.setScale(vec3_from_pyobject(value)); },
            py::return_value_policy::take_ownership)
        .def_property("rotation",
            [](Node& n) {
                return new QuatProxy(
                    [&n]() { return n.getRotation(); },
                    [&n](const glm::quat& q) { n.setRotation(q); }
                );
            },
            [](Node& n, py::object value) { n.setRotation(quat_from_pyobject(value)); },
            py::return_value_policy::take_ownership)

        // PyGLM vec3/quat for glm.mix etc. (pos, scl, rot return native PyGLM types)
        .def_property_readonly("pos", [](const Node& n) { return n.getPosition(); })
        .def_property_readonly("scl", [](const Node& n) { return n.getScale(); })
        .def_property_readonly("rot", [](const Node& n) { return n.getRotation(); })

        // Hierarchy
        .def("add", &Node::add, py::arg("child"))
        .def("remove", &Node::remove, py::arg("child"));
}
