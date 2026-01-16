#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/util/resolvePath.h>
#include <filesystem>

namespace py = pybind11;

void bind_engine(py::module_&);
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
void bind_frame(py::module_&);
void bind_material(py::module_&);
void bind_rigid(py::module_&);
void bind_solver(py::module_&);
void bind_collider(py::module_&);
void bind_force(py::module_&);
void bind_joint(py::module_&);
void bind_spring(py::module_&);
void bind_motor(py::module_&);
void bind_manifold(py::module_&);

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
    bind_engine(m);
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
    
    // Physics bindings - order matters: base classes before derived
    bind_solver(m);
    bind_collider(m);  // Collider is used by Rigid, so bind it before Rigid
    bind_rigid(m);
    bind_force(m);  // Base class must be bound before derived classes
    bind_joint(m);
    bind_spring(m);
    bind_motor(m);
    bind_manifold(m);
}