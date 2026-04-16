#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <basilisk/physics/cellular/color.h>
#include <basilisk/compute/gpuWrapper.hpp>

namespace bci = bsk::internal;

// Must match @workgroup_size in WGSL shaders
static constexpr int CHUNK_SIZE = 16;
static constexpr float CELL_WIDTH = 1.0f;
static constexpr float GRAVITY = 9.8f;

class RenderBuffer {
public:
    RenderBuffer(int width, int height);
    ~RenderBuffer();

    RenderBuffer(const RenderBuffer&)            = delete;
    RenderBuffer& operator=(const RenderBuffer&) = delete;

    bool initialize(const char* vertexShaderPath, const char* fragmentShaderPath);
    void initializeCompute();

    void render();

    void setActivePixel(int x, int y, const Color& color);
    Color getActivePixel(int x, int y) const;
    void setBackPixel(int x, int y, const Color& color);
    Color getBackPixel(int x, int y) const;
    void clear(const Color& color);

    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    std::vector<Color>& getData() { return getActiveBuffer(); }

    void simulate();
    void simulateGPU();

    bool windowToPixel(int windowX, int windowY, int windowWidth, int windowHeight,
                       int& pixelX, int& pixelY) const;

    void applyBrush(int pixelX, int pixelY, int radius, const Color& color);
    void applyParticleBrush(int pixelX, int pixelY, int radius, uint32_t spawnCount, const Color& color);

private:
    int width;
    int height;

    struct BrushPixel { int idx; Color color; };
    std::vector<BrushPixel> pendingBrushPixels;  // pixels to inject into GPU each frame
    std::vector<int> pendingBrushChunks;         // chunk indices marked dirty by brush this frame

    // Chunk grid
    int chunksWide;
    int chunksHigh;
    int numChunks;

    // CPU-side double buffers for pixel data
    std::pair<std::vector<Color>, std::vector<Color>> buffers;
    bool isLeftFrame = true;
    bool firstActive = true;
    bool computeInitialized = false;

    // CPU-side chunk state
    std::vector<uint32_t> chunkActive;    // fed to GPU each frame
    std::vector<uint32_t> chunkActiveOut; // staging readback lands here
    std::vector<uint32_t> gpuCellScratch; // temporary packed cell storage for GPU I/O

    // Pipelining: track whether an async readback is in flight
    bool pendingCellsReadback = false;
    bool pendingChunkReadback = false;
    bool pendingParticleReadback = false;

    // OpenGL
    GLuint shaderProgram = 0;
    GLuint VAO = 0, VBO = 0, EBO = 0;
    GLuint texture = 0;
    GLuint particleProgram = 0;
    GLuint particleVAO = 0;
    GLuint particleVBO = 0;
    bool initialized = false;

    // GPU storage buffers
    bci::GpuBuffer<uint32_t>* cellsA            = nullptr; // shader reads from here
    bci::GpuBuffer<uint32_t>* cellsB            = nullptr; // shader writes here
    bci::GpuBuffer<uint32_t>* intent            = nullptr;
    bci::GpuBuffer<uint32_t>* claim             = nullptr;
    bci::GpuBuffer<uint32_t>* gpuChunkActive    = nullptr;
    bci::GpuBuffer<uint32_t>* gpuActiveChunkList = nullptr; // compact list of active chunk indices for intent dispatch
    bci::GpuBuffer<uint32_t>* gpuChunkIntent    = nullptr;
    bci::GpuBuffer<uint32_t>* gpuChunkActiveOut = nullptr;

    // All particles are read each frame but only written when created.
    // Layout must match shaders/particles.wgsl (vec2, vec2, u32 color, u32 flags).
    struct Particle {
        glm::vec2 pos;
        glm::vec2 vel;
        uint32_t color = 0; // same packing as GPU cells: packCell(unpack, 0)
        uint32_t _pad = 0;  // bit0: active flag
    };
    static_assert(sizeof(Particle) == 24, "Particle must match WGSL struct layout");

    bci::GpuBuffer<Particle>* particlesA = nullptr;
    bci::StagingBuffer<Particle>* particlesStaging = nullptr;
    bci::GpuBuffer<uint32_t>* particleFreeStack = nullptr;
    bci::GpuBuffer<uint32_t>* particleFreeCount = nullptr;
    bci::StagingBuffer<uint32_t>* particleFreeStackStaging = nullptr;
    bci::StagingBuffer<uint32_t>* particleFreeCountStaging = nullptr;
    std::vector<Particle> particleCpu;
    std::vector<glm::vec2> particleRenderPos;
    std::vector<glm::vec3> particleRenderColor;
    std::vector<float> particleRenderInterleaved;
    std::vector<uint32_t> particleFreeStackCpu;
    uint32_t particleFreeCountCpu = 0;
    uint32_t nextParticleIndex = 0;
    uint32_t activeParticleCount = 0;
    static constexpr uint32_t MAX_PARTICLES = 100'000;

    // Staging buffers (CPU-readable, written by encoder copy commands)
    bci::StagingBuffer<uint32_t>* cellsStaging = nullptr; // full cell readback
    bci::StagingBuffer<uint32_t>* chunkStaging = nullptr; // chunk_active_out readback

    // Shaders
    bci::ComputeShader* intentShader   = nullptr;
    bci::ComputeShader* resolveShader  = nullptr;
    bci::ComputeShader* applyShader    = nullptr;
    bci::ComputeShader* particleShader = nullptr;

    // Chunk helpers
    void markChunkDirty(int px, int py);
    void markChunkAndNeighborsDirty(int cx, int cy);

    // GL helpers
    std::string loadShaderSource(const char* filepath);
    GLuint compileShader(GLenum type, const char* source);
    GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);
    void setupQuad();
    void setupTexture();

    std::vector<Color>& getActiveBuffer() { return firstActive ? buffers.first  : buffers.second; }
    std::vector<Color>& getBackBuffer()   { return firstActive ? buffers.second : buffers.first;  }
    const std::vector<Color>& getActiveBuffer() const;
    const std::vector<Color>& getBackBuffer()   const;
    void swapBuffers() { firstActive = !firstActive; }
};