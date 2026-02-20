#include <pybind11/pybind11.h>
#include <basilisk/engine/engine.h>
#include <basilisk/IO/keyboard.h>
#include <basilisk/IO/mouse.h>
#include <basilisk/camera/staticCamera2d.h>

namespace py = pybind11;

void bind_engine(py::module_& m) {
    using namespace bsk::internal;

    py::class_<Engine>(m, "Engine")
        .def(py::init<int, int, const char*, bool, bool>(),
             py::arg("width") = 800,
             py::arg("height") = 800,
             py::arg("title") = "Basilisk",
             py::arg("auto_mouse_grab") = true,
             py::arg("show_splash") = false)
             
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
             "True if key was released this frame.")
        // Mouse convenience methods
        .def("get_mouse_x", [](Engine& e) { return e.getMouse()->getX(); },
             "Mouse X position (screen coordinates, scaled).")
        .def("get_mouse_y", [](Engine& e) { return e.getMouse()->getY(); },
             "Mouse Y position (screen coordinates, scaled).")
        .def("get_mouse_world_x", [](Engine& e, StaticCamera2D* camera) { return e.getMouse()->getWorldX(camera); },
             py::arg("camera"),
             "Mouse X in world coordinates. Pass the scene's camera, e.g. scene.get_camera().")
        .def("get_mouse_world_y", [](Engine& e, StaticCamera2D* camera) { return e.getMouse()->getWorldY(camera); },
             py::arg("camera"),
             "Mouse Y in world coordinates. Pass the scene's camera, e.g. scene.get_camera().")
        .def("is_left_down", [](Engine& e) { return e.getMouse()->getLeftDown(); },
             "True if left mouse button is held down.")
        .def("is_middle_down", [](Engine& e) { return e.getMouse()->getMiddleDown(); },
             "True if middle mouse button is held down.")
        .def("is_right_down", [](Engine& e) { return e.getMouse()->getRightDown(); },
             "True if right mouse button is held down.")
        .def("is_left_clicked", [](Engine& e) { return e.getMouse()->getLeftClicked(); },
             "True if left button was clicked this frame (down now, was up last frame).")
        .def("is_middle_clicked", [](Engine& e) { return e.getMouse()->getMiddleClicked(); },
             "True if middle button was clicked this frame.")
        .def("is_right_clicked", [](Engine& e) { return e.getMouse()->getRightClicked(); },
             "True if right button was clicked this frame.")
        .def("is_left_released", [](Engine& e) { return e.getMouse()->getLeftReleased(); },
             "True if left button was released this frame.")
        .def("is_middle_released", [](Engine& e) { return e.getMouse()->getMiddleReleased(); },
             "True if middle button was released this frame.")
        .def("is_right_released", [](Engine& e) { return e.getMouse()->getRightReleased(); },
             "True if right button was released this frame.")
        .def("show_splash", &Engine::showSplash);
}