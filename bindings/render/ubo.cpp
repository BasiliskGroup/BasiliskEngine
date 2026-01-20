#include <pybind11/pybind11.h>
#include <basilisk/render/ubo.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_ubo(py::module_& m) {
    py::class_<UBO>(m, "UBO")
        .def(py::init<const void*, unsigned int, unsigned int>(), py::arg("data"), py::arg("size"), py::arg("drawType"))
        .def(py::init([](const std::vector<float>& data, unsigned int drawType) {
            return new UBO(data, drawType);
        }), py::arg("data"), py::arg("drawType") = GL_STATIC_DRAW)
        .def("bind", &UBO::bind)
        .def("unbind", &UBO::unbind)
        .def("get_size", &UBO::getSize)
        .def("write", static_cast<void (UBO::*)(const void*, unsigned int, unsigned int)>(&UBO::write), py::arg("data"), py::arg("size"), py::arg("offset"));
}