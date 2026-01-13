#include <pybind11/pybind11.h>
#include <basilisk/physics/solver.h>

namespace py = pybind11;

void bind_rigid(py::module_& m);

void bind_solver(py::module_& m) {
    py::class_<bsk::internal::Solver>(m, "Solver")
        .def(py::init<>())
        .def("step", &bsk::internal::Solver::step);

    bind_rigid(m);
}