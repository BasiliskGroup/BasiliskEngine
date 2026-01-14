#include <pybind11/pybind11.h>
#include <basilisk/render/image.h>
#include <string>

namespace py = pybind11;

void bind_image(py::module_& m) {
    py::class_<bsk::internal::Image>(m, "Image")
        .def(py::init<std::string>(), py::arg("file"))
        .def("getWidth", &bsk::internal::Image::getWidth)
        .def("getHeight", &bsk::internal::Image::getHeight)
        .def("getData", &bsk::internal::Image::getData);
}