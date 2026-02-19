#include <memory>
#include <pybind11/pybind11.h>
#include <basilisk/scene/scene.h>
#include <basilisk/light/light.h>
#include <basilisk/nodes/node.h>
#include <basilisk/camera/staticCamera.h>

namespace py = pybind11;
using namespace bsk::internal;

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
            static_cast<void (Scene::*)(Light*)>(
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
        );
}
