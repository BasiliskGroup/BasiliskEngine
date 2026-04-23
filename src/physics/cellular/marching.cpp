#include <basilisk/physics/cellular/marching.h>

namespace bsk::internal {

namespace {
constexpr float RAY_EPS = 1e-5f;
constexpr float BAYAZIT_EPS = 1e-6f;
constexpr int BAYAZIT_MAX_RECURSION = 32;

std::vector<glm::vec2> toOpenRing(const std::vector<glm::vec2>& chain) {
    std::vector<glm::vec2> ring = chain;
    if (!ring.empty() && vec2Eq(ring.front(), ring.back())) {
        ring.pop_back();
    }
    return ring;
}

std::vector<glm::vec2> simplifyRingForDecompose(const std::vector<glm::vec2>& chain) {
    std::vector<glm::vec2> openRing = toOpenRing(chain);
    if (openRing.size() < 3) {
        return {};
    }

    std::vector<glm::vec2> simplified;
    RDP(openRing, simplified, RDP_EPSILON);
    if (simplified.size() < 3) {
        return {};
    }
    return simplified;
}

float signedAreaOpen(const std::vector<glm::vec2>& ring) {
    if (ring.size() < 3) {
        return 0.0f;
    }
    float area = 0.0f;
    for (std::size_t i = 0; i < ring.size(); ++i) {
        const glm::vec2& p = ring[i];
        const glm::vec2& q = ring[(i + 1) % ring.size()];
        area += p.x * q.y - q.x * p.y;
    }
    return 0.5f * area;
}

const glm::vec2& at(const std::vector<glm::vec2>& poly, int i) {
    const int n = static_cast<int>(poly.size());
    int idx = i % n;
    if (idx < 0) {
        idx += n;
    }
    return poly[static_cast<std::size_t>(idx)];
}

float cross2(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    const glm::vec2 ab = b - a;
    const glm::vec2 ac = c - a;
    return ab.x * ac.y - ab.y * ac.x;
}

float sqDist(const glm::vec2& a, const glm::vec2& b) {
    const glm::vec2 d = b - a;
    return glm::dot(d, d);
}

// ----------------------------------------------
// Hole bridging
// ----------------------------------------------

bool segmentCrossesRing(const glm::vec2& a, const glm::vec2& b,
    const std::vector<glm::vec2>& ring)
{
    const float eps = 1e-5f;
    const std::size_t n = ring.size();
    for (std::size_t i = 0; i < n; ++i) {
        const glm::vec2& p = ring[i];
        const glm::vec2& q = ring[(i + 1) % n];

        // Skip edges that share an endpoint with the bridge
        if (vec2Eq(p, a) || vec2Eq(p, b) || vec2Eq(q, a) || vec2Eq(q, b)) {
            continue;
        }

        const float d1 = cross2(a, b, p);
        const float d2 = cross2(a, b, q);
        const float d3 = cross2(p, q, a);
        const float d4 = cross2(p, q, b);

        if (((d1 > eps && d2 < -eps) || (d1 < -eps && d2 > eps)) &&
            ((d3 > eps && d4 < -eps) || (d3 < -eps && d4 > eps)))
        {
            return true;
        }
    }
    return false;
}

bool tryBridgeHole(std::vector<glm::vec2>& outer, const std::vector<glm::vec2>& hole) {
    if (outer.size() < 3 || hole.size() < 3) {
        return false;
    }

    float bestDist = std::numeric_limits<float>::infinity();
    std::size_t bestOuter = static_cast<std::size_t>(-1);
    std::size_t bestHole  = static_cast<std::size_t>(-1);

    for (std::size_t hi = 0; hi < hole.size(); ++hi) {
        for (std::size_t oi = 0; oi < outer.size(); ++oi) {
            const float d = sqDist(hole[hi], outer[oi]);
            if (d >= bestDist) {
                continue;
            }
            // Reject if the bridge crosses the outer ring or the hole itself
            if (segmentCrossesRing(hole[hi], outer[oi], outer)) {
                continue;
            }
            if (segmentCrossesRing(hole[hi], outer[oi], hole)) {
                continue;
            }
            bestDist  = d;
            bestOuter = oi;
            bestHole  = hi;
        }
    }

    if (bestOuter == static_cast<std::size_t>(-1)) {
        return false;
    }

    // Stitch: outer[0..bestOuter] -> hole[bestHole..bestHole] -> outer[bestOuter..end]
    // The bridge edge is duplicated (once each direction) to close the seam.
    std::vector<glm::vec2> merged;
    merged.reserve(outer.size() + hole.size() + 2);

    for (std::size_t i = 0; i <= bestOuter; ++i) {
        merged.push_back(outer[i]);
    }
    for (std::size_t k = 0; k <= hole.size(); ++k) {
        merged.push_back(hole[(bestHole + k) % hole.size()]);
    }
    // Return to the outer bridge vertex to close the seam
    merged.push_back(outer[bestOuter]);
    for (std::size_t i = bestOuter + 1; i < outer.size(); ++i) {
        merged.push_back(outer[i]);
    }

    outer = std::move(merged);
    return true;
}

// ----------------------------------------------
// Bayazit helpers
// ----------------------------------------------

bool left(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    return cross2(a, b, c) > BAYAZIT_EPS;
}

bool leftOn(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    return cross2(a, b, c) >= -BAYAZIT_EPS;
}

bool right(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    return cross2(a, b, c) < -BAYAZIT_EPS;
}

bool rightOn(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    return cross2(a, b, c) <= BAYAZIT_EPS;
}

bool lineIntersection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& q1, const glm::vec2& q2,
    glm::vec2& out)
{
    const glm::vec2 r = p2 - p1;
    const glm::vec2 s = q2 - q1;
    const float denom = r.x * s.y - r.y * s.x;
    if (std::abs(denom) <= BAYAZIT_EPS) {
        return false;
    }
    const glm::vec2 qp = q1 - p1;
    const float t = (qp.x * s.y - qp.y * s.x) / denom;
    out = p1 + t * r;
    return true;
}

// Point-in-polygon via ray casting (for open rings).
bool pointInPolygon(const glm::vec2& pt, const std::vector<glm::vec2>& poly) {
    int crossings = 0;
    const int n = static_cast<int>(poly.size());
    for (int i = 0; i < n; ++i) {
        const glm::vec2& a = poly[static_cast<std::size_t>(i)];
        const glm::vec2& b = poly[static_cast<std::size_t>((i + 1) % n)];
        if ((a.y <= pt.y && pt.y < b.y) || (b.y <= pt.y && pt.y < a.y)) {
            const float t = (pt.y - a.y) / (b.y - a.y);
            if (pt.x < a.x + t * (b.x - a.x)) {
                ++crossings;
            }
        }
    }
    return (crossings % 2) == 1;
}

// Returns true if the diagonal from poly[i] to p is interior to the polygon.
bool diagonalIsValid(const std::vector<glm::vec2>& poly, int i, const glm::vec2& p) {
    const glm::vec2 mid = 0.5f * (at(poly, i) + p);
    return pointInPolygon(mid, poly);
}

bool isConvexPolygon(const std::vector<glm::vec2>& poly) {
    if (poly.size() < 3) {
        return false;
    }
    for (int i = 0; i < static_cast<int>(poly.size()); ++i) {
        if (right(at(poly, i - 1), at(poly, i), at(poly, i + 1))) {
            return false;
        }
    }
    return true;
}

std::vector<glm::vec2> slicePoly(const std::vector<glm::vec2>& poly, int i, int j) {
    std::vector<glm::vec2> out;
    const int n = static_cast<int>(poly.size());
    if (n == 0) {
        return out;
    }
    int idx = i;
    out.push_back(at(poly, idx));
    while (idx != j) {
        idx = (idx + 1) % n;
        out.push_back(at(poly, idx));
    }
    return out;
}

void emitConvexPiece(const std::vector<glm::vec2>& piece, std::vector<BayazitConvex>& outPieces) {
    if (piece.size() < 3) {
        return;
    }
    BayazitConvex c(piece[0], piece[1], piece[2]);
    c.vertices = piece;
    outPieces.push_back(std::move(c));
}

void bayazitDecompose(const std::vector<glm::vec2>& poly, std::vector<BayazitConvex>& outPieces, int depth) {
    if (poly.size() < 3 || depth > BAYAZIT_MAX_RECURSION) {
        return;
    }
    if (isConvexPolygon(poly)) {
        emitConvexPiece(poly, outPieces);
        return;
    }

    const int n = static_cast<int>(poly.size());
    for (int i = 0; i < n; ++i) {
        if (!right(at(poly, i - 1), at(poly, i), at(poly, i + 1))) {
            continue;
        }

        float lowerDist = std::numeric_limits<float>::infinity();
        float upperDist = std::numeric_limits<float>::infinity();
        int lowerIndex = -1;
        int upperIndex = -1;
        glm::vec2 lowerInt(0.0f);
        glm::vec2 upperInt(0.0f);

        for (int j = 0; j < n; ++j) {
            glm::vec2 p(0.0f);

            if (left(at(poly, i - 1), at(poly, i), at(poly, j)) &&
                rightOn(at(poly, i - 1), at(poly, i), at(poly, j - 1)) &&
                lineIntersection(at(poly, i - 1), at(poly, i), at(poly, j), at(poly, j - 1), p) &&
                right(at(poly, i + 1), at(poly, i), p) &&
                diagonalIsValid(poly, i, p))
            {
                const float d = sqDist(at(poly, i), p);
                if (d < lowerDist) {
                    lowerDist = d;
                    lowerIndex = j;
                    lowerInt = p;
                }
            }

            if (left(at(poly, i + 1), at(poly, i), at(poly, j + 1)) &&
                rightOn(at(poly, i + 1), at(poly, i), at(poly, j)) &&
                lineIntersection(at(poly, i + 1), at(poly, i), at(poly, j), at(poly, j + 1), p) &&
                left(at(poly, i - 1), at(poly, i), p) &&
                diagonalIsValid(poly, i, p))
            {
                const float d = sqDist(at(poly, i), p);
                if (d < upperDist) {
                    upperDist = d;
                    upperIndex = j;
                    upperInt = p;
                }
            }
        }

        std::vector<glm::vec2> lowerPoly;
        std::vector<glm::vec2> upperPoly;

        if (lowerIndex == -1 || upperIndex == -1) {
            // Fallback split to guarantee progress.
            const int j = (i + 2) % n;
            lowerPoly = slicePoly(poly, i, j);
            upperPoly = slicePoly(poly, j, i);
        } else if (lowerIndex == (upperIndex + 1) % n) {
            const glm::vec2 steiner = 0.5f * (lowerInt + upperInt);
            lowerPoly = slicePoly(poly, i, upperIndex);
            lowerPoly.push_back(steiner);
            upperPoly = slicePoly(poly, lowerIndex, i);
            upperPoly.push_back(steiner);
        } else {
            float bestDist = std::numeric_limits<float>::infinity();
            int bestIndex = -1;

            int j = lowerIndex;
            int steps = 0;
            while (j != upperIndex && steps < n) {
                if (leftOn(at(poly, i - 1), at(poly, i), at(poly, j)) &&
                    rightOn(at(poly, i + 1), at(poly, i), at(poly, j)))
                {
                    const float d = sqDist(at(poly, i), at(poly, j));
                    if (d < bestDist) {
                        bestDist = d;
                        bestIndex = j;
                    }
                }
                j = (j + 1) % n;
                ++steps;
            }

            if (bestIndex == -1) {
                bestIndex = lowerIndex;
            }

            lowerPoly = slicePoly(poly, i, bestIndex);
            upperPoly = slicePoly(poly, bestIndex, i);
        }

        if (lowerPoly.size() < 3 || upperPoly.size() < 3) {
            // Final fallback: emit fan to avoid losing geometry.
            for (int k = 1; k + 1 < n; ++k) {
                const glm::vec2& a = poly[0];
                const glm::vec2& b = poly[static_cast<std::size_t>(k)];
                const glm::vec2& c = poly[static_cast<std::size_t>(k + 1)];
                if (cross2(a, b, c) > BAYAZIT_EPS) {
                    emitConvexPiece({a, b, c}, outPieces);
                }
            }
            return;
        }

        if (lowerPoly.size() < upperPoly.size()) {
            bayazitDecompose(lowerPoly, outPieces, depth + 1);
            bayazitDecompose(upperPoly, outPieces, depth + 1);
        } else {
            bayazitDecompose(upperPoly, outPieces, depth + 1);
            bayazitDecompose(lowerPoly, outPieces, depth + 1);
        }
        return;
    }

    // Should not happen for non-degenerate polygons; fallback to fan.
    for (int k = 1; k + 1 < static_cast<int>(poly.size()); ++k) {
        const glm::vec2& a = poly[0];
        const glm::vec2& b = poly[static_cast<std::size_t>(k)];
        const glm::vec2& c = poly[static_cast<std::size_t>(k + 1)];
        if (cross2(a, b, c) > BAYAZIT_EPS) {
            emitConvexPiece({a, b, c}, outPieces);
        }
    }
}
} // anonymous namespace

// ----------------------------------------------
// BayazitSeg
// ----------------------------------------------

bool BayazitSeg::add(const glm::vec2& a, const glm::vec2& b) {
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

bool BayazitSeg::merge(BayazitSeg& other) {
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

bool BayazitSeg::isLoop() const {
    // Earcut expects closed rings (first point duplicated at end).
    if (chain.size() < 3) return false;
    return vec2Eq(chain.front(), chain.back());
}

float BayazitSeg::signedArea() const {
    float area = 0.0f;
    for (size_t i = 0; i + 1 < chain.size(); ++i) {
        const auto& p = chain[i];
        const auto& q = chain[i + 1];
        area += (p.x * q.y - q.x * p.y);
    }
    return 0.5f * area;
}

bool BayazitSeg::orientCCW() {
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
        BayazitSeg merged = segs[i];
        segs.erase(segs.begin() + static_cast<std::ptrdiff_t>(i));
        merge(merged);
        return;
    }
    segs.emplace_back(a, b);
}

void Polygon::merge(BayazitSeg& other) {
    for (std::size_t i = 0; i < segs.size(); ++i) {
        if (!segs[i].merge(other)) {
            continue;
        }
        BayazitSeg merged = segs[i];
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

    std::vector<BayazitSeg> rings = segs;

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

void Polygon::decomposeBayazitEntry(const std::vector<glm::vec2>& ring) {
    if (ring.size() < 3) {
        return;
    }
    std::vector<glm::vec2> poly = ring;
    if (signedAreaOpen(poly) < 0.0f) {
        std::reverse(poly.begin(), poly.end());
    }
    bayazitDecompose(poly, polygons, 0);
}

std::vector<BayazitConvex>& Polygon::decompose() {
    polygons.clear();

    std::vector<std::vector<glm::vec2>> rings;
    rings.reserve(segs.size());

    for (const BayazitSeg& seg : segs) {
        if (!seg.isLoop()) {
            continue;
        }
        std::vector<glm::vec2> ring = simplifyRingForDecompose(seg.chain);
        if (ring.size() < 3) {
            continue;
        }
        rings.push_back(std::move(ring));
    }

    if (rings.empty()) {
        return polygons;
    }

    std::size_t outerIdx = 0;
    float outerAbsArea = 0.0f;
    for (std::size_t i = 0; i < rings.size(); ++i) {
        const float area = std::abs(signedAreaOpen(rings[i]));
        if (area > outerAbsArea) {
            outerAbsArea = area;
            outerIdx = i;
        }
    }

    std::vector<glm::vec2> outer = rings[outerIdx];
    if (signedAreaOpen(outer) < 0.0f) {
        std::reverse(outer.begin(), outer.end());
    }

    for (std::size_t i = 0; i < rings.size(); ++i) {
        if (i == outerIdx) {
            continue;
        }

        std::vector<glm::vec2> hole = rings[i];
        if (signedAreaOpen(hole) > 0.0f) {
            std::reverse(hole.begin(), hole.end());
        }

        if (!tryBridgeHole(outer, hole)) {
            // Fallback: still emit hole geometry instead of dropping it.
            // decomposeBayazitEntry(hole);
            std::cout << "[bayazit] skipped hole: bridge failed\n";
        }
    }

    decomposeBayazitEntry(outer);
    return polygons;
}

// ----------------------------------------------
// MarchingGrid
// ----------------------------------------------

void MarchingGrid::bfs() {
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

std::vector<MarchComponentGeometry> MarchingGrid::genMarch() {
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
        geom.convexPieces = std::move(polygon.decompose());
        ++componentIndex;
        out.push_back(std::move(geom));
    }

    return out;
}

} // namespace bsk::internal