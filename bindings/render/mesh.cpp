#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <basilisk/render/mesh.h>

namespace py = pybind11;

void bind_mesh(py::module_& m) {
    using Mesh = bsk::internal::Mesh;
    py::class_<Mesh, std::shared_ptr<Mesh>>(m, "Mesh")
        .def(py::init<const std::string&, bool, bool>(), py::arg("modelPath"), py::arg("generateUV") = false, py::arg("generateNormals") = false)
        .def(py::init([](const std::vector<float>& vertices) {
            return std::make_shared<Mesh>(vertices);
        }), py::arg("vertices"))
        .def(py::init([](const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
            return std::make_shared<Mesh>(vertices, indices);
        }), py::arg("vertices"), py::arg("indices"))
        .def("get_vertices", &Mesh::getVertices)
        .def("get_indices", &Mesh::getIndices);
}