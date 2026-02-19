#include <pybind11/pybind11.h>
#include <basilisk/engine/engine.h>
#include <basilisk/IO/keyboard.h>

namespace py = pybind11;

void bind_engine(py::module_& m) {
    using namespace bsk::internal;

    py::class_<Engine>(m, "Engine")
        .def(py::init<int, int, const char*, bool>(),
             py::arg("width") = 800,
             py::arg("height") = 800,
             py::arg("title") = "Basilisk",
             py::arg("auto_mouse_grab") = true)
        .def("is_running", &Engine::isRunning)
        .def("update", &Engine::update)
        .def("render", &Engine::render)
        .def("use_context", &Engine::useContext)
        .def("set_resolution", &Engine::setResolution, py::arg("width"), py::arg("height"))
        .def("get_window", &Engine::getWindow, py::return_value_policy::reference_internal,
             "Get the window. Access width, height, FPS, etc.")
        .def("get_mouse", &Engine::getMouse, py::return_value_policy::reference_internal,
             "Get the mouse. Access position, clicks, and button states.")
        .def("get_keyboard", &Engine::getKeyboard, py::return_value_policy::reference_internal,
             "Get the keyboard. Use get_pressed, get_down, get_released with basilisk.key constants.")
        .def("get_frame", &Engine::getFrame, py::return_value_policy::reference_internal)
        .def("get_resource_server", &Engine::getResourceServer, py::return_value_policy::reference_internal)
        .def("get_delta_time", &Engine::getDeltaTime)
        // Keyboard convenience methods
        .def("is_key_pressed", [](Engine& e, KeyCode key) { return e.getKeyboard()->getPressed(key); },
             py::arg("key_code"),
             "True if key was pressed this frame (down now, was up last frame). Use basilisk.key.K_W, etc.")
        .def("is_key_down", [](Engine& e, KeyCode key) { return e.getKeyboard()->getDown(key); },
             py::arg("key_code"),
             "True if key is currently held down.")
        .def("is_key_released", [](Engine& e, KeyCode key) { return e.getKeyboard()->getReleased(key); },
             py::arg("key_code"),
             "True if key was released this frame.");
}