#include <pybind11/pybind11.h>
#include <basilisk/engine/engine.h>

namespace py = pybind11;

void bind_engine(py::module_& m) {
    py::class_<bsk::internal::Engine>(m, "Engine")
        .def(py::init<int, int, const char*, bool>(), 
             py::arg("width"), 
             py::arg("height"), 
             py::arg("title"), 
             py::arg("autoMouseGrab") = true)
        .def("isRunning", &bsk::internal::Engine::isRunning)
        .def("update", &bsk::internal::Engine::update)
        .def("render", &bsk::internal::Engine::render)
        .def("useContext", &bsk::internal::Engine::useContext)
        .def("setResolution", &bsk::internal::Engine::setResolution, py::arg("width"), py::arg("height"))
        .def("getWindow", &bsk::internal::Engine::getWindow, py::return_value_policy::reference_internal)
        .def("getMouse", &bsk::internal::Engine::getMouse, py::return_value_policy::reference_internal)
        .def("getKeyboard", &bsk::internal::Engine::getKeyboard, py::return_value_policy::reference_internal)
        .def("getFrame", &bsk::internal::Engine::getFrame, py::return_value_policy::reference_internal)
        .def("getResourceServer", &bsk::internal::Engine::getResourceServer, py::return_value_policy::reference_internal)
        .def("getDeltaTime", &bsk::internal::Engine::getDeltaTime);
}