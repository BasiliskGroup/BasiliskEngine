#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_engine(py::module_&);

PYBIND11_MODULE(basilisk_python, m, py::mod_gil_not_used()) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    // bind submodules
    bind_engine(m);
}