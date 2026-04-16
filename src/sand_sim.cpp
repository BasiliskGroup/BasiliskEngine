#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <array>
#include <chrono>
#include <thread>
#include <basilisk/physics/cellular/renderBuffer.h>

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

const int BUFFER_WIDTH  = 512;
const int BUFFER_HEIGHT = 512;
static_assert(BUFFER_WIDTH % CHUNK_SIZE == 0 && BUFFER_HEIGHT % CHUNK_SIZE == 0);
const int WINDOW_WIDTH  = 900;
const int WINDOW_HEIGHT = 900;
const int BRUSH_RADIUS  = BUFFER_HEIGHT / 50;

// Highest base material index selectable with arrow keys (wraps 0..MAX_MATERIAL_ID).
// Bit 4 (value 16) of the packed material field is reserved for the "static" property.
constexpr unsigned char MAX_MATERIAL_ID = 15;
constexpr unsigned char STATIC_MAT_BIT = 1u << 4u;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

static void initializeBuffer(RenderBuffer& renderBuffer) {
    for (int y = 0; y < renderBuffer.getHeight(); ++y) {
        for (int x = 0; x < renderBuffer.getWidth(); ++x) {
            renderBuffer.setActivePixel(x, y, Color::Empty());
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sand Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    RenderBuffer renderBuffer(BUFFER_WIDTH, BUFFER_HEIGHT);
    if (!renderBuffer.initialize("shaders/physics/vertex.glsl", "shaders/physics/fragment.glsl")) {
        std::cerr << "Failed to initialize RenderBuffer" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set up GPU compute for sand sim
    renderBuffer.initializeCompute();
    glfwSetWindowTitle(window, "Sand Simulation (GPU)");

    initializeBuffer(renderBuffer);

    unsigned char mat_id = 1;
    bool particleMode = false;
    bool fireMode = false;  // F key toggles on_fire bit for newly spawned cells
    bool staticMode = false; // S key toggles static bit in material field for newly spawned cells

    using clock = std::chrono::steady_clock;
    const auto targetFrameTime = std::chrono::milliseconds(2); // 4 FPS
    auto nextFrameTime = clock::now();

    while (!glfwWindowShouldClose(window)) {
        nextFrameTime += targetFrameTime;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        static bool lastSpaceState = false;
        bool currentSpaceState = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (currentSpaceState && !lastSpaceState) {
            particleMode = !particleMode;
            std::cout << "Brush mode: " << (particleMode ? "particles" : "cells") << std::endl;
        }
        lastSpaceState = currentSpaceState;

        // F key toggles fire mode — newly spawned cells will have on_fire bit set
        static bool lastFState = false;
        bool fDown = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (fDown && !lastFState) {
            fireMode = !fireMode;
            std::cout << "Fire mode: " << (fireMode ? "ON" : "OFF") << std::endl;
        }
        lastFState = fDown;

        // S key toggles static mode — newly spawned cells will have static material bit set
        static bool lastSState = false;
        bool sDown = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        if (sDown && !lastSState) {
            staticMode = !staticMode;
            std::cout << "Static mode: " << (staticMode ? "ON" : "OFF") << std::endl;
        }
        lastSState = sDown;

        // Left mouse draws sand or spawns particles
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int windowWidth, windowHeight;
            glfwGetWindowSize(window, &windowWidth, &windowHeight);

            int pixelX, pixelY;
            if (renderBuffer.windowToPixel(static_cast<int>(xpos), static_cast<int>(ypos),
                                           windowWidth, windowHeight, pixelX, pixelY)) {
                Color brushColor = rainbowBrushColor(mat_id, fireMode);
                brushColor.mat_id =
                    static_cast<unsigned char>((brushColor.mat_id & 0x0Fu) | (staticMode ? STATIC_MAT_BIT : 0u));
                if (particleMode) {
                    renderBuffer.applyParticleBrush(pixelX, pixelY, BRUSH_RADIUS, 64, brushColor);
                } else {
                    renderBuffer.applyBrush(pixelX, pixelY, BRUSH_RADIUS, brushColor);
                }
            }
        }
        
        // Arrow Right = next id, Left = previous (wraps 0..MAX_MATERIAL_ID)
        static bool lastMatLeft = false;
        static bool lastMatRight = false;
        const bool matLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
        const bool matRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
        if (matRight && !lastMatRight) {
            mat_id = (mat_id >= MAX_MATERIAL_ID) ? 0 : static_cast<unsigned char>(mat_id + 1);
            std::cout << "Material id: " << static_cast<int>(mat_id) << std::endl;
        }
        if (matLeft && !lastMatLeft) {
            mat_id = (mat_id == 0) ? MAX_MATERIAL_ID : static_cast<unsigned char>(mat_id - 1);
            std::cout << "Material id: " << static_cast<int>(mat_id) << std::endl;
        }
        lastMatLeft = matLeft;
        lastMatRight = matRight;

        renderBuffer.simulate();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderBuffer.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_until(nextFrameTime);
    }

    glfwTerminate();
    return 0;
}