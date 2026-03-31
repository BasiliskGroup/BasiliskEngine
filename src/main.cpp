#include <basilisk/basilisk.h>
#include <earcut.hpp>

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <memory>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

#include "perlin.h"

namespace {

constexpr int SIDE_LENGTH = 32;
constexpr float DELTA = SIDE_LENGTH * 0.6f;
constexpr int OCTAVES = 10;
constexpr int N = 4;

inline bool vec2Eq(const glm::vec2& a, const glm::vec2& b) {
    return glm::length2(a - b) < 1e-6f;
}

struct Edge {
    glm::vec2 a;
    glm::vec2 b;
};

class Seg {
public:
    Seg(const glm::vec2& a, const glm::vec2& b) : chain{a, b}, bl(glm::min(a, b)), tr(glm::max(a, b)) {}

    bool add(const glm::vec2& a, const glm::vec2& b) {
        if (glm::length(a - b) >= 2.0f) {
            return false;
        }

        if (vec2Eq(chain.front(), a)) {
            chain.insert(chain.begin(), b);
        } else if (vec2Eq(chain.back(), a)) {
            chain.push_back(b);
        } else if (vec2Eq(chain.front(), b)) {
            chain.insert(chain.begin(), a);
        } else if (vec2Eq(chain.back(), b)) {
            chain.push_back(a);
        } else {
            return false;
        }

        bl = glm::min(bl, glm::min(a, b));
        tr = glm::max(tr, glm::max(a, b));
        return true;
    }

    bool merge(Seg& other) {
        if (vec2Eq(chain.front(), other.chain.front())) {
            std::reverse(chain.begin(), chain.end());
            chain.insert(chain.end(), other.chain.begin(), other.chain.end());
        } else if (vec2Eq(chain.front(), other.chain.back())) {
            auto merged = other.chain;
            merged.insert(merged.end(), chain.begin(), chain.end());
            chain = std::move(merged);
        } else if (vec2Eq(chain.back(), other.chain.front())) {
            chain.insert(chain.end(), other.chain.begin(), other.chain.end());
        } else if (vec2Eq(chain.back(), other.chain.back())) {
            std::reverse(other.chain.begin(), other.chain.end());
            chain.insert(chain.end(), other.chain.begin(), other.chain.end());
        } else {
            return false;
        }

        bl = glm::min(bl, other.bl);
        tr = glm::max(tr, other.tr);
        return true;
    }

    bool validate() const {
        if (chain.size() < 3) return false;
        return vec2Eq(chain.front(), chain.back());
    }
    
    float signedArea() const {
        float area = 0.0f;
        for (size_t i = 0; i + 1 < chain.size(); ++i) {
            const auto& p = chain[i];
            const auto& q = chain[i + 1];
            area += (p.x * q.y - q.x * p.y);
        }
        return 0.5f * area;
    }
    
    bool orientCCW() {
        if (!validate()) return false;
    
        float area = signedArea();
    
        // If clockwise, reverse to make CCW
        if (area < 0.0f) {
            std::reverse(chain.begin(), chain.end());
        }
    
        return true;
    }

    std::vector<glm::vec2> chain;
    glm::vec2 bl;
    glm::vec2 tr;
};

class Polygon {
public:
    void add(const glm::vec2& a, const glm::vec2& b) {
        for (std::size_t i = 0; i < segs.size(); ++i) {
            if (!segs[i].add(a, b)) {
                continue;
            }
            Seg merged = segs[i];
            segs.erase(segs.begin() + static_cast<std::ptrdiff_t>(i));
            merge(merged);
            return;
        }
        segs.emplace_back(a, b);
    }

    void draw(
        bsk::Scene2D* scene,
        const glm::vec2& offset,
        bsk::Material* material
    ) const {
        if (segs.empty()) {
            return;
        }

        std::vector<Seg> rings = segs;

        // Largest ring is treated as the outer shell.
        std::size_t outerIdx = 0;
        float outerArea = 0.0f;
        for (std::size_t i = 0; i < rings.size(); ++i) {
            const float area = std::abs(rings[i].signedArea());
            if (area > outerArea) {
                outerArea = area;
                outerIdx = i;
            }
        }

        using EarcutPoint = std::array<double, 2>;
        std::vector<std::vector<EarcutPoint>> polygon;
        polygon.reserve(rings.size());

        // Optional helper payload you can feed into your mesh builder.
        std::vector<glm::vec2> earcutVertices;
        earcutVertices.reserve(rings.size() * 8);

        auto pointInRing = [](const glm::vec2& point, const Seg& ringSeg) -> bool {
            if (ringSeg.chain.size() < 4) {
                return false;
            }
            bool inside = false;
            // Ignore the duplicated closing vertex by stopping at size()-1.
            for (std::size_t i = 0, j = ringSeg.chain.size() - 2; i < ringSeg.chain.size() - 1; j = i++) {
                const glm::vec2& a = ringSeg.chain[i];
                const glm::vec2& b = ringSeg.chain[j];
                const bool intersects = ((a.y > point.y) != (b.y > point.y)) &&
                    (point.x < (b.x - a.x) * (point.y - a.y) / ((b.y - a.y) + 1e-12f) + a.x);
                if (intersects) {
                    inside = !inside;
                }
            }
            return inside;
        };

        auto emitRing = [&](std::size_t i, bool asOuter) {
            // Outer ring must be CCW; holes must be CW.
            rings[i].orientCCW();
            if (!asOuter) {
                std::reverse(rings[i].chain.begin(), rings[i].chain.end());
            }

            polygon.emplace_back();
            auto& ring = polygon.back();
            ring.reserve(rings[i].chain.size());
            for (const glm::vec2& p : rings[i].chain) {
                const glm::vec2 world = p + offset;
                ring.push_back({static_cast<double>(world.x), static_cast<double>(world.y)});
                earcutVertices.push_back(world);
            }
        };

        if (!rings[outerIdx].validate()) {
            return;
        }

        // Earcut expects ring 0 to be the outer shell.
        emitRing(outerIdx, true);

        for (std::size_t i = 0; i < rings.size(); ++i) {
            if (i == outerIdx) {
                continue;
            }
            if (!rings[i].validate()) {
                continue;
            }
            // Option 2: only emit contained rings as holes.
            if (rings[i].chain.empty() || !pointInRing(rings[i].chain[0], rings[outerIdx])) {
                continue;
            }
            emitRing(i, false);
        }

        if (polygon.empty()) {
            return;
        }

        std::vector<uint32_t> earcutIndices = mapbox::earcut<uint32_t>(polygon);
        (void)earcutVertices;
        (void)earcutIndices;

        // generate mesh
        std::vector<float> meshVertices;
        meshVertices.reserve(earcutVertices.size() * 5);
        for (const glm::vec2& v : earcutVertices) {
            meshVertices.push_back(v.x);
            meshVertices.push_back(-v.y);
            meshVertices.push_back(0.0f);
            meshVertices.push_back(0.0f);
            meshVertices.push_back(0.0f);
        }
        bsk::Mesh* mesh = new bsk::Mesh(meshVertices, earcutIndices);
        bsk::Node2D* node = new bsk::Node2D(mesh, material, { 0.0f, 0.0f }, 0.0f, glm::vec2(1.0f, 1.0f));
        scene->add(node);
    }

private:
    void merge(Seg& other) {
        for (std::size_t i = 0; i < segs.size(); ++i) {
            if (!segs[i].merge(other)) {
                continue;
            }
            Seg merged = segs[i];
            segs.erase(segs.begin() + static_cast<std::ptrdiff_t>(i));
            merge(merged);
            return;
        }
        segs.push_back(other);
    }

    std::vector<Seg> segs;
};

class Grid {
public:
    explicit Grid(std::vector<std::vector<int>> grid) : weights(std::move(grid)) { dfs(); }

    bool isInside(int x, int y) const {
        return x >= 0 && x < SIDE_LENGTH && y >= 0 && y < SIDE_LENGTH;
    }

    bool isValid(int x, int y) const {
        return isInside(x, y) && weights[x][y] == 1;
    }

    void dfs() {
        components.clear();
        std::unordered_set<int> unvisited;
        unvisited.reserve(SIDE_LENGTH * SIDE_LENGTH);
        for (int x = 0; x < SIDE_LENGTH; ++x) {
            for (int y = 0; y < SIDE_LENGTH; ++y) {
                if (isValid(x, y)) {
                    unvisited.insert(encode(x, y));
                }
            }
        }

        std::deque<std::pair<int, int>> queue;
        while (!unvisited.empty()) {
            const int seed = *unvisited.begin();
            unvisited.erase(seed);
            queue.push_back(decode(seed));
            components.emplace_back();
            auto& current = components.back();

            while (!queue.empty()) {
                const auto [x, y] = queue.front();
                queue.pop_front();
                current.emplace_back(x, y);

                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx == 0 && dy == 0) {
                            continue;
                        }
                        const int nx = x + dx;
                        const int ny = y + dy;
                        if (!isInside(nx, ny)) {
                            continue;
                        }
                        const int key = encode(nx, ny);
                        auto it = unvisited.find(key);
                        if (it != unvisited.end()) {
                            queue.emplace_back(nx, ny);
                            unvisited.erase(it);
                        }
                    }
                }
            }
        }
    }

    bsk::Material getCircularColor(int i, int comps) const {
        const float pi = static_cast<float>(M_PI);
        const float t = static_cast<float>(i) / static_cast<float>(std::max(comps, 1));
        return bsk::Material(glm::vec3(
            (200.0f + 50.0f * std::sin(2.0f * pi * t + 0.0f * pi / 3.0f)) / 255.0f,
            (200.0f + 50.0f * std::sin(2.0f * pi * t + 2.0f * pi / 3.0f)) / 255.0f,
            (200.0f + 50.0f * std::sin(2.0f * pi * t + 4.0f * pi / 3.0f)) / 255.0f
        ));
    }

    void genComps(bsk::Scene2D* scene, const glm::vec2& offset) {
        const int compCount = static_cast<int>(components.size());
        for (int i = 0; i < compCount; ++i) {
            materials.emplace_back(std::make_unique<bsk::Material>(getCircularColor(i, compCount)));
            bsk::Material* mat = materials.back().get();
            for (const auto& [x, y] : components[i]) {
                new bsk::Node2D(scene, nullptr, mat, glm::vec2(static_cast<float>(x), static_cast<float>(y)) + offset);
            }
        }
    }

    void genMarch(
        bsk::Scene2D* scene,
        const glm::vec2& offset
    ) {
        const int compCount = static_cast<int>(components.size());
        // Reuse BFS component materials for marching polygons.
        if (static_cast<int>(materials.size()) < compCount) {
            for (int i = static_cast<int>(materials.size()); i < compCount; ++i) {
                materials.emplace_back(std::make_unique<bsk::Material>(getCircularColor(i, compCount)));
            }
        }

        for (std::size_t i = 0; i < components.size(); ++i) {
            Polygon polygon;
            std::unordered_set<int> seenAnchors;
            std::vector<std::pair<int, int>> orderedAnchors;

            for (const auto& [x, y] : components[i]) {
                for (int dx : {0, -1}) {
                    for (int dy : {0, -1}) {
                        const int ax = x + dx;
                        const int ay = y + dy;
                        const int key = encode(ax, ay);
                        if (seenAnchors.insert(key).second) {
                            orderedAnchors.emplace_back(ax, ay);
                        }
                    }
                }
            }

            for (const auto& [qx, qy] : orderedAnchors) {
                const int idx = getMarchQuad(qx, qy);
                for (const auto& e : marchCases[idx]) {
                    polygon.add(glm::vec2(static_cast<float>(qx), static_cast<float>(qy)) + e.a,
                                glm::vec2(static_cast<float>(qx), static_cast<float>(qy)) + e.b);
                }
            }

            polygon.draw(scene, offset, materials[i].get());
        }
    }

private:
    static int encode(int x, int y) {
        return (x + SIDE_LENGTH) * 1024 + (y + SIDE_LENGTH);
    }

    static std::pair<int, int> decode(int key) {
        const int x = (key / 1024) - SIDE_LENGTH;
        const int y = (key % 1024) - SIDE_LENGTH;
        return {x, y};
    }

    int getMarchQuad(int x, int y) const {
        return 4 * static_cast<int>(isValid(x + 1, y + 1))
             + 2 * static_cast<int>(isValid(x + 1, y))
             + 8 * static_cast<int>(isValid(x, y + 1))
             + 1 * static_cast<int>(isValid(x, y));
    }

    std::vector<std::vector<int>> weights;
    std::vector<std::vector<std::pair<int, int>>> components;
    std::vector<std::unique_ptr<bsk::Material>> materials;

    const std::array<std::vector<Edge>, 16> marchCases {{
        {}, // 0
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 0.0f)}}, // 1
        {Edge{glm::vec2(0.5f, 0.0f), glm::vec2(1.0f, 0.5f)}}, // 2
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(1.0f, 0.5f)}}, // 3
        {Edge{glm::vec2(0.5f, 1.0f), glm::vec2(1.0f, 0.5f)}}, // 4
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 1.0f)}, Edge{glm::vec2(0.5f, 0.0f), glm::vec2(1.0f, 0.5f)}}, // 5
        {Edge{glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 1.0f)}}, // 6
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 1.0f)}}, // 7
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 1.0f)}}, // 8
        {Edge{glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 1.0f)}}, // 9
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 0.0f)}, Edge{glm::vec2(0.5f, 1.0f), glm::vec2(1.0f, 0.5f)}}, // 10
        {Edge{glm::vec2(0.5f, 1.0f), glm::vec2(1.0f, 0.5f)}}, // 11
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(1.0f, 0.5f)}}, // 12
        {Edge{glm::vec2(0.5f, 0.0f), glm::vec2(1.0f, 0.5f)}}, // 13
        {Edge{glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 0.0f)}}, // 14
        {} // 15
    }};
};

} // namespace

int main() {
    bsk::Engine* engine = new bsk::Engine(1200, 900, "Marching Squares");
    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    auto* camera = new bsk::StaticCamera2D(engine, glm::vec2((SIDE_LENGTH - 1) / 2.0f), SIDE_LENGTH * 2.0f * 1.25f);
    scene->setCamera(camera);

    bsk::Material* whiteMat = new bsk::Material(glm::vec3(1.0f, 1.0f, 1.0f));
    bsk::Material* redMat = new bsk::Material(glm::vec3(1.0f, 0.5f, 0.5f));

    std::vector<std::vector<int>> weights(SIDE_LENGTH, std::vector<int>(SIDE_LENGTH, -1));
    PerlinNoise noise;
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> startDist(0.0, 10000.0);
    const double noiseStartX = startDist(rng);
    const double noiseStartY = startDist(rng);
    for (int x = 0; x < SIDE_LENGTH; ++x) {
        for (int y = 0; y < SIDE_LENGTH; ++y) {
            // Use OCTAVES as a direct frequency multiplier to mirror
            // python perlin_noise.PerlinNoise(octaves=...) behavior.
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
                new bsk::Node2D(scene, nullptr, whiteMat, glm::vec2(static_cast<float>(x), static_cast<float>(y)) + perlinOffset);
            }
        }
    }

    Grid grid(weights);
    grid.dfs();
    grid.genComps(scene, glm::vec2(DELTA, DELTA));
    grid.genMarch(scene, glm::vec2(-DELTA, -DELTA));

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    delete redMat;
    delete whiteMat;
    delete camera;
    delete scene;
    delete engine;
    return 0;
}