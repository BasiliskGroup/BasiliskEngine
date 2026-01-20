#include <pybind11/pybind11.h>
#include <basilisk/render/image.h>
#include <string>

namespace py = pybind11;

void bind_image(py::module_& m) {
    py::class_<bsk::internal::Image>(m, "Image")
        .def(py::init<std::string>(), py::arg("file"))
        .def("get_width", &bsk::internal::Image::getWidth)
        .def("get_height", &bsk::internal::Image::getHeight)
        .def("get_data", &bsk::internal::Image::getData);
}