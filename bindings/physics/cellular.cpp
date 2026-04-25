#include <pybind11/pybind11.h>
#include <basilisk/physics/cellular/color.h>
#include <basilisk/physics/cellular/cellBuffer.h>
#include "glm/glmCasters.hpp"

namespace py = pybind11;
using namespace bsk::internal;

namespace {

using SetActivePixelColor = void (CellBuffer::*)(int, int, const Color&);
using SetActivePixelComponents = void (CellBuffer::*)(int, int, char, char, char, int, bool, bool);
using AddParticleColor = bool (CellBuffer::*)(const glm::vec2&, const glm::vec2&, const Color&);

} // namespace

void bind_cellular(py::module_& m) {
    py::class_<Color>(m, "Color")
        .def(py::init<unsigned char, unsigned char, unsigned char, unsigned char, unsigned char>(),
             py::arg("r") = 0,
             py::arg("g") = 0,
             py::arg("b") = 0,
             py::arg("mat_id") = 0,
             py::arg("on_fire") = 0)
        .def_readwrite("r", &Color::r)
        .def_readwrite("g", &Color::g)
        .def_readwrite("b", &Color::b)
        .def_readwrite("mat_id", &Color::mat_id)
        .def_readwrite("on_fire", &Color::on_fire)
        .def_static("white", &Color::White)
        .def_static("empty", &Color::Empty)
        .def_static("sand", &Color::Sand)
        .def_static("water", &Color::Water);

    py::class_<CellParticle>(m, "CellParticle")
        .def_readonly("pos", &CellParticle::pos)
        .def_readonly("vel", &CellParticle::vel)
        .def_readonly("color", &CellParticle::color);

    py::class_<CellBuffer>(m, "CellBuffer")
        .def("initialize_compute", &CellBuffer::initializeCompute)
        .def("update_texture", &CellBuffer::updateTexture)
        .def("set_active_pixel", static_cast<SetActivePixelColor>(&CellBuffer::setActivePixel), py::arg("x"), py::arg("y"), py::arg("color"))
        .def("set_active_pixel", static_cast<SetActivePixelComponents>(&CellBuffer::setActivePixel), py::arg("x"), py::arg("y"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("mat_id"), py::arg("on_fire") = false, py::arg("is_static") = false)
        .def("set_active_pixel", [](CellBuffer& self, const glm::vec2& pos, unsigned char r, unsigned char g, unsigned char b, int mat_id, bool on_fire, bool is_static) {
            char c[3] = { static_cast<char>(r), static_cast<char>(g), static_cast<char>(b) };
            self.setActivePixel(pos, c, mat_id, on_fire, is_static);
        }, py::arg("pos"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("mat_id"), py::arg("on_fire") = false, py::arg("is_static") = false)
        .def("get_active_pixel", &CellBuffer::getActivePixel, py::arg("x"), py::arg("y"))
        .def("set_back_pixel", &CellBuffer::setBackPixel, py::arg("x"), py::arg("y"), py::arg("color"))
        .def("get_back_pixel", &CellBuffer::getBackPixel, py::arg("x"), py::arg("y"))
        .def("clear", &CellBuffer::clear, py::arg("color"))
        .def("simulate", &CellBuffer::simulate, py::arg("delta_time"))
        .def("window_to_pixel", [](const CellBuffer& cb, int windowX, int windowY, int windowWidth, int windowHeight) {
            int pixelX = -1;
            int pixelY = -1;
            const bool ok = cb.windowToPixel(windowX, windowY, windowWidth, windowHeight, pixelX, pixelY);
            return py::make_tuple(ok, pixelX, pixelY);
        }, py::arg("window_x"), py::arg("window_y"), py::arg("window_width"), py::arg("window_height"))
        .def("world_to_pixel", [](const CellBuffer& cb, const glm::vec2& worldPos) {
            int pixelX = -1;
            int pixelY = -1;
            const bool ok = cb.worldToPixel(worldPos, pixelX, pixelY);
            return py::make_tuple(ok, pixelX, pixelY);
        }, py::arg("world_pos"))
        .def("pixel_to_world", [](const CellBuffer& cb, int pixelX, int pixelY) {
            glm::vec2 worldPos(0.0f, 0.0f);
            cb.pixelToWorld(pixelX, pixelY, worldPos);
            return worldPos;
        }, py::arg("pixel_x"), py::arg("pixel_y"))
        .def("apply_brush", &CellBuffer::applyBrush, py::arg("pixel_x"), py::arg("pixel_y"), py::arg("radius"), py::arg("color"))
        .def("apply_particle_brush", &CellBuffer::applyParticleBrush, py::arg("pixel_x"), py::arg("pixel_y"), py::arg("radius"), py::arg("spawn_count"), py::arg("color"))
        .def("add_particle", static_cast<AddParticleColor>(&CellBuffer::addParticle), py::arg("pos"), py::arg("vel"), py::arg("color"))
        .def("add_particle", [](CellBuffer& self, const glm::vec2& pos, const glm::vec2& vel, unsigned char r, unsigned char g, unsigned char b, int mat_id, bool on_fire, bool is_static) {
            char c[3] = { static_cast<char>(r), static_cast<char>(g), static_cast<char>(b) };
            return self.addParticle(pos, vel, c, mat_id, on_fire, is_static);
        }, py::arg("pos"), py::arg("vel"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("mat_id"), py::arg("on_fire") = false, py::arg("is_static") = false)
        .def("get_width", &CellBuffer::getWidth)
        .def("get_height", &CellBuffer::getHeight)
        .def("get_cell_scale", &CellBuffer::getCellScale)
        .def("set_cell_scale", &CellBuffer::setCellScale, py::arg("value"))
        .def("get_render_texture", &CellBuffer::getRenderTexture);
}
