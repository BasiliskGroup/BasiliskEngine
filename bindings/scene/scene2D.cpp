#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/scene/scene2d.h>
#include <basilisk/engine/engine.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/camera/staticCamera2d.h>
#include <basilisk/scene/raycast.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

// Python-facing 2D raycast result: node is shared_ptr when available
struct RayCastResult2DPy {
    std::shared_ptr<Node2D> node;
    glm::vec2 intersection;
    glm::vec2 normal;
    float distance;
};

void bind_scene2d(py::module_& m) {
    py::class_<RayCastResult2DPy>(m, "RayCastResult2D")
        .def_readonly("node", &RayCastResult2DPy::node,
                      "Hit node (shared_ptr) or None if no hit")
        .def_readonly("intersection", &RayCastResult2DPy::intersection)
        .def_readonly("normal", &RayCastResult2DPy::normal)
        .def_readonly("distance", &RayCastResult2DPy::distance);

    py::class_<Scene2D>(m, "Scene2D")
        .def(py::init<Engine*>(), py::arg("engine"))
        .def("update", &Scene2D::update)
        .def("render", &Scene2D::render)
        .def("add", static_cast<void (Scene2D::*)(std::shared_ptr<Node2D>)>(&Scene2D::add), py::arg("node"))
        .def("remove", static_cast<void (Scene2D::*)(Node2D*)>(&Scene2D::remove), py::arg("node"))
        // Camera setters: raw pointer or shared_ptr
        .def("set_camera",
             static_cast<void (Scene2D::*)(StaticCamera2D*)>(&Scene2D::setCamera),
             py::arg("camera"))
        .def("set_camera",
             static_cast<void (Scene2D::*)(std::shared_ptr<StaticCamera2D>)>(&Scene2D::setCamera),
             py::arg("camera"))
        .def("get_camera", &Scene2D::getCamera, py::return_value_policy::reference_internal)
        .def_property("camera",
            [](Scene2D& s) { return s.getCamera(); },
            [](Scene2D& s, std::shared_ptr<StaticCamera2D> c) { s.setCamera(std::move(c)); },
            py::return_value_policy::reference_internal)
        .def("get_shader", &Scene2D::getShader)
        .def("get_engine", &Scene2D::getEngine)
        .def("get_root", &Scene2D::getRoot)
        .def("get_solver", &Scene2D::getSolver)

        // Mouse / world interaction - same pattern as 3D Scene
        .def("pick", [](Scene2D& s, const glm::vec2& position) {
            Node2D* raw = s.pick(position);
            if (!raw) return std::shared_ptr<Node2D>{};
            std::shared_ptr<Node2D> sp = s.findSharedNode(raw);
            if (!sp) {
                throw py::value_error("Pick hit a node that was not added via shared_ptr (add(node)); "
                                     "nodes must be added with scene.add(node) for pick to return them.");
            }
            return sp;
        }, py::arg("position"),
           "Return the topmost node at the given world position (by layer). Returns None if no hit. Raises if hit node lacks shared_ptr.")
        .def("raycast", [](Scene2D& s, const glm::vec2& origin, const glm::vec2& direction) {
            RayCastResult2D r = s.raycast(origin, direction);
            RayCastResult2DPy out;
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
           "Cast a ray in world space; returns RayCastResult2D with shared_ptr node. Raises if hit node lacks shared_ptr.");
}