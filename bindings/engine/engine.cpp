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
        .def("is_running", &bsk::internal::Engine::isRunning)
        .def("update", &bsk::internal::Engine::update)
        .def("render", &bsk::internal::Engine::render)
        .def("use_context", &bsk::internal::Engine::useContext)
        .def("set_resolution", &bsk::internal::Engine::setResolution, py::arg("width"), py::arg("height"))
        .def("get_window", &bsk::internal::Engine::getWindow, py::return_value_policy::reference_internal)
        .def("get_mouse", &bsk::internal::Engine::getMouse, py::return_value_policy::reference_internal)
        .def("get_keyboard", &bsk::internal::Engine::getKeyboard, py::return_value_policy::reference_internal)
        .def("get_frame", &bsk::internal::Engine::getFrame, py::return_value_policy::reference_internal)
        .def("get_resource_server", &bsk::internal::Engine::getResourceServer, py::return_value_policy::reference_internal)
        .def("get_delta_time", &bsk::internal::Engine::getDeltaTime);
}