#include <pybind11/pybind11.h>

#include <basilisk/nodes/node2d.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp" // DO NOT REMOVE THIS LINE

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
        py::arg("position") = default_position,
        py::arg("rotation") = default_rotation,
        py::arg("scale") = default_scale,
        py::arg("velocity") = default_velocity,
        py::arg("collider") = nullptr,
        py::arg("density") = default_density,
        py::arg("friction") = default_friction)

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
        py::arg("position") = default_position,
        py::arg("rotation") = default_rotation,
        py::arg("scale") = default_scale,
        py::arg("velocity") = default_velocity,
        py::arg("collider") = nullptr,
        py::arg("density") = default_density,
        py::arg("friction") = default_friction)

        // Empty Node2D with scene
        .def(py::init<Scene2D*>(), py::arg("scene"))

        // Orphan Node2D (mesh, material, ...)
        .def(py::init<Mesh*, Material*, glm::vec2, float, glm::vec2, glm::vec3, Collider*, float, float>(),
        py::arg("mesh"),
        py::arg("material"),
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

        // Getters (Node2D)
        .def("get_scene", &Node2D::getScene)
        .def("get_rigid", &Node2D::getRigid)
        .def("get_velocity", &Node2D::getVelocity)
        .def("get_layer", &Node2D::getLayer)

        // Getters (VirtualNode)
        .def("get_position", &Node2D::getPosition)
        .def("get_rotation", &Node2D::getRotation)
        .def("get_scale", &Node2D::getScale)
        .def("get_parent", &Node2D::getParent)
        .def("get_shader", &Node2D::getShader)
        .def("get_material", &Node2D::getMaterial)
        .def("get_mesh", &Node2D::getMesh)
        .def("get_engine", &Node2D::getEngine)
        .def("get_children", &Node2D::getChildren, py::return_value_policy::reference_internal)

        // Hierarchy
        .def("add", &Node2D::add, py::arg("child"))
        .def("remove", &Node2D::remove, py::arg("child"))

        // Collision
        .def("constrained_to", &Node2D::constrainedTo, py::arg("other"))
        .def("is_touching", &Node2D::isTouching, py::arg("other"));
}