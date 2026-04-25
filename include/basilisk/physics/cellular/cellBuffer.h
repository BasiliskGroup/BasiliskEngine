#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <basilisk/physics/cellular/color.h>
#include <basilisk/compute/gpuWrapper.hpp>

namespace bsk::internal {

inline uint32_t packCell(const Color& c, uint32_t momentum = 0u) {
    const uint32_t mat = static_cast<uint32_t>(c.mat_id) & 0x1Fu;
    const uint32_t fire = static_cast<uint32_t>(c.on_fire) & 1u;
    const uint32_t r = static_cast<uint32_t>(c.r);
    const uint32_t g = static_cast<uint32_t>(c.g);
    const uint32_t b = static_cast<uint32_t>(c.b);
    const uint32_t mom = momentum & 0x3u;
    return (mom << 30u) | (fire << 29u) | (mat << 24u) | (r << 16u) | (g << 8u) | b;
}

inline Color unpackCell(uint32_t v) {
    const uint8_t mat = static_cast<uint8_t>((v >> 24u) & 0x1Fu);
    const uint8_t fire = static_cast<uint8_t>((v >> 29u) & 1u);
    const uint8_t r = static_cast<uint8_t>((v >> 16u) & 0xFFu);
    const uint8_t g = static_cast<uint8_t>((v >> 8u) & 0xFFu);
    const uint8_t b = static_cast<uint8_t>(v & 0xFFu);
    return Color(r, g, b, mat, fire);
}

// Must match @workgroup_size in WGSL shaders
static constexpr int CHUNK_SIZE = 16;
static constexpr float GRAVITY = 32.0f; // for particle system

// All particles are read each frame but only written when created.
// Layout must match shaders/particles.wgsl (vec2, vec2, u32 color, u32 flags).
struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    uint32_t color = 0; // same packing as GPU cells: packCell(unpack, 0)
    uint32_t _pad = 0;  // bit0: active flag
};
static_assert(sizeof(Particle) == 24, "Particle must match WGSL struct layout");

class CellBuffer {
private:
    int width;
    int height;
    float cellScale; // scale of a single cell

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
    std::vector<uint32_t> gpuCellScratch;   // authoritative packed cells mirrored from GPU
    std::vector<uint32_t> gpuRenderScratch; // render staging (cells + particle overlay)

    // Pipelining: track whether an async readback is in flight
    bool pendingCellsReadback = false;
    bool pendingChunkReadback = false;
    bool pendingParticleReadback = false;

    // OpenGL
    GLuint renderTexture = 0;
    bool initialized = false;

    // GPU storage buffers
    GpuBuffer<uint32_t>* cellsA            = nullptr; // shader reads from here
    GpuBuffer<uint32_t>* cellsB            = nullptr; // shader writes here
    GpuBuffer<uint32_t>* intent            = nullptr;
    GpuBuffer<uint32_t>* claim             = nullptr;
    GpuBuffer<uint32_t>* gpuChunkActive    = nullptr;
    GpuBuffer<uint32_t>* gpuActiveChunkList = nullptr; // compact list of active chunk indices for intent dispatch
    GpuBuffer<uint32_t>* gpuChunkIntent    = nullptr;
    GpuBuffer<uint32_t>* gpuChunkActiveOut = nullptr;

    GpuBuffer<Particle>* particlesA = nullptr;
    StagingBuffer<Particle>* particlesStaging = nullptr;
    GpuBuffer<uint32_t>* particleFreeStack = nullptr;
    GpuBuffer<uint32_t>* particleFreeCount = nullptr;
    StagingBuffer<uint32_t>* particleFreeStackStaging = nullptr;
    StagingBuffer<uint32_t>* particleFreeCountStaging = nullptr;
    std::vector<Particle> particleCpu;
    std::vector<uint32_t> particleFreeStackCpu;
    uint32_t particleFreeCountCpu = 0;
    uint32_t nextParticleIndex = 0;
    uint32_t activeParticleCount = 0;
    static constexpr uint32_t MAX_PARTICLES = 100'000;
    float cellUpdatesPerSecond = 40.0f;
    float cellUpdateAccumulator = 0.0f;

    // Staging buffers (CPU-readable, written by encoder copy commands)
    StagingBuffer<uint32_t>* cellsStaging = nullptr; // full cell readback
    StagingBuffer<uint32_t>* chunkStaging = nullptr; // chunk_active_out readback

    // Shaders
    ComputeShader* intentShader   = nullptr;
    ComputeShader* resolveShader  = nullptr;
    ComputeShader* applyShader    = nullptr;
    ComputeShader* particleShader = nullptr;

    // Chunk helpers
    void markChunkDirty(int px, int py);
    void markChunkAndNeighborsDirty(int cx, int cy);

    // GL helpers
    std::string loadShaderSource(const char* filepath);
    void setupTexture();

    std::vector<Color>& getActiveBuffer() { return firstActive ? buffers.first  : buffers.second; }
    std::vector<Color>& getBackBuffer()   { return firstActive ? buffers.second : buffers.first;  }
    const std::vector<Color>& getActiveBuffer() const;
    const std::vector<Color>& getBackBuffer()   const;
    void swapBuffers() { firstActive = !firstActive; }

    bool pixelInBounds(int px, int py) const { return px >= 0 && px < width && py >= 0 && py < height; }

public:
    CellBuffer(int width, int height, float cellScale);
    ~CellBuffer();

    CellBuffer(const CellBuffer&)            = delete;
    CellBuffer& operator=(const CellBuffer&) = delete;

    bool initialize(const char* vertexShaderPath, const char* fragmentShaderPath);
    void initializeCompute();

    void updateTexture();

    Color getActivePixel(int x, int y) const;

    // all overrides of the same function
    void setActivePixel(int x, int y, const Color& color);
    void setActivePixel(int x, int y, char r, char g, char b, int mat_id, bool on_fire=false, bool is_static=false);
    void setActivePixel(glm::vec2 pos, char color[3], int mat_id, bool on_fire=false, bool is_static=false);
    void setPixel(int x, int y, char r, char g, char b, int mat_id, bool on_fire=false, bool is_static=false);
    void setPixel(glm::vec2 pos, char color[3], int mat_id, bool on_fire=false, bool is_static=false);

    void setBackPixel(int x, int y, const Color& color);
    Color getBackPixel(int x, int y) const;
    void clear(const Color& color);

    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    float getCellScale() const { return cellScale; }
    void setCellScale(float value) { cellScale = value; }

    std::vector<Color>& getData() { return getActiveBuffer(); }

    void simulate(float deltaTime);

    bool windowToPixel(int windowX, int windowY, int windowWidth, int windowHeight, int& pixelX, int& pixelY) const;
    bool worldToPixel(const glm::vec2& worldPos, int& pixelX, int& pixelY) const;
    void pixelToWorld(int pixelX, int pixelY, glm::vec2& worldPos) const;

    void applyBrush(int pixelX, int pixelY, int radius, const Color& color);
    void applyParticleBrush(int pixelX, int pixelY, int radius, uint32_t spawnCount, const Color& color);

    // basically what we're gonna do is dance
    bool addParticle(const glm::vec2& pos, const glm::vec2& vel, const Color& color);
    bool addParticle(const glm::vec2& pos, const glm::vec2& vel, char color[3], int mat_id, bool on_fire=false, bool is_static=false);

    unsigned int getRenderTexture() { return renderTexture; }

    // particle getter
    const std::vector<Particle>& getParticles() const { return particleCpu; }
};

}