#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_engine(py::module_&);
void bind_solver(py::module_&);
void bind_image(py::module_&);

PYBIND11_MODULE(basilisk, m, py::mod_gil_not_used()) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    // bind submodules
    bind_engine(m);
    bind_solver(m);
    bind_image(m);
}