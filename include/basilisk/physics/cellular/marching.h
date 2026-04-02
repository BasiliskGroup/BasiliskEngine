#ifndef BSK_PHYSICS_CELLULAR_MARCHING_H
#define BSK_PHYSICS_CELLULAR_MARCHING_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/cellular/helper.h>
#include <basilisk/physics/cellular/rdp.h>
#include <basilisk/physics/cellular/convexDecompose.h>

#include <earcut.hpp>
#include <utility>
#include <vector>

namespace bsk::internal {

struct MarchComponentGeometry {
    std::vector<glm::vec2> filledVertices;
    std::vector<uint32_t> filledIndices;
    std::vector<Convex> convexPieces;
};

// ----------------------------------------------
// Seg
// ----------------------------------------------

class Seg {
private:
    std::vector<glm::vec2> chain;
    glm::vec2 bl;
    glm::vec2 tr;

    friend class Polygon;

public:
    Seg(const glm::vec2& a, const glm::vec2& b) : chain{a, b}, bl(glm::min(a, b)), tr(glm::max(a, b)) {}
    
    // adding new line segments
    bool add(const glm::vec2& a, const glm::vec2& b);
    bool merge(Seg& other);
    
    // preparing the segment for earcut
    bool isLoop() const;
    float signedArea() const;
    bool orientCCW(); // TODO, even better, simply check winding direction and change how iteration works. No need to reverse.
};

// ----------------------------------------------
// Polygon
// ----------------------------------------------

class Polygon {
private:
    std::vector<Seg> segs;
    std::vector<uint32_t> earcutIndices;
    std::vector<glm::vec2> earcutVertices;
    std::vector<Convex> polygons;
        
public:
    void add(const glm::vec2& a, const glm::vec2& b);
    void earcut();

    const std::vector<glm::vec2>& filledVerts() const { return earcutVertices; }
    const std::vector<uint32_t>& filledIndices() const { return earcutIndices; }

    std::vector<Convex>& decompose();

private:
    void merge(Seg& other);
    void decomposeBayazitEntry(const std::vector<glm::vec2>& ring);
};

// ----------------------------------------------
// Marching Grid
// ----------------------------------------------

class Grid {
private:
    std::vector<std::vector<int>> weights;
    std::vector<std::vector<std::pair<int, int>>> components;
    int width, height;

    static constexpr int ENCODING = 1024;

    // marching squares cases hard coded
    const std::array<std::vector<Edge>, 16> marchCases {{
        {},
        {Edge{{0.0f, 0.5f}, {0.5f, 0.0f}}},
        {Edge{{0.5f, 0.0f}, {1.0f, 0.5f}}},
        {Edge{{0.0f, 0.5f}, {1.0f, 0.5f}}},
        {Edge{{0.5f, 1.0f}, {1.0f, 0.5f}}},
        {Edge{{0.0f, 0.5f}, {0.5f, 1.0f}}, Edge{{0.5f, 0.0f}, {1.0f, 0.5f}}},
        {Edge{{0.5f, 0.0f}, {0.5f, 1.0f}}},
        {Edge{{0.0f, 0.5f}, {0.5f, 1.0f}}},
        {Edge{{0.0f, 0.5f}, {0.5f, 1.0f}}},
        {Edge{{0.5f, 0.0f}, {0.5f, 1.0f}}},
        {Edge{{0.0f, 0.5f}, {0.5f, 0.0f}}, Edge{{0.5f, 1.0f}, {1.0f, 0.5f}}},
        {Edge{{0.5f, 1.0f}, {1.0f, 0.5f}}},
        {Edge{{0.0f, 0.5f}, {1.0f, 0.5f}}},
        {Edge{{0.5f, 0.0f}, {1.0f, 0.5f}}},
        {Edge{{0.0f, 0.5f}, {0.5f, 0.0f}}},
        {}
    }};

public:
    explicit Grid(std::vector<std::vector<int>> grid)
        : weights(std::move(grid))
        , width(static_cast<int>(weights.size()))
        , height(weights.empty() ? 0 : static_cast<int>(weights[0].size()))
    {
        bfs();
    }

    bool isInside(int x, int y) const { return x >= 0 && x < width && y >= 0 && y < height; }
    bool isValid(int x, int y) const { return isInside(x, y) && weights[x][y] == 1; }

    void bfs();

    const std::vector<std::vector<std::pair<int, int>>>& connectedComponents() const { return components; }

    std::vector<MarchComponentGeometry> genMarch();
    
private:
    int encode(int x, int y) {
        return (x + width) * ENCODING + (y + height);
    }

    std::pair<int, int> decode(int key) {
        const int x = (key / ENCODING) - width;
        const int y = (key % ENCODING) - height;
        return {x, y};
    }

    int getMarchQuad(int x, int y) const {
        return 4 * static_cast<int>(isValid(x + 1, y + 1))
             + 2 * static_cast<int>(isValid(x + 1, y       ))
             + 8 * static_cast<int>(isValid(x,      y + 1))
             + 1 * static_cast<int>(isValid(x,      y ));
    }
};

}

#endif