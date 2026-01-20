#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/util/resolvePath.h>
#include <filesystem>

namespace py = pybind11;

void bind_engine(py::module_&);
void bind_keyboard(py::module_&);
void bind_mouse(py::module_&);
void bind_scene(py::module_&);
void bind_node(py::module_&);
void bind_node2d(py::module_&);
void bind_image(py::module_&);
void bind_mesh(py::module_&);
void bind_shader(py::module_&);
void bind_vbo(py::module_&);
void bind_ebo(py::module_&);
void bind_vao(py::module_&);
void bind_fbo(py::module_&);
void bind_ubo(py::module_&);
void bind_frame(py::module_&);
void bind_cubemap(py::module_&);
void bind_skybox(py::module_&);
void bind_material(py::module_&);
void bind_light(py::module_&);
void bind_ambientLight(py::module_&);
void bind_directionalLight(py::module_&);
void bind_pointLight(py::module_&);
void bind_rigid(py::module_&);
void bind_solver(py::module_&);
void bind_collider(py::module_&);
void bind_force(py::module_&);
void bind_key(py::module_&);

PYBIND11_MODULE(basilisk, m, py::mod_gil_not_used()) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    // Initialize the working directory from Python's perspective
    // This is more reliable than getcwd() on macOS when Python is a framework
    try {
        py::object os = py::module_::import("os");
        py::object cwd = os.attr("getcwd")();
        std::string python_cwd = cwd.cast<std::string>();
        bsk::internal::setPythonWorkingDirectory(python_cwd);
    } catch (...) {
        // If we can't get it from Python, fall back to getcwd()
        // This will be handled by getCurrentWorkingDirectory()
    }

    // bind submodules
    // Bind key enum before keyboard (so Keyboard methods can use KeyCode)
    bind_engine(m);
    bind_key(m);
    bind_keyboard(m);
    bind_mouse(m);
    bind_scene(m);
    bind_node(m);
    bind_image(m);
    bind_mesh(m);
    bind_material(m);
    bind_node2d(m);
    bind_shader(m);
    bind_vbo(m);
    bind_ebo(m);
    bind_vao(m);
    bind_fbo(m);
    bind_frame(m);
    bind_ubo(m);
    bind_cubemap(m);
    bind_skybox(m);
    
    // Light bindings
    bind_light(m);
    bind_ambientLight(m);
    bind_directionalLight(m);
    bind_pointLight(m);

    // Physics bindings - order matters: base classes before derived
    bind_solver(m);
    bind_collider(m);  // Collider is used by Rigid, so bind it before Rigid
    bind_rigid(m);  

    // Create forces submodule
    auto forces = m.def_submodule("forces", "Physics forces submodule");
    bind_force(forces);  // Base class must be bound before derived classes

}