#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/cubemap.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_cubemap(py::module_& m) {
    py::class_<Cubemap>(m, "Cubemap")
        .def(py::init<const std::vector<Image*>&>(), py::arg("faces"))
        .def(py::init<const std::vector<std::string>&>(), py::arg("faces"))
        .def("bind", &Cubemap::bind)
        .def("unbind", &Cubemap::unbind)
        .def("get_id", &Cubemap::getID);
}