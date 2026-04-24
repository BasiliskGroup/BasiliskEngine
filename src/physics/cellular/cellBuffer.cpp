#include <basilisk/physics/cellular/cellBuffer.h>
#include <basilisk/compute/gpuWrapper.hpp>
#include <basilisk/util/resolvePath.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <random>


using namespace bsk::internal;

namespace {
struct ParticleVertex {
    glm::vec2 pos;
    glm::vec3 color;
};

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
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

CellBuffer::CellBuffer(int width, int height, float cellScale)
    : width(width), height(height), cellScale(cellScale),
      buffers({ std::vector<Color>(width * height), std::vector<Color>(width * height) }),
      chunksWide((width  + CHUNK_SIZE - 1) / CHUNK_SIZE),
      chunksHigh((height + CHUNK_SIZE - 1) / CHUNK_SIZE),
      numChunks (((width  + CHUNK_SIZE - 1) / CHUNK_SIZE) *
                 ((height + CHUNK_SIZE - 1) / CHUNK_SIZE)),
      chunkActive   (numChunks, 1u),  // start fully active so frame 0 runs everywhere
      chunkActiveOut(numChunks, 0u)
{}

CellBuffer::~CellBuffer() {
    if (initialized) {
        glDeleteTextures(1, &renderTexture);
    }

    if (computeInitialized) {
        // sand
        delete cellsA;
        delete cellsB;
        delete intent;
        delete claim;
        delete gpuChunkActive;
        delete gpuActiveChunkList;
        delete gpuChunkIntent;
        delete gpuChunkActiveOut;
        delete cellsStaging;
        delete chunkStaging;
        delete intentShader;
        delete resolveShader;
        delete applyShader;

        // particles
        delete particlesA;
        delete particlesStaging;
        delete particleFreeStack;
        delete particleFreeCount;
        delete particleFreeStackStaging;
        delete particleFreeCountStaging;
        delete particleShader;
    }
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

bool CellBuffer::initialize(const char* vertexShaderPath, const char* fragmentShaderPath) {
    if (initialized) {
        std::cerr << "CellBuffer already initialized!" << std::endl;
        return false;
    }

    setupTexture();

    initialized = true;
    return true;
}

void CellBuffer::initializeCompute() {
    if (computeInitialized) return;

    std::cout << "Initializing GPU compute..." << std::endl;
    initGpu();

    const size_t cellCount  = width * height;
    gpuCellScratch.resize(cellCount);

    // Storage buffers
    cellsA             = new GpuBufferU32(cellCount);
    cellsB             = new GpuBufferU32(cellCount);
    intent             = new GpuBufferU32(cellCount);
    claim              = new GpuBufferU32(cellCount);
    gpuChunkActive     = new GpuBufferU32(numChunks);
    gpuActiveChunkList = new GpuBufferU32(numChunks);
    gpuChunkIntent     = new GpuBufferU32(numChunks);
    gpuChunkActiveOut  = new GpuBufferU32(numChunks);

    // particles
    particlesA = new GpuBuffer<Particle>(MAX_PARTICLES);
    particlesStaging = new StagingBuffer<Particle>(MAX_PARTICLES);
    particleFreeStack = new GpuBufferU32(MAX_PARTICLES);
    particleFreeCount = new GpuBufferU32(1);
    particleFreeStackStaging = new StagingBufferU32(MAX_PARTICLES);
    particleFreeCountStaging = new StagingBufferU32(1);
    particleCpu.resize(MAX_PARTICLES);
    particleFreeStackCpu.resize(MAX_PARTICLES, 0u);
    particleFreeCountCpu = 0u;
    nextParticleIndex = 0u;
    std::fill(particleCpu.begin(), particleCpu.end(),
              Particle{glm::vec2(-1000.0f), glm::vec2(0.0f), packCell(Color::Sand(), 0u), 0u});
    particlesA->write(particleCpu.data(), particleCpu.size());
    {
        const uint32_t zero = 0u;
        particleFreeCount->write(&zero, 1);
    }

    // Staging buffers — sized to match what they'll receive
    cellsStaging = new StagingBufferU32(cellCount);
    chunkStaging = new StagingBufferU32(numChunks);

    struct SimUniforms {
        uint32_t width;
        uint32_t height;
        uint32_t is_left_frame;
        uint32_t chunk_size;
        uint32_t chunks_wide;
        uint32_t random_seed;
        uint32_t pad[2];
    };
    const size_t uSize = sizeof(SimUniforms);

    // Binding layouts must match the WGSL shaders exactly (storage order in ctor order → @1..):
    //   intent:  @0=uniform  @1=cells  @2=intent  @3=active_chunk_list  @4=chunk_intent
    //   resolve: @0=uniform  @1=cells  @2=intent  @3=claim   @4=chunk_intent
    //   apply:   @0=uniform  @1=cells  @2=intent  @3=claim  @4=out_cells
    //            @5=chunk_active_out  @6=chunk_intent  @7=chunk_active
    intentShader = new ComputeShader(
        loadShaderSource("shaders/cellular/intent.wgsl"),
        { cellsA->handle(), intent->handle(),
          gpuActiveChunkList->handle(), gpuChunkIntent->handle() },
        uSize
    );
    resolveShader = new ComputeShader(
        loadShaderSource("shaders/cellular/resolve.wgsl"),
        { cellsA->handle(), intent->handle(), claim->handle(), gpuChunkIntent->handle() },
        uSize
    );
    applyShader = new ComputeShader(
        loadShaderSource("shaders/cellular/apply.wgsl"),
        { cellsA->handle(), intent->handle(), claim->handle(), cellsB->handle(),
          gpuChunkActiveOut->handle(), gpuChunkIntent->handle(), gpuChunkActive->handle() },
        uSize
    );
    struct ParticleUniforms {
        float dt;
        uint32_t num_particles;
        float gravity;
        float cell_width;
        uint32_t grid_width;
        uint32_t grid_height;
        uint32_t chunk_size;
        uint32_t chunks_wide;
    };
    const size_t pUSize = sizeof(ParticleUniforms);
    particleShader = new ComputeShader(
        loadShaderSource("shaders/cellular/particles.wgsl"),
        { particlesA->handle(), cellsB->handle(), gpuChunkActiveOut->handle(),
          particleFreeStack->handle(), particleFreeCount->handle() },
        pUSize
    );

    computeInitialized = true;
    std::cout << "GPU compute initialized. Chunk grid: "
              << chunksWide << "x" << chunksHigh
              << " (" << numChunks << " chunks, "
              << (numChunks * 4) / 1024 << " KB chunk buffer)" << std::endl;
}

// ---------------------------------------------------------------------------
// Chunk dirty tracking
// ---------------------------------------------------------------------------

void CellBuffer::markChunkAndNeighborsDirty(int cx, int cy) {
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = cx + dx, ny = cy + dy;
            if (nx >= 0 && nx < chunksWide && ny >= 0 && ny < chunksHigh)
                chunkActive[ny * chunksWide + nx] = 1u;
        }
    }
}

void CellBuffer::markChunkDirty(int px, int py) {
    int cx = px / CHUNK_SIZE;
    int cy = py / CHUNK_SIZE;
    // Record chunk indices — don't write chunkActive directly.
    // simulateGPU applies these AFTER rebuilding from GPU output, so
    // they don't contaminate the GPU-activity-based rebuild.
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = cx + dx, ny = cy + dy;
            if (nx >= 0 && nx < chunksWide && ny >= 0 && ny < chunksHigh)
                pendingBrushChunks.push_back(ny * chunksWide + nx);
        }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void CellBuffer::updateTexture() {

    // Add the particles to the gpuCellScratch
    for (uint32_t i = 0; i < nextParticleIndex; ++i) {
        // Skip inactive particles
        if ((particleCpu[i]._pad & 1u) == 0u) { continue; }

        // Check if the particle is within the bounds of the buffer
        int x = (int)(particleCpu[i].pos.x);
        int y = (int)(particleCpu[i].pos.y);
        if (x < 0 || x >= width || y < 0 || y >= height) { continue; }

        // Write the particle to the gpuCellScratch
        unsigned int index = static_cast<unsigned int>(y * width + x);
        gpuCellScratch[index] = particleCpu[i].color;
    }

    // Write buffer data to texture
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, gpuCellScratch.data());
}

void CellBuffer::setActivePixel(int x, int y, const Color& color) {
    if (x >= 0 && x < width && y >= 0 && y < height)
        getActiveBuffer()[y * width + x] = color;
}

void CellBuffer::setActivePixel(int x, int y, char r, char g, char b, int mat_id, bool on_fire, bool is_static) {
    Color color(r, g, b, mat_id, on_fire, is_static);
    setActivePixel(x, y, color);
}

void CellBuffer::setActivePixel(glm::vec2 pos, char color[3], int mat_id, bool on_fire, bool is_static) {
    int x = (int)pos.x;
    int y = (int)pos.y;
    Color colorObj(color[0], color[1], color[2], mat_id, on_fire, is_static);
    setActivePixel(x, y, colorObj);
}

void CellBuffer::setPixel(int x, int y, char r, char g, char b, int mat_id, bool on_fire, bool is_static) {
    setActivePixel(x, y, r, g, b, mat_id, on_fire, is_static);
}

void CellBuffer::setPixel(glm::vec2 pos, char color[3], int mat_id, bool on_fire, bool is_static) {
    setActivePixel(pos, color, mat_id, on_fire, is_static);
}

void CellBuffer::setBackPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < width && y >= 0 && y < height)
        getBackBuffer()[y * width + x] = color;
}
Color CellBuffer::getActivePixel(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        const size_t idx = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);

        // In compute mode, simulation state lives on GPU and is mirrored into gpuCellScratch via staging readback.
        if (computeInitialized && idx < gpuCellScratch.size()) {
            return unpackCell(gpuCellScratch[idx]);
        }

        return getActiveBuffer()[idx];
    }
    return Color::Empty();
}
Color CellBuffer::getBackPixel(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height)
        return getBackBuffer()[y * width + x];
    return Color::Empty();
}
void CellBuffer::clear(const Color& color) {
    for (auto& p : getActiveBuffer()) p = color;
}

void CellBuffer::applyBrush(int pixelX, int pixelY, int radius, const Color& color) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx*dx + dy*dy <= radius*radius) {
                int x = pixelX + dx, y = pixelY + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    markChunkDirty(x, y);
                    if (computeInitialized) {
                        pendingBrushPixels.push_back({y * width + x, color});
                    } else {
                        setActivePixel(x, y, color);
                    }
                }
            }
        }
    }
}

void CellBuffer::applyParticleBrush(int pixelX, int pixelY, int radius, uint32_t spawnCount, const Color& color) {
    if (!computeInitialized || !particlesA || spawnCount == 0) return;

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.0f, 6.283185307f);
    std::uniform_real_distribution<float> speedDist(5.0f, 25.0f);
    std::uniform_real_distribution<float> radiusDist(0.0f, static_cast<float>(radius));

    for (uint32_t n = 0; n < spawnCount; ++n) {
        const float angle = angleDist(rng);
        const float r = radiusDist(rng);
        const float sx = pixelX + std::cos(angle) * r;
        const float sy = pixelY + std::sin(angle) * r;
        const float speed = speedDist(rng);
        const float vx = std::cos(angle) * speed;
        const float vy = std::sin(angle) * speed;
        if (!addParticle(glm::vec2(sx, sy), glm::vec2(vx, vy), color)) {
            break;
        }
    }
}

bool CellBuffer::addParticle(const glm::vec2& pos, const glm::vec2& vel, const Color& color) {
    if (!computeInitialized || !particlesA) {
        return false;
    }

    bool poppedFromFree = false;
    uint32_t idx = 0u;
    if (particleFreeCountCpu > 0u) {
        idx = particleFreeStackCpu[particleFreeCountCpu - 1u];
        particleFreeCountCpu--;
        poppedFromFree = true;
    } else if (nextParticleIndex < MAX_PARTICLES) {
        idx = nextParticleIndex++;
    } else {
        return false;
    }

    Color particleColor = color;
    if (particleColor.mat_id == 0) {
        particleColor.mat_id = 2;
    }

    particleCpu[idx] = Particle{pos, vel, packCell(particleColor, 0u), 1u};
    particlesA->writeRegion(idx, &particleCpu[idx], 1);
    activeParticleCount++;
    if (poppedFromFree) {
        particleFreeCount->write(&particleFreeCountCpu, 1);
    }
    return true;
}

bool CellBuffer::addParticle(const glm::vec2& pos, const glm::vec2& vel, char color[3], int mat_id, bool on_fire, bool is_static) {
    Color particleColor(color[0], color[1], color[2], mat_id, on_fire, is_static);
    return addParticle(pos, vel, particleColor);
}

// ---------------------------------------------------------------------------
// GPU simulation — pipelined
// ---------------------------------------------------------------------------

void CellBuffer::simulate(float deltaTime) {
    if (!computeInitialized) return;
    const float frameDt = std::clamp(std::max(0.0f, deltaTime), 1.0f / 240.0f, 1.0f / 15.0f);
    const float fixedStep = 1.0f / std::max(cellUpdatesPerSecond, 1.0f);
    cellUpdateAccumulator += frameDt;
    // Avoid huge catch-up bursts after pauses.
    cellUpdateAccumulator = std::min(cellUpdateAccumulator, fixedStep * 4.0f);
    const bool runSandStep = cellUpdateAccumulator >= fixedStep;
    if (runSandStep) {
        cellUpdateAccumulator -= fixedStep;
    }

    // ------------------------------------------------------------------
    // STEP 1 — Collect last frame's cell readback, merge brush pixels
    // ------------------------------------------------------------------
    if (runSandStep && pendingCellsReadback) {
        gpuCellScratch.resize(static_cast<size_t>(width * height));
        cellsStaging->collect(gpuCellScratch.data(), gpuCellScratch.size());
        pendingCellsReadback = false;
    }
    if (pendingParticleReadback && nextParticleIndex > 0u) {
        particlesStaging->collectRegion(particleCpu.data(), 0, nextParticleIndex);
        uint32_t freeCountTmp = 0u;
        particleFreeCountStaging->collect(&freeCountTmp, 1);
        particleFreeCountCpu = std::min<uint32_t>(freeCountTmp, MAX_PARTICLES);
        // Always collect the mapped free-stack staging buffer so it gets unmapped.
        // We only consume the first `particleFreeCountCpu` entries below.
        particleFreeStackStaging->collect(particleFreeStackCpu.data(), particleFreeStackCpu.size());
        activeParticleCount = 0u;
        for (uint32_t i = 0; i < nextParticleIndex; ++i) {
            if ((particleCpu[i]._pad & 1u) != 0u) {
                activeParticleCount++;
            }
        }
        pendingParticleReadback = false;
    }

    // ------------------------------------------------------------------
    // STEP 2 — Collect chunk readback, rebuild chunkActive cleanly
    // ------------------------------------------------------------------
    if (runSandStep && pendingChunkReadback) {
        chunkStaging->collect(chunkActiveOut.data(), chunkActiveOut.size());
        pendingChunkReadback = false;

        // Zero, then re-expand from GPU output only
        std::fill(chunkActive.begin(), chunkActive.end(), 0u);
        for (int cy = 0; cy < chunksHigh; ++cy)
            for (int cx = 0; cx < chunksWide; ++cx)
                if (chunkActiveOut[cy * chunksWide + cx] != 0u)
                    markChunkAndNeighborsDirty(cx, cy);

        // Apply brush chunk marks recorded this frame on top of GPU expansion
        for (int idx : pendingBrushChunks)
            chunkActive[idx] = 1u;
        pendingBrushChunks.clear();
    }

    // ------------------------------------------------------------------
    // STEP 3 — If brush happened before the first chunk readback,
    // integrate brush chunk marks now. (Later frames: step 2 already did.)
    // ------------------------------------------------------------------
    if (runSandStep && !pendingBrushChunks.empty()) {
        for (int idx : pendingBrushChunks)
            chunkActive[idx] = 1u;
        pendingBrushChunks.clear();
    }

    // ------------------------------------------------------------------
    // STEP 4 — Upload pending brush pixels to GPU cellsA only.
    // (No full-grid CPU->GPU upload; simulation state stays on GPU.)
    // ------------------------------------------------------------------
    if (runSandStep && !pendingBrushPixels.empty()) {

        std::sort(pendingBrushPixels.begin(), pendingBrushPixels.end(),
                  [](const BrushPixel& a, const BrushPixel& b) { return a.idx < b.idx; });

        size_t i = 0;
        while (i < pendingBrushPixels.size()) {
            int startIdx = pendingBrushPixels[i].idx;
            size_t j = i + 1;
            while (j < pendingBrushPixels.size() &&
                   pendingBrushPixels[j].idx == startIdx + static_cast<int>(j - i)) {
                ++j;
            }

            const size_t len = j - i;
            std::vector<uint32_t> values;
            values.resize(len);
            for (size_t k = 0; k < len; ++k) {
                const Color& c = pendingBrushPixels[i + k].color;
                values[k] = packCell(c, 0u);
            }
            cellsA->writeRegion(static_cast<size_t>(startIdx), values.data(), len);
            i = j;
        }

        pendingBrushPixels.clear();
    }

    // ------------------------------------------------------------------
    // STEP 5 — Advance frame parity
    // ------------------------------------------------------------------
    if (runSandStep) {
        isLeftFrame = !isLeftFrame;
    }

    // ------------------------------------------------------------------
    // STEP 6 — Upload chunk flags, zero transient GPU buffers
    // ------------------------------------------------------------------
    // Build a compact list of active chunks so the intent pass can be
    // dispatched sparsely (one workgroup per active chunk).
    std::vector<uint32_t> activeChunkListIndices(numChunks, 0u);
    uint32_t activeChunkCount = 0u;
    if (runSandStep) {
        for (uint32_t ci = 0; ci < chunkActive.size(); ++ci) {
            if (chunkActive[ci] != 0u) {
                activeChunkListIndices[activeChunkCount++] = ci;
            }
        }
    }

    if (runSandStep) {
        gpuChunkActive->write(chunkActive.data(), chunkActive.size());
        gpuActiveChunkList->write(activeChunkListIndices.data(), activeChunkListIndices.size());
        gpuChunkIntent   ->zero();
        gpuChunkActiveOut->zero();
    }

    // ------------------------------------------------------------------
    // STEP 7 — Build encoder, submit ONE batch
    // cellsA = permanent input, cellsB = permanent output, no pointer swap
    // ------------------------------------------------------------------
    struct SimUniforms {
        uint32_t width, height, is_left_frame, chunk_size, chunks_wide;
        uint32_t random_seed;
        uint32_t pad[2];
    } u;
    struct ParticleUniforms {
        float dt;
        uint32_t num_particles;
        float gravity;
        float cell_width;
        uint32_t grid_width;
        uint32_t grid_height;
        uint32_t chunk_size;
        uint32_t chunks_wide;
    } pU;
    u.width         = width;
    u.height        = height;
    u.is_left_frame = isLeftFrame ? 1u : 0u;
    u.chunk_size    = CHUNK_SIZE;
    u.chunks_wide   = chunksWide;
    thread_local std::mt19937 simUniformRng{std::random_device{}()};
    u.random_seed   = static_cast<uint32_t>(simUniformRng());
    u.pad[0] = u.pad[1] = 0;

    intentShader ->setUniform(u);
    resolveShader->setUniform(u);
    applyShader  ->setUniform(u);
    pU.dt = frameDt;
    pU.num_particles = nextParticleIndex;
    pU.gravity = GRAVITY;
    pU.cell_width = cellScale;
    pU.grid_width = static_cast<uint32_t>(width);
    pU.grid_height = static_cast<uint32_t>(height);
    pU.chunk_size = static_cast<uint32_t>(CHUNK_SIZE);
    pU.chunks_wide = static_cast<uint32_t>(chunksWide);
    particleShader->setUniform(pU);

    const uint32_t gx = static_cast<uint32_t>(chunksWide);
    const uint32_t gy = static_cast<uint32_t>(chunksHigh);

    {
        GpuEncoder enc;
        if (runSandStep) {
            if (activeChunkCount > 0u) {
                enc.dispatch(intentShader->handle(), activeChunkCount, 1u, 1u);
            }
            enc.dispatch(resolveShader->handle(), gx, gy);
            enc.dispatch(applyShader  ->handle(), gx, gy);
        } else if (nextParticleIndex > 0u) {
            // Keep particle updates smooth between fixed sand ticks:
            // seed cellsB from authoritative cellsA for particle collision/deposit.
            enc.copyBuffer(*cellsA, *cellsB);
        }
        if (nextParticleIndex > 0u) {
            const uint32_t particleGroups = (nextParticleIndex + 255u) / 256u;
            enc.dispatch(particleShader->handle(), particleGroups, 1u, 1u);
            enc.copyToStaging(*particlesA, *particlesStaging);
            enc.copyToStaging(*particleFreeCount, *particleFreeCountStaging);
            enc.copyToStaging(*particleFreeStack, *particleFreeStackStaging);
        }
        if (runSandStep || nextParticleIndex > 0u) {
            // Keep simulation state on GPU: cellsA remains authoritative.
            enc.copyBuffer(*cellsB, *cellsA);
        }
        if (runSandStep) {
            // Lazy readback: copy only active chunk squares into staging.
            // Memory layout is row-major, so each 16x16 chunk is uploaded as
            // CHUNK_SIZE contiguous row segments.
            const uint64_t sparseRowCopyOpsLimit = 4096ull; // avoid thousands of tiny copies
            const bool forceFullReadbackForParticles = activeParticleCount > 0u;
            if (forceFullReadbackForParticles ||
                activeChunkCount == static_cast<uint32_t>(numChunks) ||
                (static_cast<uint64_t>(activeChunkCount) * static_cast<uint64_t>(CHUNK_SIZE)) > sparseRowCopyOpsLimit) {
                // Too large/sparse not beneficial: just read back the full buffer.
                enc.copyToStaging(*cellsB, *cellsStaging);
            } else if (activeChunkCount > 0u) {
                // Sparse readback: one 16-element copy per chunk row.
                for (uint32_t ai = 0; ai < activeChunkCount; ++ai) {
                    const uint32_t chunkIndex = activeChunkListIndices[ai];
                    const uint32_t cx = chunkIndex % static_cast<uint32_t>(chunksWide);
                    const uint32_t cy = chunkIndex / static_cast<uint32_t>(chunksWide);

                    const uint32_t originX = cx * static_cast<uint32_t>(CHUNK_SIZE);
                    const uint32_t originY = cy * static_cast<uint32_t>(CHUNK_SIZE);

                    if (originX >= static_cast<uint32_t>(width)) continue;

                    const uint32_t rowCopyLen =
                        std::min<uint32_t>(static_cast<uint32_t>(CHUNK_SIZE),
                                           static_cast<uint32_t>(width) - originX);

                    for (uint32_t ly = 0; ly < static_cast<uint32_t>(CHUNK_SIZE); ++ly) {
                        const uint32_t y = originY + ly;
                        if (y >= static_cast<uint32_t>(height)) break;

                        const size_t srcOffset =
                            static_cast<size_t>(y) * static_cast<size_t>(width) +
                            static_cast<size_t>(originX);

                        enc.copyRegionToStagingAtOffset(*cellsB, *cellsStaging,
                                                        srcOffset, srcOffset,
                                                        rowCopyLen);
                    }
                }
            }
            enc.copyToStaging(*gpuChunkActiveOut, *chunkStaging);
        }
        enc.submit();
    }

    // ------------------------------------------------------------------
    // STEP 7 — Kick async maps (non-blocking)
    // ------------------------------------------------------------------
    if (runSandStep) {
        cellsStaging->mapAsync();
        chunkStaging->mapAsync();
        pendingCellsReadback = true;
        pendingChunkReadback = true;
    }
    if (nextParticleIndex > 0u) {
        particlesStaging->mapAsync();
        particleFreeCountStaging->mapAsync();
        particleFreeStackStaging->mapAsync();
        pendingParticleReadback = true;
    }
}

// ---------------------------------------------------------------------------
// GL helpers
// ---------------------------------------------------------------------------

std::string CellBuffer::loadShaderSource(const char* filepath) {
    const std::string resolvedPath = externalPath(filepath);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << filepath << " (resolved: " << resolvedPath << ")\n";
        return "";
    }
    std::stringstream ss; ss << file.rdbuf(); return ss.str();
}

void CellBuffer::setupTexture() {
    glGenTextures(1, &renderTexture); 
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
}

bool CellBuffer::windowToPixel(int wx, int wy, int ww, int wh, int& px, int& py) const {
    px = (wx * width)  / ww;
    py = ((wh - wy) * height) / wh;
    return pixelInBounds(px, py);
}

bool CellBuffer::worldToPixel(const glm::vec2& worldPos, int& px, int& py) const {
    px = (int) (worldPos.x / cellScale + width / 2.0f);
    py = (int) (worldPos.y / cellScale + height / 2.0f);
    return pixelInBounds(px, py);
}

void CellBuffer::pixelToWorld(int px, int py, glm::vec2& worldPos) const {
    worldPos.x = (px - width / 2.0f) * cellScale;
    worldPos.y = (py - height / 2.0f) * cellScale;
}

const std::vector<Color>& CellBuffer::getActiveBuffer() const {
    return firstActive ? buffers.first : buffers.second;
}
const std::vector<Color>& CellBuffer::getBackBuffer() const {
    return firstActive ? buffers.second : buffers.first;
}