#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/scene/scene.h>
#include <basilisk/light/light.h>
#include <basilisk/nodes/node.h>
#include <basilisk/camera/staticCamera.h>
#include <basilisk/scene/raycast.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

// Python-facing raycast result: node is shared_ptr when available
struct RayCastResultPy {
    std::shared_ptr<Node> node;
    glm::vec3 intersection;
    glm::vec3 normal;
    float distance;
};

void bind_scene(py::module_& m) {
    py::class_<Scene>(m, "Scene")
        .def(py::init<Engine*, bool, bool, bool>(),
             py::arg("engine"),
             py::arg("addSkybox") = true,
             py::arg("addLight") = true,
             py::arg("addCube") = false)
        .def(py::init<Engine*, Shader*, bool, bool, bool>(),
             py::arg("engine"),
             py::arg("shader"),
             py::arg("addSkybox") = true,
             py::arg("addLight") = true,
             py::arg("addCube") = false)
        
        .def("update", &Scene::update)
        .def("render", &Scene::render)

        // Camera setters: raw pointer or shared_ptr
        .def("set_camera",
             static_cast<void (Scene::*)(StaticCamera*)>(&Scene::setCamera),
             py::arg("camera"))
        .def("set_camera",
             static_cast<void (Scene::*)(std::shared_ptr<StaticCamera>)>(&Scene::setCamera),
             py::arg("camera"))
        .def("set_skybox", &Scene::setSkybox,
             py::arg("skybox"))

        .def("get_shader", &Scene::getShader,
             py::return_value_policy::reference)
        .def("get_camera", &Scene::getCamera,
             py::return_value_policy::reference)

        // ---- add overloads ----
        .def(
            "add",
            static_cast<void (Scene::*)(std::shared_ptr<Light>)>(
                &Scene::add),
            py::arg("light")
        )
        .def(
            "add",
            static_cast<void (Scene::*)(std::shared_ptr<Node>)>(
                &Scene::add),
            py::arg("node")
        )

        // ---- remove overload ----
        .def(
            "remove",
            static_cast<void (Scene::*)(Node*)>(
                &Scene::remove),
            py::arg("node")
        )

        // Mouse interaction - returns result with shared_ptr node (raises if hit node has no shared_ptr)
        .def("pick", [](Scene& s, const glm::vec2& mousePosition) {
            RayCastResult r = s.pick(mousePosition);
            RayCastResultPy out;
            out.intersection = r.intersection;
            out.normal = r.normal;
            out.distance = r.distance;
            if (r.node) {
                out.node = s.findSharedNode(r.node);
                if (!out.node) {
                    throw py::value_error("Raycast hit a node that was not added via shared_ptr (add(node)); "
                                         "nodes must be added with scene.add(node) for pick/raycast to return them.");
                }
            }
            return out;
        }, py::arg("mouse_position"),
           "Cast a ray from the mouse position; returns RayCastResult with shared_ptr node. Raises if hit node lacks shared_ptr.")
        .def("raycast", [](Scene& s, const glm::vec3& origin, const glm::vec3& direction) {
            RayCastResult r = s.raycast(origin, direction);
            RayCastResultPy out;
            out.intersection = r.intersection;
            out.normal = r.normal;
            out.distance = r.distance;
            if (r.node) {
                out.node = s.findSharedNode(r.node);
                if (!out.node) {
                    throw py::value_error("Raycast hit a node that was not added via shared_ptr (add(node)); "
                                         "nodes must be added with scene.add(node) for raycast to return them.");
                }
            }
            return out;
        }, py::arg("origin"), py::arg("direction"),
           "Cast a ray in world space; returns RayCastResult with shared_ptr node. Raises if hit node lacks shared_ptr.");

    py::class_<RayCastResultPy>(m, "RayCastResult")
        .def_readonly("node", &RayCastResultPy::node,
                      "Hit node (shared_ptr) or None if no hit")
        .def_readonly("intersection", &RayCastResultPy::intersection)
        .def_readonly("normal", &RayCastResultPy::normal)
        .def_readonly("distance", &RayCastResultPy::distance);
}
