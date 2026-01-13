#include "pybind11/pybind11.h"
#include <basilisk/nodes/node.h>
#include <basilisk/scene/scene.h>
#include <basilisk/render/mesh.h>
#include <basilisk/render/material.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace py = pybind11;

void bind_node(py::module_& m) {
    py::class_<bsk::internal::Node>(m, "Node")
        .def(py::init([](bsk::internal::Scene* scene, 
                         bsk::internal::Mesh* mesh, 
                         bsk::internal::Material* material,
                         py::tuple position,
                         py::tuple rotation,
                         py::tuple scale) {
            if (py::len(position) != 3) {
                throw std::runtime_error("Position tuple must have exactly 3 elements (x, y, z)");
            }
            if (py::len(rotation) != 4) {
                throw std::runtime_error("Rotation tuple must have exactly 4 elements (w, x, y, z)");
            }
            if (py::len(scale) != 3) {
                throw std::runtime_error("Scale tuple must have exactly 3 elements (x, y, z)");
            }
            
            glm::vec3 posVec(
                py::cast<float>(position[0]),
                py::cast<float>(position[1]),
                py::cast<float>(position[2])
            );
            glm::quat rotQuat(
                py::cast<float>(rotation[0]),  // w
                py::cast<float>(rotation[1]),  // x
                py::cast<float>(rotation[2]),  // y
                py::cast<float>(rotation[3])   // z
            );
            glm::vec3 scaleVec(
                py::cast<float>(scale[0]),
                py::cast<float>(scale[1]),
                py::cast<float>(scale[2])
            );
            
            bsk::internal::Node::Params params;
            params.mesh = mesh;
            params.material = material;
            params.position = posVec;
            params.rotation = rotQuat;
            params.scale = scaleVec;
            return new bsk::internal::Node(scene, params);
        }), py::arg("scene"), py::arg("mesh") = nullptr, py::arg("material") = nullptr,
            py::arg("position"), py::arg("rotation"), py::arg("scale"))
        .def(py::init([](bsk::internal::Node* parent,
                         bsk::internal::Mesh* mesh,
                         bsk::internal::Material* material,
                         py::tuple position,
                         py::tuple rotation,
                         py::tuple scale) {
            if (py::len(position) != 3) {
                throw std::runtime_error("Position tuple must have exactly 3 elements (x, y, z)");
            }
            if (py::len(rotation) != 4) {
                throw std::runtime_error("Rotation tuple must have exactly 4 elements (w, x, y, z)");
            }
            if (py::len(scale) != 3) {
                throw std::runtime_error("Scale tuple must have exactly 3 elements (x, y, z)");
            }
            
            glm::vec3 posVec(
                py::cast<float>(position[0]),
                py::cast<float>(position[1]),
                py::cast<float>(position[2])
            );
            glm::quat rotQuat(
                py::cast<float>(rotation[0]),  // w
                py::cast<float>(rotation[1]),  // x
                py::cast<float>(rotation[2]),  // y
                py::cast<float>(rotation[3])   // z
            );
            glm::vec3 scaleVec(
                py::cast<float>(scale[0]),
                py::cast<float>(scale[1]),
                py::cast<float>(scale[2])
            );
            
            bsk::internal::Node::Params params;
            params.mesh = mesh;
            params.material = material;
            params.position = posVec;
            params.rotation = rotQuat;
            params.scale = scaleVec;
            return new bsk::internal::Node(parent, params);
        }), py::arg("parent"), py::arg("mesh") = nullptr, py::arg("material") = nullptr,
            py::arg("position"), py::arg("rotation"), py::arg("scale"))
        .def(py::init<bsk::internal::Scene*, bsk::internal::Node*>())
        .def("setPosition", &bsk::internal::Node::setPosition)
        .def("setRotation", &bsk::internal::Node::setRotation)
        .def("setScale", &bsk::internal::Node::setScale);
}