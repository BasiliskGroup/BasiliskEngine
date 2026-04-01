#include <basilisk/basilisk.h>
#include <basilisk/physics/cellular/marching.h>

#include <array>
#include <iostream>
#include <random>
#include <vector>

#include <earcut.hpp>

#include "perlin.h"

namespace {

constexpr int SIDE_LENGTH = 100;
constexpr float DELTA = SIDE_LENGTH * 0.6f;
constexpr int OCTAVES = 5;

using namespace bsk::internal;

std::vector<float> interleavedMeshFrom2D(const std::vector<glm::vec2>& verts2d) {
    std::vector<float> meshVertices;
    meshVertices.reserve(verts2d.size() * 5);
    for (const glm::vec2& v : verts2d) {
        meshVertices.push_back(v.x);
        meshVertices.push_back(-v.y);
        meshVertices.push_back(0.0f);
        meshVertices.push_back(0.0f);
        meshVertices.push_back(0.0f);
    }
    return meshVertices;
}

void addEarcutPolygonToScene(bsk::Scene2D* scene, const std::vector<glm::vec2>& verts2d,
    const std::vector<uint32_t>& indices, const glm::vec2& offset, const bsk::Material& material)
{
    if (verts2d.empty() || indices.empty()) {
        return;
    }
    auto meshVerts = interleavedMeshFrom2D(verts2d);
    auto* mesh = new bsk::Mesh(std::move(meshVerts), indices);
    auto* mat = new bsk::Material(material);
    new bsk::Node2D(scene, mesh, mat, offset, 0.0f, glm::vec2(1.0f, 1.0f));
}

void addConvexToScene(bsk::Scene2D* scene, const Convex& convex, const glm::vec2& offset,
    const bsk::Material& material)
{
    std::vector<glm::vec2> ring;
    for (auto it = convex.begin(); it != convex.end(); ++it) {
        ring.push_back(*it);
    }
    if (ring.size() < 3) {
        return;
    }
    float area = 0.0f;
    for (std::size_t i = 0; i < ring.size(); ++i) {
        const glm::vec2& p = ring[i];
        const glm::vec2& q = ring[(i + 1) % ring.size()];
        area += p.x * q.y - q.x * p.y;
    }
    if (area < 0.0f) {
        std::reverse(ring.begin(), ring.end());
    }

    using EarcutPoint = std::array<double, 2>;
    std::vector<std::vector<EarcutPoint>> polygon;
    polygon.emplace_back();
    polygon[0].reserve(ring.size());
    for (const glm::vec2& p : ring) {
        polygon[0].push_back({static_cast<double>(p.x), static_cast<double>(p.y)});
    }

    const std::vector<uint32_t> earcutIndices = mapbox::earcut<uint32_t>(polygon);
    if (earcutIndices.empty()) {
        return;
    }

    auto meshVerts = interleavedMeshFrom2D(ring);
    auto* mesh = new bsk::Mesh(std::move(meshVerts), earcutIndices);
    auto* mat = new bsk::Material(material);
    new bsk::Node2D(scene, mesh, mat, offset, 0.0f, glm::vec2(1.0f, 1.0f));
}

void drawConnectedComponents(bsk::Scene2D* scene, const Grid& grid, const glm::vec2& offset)
{
    const auto& comps = grid.connectedComponents();
    const int compCount = static_cast<int>(comps.size());
    for (int i = 0; i < compCount; ++i) {
        bsk::Material* mat = new bsk::Material(getCircularColor(i, compCount));
        for (const auto& [x, y] : comps[i]) {
            new bsk::Node2D(scene, nullptr, mat,
                glm::vec2(static_cast<float>(x), static_cast<float>(y)) + offset);
        }
    }
}

void drawMarchGeometry(bsk::Scene2D* scene, const std::vector<MarchComponentGeometry>& geometry)
{
    const int polyMax = static_cast<int>(geometry.size());
    for (int poly = 0; poly < polyMax; ++poly) {
        const auto& g = geometry[static_cast<std::size_t>(poly)];
        addEarcutPolygonToScene(scene, g.filledVertices, g.filledIndices, glm::vec2(-DELTA, -DELTA),
            getCircularColor(poly, polyMax));

        const int convexMax = static_cast<int>(g.convexPieces.size());
        for (int j = 0; j < convexMax; ++j) {
            addConvexToScene(scene, g.convexPieces[static_cast<std::size_t>(j)], glm::vec2(DELTA, -DELTA),
                getCircularColor(poly, polyMax, j, convexMax));
        }
    }
}

} // namespace

int main()
{
    bsk::Engine* engine = new bsk::Engine(1200, 900, "Marching Squares");
    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    auto* camera =
        new bsk::StaticCamera2D(engine, glm::vec2((SIDE_LENGTH - 1) / 2.0f), SIDE_LENGTH * 2.0f * 1.25f);
    scene->setCamera(camera);

    bsk::Material* whiteMat = new bsk::Material(glm::vec3(1.0f, 1.0f, 1.0f));

    std::vector<std::vector<int>> weights(SIDE_LENGTH, std::vector<int>(SIDE_LENGTH, -1));
    PerlinNoise noise;
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> startDist(0.0, 0.0); // 10000.0);
    const double noiseStartX = startDist(rng);
    const double noiseStartY = startDist(rng);
    for (int x = 0; x < SIDE_LENGTH; ++x) {
        for (int y = 0; y < SIDE_LENGTH; ++y) {
            const double sx = noiseStartX + (static_cast<double>(x) / SIDE_LENGTH) * static_cast<double>(OCTAVES);
            const double sy = noiseStartY + (static_cast<double>(y) / SIDE_LENGTH) * static_cast<double>(OCTAVES);
            const float n = static_cast<float>(noise.noise(sx, sy, 0.0));
            weights[x][y] = (n > 0.0f) ? 1 : -1;
        }
    }

    const glm::vec2 perlinOffset(-DELTA, DELTA);
    for (int x = 0; x < SIDE_LENGTH; ++x) {
        for (int y = 0; y < SIDE_LENGTH; ++y) {
            if (weights[x][y] > 0) {
                new bsk::Node2D(scene, nullptr, whiteMat,
                    glm::vec2(static_cast<float>(x), static_cast<float>(y)) + perlinOffset);
            }
        }
    }

    Grid grid(std::move(weights));
    drawConnectedComponents(scene, grid, glm::vec2(DELTA, DELTA));

    std::vector<MarchComponentGeometry> marchGeom = grid.genMarch();
    drawMarchGeometry(scene, marchGeom);

    std::cout << "Done\n";

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    delete whiteMat;
    delete camera;
    delete scene;
    delete engine;
    return 0;
}
