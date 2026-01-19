#include <pybind11/pybind11.h>
#include <basilisk/scene/scene.h>

namespace py = pybind11;

void bind_scene(py::module_& m) {
    py::class_<bsk::internal::Scene>(m, "Scene")
        .def(py::init<bsk::internal::Engine*>(), py::arg("engine"))
        .def("update", &bsk::internal::Scene::update)
        .def("render", &bsk::internal::Scene::render)
        .def("set_camera", &bsk::internal::Scene::setCamera, py::arg("camera"))
        .def("set_skybox", &bsk::internal::Scene::setSkybox, py::arg("skybox"))
        .def("get_shader", &bsk::internal::Scene::getShader)
        .def("get_camera", &bsk::internal::Scene::getCamera)
        .def("add", &bsk::internal::Scene::add, py::arg("light"));
}