#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "glm/glmCasters.hpp"  // DO NOT REMOVE - must be before vector_proxy.h (uses py::cast<glm::vec2/vec3>)
#include <basilisk/nodes/node2d.h>
#include <basilisk/scene/scene2d.h>
#include "vector_proxy.h"

namespace py = pybind11;
using namespace bsk::internal;

namespace {
const glm::vec2 default_position(0.0f, 0.0f);
const glm::vec2 default_scale(1.0f, 1.0f);
const glm::vec3 default_velocity(0.0f, 0.0f, 0.0f);
const float default_rotation = 0.0f;
const float default_density = 1.0f;
const float default_friction = 0.5f;
}  // namespace

void bind_node2d(py::module_& m) {
    // Bind Vec2 proxy (Vec3 is bound by Node which runs first)
    bind_vector_proxy<glm::vec2>(m, "_Vec2PropertyProxy");

    py::class_<Node2D, std::shared_ptr<Node2D>>(m, "Node2D")

        // Node2D with scene - use shared_ptr for mesh/material so node keeps them alive when Python ref drops
        .def(py::init([](Scene2D* scene, py::object mesh_obj, py::object material_obj,
                         glm::vec2 position, float rotation, glm::vec2 scale,
                         glm::vec3 velocity, Collider* collider, float density, float friction) {
            Mesh* mesh = mesh_obj.is_none() ? nullptr : mesh_obj.cast<Mesh*>();
            Material* material = material_obj.is_none() ? nullptr : material_obj.cast<Material*>();
            auto node = std::make_shared<Node2D>(scene, mesh, material, position, rotation,
                                                 scale, velocity, collider, density, friction);
            if (!mesh_obj.is_none()) node->setMesh(py::cast<std::shared_ptr<Mesh>>(mesh_obj));
            if (!material_obj.is_none()) node->setMaterial(py::cast<std::shared_ptr<Material>>(material_obj));
            scene->add(node);
            return node;
        }),
        py::arg("scene"),
        py::arg("mesh"),
        py::arg("material"),
        py::arg("position") = default_position,
        py::arg("rotation") = default_rotation,
        py::arg("scale") = default_scale,
        py::arg("velocity") = default_velocity,
        py::arg("collider") = nullptr,
        py::arg("density") = default_density,
        py::arg("friction") = default_friction)

        // Node2D with parent - use shared_ptr for mesh/material so node keeps them alive
        .def(py::init([](Node2D* parent, py::object mesh_obj, py::object material_obj,
                         glm::vec2 position, float rotation, glm::vec2 scale,
                         glm::vec3 velocity, Collider* collider, float density, float friction) {
            Mesh* mesh = mesh_obj.is_none() ? nullptr : mesh_obj.cast<Mesh*>();
            Material* material = material_obj.is_none() ? nullptr : material_obj.cast<Material*>();
            auto node = std::make_shared<Node2D>(parent, mesh, material, position, rotation,
                                                 scale, velocity, collider, density, friction);
            if (!mesh_obj.is_none()) node->setMesh(py::cast<std::shared_ptr<Mesh>>(mesh_obj));
            if (!material_obj.is_none()) node->setMaterial(py::cast<std::shared_ptr<Material>>(material_obj));
            if (parent && parent->getScene()) {
                parent->getScene()->add(node);
            }
            return node;
        }),
        py::arg("parent"),
        py::arg("mesh"),
        py::arg("material"),
        py::arg("position") = default_position,
        py::arg("rotation") = default_rotation,
        py::arg("scale") = default_scale,
        py::arg("velocity") = default_velocity,
        py::arg("collider") = nullptr,
        py::arg("density") = default_density,
        py::arg("friction") = default_friction)

        // Empty Node2D with scene - use lambda to add to childrenPythonMap
        .def(py::init([](Scene2D* scene) {
            auto node = std::make_shared<Node2D>(scene);
            scene->add(node);
            return node;
        }),
        py::arg("scene"))

        // Orphan Node2D (mesh, material, ...) - use shared_ptr so node keeps them alive
        .def(py::init([](py::object mesh_obj, py::object material_obj,
                         glm::vec2 position, float rotation, glm::vec2 scale,
                         glm::vec3 velocity, Collider* collider, float density, float friction) {
            Mesh* mesh = mesh_obj.is_none() ? nullptr : mesh_obj.cast<Mesh*>();
            Material* material = material_obj.is_none() ? nullptr : material_obj.cast<Material*>();
            auto node = std::make_shared<Node2D>(mesh, material, position, rotation,
                                                 scale, velocity, collider, density, friction);
            if (!mesh_obj.is_none()) node->setMesh(py::cast<std::shared_ptr<Mesh>>(mesh_obj));
            if (!material_obj.is_none()) node->setMaterial(py::cast<std::shared_ptr<Material>>(material_obj));
            return node;
        }),
        py::arg("mesh") = py::none(),
        py::arg("material") = py::none(),
             py::arg("position") = default_position,
             py::arg("rotation") = default_rotation,
             py::arg("scale") = default_scale,
             py::arg("velocity") = default_velocity,
             py::arg("collider") = nullptr,
             py::arg("density") = default_density,
             py::arg("friction") = default_friction)

        // Setters
        .def("set_position", py::overload_cast<glm::vec2>(&Node2D::setPosition), py::arg("position"))
        .def("set_position", py::overload_cast<glm::vec3>(&Node2D::setPosition), py::arg("position"))
        .def("set_rotation", &Node2D::setRotation, py::arg("rotation"))
        .def("set_scale", &Node2D::setScale, py::arg("scale"))
        .def("set_velocity", &Node2D::setVelocity, py::arg("velocity"))
        .def("set_layer", &Node2D::setLayer, py::arg("layer"))
        .def("set_collider", &Node2D::setCollider, py::arg("collider"))
        .def("set_density", &Node2D::setDensity, py::arg("density"))
        .def("set_friction", &Node2D::setFriction, py::arg("friction"))
        .def("set_mesh", py::overload_cast<Mesh*>(&Node2D::setMesh), py::arg("mesh"))
        .def("set_mesh", py::overload_cast<std::shared_ptr<Mesh>>(&Node2D::setMesh), py::arg("mesh"))
        .def("set_material", py::overload_cast<Material*>(&Node2D::setMaterial), py::arg("material"))
        .def("set_material", py::overload_cast<std::shared_ptr<Material>>(&Node2D::setMaterial), py::arg("material"))

        // Getters (Node2D)
        .def("get_scene", &Node2D::getScene)
        .def("get_rigid", &Node2D::getRigid, py::return_value_policy::reference_internal)
        .def("get_velocity", &Node2D::getVelocity)
        .def("get_layer", &Node2D::getLayer)
        .def("get_density", &Node2D::getDensity)
        .def("get_friction", &Node2D::getFriction)
        .def("get_collider", &Node2D::getCollider, py::return_value_policy::reference_internal)

        // Getters (VirtualNode)
        .def("get_position", &Node2D::getPosition)
        .def("get_rotation", &Node2D::getRotation)
        .def("get_scale", &Node2D::getScale)
        .def("get_parent", &Node2D::getParent, py::return_value_policy::reference_internal)
        .def("get_shader", &Node2D::getShader, py::return_value_policy::reference_internal)
        .def("get_material", &Node2D::getMaterial, py::return_value_policy::reference_internal)
        .def("get_mesh", &Node2D::getMesh, py::return_value_policy::reference_internal)
        .def("get_engine", &Node2D::getEngine, py::return_value_policy::reference_internal)
        .def("get_children", &Node2D::getChildren, py::return_value_policy::reference_internal)

        // Proxies: in-place (node.velocity.y += 1) + tuple/PyGLM (glm.vec2(*node.position), glm.mix(...))
        .def_property("position",
            [](Node2D& n) {
                return new Vec2Proxy(
                    [&n]() { return n.getPosition(); },
                    [&n](const glm::vec2& v) { n.setPosition(v); }
                );
            },
            [](Node2D& n, py::object value) { n.setPosition(vec2_from_pyobject(value)); },
            py::return_value_policy::take_ownership)
        .def_property("scale",
            [](Node2D& n) {
                return new Vec2Proxy(
                    [&n]() { return n.getScale(); },
                    [&n](const glm::vec2& v) { n.setScale(v); }
                );
            },
            [](Node2D& n, py::object value) { n.setScale(vec2_from_pyobject(value)); },
            py::return_value_policy::take_ownership)
        .def_property("velocity",
            [](Node2D& n) {
                return new Vec3Proxy(
                    [&n]() { return n.getVelocity(); },
                    [&n](const glm::vec3& v) { n.setVelocity(v); }
                );
            },
            [](Node2D& n, py::object value) { n.setVelocity(vec3_from_pyobject(value)); },
            py::return_value_policy::take_ownership)

        // PyGLM vec2/vec3 for glm.mix etc. (pos, vel, scl return native PyGLM types)
        .def_property_readonly("pos", [](const Node2D& n) { return n.getPosition(); })
        .def_property_readonly("vel", [](Node2D& n) { return n.getVelocity(); })
        .def_property_readonly("scl", [](const Node2D& n) { return n.getScale(); })

        // Simple properties (get/set full value)
        .def_property("rotation",
            [](const Node2D& n) { return n.getRotation(); },
            [](Node2D& n, float v) { n.setRotation(v); })
        .def_property("layer",
            [](Node2D& n) { return n.getLayer(); },
            [](Node2D& n, float v) { n.setLayer(v); })

        // Hierarchy
        .def("add", &Node2D::add, py::arg("child"))
        .def("remove", &Node2D::remove, py::arg("child"))

        // Collision
        .def("constrained_to", &Node2D::constrainedTo, py::arg("other"))
        .def("just_collided", &Node2D::justCollided, py::arg("other"))
        .def("is_touching", &Node2D::isTouching, py::arg("other"));
}