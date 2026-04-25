#include <basilisk/physics/cellular/cellBuffer.h>
#include <basilisk/compute/gpuWrapper.hpp>
#include <basilisk/util/resolvePath.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <random>


using namespace bsk::internal;

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
        glDeleteBuffers(UPLOAD_PBO_COUNT, uploadPbos);
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
        delete explosionStack;
        delete explosionCount;
        delete explosionStackStaging;
        delete explosionCountStaging;
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
    gpuRenderScratch.resize(cellCount);

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
    explosionStack = new GpuBuffer<ExplosionEvent>(MAX_PARTICLES);
    explosionCount = new GpuBufferU32(1);
    explosionStackStaging = new StagingBuffer<ExplosionEvent>(MAX_PARTICLES);
    explosionCountStaging = new StagingBufferU32(1);
    particleCpu.resize(MAX_PARTICLES);
    explosionCpu.resize(MAX_PARTICLES);
    particleFreeStackCpu.resize(MAX_PARTICLES, 0u);
    particleFreeCountCpu = 0u;
    nextParticleIndex = 0u;
    std::fill(particleCpu.begin(), particleCpu.end(),
              Particle{glm::vec2(-1000.0f), glm::vec2(0.0f), 0u, 0.0f, 0u, 0.0f});
    particlesA->write(particleCpu.data(), particleCpu.size());
    {
        const uint32_t zero = 0u;
        particleFreeCount->write(&zero, 1);
        explosionCount->write(&zero, 1);
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
          particleFreeStack->handle(), particleFreeCount->handle(),
          explosionStack->handle(), explosionCount->handle() },
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
    if (gpuRenderScratch.size() != gpuCellScratch.size()) {
        gpuRenderScratch.resize(gpuCellScratch.size());
    }

    // Start from authoritative sand cells; keep physics readback untouched.
    gpuRenderScratch = gpuCellScratch;

    // Add particles only to the render copy.
    for (uint32_t i = 0; i < nextParticleIndex; ++i) {
        // Skip inactive particles
        if (particleCpu[i].color == 0u) { continue; }

        // Check if the particle is within the bounds of the buffer
        int x = (int)(particleCpu[i].pos.x);
        int y = (int)(particleCpu[i].pos.y);
        if (x < 0 || x >= width || y < 0 || y >= height) { continue; }

        // Write the particle to the render scratch
        unsigned int index = static_cast<unsigned int>(y * width + x);
        gpuRenderScratch[index] = particleCpu[i].color;
    }

    // Upload via a small PBO ring to reduce CPU/GPU sync stalls.
    // This keeps texture updates in OpenGL while pipelining transfers.
    const size_t uploadBytes = static_cast<size_t>(width) * static_cast<size_t>(height) * sizeof(uint32_t);
    GLuint pbo = uploadPbos[uploadPboIndex];
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(uploadBytes), nullptr, GL_STREAM_DRAW);
    void* mapped = glMapBufferRange(
        GL_PIXEL_UNPACK_BUFFER, 0, static_cast<GLsizeiptr>(uploadBytes),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
    );
    if (mapped) {
        std::memcpy(mapped, gpuRenderScratch.data(), uploadBytes);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    // Write buffer data to texture
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    if (mapped) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    } else {
        // Fallback if map fails on a given driver/frame.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, gpuRenderScratch.data());
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    uploadPboIndex = (uploadPboIndex + 1) % UPLOAD_PBO_COUNT;
}

void CellBuffer::setActivePixel(int x, int y, const Color& color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    markChunkDirty(x, y);
    if (computeInitialized) {
        pendingBrushPixels.push_back({y * width + x, color});
        return;
    }

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

void CellBuffer::explode(int pixelX, int pixelY, int radius, float fireChance) {
    if (radius < 0) {
        return;
    }

    const float radiusF = std::max(1.0f, static_cast<float>(radius));
    const float minSpeed = 8.0f;
    const float maxSpeed = 36.0f;
    const Color empty = Color::Empty();
    const float clampedFireChance = glm::clamp(fireChance, 0.0f, 1.0f);
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> fireDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> angleJitterDist(-0.35f, 0.35f);
    std::uniform_real_distribution<float> speedJitterDist(0.85f, 1.15f);

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx*dx + dy*dy <= radius*radius) {
                int x = pixelX + dx, y = pixelY + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    const Color source = getActivePixel(x, y);
                    if (source.getMatId() == 0u) {
                        continue;
                    }

                    const glm::vec2 delta(static_cast<float>(dx), static_cast<float>(dy));
                    const float dist = glm::length(delta);
                    const glm::vec2 dir = (dist > 0.0001f) ? (delta / dist) : glm::vec2(0.0f, 1.0f);
                    const float distNorm = glm::clamp(dist / radiusF, 0.0f, 1.0f);
                    const float baseSpeed = minSpeed + (maxSpeed - minSpeed) * (1.0f - distNorm);
                    const float dirAngle = std::atan2(dir.y, dir.x) + angleJitterDist(rng);
                    const glm::vec2 jitteredDir(std::cos(dirAngle), std::sin(dirAngle));
                    const float speed = baseSpeed * speedJitterDist(rng);
                    const glm::vec2 vel = jitteredDir * speed;
                    Color particleColor = source;
                    particleColor.setOnFire(fireDist(rng) < clampedFireChance ? 1u : 0u);

                    bool converted = true;
                    if (computeInitialized) {
                        converted = addParticle(
                            glm::vec2(static_cast<float>(x), static_cast<float>(y)),
                            vel,
                            particleColor,
                            0.1f,
                            0u,
                            0.0f
                        );
                    }
                    if (!converted) {
                        continue;
                    }

                    markChunkDirty(x, y);
                    if (computeInitialized) {
                        pendingBrushPixels.push_back({y * width + x, empty});
                    } else {
                        setActivePixel(x, y, empty);
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
        if (!addParticle(glm::vec2(sx, sy), glm::vec2(vx, vy), color, 0.1f, 5u, 0.0f)) {
            break;
        }
    }
}

bool CellBuffer::blitRgbaBuffer(
    const std::vector<uint8_t>& rgba,
    int imageWidth,
    int imageHeight,
    int offsetX,
    int offsetY,
    uint8_t materialId,
    bool onFire,
    bool isStatic,
    bool flipYToBufferSpace,
    uint8_t alphaThreshold
) {
    if (imageWidth <= 0 || imageHeight <= 0) {
        return false;
    }
    const size_t expectedBytes = static_cast<size_t>(imageWidth) * static_cast<size_t>(imageHeight) * 4u;
    if (rgba.size() < expectedBytes) {
        return false;
    }

    for (int y = 0; y < imageHeight; ++y) {
        for (int x = 0; x < imageWidth; ++x) {
            const size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(imageWidth) + static_cast<size_t>(x)) * 4u;
            const uint8_t a = rgba[idx + 3u];
            if (a < alphaThreshold) {
                continue;
            }

            const Color c(
                rgba[idx + 0u],
                rgba[idx + 1u],
                rgba[idx + 2u],
                materialId,
                onFire ? 1u : 0u,
                isStatic
            );

            const int px = x + offsetX;
            const int py = flipYToBufferSpace ? (height - y + offsetY) : (y + offsetY);
            setActivePixel(px, py, c);
        }
    }

    return true;
}

bool CellBuffer::addParticle(const glm::vec2& pos, const glm::vec2& vel, const Color& color, float forcedLifetime, uint32_t explodeRadius, float explodeFireChance) {
    if (!computeInitialized || !particlesA) {
        return false;
    }

    bool poppedFromFree = false;
    uint32_t idx = 0u;
    while (particleFreeCountCpu > 0u) {
        const uint32_t candidate = particleFreeStackCpu[particleFreeCountCpu - 1u];
        particleFreeCountCpu--;
        // Validate free-list entries read back from GPU before using them.
        // Corrupt/stale entries can otherwise poison spawning permanently.
        if (candidate < nextParticleIndex && particleCpu[candidate].color == 0u) {
            idx = candidate;
            poppedFromFree = true;
            break;
        }
    }

    if (!poppedFromFree && nextParticleIndex < MAX_PARTICLES) {
        idx = nextParticleIndex++;
    } else if (!poppedFromFree) {
        return false;
    }

    Color particleColor = color;
    if (particleColor.mat_id == 0) {
        particleColor.mat_id = 2;
    }
    particleColor.setIsStatic(false);

    particleCpu[idx] = Particle{
        pos,
        vel,
        packCell(particleColor, 0u),
        std::max(0.0f, forcedLifetime),
        explodeRadius,
        glm::clamp(explodeFireChance, 0.0f, 1.0f)
    };
    particlesA->writeRegion(idx, &particleCpu[idx], 1);
    activeParticleCount++;
    if (poppedFromFree) {
        particleFreeCount->write(&particleFreeCountCpu, 1);
    }
    return true;
}

bool CellBuffer::addParticle(const glm::vec2& pos, const glm::vec2& vel, char color[3], int mat_id, bool on_fire, bool is_static, float forcedLifetime, uint32_t explodeRadius, float explodeFireChance) {
    Color particleColor(color[0], color[1], color[2], mat_id, on_fire, is_static);
    return addParticle(pos, vel, particleColor, forcedLifetime, explodeRadius, explodeFireChance);
}

// ---------------------------------------------------------------------------
// GPU simulation — pipelined
// ---------------------------------------------------------------------------

void CellBuffer::simulate(float deltaTime) {
    explosionHappened = false;
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
        if (freeCountTmp > nextParticleIndex || freeCountTmp > MAX_PARTICLES) {
            // Recover from invalid counter states instead of consuming garbage stack entries.
            freeCountTmp = 0u;
            particleFreeCount->write(&freeCountTmp, 1);
        }
        particleFreeCountCpu = freeCountTmp;
        // Always collect the mapped free-stack staging buffer so it gets unmapped.
        // We only consume the first `particleFreeCountCpu` entries below.
        particleFreeStackStaging->collect(particleFreeStackCpu.data(), particleFreeStackCpu.size());
        activeParticleCount = 0u;
        for (uint32_t i = 0; i < nextParticleIndex; ++i) {
            if (particleCpu[i].color != 0u) {
                activeParticleCount++;
            }
        }
        pendingParticleReadback = false;
    }

    if (pendingExplosionReadback) {
        uint32_t explosionCountCpu = 0u;
        explosionCountStaging->collect(&explosionCountCpu, 1);
        explosionCountCpu = std::min<uint32_t>(explosionCountCpu, MAX_PARTICLES);
        explosionStackStaging->collect(explosionCpu.data(), explosionCpu.size());
        pendingExplosionReadback = false;

        const uint32_t maxProcess = std::min<uint32_t>(explosionCountCpu, MAX_EXPLOSIONS_PER_FRAME);
        for (uint32_t i = 0u; i < maxProcess; ++i) {
            const ExplosionEvent& ev = explosionCpu[i];
            if (ev.radius == 0u) {
                continue;
            }
            const glm::vec2 explosionPos = ev.pos;
            const int ex = static_cast<int>(std::floor(explosionPos.x));
            const int ey = static_cast<int>(std::floor(explosionPos.y));
            if (ex < 0 || ex >= width || ey < 0 || ey >= height) {
                continue;
            }
            // GPU-origin explosion processing always uses zero ignition chance.
            explode(ex, ey, static_cast<int>(ev.radius), 0.0f);
            explosionHappened = true;
        }
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
    {
        const uint32_t zero = 0u;
        explosionCount->write(&zero, 1);
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
            enc.copyToStaging(*explosionCount, *explosionCountStaging);
            enc.copyToStaging(*explosionStack, *explosionStackStaging);
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
        explosionCountStaging->mapAsync();
        explosionStackStaging->mapAsync();
        pendingParticleReadback = true;
        pendingExplosionReadback = true;
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

    const size_t uploadBytes = static_cast<size_t>(width) * static_cast<size_t>(height) * sizeof(uint32_t);
    glGenBuffers(UPLOAD_PBO_COUNT, uploadPbos);
    for (int i = 0; i < UPLOAD_PBO_COUNT; ++i) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uploadPbos[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(uploadBytes), nullptr, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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