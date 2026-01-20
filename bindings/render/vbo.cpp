#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/vbo.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_vbo(py::module_& m) {
    py::class_<VBO>(m, "VBO")
        .def(py::init<const void*, unsigned int, unsigned int>(), py::arg("data"), py::arg("size"), py::arg("drawType"))
        .def(py::init([](py::sequence data, unsigned int drawType) {
            std::vector<float> floatData;
            floatData.reserve(py::len(data));
            for (auto item : data) {
                floatData.push_back(py::cast<float>(item));
            }
            return new VBO(floatData, drawType);
        }), py::arg("data"), py::arg("drawType") = GL_STATIC_DRAW)
        .def("bind", &VBO::bind)
        .def("unbind", &VBO::unbind)
        .def("get_size", &VBO::getSize);
}