#include <pybind11/pybind11.h>
#include <basilisk/scene/scene.h>

namespace py = pybind11;

void bind_scene(py::module_& m) {
    py::class_<bsk::internal::Scene>(m, "Scene")
        .def(py::init<bsk::internal::Engine*>(), py::arg("engine"))
        .def("update", &bsk::internal::Scene::update)
        .def("render", &bsk::internal::Scene::render)
        .def("setCamera", &bsk::internal::Scene::setCamera, py::arg("camera"))
        .def("getShader", &bsk::internal::Scene::getShader)
        .def("getCamera", &bsk::internal::Scene::getCamera);
}