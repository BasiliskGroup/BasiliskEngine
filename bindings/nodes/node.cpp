#include "pybind11/pybind11.h"
#include <basilisk/nodes/node.h>
#include <basilisk/scene/scene.h>
#include <basilisk/render/mesh.h>
#include <basilisk/render/material.h>
#include "../helpers/glm_helpers.h"

namespace py = pybind11;
using namespace bsk::bindings::helpers;
using namespace bsk::internal;

void bind_node(py::module_& m) {
    py::class_<Node>(m, "Node")

        // Node with scene
        .def(py::init([](Scene* scene, 
            Mesh* mesh, 
            Material* material,
            py::tuple position,
            py::tuple rotation,
            py::tuple scale) 
        {
            return new Node(
                scene, 
                mesh, 
                material, 
                tuple_to_vec3(position), 
                tuple_to_quat(rotation), 
                tuple_to_vec3(scale)
            );
        }), py::arg("scene"), py::arg("mesh") = nullptr, py::arg("material") = nullptr,
            py::arg("position"), py::arg("rotation"), py::arg("scale"))

        // Node with parent
        .def(py::init([](Node* parent,
            Mesh* mesh,
            Material* material,
            py::tuple position,
            py::tuple rotation,
            py::tuple scale) 
        {
            return new Node(
                parent, 
                mesh, 
                material, 
                tuple_to_vec3(position), 
                tuple_to_quat(rotation), 
                tuple_to_vec3(scale)
            );
        }), py::arg("parent"), py::arg("mesh") = nullptr, py::arg("material") = nullptr,
            py::arg("position"), py::arg("rotation"), py::arg("scale"))

        .def(py::init<Scene*, Node*>())
        .def("setPosition", &Node::setPosition)
        .def("setRotation", &Node::setRotation)
        .def("setScale", &Node::setScale);
}