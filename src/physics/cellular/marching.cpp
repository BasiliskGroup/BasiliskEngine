#include <basilisk/physics/cellular/marching.h>

namespace bsk::internal {

// ----------------------------------------------
// Seg
// ----------------------------------------------

bool Seg::add(const glm::vec2& a, const glm::vec2& b) {
    if (glm::length(a - b) >= 2.0f) {
        return false;
    }

    if (vec2Eq(chain.front(), a)) chain.insert(chain.begin(), b);
    else if (vec2Eq(chain.back(), a)) chain.push_back(b);
    else if (vec2Eq(chain.front(), b)) chain.insert(chain.begin(), a);
    else if (vec2Eq(chain.back(), b)) chain.push_back(a);
    else return false;

    bl = glm::min(bl, glm::min(a, b));
    tr = glm::max(tr, glm::max(a, b));
    return true;
}

bool Seg::merge(Seg& other) {
    // Merge two chains when their endpoints touch.
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

bool Seg::isLoop() const {
    // Earcut expects closed rings (first point duplicated at end).
    if (chain.size() < 3) return false;
    return vec2Eq(chain.front(), chain.back());
}

float Seg::signedArea() const {
    float area = 0.0f;
    for (size_t i = 0; i + 1 < chain.size(); ++i) {
        const auto& p = chain[i];
        const auto& q = chain[i + 1];
        area += (p.x * q.y - q.x * p.y);
    }
    return 0.5f * area;
}

bool Seg::orientCCW() {
    if (!isLoop()) return false;

    float area = signedArea();

    // If clockwise, reverse to make CCW
    if (area < 0.0f) {
        std::reverse(chain.begin(), chain.end());
    }

    return true;
}

// ----------------------------------------------
// Polygon
// ----------------------------------------------

void Polygon::add(const glm::vec2& a, const glm::vec2& b) {
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

void Polygon::merge(Seg& other) {
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

void Polygon::earcut() {
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

    earcutVertices.reserve(rings.size() * 8);

    // TODO optimize this, reduce memory allocations
    auto emitRing = [&](std::size_t i, bool asOuter) {
        // Outer ring must be CCW; holes must be CW.
        rings[i].orientCCW();
        if (!asOuter) {
            std::reverse(rings[i].chain.begin(), rings[i].chain.end());
        }

        // simplify with RDP
        std::vector<glm::vec2> ringPoints = rings[i].chain;
        if (!ringPoints.empty() && vec2Eq(ringPoints.front(), ringPoints.back())) {
            ringPoints.pop_back();
        }
        if (ringPoints.size() < 3) {
            return;
        }

        std::vector<glm::vec2> simplifiedRing;
        RDP(ringPoints, simplifiedRing, RDP_EPSILON);
        if (simplifiedRing.size() < 3) {
            return;
        }

        polygon.emplace_back();
        auto& ring = polygon.back();
        ring.reserve(simplifiedRing.size());
        for (const glm::vec2& p : simplifiedRing) {
            const glm::vec2 world = p;
            ring.push_back({static_cast<double>(world.x), static_cast<double>(world.y)});
            earcutVertices.push_back(world);
        }
    };

    if (!rings[outerIdx].isLoop()) {
        return;
    }

    // Earcut expects ring 0 to be the outer shell.
    emitRing(outerIdx, true);

    for (std::size_t i = 0; i < rings.size(); ++i) {
        if (i == outerIdx) {
            continue;
        }
        if (!rings[i].isLoop()) {
            continue;
        }
        emitRing(i, false);
    }

    if (polygon.empty()) {
        return;
    }

    this->earcutIndices = mapbox::earcut<uint32_t>(polygon);
}

std::vector<Convex> Polygon::decompose() {
    std::vector<Convex> polygons;

    for (std::size_t i = 0; i + 2 < earcutIndices.size(); i += 3) {
        const int a = static_cast<int>(earcutIndices[i]);
        const int b = static_cast<int>(earcutIndices[i + 1]);
        const int c = static_cast<int>(earcutIndices[i + 2]);

        bool found = false;
        for (Convex& p : polygons) {
            if (p.add(earcutVertices[static_cast<std::size_t>(a)], earcutVertices[static_cast<std::size_t>(b)],
                    earcutVertices[static_cast<std::size_t>(c)])) {
                found = true;
                break;
            }
        }

        if (!found) {
            polygons.emplace_back(earcutVertices[static_cast<std::size_t>(a)],
                earcutVertices[static_cast<std::size_t>(b)], earcutVertices[static_cast<std::size_t>(c)]);
        }
    }

    return polygons;
}

// ----------------------------------------------
// Marching Grid
// ----------------------------------------------

void Grid::bfs() {
    components.clear();
    std::unordered_set<int> unvisited;
    unvisited.reserve(width * height);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (isValid(x, y)) {
                unvisited.insert(encode(x, y));
            }
        }
    }

    std::deque<std::pair<int, int>> queue;
    while (!unvisited.empty()) {
        // Flood-fill one connected component at a time.
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

std::vector<MarchComponentGeometry> Grid::genMarch() {
    std::vector<MarchComponentGeometry> out;
    out.reserve(components.size());

    std::size_t componentIndex = 0;
    for (const auto& component : components) {
        Polygon polygon;
        std::unordered_set<int> seenAnchors;
        std::vector<std::pair<int, int>> orderedAnchors;

        for (const auto& [x, y] : component) {
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
            for (const auto& e : marchCases[static_cast<std::size_t>(idx)]) {
                polygon.add(glm::vec2(static_cast<float>(qx), static_cast<float>(qy)) + e.a,
                    glm::vec2(static_cast<float>(qx), static_cast<float>(qy)) + e.b);
            }
        }

        polygon.earcut();

        MarchComponentGeometry geom;
        geom.filledVertices = polygon.filledVerts();
        geom.filledIndices = polygon.filledIndices();
        geom.convexPieces = polygon.decompose();
        std::cout << geom.convexPieces.size()
                  << " convex piece(s)\n";
        ++componentIndex;
        out.push_back(std::move(geom));
    }

    return out;
}

}
