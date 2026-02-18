#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/image.h>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace py = pybind11;
using namespace bsk::internal;

void bind_image(py::module_& m) {
    py::class_<Image>(m, "Image")
        .def(py::init<std::string>(), py::arg("file"))
        .def(py::init<const std::vector<float>&, int, int, int>(), py::arg("data"), py::arg("width"), py::arg("height"), py::arg("nChannels") = 4)
        .def("get_width", &Image::getWidth)
        .def("get_height", &Image::getHeight)
        .def("get_data", &Image::getData);
}