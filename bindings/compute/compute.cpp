#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

#include <basilisk/compute/gpuWrapper.hpp>

namespace py = pybind11;
using namespace bsk::internal;

namespace {

GpuBufferDtype dtype_from_string(const std::string& s) {
    if (s == "u32" || s == "uint32") return GpuBufferDtype::U32;
    if (s == "f32" || s == "float32" || s == "float") return GpuBufferDtype::F32;
    if (s == "i32" || s == "int32" || s == "int") return GpuBufferDtype::I32;
    throw std::invalid_argument("dtype must be 'u32', 'f32', or 'i32' (or uint32/float32/int32)");
}

std::string dtype_to_string(GpuBufferDtype d) {
    switch (d) {
        case GpuBufferDtype::U32: return "u32";
        case GpuBufferDtype::F32: return "f32";
        case GpuBufferDtype::I32: return "i32";
    }
    return "u32";
}

} // namespace

void bind_compute(py::module_& m) {
    m.def("init_gpu", &initGpu, "Initialize the GPU compute backend (call once at startup).");

    py::enum_<GpuBufferDtype>(m, "GpuBufferDtype")
        .value("U32", GpuBufferDtype::U32)
        .value("F32", GpuBufferDtype::F32)
        .value("I32", GpuBufferDtype::I32)
        .export_values();

    py::class_<GpuBufferAny>(m, "GpuBuffer")
        .def(py::init([](const py::object& dtype, size_t len) {
            GpuBufferDtype d;
            if (py::isinstance<py::str>(dtype)) {
                d = dtype_from_string(dtype.cast<std::string>());
            } else if (py::hasattr(dtype, "name")) {
                // e.g. numpy dtype
                std::string name = dtype.attr("name").cast<std::string>();
                d = dtype_from_string(name);
            } else {
                py::object builtins = py::module_::import("builtins");
                if (dtype.is(builtins.attr("float"))) {
                    d = GpuBufferDtype::F32;
                } else if (dtype.is(builtins.attr("int"))) {
                    d = GpuBufferDtype::I32;
                } else {
                    throw std::invalid_argument(
                        "dtype must be a string ('u32', 'f32', 'i32'), a type (int, float), or an object with .name");
                }
            }
            return new GpuBufferAny(d, len);
        }), py::arg("dtype"), py::arg("len"),
            "Create a GPU buffer. dtype: 'u32'|'f32'|'i32', type (int/float), or object with .name; len: element count.")
        .def("write", [](GpuBufferAny& self, py::sequence data) {
            const size_t n = self.len();
            const size_t count = static_cast<size_t>(py::len(data));
            if (count > n) {
                throw std::runtime_error("write: data length exceeds buffer length");
            }
            const size_t byte_count = count * self.element_size();
            std::vector<uint8_t> bytes(byte_count);
            uint8_t* ptr = bytes.data();
            switch (self.dtype()) {
                case GpuBufferDtype::U32:
                    for (size_t i = 0; i < count; ++i)
                        *reinterpret_cast<uint32_t*>(ptr + i * 4) = static_cast<uint32_t>(py::cast<int64_t>(data[i]));
                    break;
                case GpuBufferDtype::F32:
                    for (size_t i = 0; i < count; ++i)
                        *reinterpret_cast<float*>(ptr + i * 4) = py::cast<float>(data[i]);
                    break;
                case GpuBufferDtype::I32:
                    for (size_t i = 0; i < count; ++i)
                        *reinterpret_cast<int32_t*>(ptr + i * 4) = py::cast<int32_t>(data[i]);
                    break;
            }
            self.writeBytes(bytes.data(), byte_count);
        }, py::arg("data"), "Write a sequence of elements (length must not exceed buffer len).")
        .def("read", [](GpuBufferAny& self) {
            const size_t count = self.len();
            const size_t byte_count = count * self.element_size();
            std::vector<uint8_t> bytes(byte_count);
            self.readBytes(bytes.data(), byte_count);
            const uint8_t* ptr = bytes.data();
            py::list out;
            switch (self.dtype()) {
                case GpuBufferDtype::U32:
                    for (size_t i = 0; i < count; ++i)
                        out.append(*reinterpret_cast<const uint32_t*>(ptr + i * 4));
                    break;
                case GpuBufferDtype::F32:
                    for (size_t i = 0; i < count; ++i)
                        out.append(*reinterpret_cast<const float*>(ptr + i * 4));
                    break;
                case GpuBufferDtype::I32:
                    for (size_t i = 0; i < count; ++i)
                        out.append(*reinterpret_cast<const int32_t*>(ptr + i * 4));
                    break;
            }
            return out;
        }, "Read buffer into a list of elements.")
        .def("len", &GpuBufferAny::len)
        .def("size_bytes", &GpuBufferAny::size_bytes)
        .def("element_size", &GpuBufferAny::element_size)
        .def("dtype", [](const GpuBufferAny& self) { return dtype_to_string(self.dtype()); },
             "Return dtype string: 'u32', 'f32', or 'i32'.")
        .def("handle", [](GpuBufferAny& self) { return reinterpret_cast<intptr_t>(self.handle()); },
             "Return opaque handle for use with ComputeShader buffer_handles.");

    py::class_<ComputeShader>(m, "ComputeShader")
        .def(py::init([](const std::string& wgsl, py::list handles_list, size_t uniform_size) {
            std::vector<void*> handles;
            for (py::handle item : handles_list) {
                handles.push_back(reinterpret_cast<void*>(item.cast<intptr_t>()));
            }
            return new ComputeShader(wgsl, handles, uniform_size);
        }), py::arg("wgsl"), py::arg("buffer_handles"), py::arg("uniform_size") = 0)
        .def("set_uniform_bytes", [](ComputeShader& self, py::buffer b) {
            py::buffer_info info = b.request();
            if (info.ndim != 1) {
                throw std::runtime_error("set_uniform_bytes expects a 1-dimensional buffer");
            }
            const uint8_t* ptr = static_cast<const uint8_t*>(info.ptr);
            size_t len = static_cast<size_t>(info.size) * info.itemsize;
            self.setUniformBytes(ptr, len);
        }, py::arg("data"), "Set uniform data from a bytes-like or buffer object.")
        .def("dispatch", &ComputeShader::dispatch,
             py::arg("x"), py::arg("y") = 1, py::arg("z") = 1,
             "Dispatch the compute shader with workgroup counts (x, y, z).");
}
