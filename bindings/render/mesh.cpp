#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/mesh.h>

namespace py = pybind11;

void bind_mesh(py::module_& m) {
    py::class_<bsk::internal::Mesh>(m, "Mesh")
        .def(py::init<const std::string&, bool, bool>(), py::arg("modelPath"), py::arg("generateUV") = false, py::arg("generateNormals") = false)
        .def("get_vertices", &bsk::internal::Mesh::getVertices)
        .def("get_indices", &bsk::internal::Mesh::getIndices);
}