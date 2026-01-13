#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_engine(py::module_&);
void bind_scene(py::module_&);
void bind_node(py::module_&);
void bind_node2d(py::module_&);
void bind_image(py::module_&);
void bind_mesh(py::module_&);
void bind_material(py::module_&);
void bind_solver(py::module_&);
void bind_rigid(py::module_&);
void bind_force(py::module_&);
void bind_joint(py::module_&);
void bind_spring(py::module_&);
void bind_motor(py::module_&);
void bind_manifold(py::module_&);

PYBIND11_MODULE(basilisk, m, py::mod_gil_not_used()) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    // bind submodules
    bind_engine(m);
    bind_scene(m);
    bind_node(m);
    bind_image(m);
    bind_mesh(m);
    bind_material(m);
    bind_node2d(m);
    
    // Physics bindings - order matters: base classes before derived
    bind_solver(m);
    bind_rigid(m);
    bind_force(m);  // Base class must be bound before derived classes
    bind_joint(m);
    bind_spring(m);
    bind_motor(m);
    bind_manifold(m);
}