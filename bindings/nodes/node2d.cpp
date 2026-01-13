#include <pybind11/pybind11.h>
#include <basilisk/nodes/node2d.h>
#include "../helpers/glm_helpers.h"

namespace py = pybind11;
using namespace bsk::bindings::helpers;
using namespace bsk::internal;

void bind_node2d(py::module_& m) {
    py::class_<Node2D>(m, "Node2D")

        // Node2D with scene
        .def(py::init([](Scene2D* scene, 
            Mesh* mesh, 
            Material* material, 
            py::tuple position, 
            float rotation, 
            py::tuple scale, 
            py::tuple velocity, 
            Collider* collider, 
            float density, 
            float friction) 
        {
            return new Node2D(
                scene, 
                mesh, 
                material, 
                tuple_to_vec2(position), 
                rotation, 
                tuple_to_vec2(scale), 
                tuple_to_vec3(velocity), 
                collider, 
                density, 
                friction
            );
        }), py::arg("scene"), py::arg("mesh"), py::arg("material"), 
            py::arg("position"), py::arg("rotation"), py::arg("scale"), 
            py::arg("velocity"), py::arg("collider"), py::arg("density"), py::arg("friction"))

        // Node2D with parent
        .def(py::init([](Node2D* parent, 
            Mesh* mesh, 
            Material* material, 
            py::tuple position, 
            float rotation, 
            py::tuple scale, 
            py::tuple velocity, 
            Collider* collider, 
            float density, 
            float friction) 
        {
            return new Node2D(
                parent, 
                mesh, 
                material, 
                tuple_to_vec2(position), 
                rotation, 
                tuple_to_vec2(scale), 
                tuple_to_vec3(velocity), 
                collider, 
                density, 
                friction
            );
        }), py::arg("parent"), py::arg("mesh"), py::arg("material"), 
            py::arg("position"), py::arg("rotation"), py::arg("scale"), 
            py::arg("velocity"), py::arg("collider"), py::arg("density"), py::arg("friction"))

        .def("setPosition", [](Node2D& self, py::tuple position) {
            switch (py::len(position)) {
                case 2:
                    self.setPosition(tuple_to_vec2(position));
                    break;
                case 3:
                    self.setPosition(tuple_to_vec3(position));
                    break;
                default:
                    throw std::runtime_error("Position tuple must have 2 (x, y) or 3 (x, y, z) elements");
            }
        })
        .def("setRotation", &Node2D::setRotation)
        .def("setScale", &bsk::internal::Node2D::setScale)
        .def("setVelocity", &Node2D::setVelocity);
}