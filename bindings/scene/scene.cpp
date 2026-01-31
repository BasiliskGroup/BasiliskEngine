#include <pybind11/pybind11.h>
#include <basilisk/scene/scene.h>
#include <basilisk/light/light.h>
#include <basilisk/nodes/node.h>

namespace py = pybind11;

void bind_scene(py::module_& m) {
    py::class_<bsk::internal::Scene>(m, "Scene")
        .def(py::init<bsk::internal::Engine*>(), py::arg("engine"))

        .def("update", &bsk::internal::Scene::update)
        .def("render", &bsk::internal::Scene::render)

        .def("set_camera", &bsk::internal::Scene::setCamera,
        py::arg("camera"))
        .def("set_skybox", &bsk::internal::Scene::setSkybox,
             py::arg("skybox"))

        .def("get_shader", &bsk::internal::Scene::getShader,
             py::return_value_policy::reference)
        .def("get_camera", &bsk::internal::Scene::getCamera,
             py::return_value_policy::reference)

        // ---- add overloads ----
        .def(
            "add",
            static_cast<void (bsk::internal::Scene::*)(bsk::internal::Light*)>(
                &bsk::internal::Scene::add),
            py::arg("light")
        )
        .def(
            "add",
            static_cast<void (bsk::internal::Scene::*)(bsk::internal::Node*)>(
                &bsk::internal::Scene::add),
            py::arg("node")
        )

        // ---- remove overload ----
        .def(
            "remove",
            static_cast<void (bsk::internal::Scene::*)(bsk::internal::Node*)>(
                &bsk::internal::Scene::remove),
            py::arg("node")
        );
}
