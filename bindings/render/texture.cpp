#include <pybind11/pybind11.h>
#include <basilisk/render/texture.h>

namespace py = pybind11;
using namespace bsk::internal;

void bind_texture(py::module_& m) {
    py::class_<Texture>(m, "Texture")
        .def("bind", &Texture::bind)
        .def("set_filter", &Texture::setFilter, py::arg("mag_filter"), py::arg("min_filter"))
        .def("set_wrap", &Texture::setWrap, py::arg("wrap"))
        .def("get_id", &Texture::getID)
        .def("get_width", &Texture::getWidth)
        .def("get_height", &Texture::getHeight);
}
