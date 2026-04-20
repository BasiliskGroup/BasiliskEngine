#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <array>
#include <chrono>
#include <basilisk/physics/cellular/cellBuffer.h>
#include <basilisk/basilisk.h>

constexpr glm::vec2 CELL_SCALE(1.0f, 1.0f);

// Rainbow color with uneven sine curves, full cycle every 10 seconds
static std::array<unsigned char, 3> baseBrushColorForMaterial(unsigned char mat_id) {
    switch (mat_id) {
        case 0:  return {0u,   0u,   0u};    // EMPTY
        case 1:  return {170u, 120u, 90u};   // SOLID
        case 2:  return {70u,  130u, 220u};  // WATER
        case 3:  return {160u, 200u, 160u};  // GAS
        case 4:  return {120u, 80u,  45u};   // WOOD
        case 5:  return {35u,  30u,  45u};   // OBSIDIAN
        case 6:  return {200u, 110u, 50u};   // FUSE
        case 7:  return {125u, 95u,  70u};   // MUD
        case 8:  return {170u, 130u, 95u};   // CLAY
        case 9:  return {185u, 215u, 230u};  // GLASS
        case 10: return {155u, 160u, 170u};  // METAL
        case 11: return {70u,  150u, 170u};  // PLASTIC
        case 12: return {200u, 200u, 200u}; // SNOW
        case 13: return {170u, 170u, 170u}; // VAPOR

        default: return {90u,  90u,  90u};
    }
}

static Color rainbowBrushColor(unsigned char mat_id, bool on_fire) {
    const double t = glfwGetTime() / 0.5;  // 0..1 over 10 seconds
    const auto base = baseBrushColorForMaterial(mat_id);
    auto waveChannel = [](unsigned char baseChannel, double phase) -> unsigned char {
        const int v = static_cast<int>(std::round(static_cast<double>(baseChannel) + 40.0 * std::sin(phase)));
        return static_cast<unsigned char>(std::clamp(v, 0, 255));
    };

    return Color(
        waveChannel(base[0], t),
        waveChannel(base[1], t),
        waveChannel(base[2], t),
        mat_id,
        on_fire ? 1u : 0u
    );
}

const int BUFFER_WIDTH  = 400;
const int BUFFER_HEIGHT = 400;
static_assert(BUFFER_WIDTH % CHUNK_SIZE == 0 && BUFFER_HEIGHT % CHUNK_SIZE == 0);
const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 800;
const int BRUSH_RADIUS  = BUFFER_HEIGHT / 50;

// Highest base material index selectable with arrow keys (wraps 0..MAX_MATERIAL_ID).
// Bit 4 (value 16) of the packed material field is reserved for the "static" property.
constexpr unsigned char MAX_MATERIAL_ID = 15;
constexpr unsigned char STATIC_MAT_BIT = 1u << 4u;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

static void initializeBuffer(CellBuffer& cellBuffer) {
    for (int y = 0; y < cellBuffer.getHeight(); ++y) {
        for (int x = 0; x < cellBuffer.getWidth(); ++x) {
            cellBuffer.setActivePixel(x, y, Color::Empty());
        }
    }
}

int main() {
    bsk::Engine* engine = new bsk::Engine(WINDOW_WIDTH, WINDOW_HEIGHT, "Sand Simulation", false, false);
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    scene->setCamera(new bsk::Camera2D(engine, glm::vec2(0.0f), 500.0f));
    scene->add(new bsk::Node2D(scene, nullptr, nullptr, glm::vec2(0.0f), 0.0f, glm::vec2(1.0f, 1.0f)));

    CellBuffer cellBuffer(BUFFER_WIDTH, BUFFER_HEIGHT, CELL_SCALE);
    if (!cellBuffer.initialize("shaders/physics/vertex.glsl", "shaders/physics/fragment.glsl")) {
        std::cerr << "Failed to initialize CellBuffer" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set up GPU compute for sand sim
    cellBuffer.initializeCompute();

    initializeBuffer(cellBuffer);

    unsigned char mat_id = 1;
    bool particleMode = false;
    bool fireMode = false;  // F key toggles on_fire bit for newly spawned cells
    bool staticMode = false; // S key toggles static bit in material field for newly spawned cells

    using clock = std::chrono::steady_clock;
    const auto targetFrameTime = std::chrono::milliseconds(2); // 4 FPS
    auto nextFrameTime = clock::now();

    bsk::Shader* sandShader = new bsk::Shader("shaders/sand.vert", "shaders/sand.frag");
    bsk::Frame* sandFrame = new bsk::Frame(engine, sandShader, BUFFER_WIDTH, BUFFER_HEIGHT);

    
    int enabled = glfwGetWindowAttrib(engine->getWindow()->getWindow(), GLFW_COCOA_RETINA_FRAMEBUFFER);
    std::cout << "Retina enabled: " << enabled << std::endl;

    // scaling for the buffer to match the scene scale
    glm::vec2 cameraPos = scene->getCamera()->getPosition();
    glm::vec2 cameraScale = scene->getCamera()->getViewScale();
    glm::vec2 bufferScale = cameraScale / glm::vec2(BUFFER_WIDTH, BUFFER_HEIGHT);

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        

        if (engine->getKeyboard()->getPressed(bsk::Key::K_SPACE)) {
            particleMode = !particleMode;
            std::cout << "Brush mode: " << (particleMode ? "particles" : "cells") << std::endl;
        }

        // F key toggles fire mode — newly spawned cells will have on_fire bit set
        if (engine->getKeyboard()->getPressed(bsk::Key::K_F)) {
            fireMode = !fireMode;
            std::cout << "Fire mode: " << (fireMode ? "ON" : "OFF") << std::endl;
        }

        // S key toggles static mode — newly spawned cells will have static material bit set
        if (engine->getKeyboard()->getPressed(bsk::Key::K_S)) {
            staticMode = !staticMode;
            std::cout << "Static mode: " << (staticMode ? "ON" : "OFF") << std::endl;
        }

        // Left mouse draws sand or spawns particles
        if (engine->getMouse()->getLeftDown()) {
            glm::vec2 mouseWorld = glm::vec2(
                engine->getMouse()->getWorldX(scene->getCamera()),
                engine->getMouse()->getWorldY(scene->getCamera())
            );
            
            const float halfW = BUFFER_WIDTH  * CELL_SCALE.x / 2;
            const float halfH = BUFFER_HEIGHT * CELL_SCALE.y / 2;
            int pixelX = (int)((2 * mouseWorld.x + halfW) / CELL_SCALE.x / bufferScale.x);
            int pixelY = (int)((2 * mouseWorld.y + halfH) / CELL_SCALE.y / bufferScale.y);
            
            if (pixelX >= 0 && pixelX < BUFFER_WIDTH && pixelY >= 0 && pixelY < BUFFER_HEIGHT) {
                Color brushColor = rainbowBrushColor(mat_id, fireMode);
                brushColor.mat_id =
                    static_cast<unsigned char>((brushColor.mat_id & 0x0Fu) | (staticMode ? STATIC_MAT_BIT : 0u));
                if (particleMode)
                    cellBuffer.applyParticleBrush(pixelX, pixelY, BRUSH_RADIUS, 64, brushColor);
                else
                    cellBuffer.applyBrush(pixelX, pixelY, BRUSH_RADIUS, brushColor);
            }
        }

        // upadte buffer
        cellBuffer.simulate();
        cellBuffer.updateTexture();

        // render everything
        scene->render();
        sandFrame->render(cellBuffer.getRenderTexture(), -8 * cameraPos.x, -8 * cameraPos.y, 2 * cameraScale.x, 2 * cameraScale.y);
        engine->render();
    }

    delete scene;
    delete engine;
    return 0;
}