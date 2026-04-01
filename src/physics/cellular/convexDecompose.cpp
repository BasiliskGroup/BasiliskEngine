#include <basilisk/physics/cellular/convexDecompose.h>

namespace bsk::internal {

// ----------------------------------------------
// Convex
// ----------------------------------------------

// TODO remove thi in favor of an O(1) operation
void Convex::rebuildEdges() {
    edgeToIndex.clear();
    const int n = static_cast<int>(vertices.size());
    if (n < 3) {
        return;
    }
    for (int i = 0; i < n; ++i) {
        const int j = (i + 1) % n;
        edgeToIndex[Edge{vertices[static_cast<std::size_t>(i)], vertices[static_cast<std::size_t>(j)]}] = j;
    }
}

bool Convex::add(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    int index = -1;
    glm::vec2 insert;

    if (edgeToIndex.find(Edge{b, a}) != edgeToIndex.end()) {
        index = edgeToIndex[Edge{b, a}];
        insert = c;
    } else if (edgeToIndex.find(Edge{c, b}) != edgeToIndex.end()) {
        index = edgeToIndex[Edge{c, b}];
        insert = a;
    } else if (edgeToIndex.find(Edge{a, c}) != edgeToIndex.end()) {
        index = edgeToIndex[Edge{a, c}];
        insert = b;
    } else {
        return false;
    }

    vertices.insert(vertices.begin() + index, insert);
    rebuildEdges();
    return true;
}

}