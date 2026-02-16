#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/scene/scene2d.h>

// IMPORTANT: include GLM casters
#include "glm/glmCasters.hpp"  // DO NOT REMOVE THIS LINE

namespace py = pybind11;
using namespace bsk::internal;

namespace {
const glm::vec2 default_position(0.0f, 0.0f);
const glm::vec2 default_scale(1.0f, 1.0f);
const glm::vec3 default_velocity(0.0f, 0.0f, 0.0f);
const float default_rotation = 0.0f;
const float default_density = 1.0f;
const float default_friction = 0.5f;

// Proxy classes for mutable properties so node.velocity.y += 1 works
class Vec2PropertyProxy {
public:
    Node2D* node;
    enum Type { Position, Scale } type;
    float get_x() const {
        auto v = type == Position ? glm::vec2(node->getPosition()) : node->getScale();
        return v.x;
    }
    void set_x(float v) {
        if (type == Position) {
            auto p = node->getPosition();
            node->setPosition(glm::vec2(v, p.y));
        } else {
            auto s = node->getScale();
            node->setScale(glm::vec2(v, s.y));
        }
    }
    float get_y() const {
        auto v = type == Position ? glm::vec2(node->getPosition()) : node->getScale();
        return v.y;
    }
    void set_y(float v) {
        if (type == Position) {
            auto p = node->getPosition();
            node->setPosition(glm::vec2(p.x, v));
        } else {
            auto s = node->getScale();
            node->setScale(glm::vec2(s.x, v));
        }
    }
};

class Vec3VelocityProxy {
public:
    Node2D* node;
    float get_x() const { return node->getVelocity().x; }
    void set_x(float v) { auto vel = node->getVelocity(); vel.x = v; node->setVelocity(vel); }
    float get_y() const { return node->getVelocity().y; }
    void set_y(float v) { auto vel = node->getVelocity(); vel.y = v; node->setVelocity(vel); }
    float get_z() const { return node->getVelocity().z; }
    void set_z(float v) { auto vel = node->getVelocity(); vel.z = v; node->setVelocity(vel); }
};

}  // namespace

void bind_node2d(py::module_& m) {
    // Bind Vec2PropertyProxy for position and scale
    py::class_<Vec2PropertyProxy>(m, "_Vec2PropertyProxy", py::module_local())
        .def_property("x", &Vec2PropertyProxy::get_x, &Vec2PropertyProxy::set_x)
        .def_property("y", &Vec2PropertyProxy::get_y, &Vec2PropertyProxy::set_y);

    // Bind Vec3VelocityProxy for velocity
    py::class_<Vec3VelocityProxy>(m, "_Vec3VelocityProxy", py::module_local())
        .def_property("x", &Vec3VelocityProxy::get_x, &Vec3VelocityProxy::set_x)
        .def_property("y", &Vec3VelocityProxy::get_y, &Vec3VelocityProxy::set_y)
        .def_property("z", &Vec3VelocityProxy::get_z, &Vec3VelocityProxy::set_z);

    py::class_<Node2D, std::shared_ptr<Node2D>>(m, "Node2D")

        // Node2D with scene - use lambda to add to childrenPythonMap
        .def(py::init([](Scene2D* scene, Mesh* mesh, Material* material,
                         glm::vec2 position, float rotation, glm::vec2 scale,
                         glm::vec3 velocity, Collider* collider, float density, float friction) {
            auto node = std::make_shared<Node2D>(scene, mesh, material, position, rotation,
                                                 scale, velocity, collider, density, friction);
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

        // Node2D with parent - use lambda to add to childrenPythonMap
        .def(py::init([](Node2D* parent, Mesh* mesh, Material* material,
                         glm::vec2 position, float rotation, glm::vec2 scale,
                         glm::vec3 velocity, Collider* collider, float density, float friction) {
            auto node = std::make_shared<Node2D>(parent, mesh, material, position, rotation,
                                                 scale, velocity, collider, density, friction);
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

        // Orphan Node2D (mesh, material, ...) - no scene, so no need to add
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
        .def("set_collider", &Node2D::setCollider, py::arg("collider"))
        .def("set_density", &Node2D::setDensity, py::arg("density"))
        .def("set_friction", &Node2D::setFriction, py::arg("friction"))

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

        // Properties (mutable: node.velocity.y += 1, node.position.x = 5, etc.)
        .def_property_readonly("position",
            [](Node2D& n) {
                auto p = std::make_unique<Vec2PropertyProxy>();
                p->node = &n;
                p->type = Vec2PropertyProxy::Position;
                return p.release();
            },
            py::return_value_policy::take_ownership)
        .def_property_readonly("scale",
            [](Node2D& n) {
                auto p = std::make_unique<Vec2PropertyProxy>();
                p->node = &n;
                p->type = Vec2PropertyProxy::Scale;
                return p.release();
            },
            py::return_value_policy::take_ownership)
        .def_property_readonly("velocity",
            [](Node2D& n) {
                auto p = std::make_unique<Vec3VelocityProxy>();
                p->node = &n;
                return p.release();
            },
            py::return_value_policy::take_ownership)

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