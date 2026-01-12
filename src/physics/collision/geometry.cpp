#include <basilisk/physics/collision/geometry.h>

namespace bsk::internal {

std::pair<glm::vec2, glm::vec2> getAABB(const std::vector<glm::vec2>& vertices) {
    if (vertices.empty()) {
        return { glm::vec2(0.0f), glm::vec2(0.0f) };
    }
    
    glm::vec2 min = vertices[0];
    glm::vec2 max = vertices[0];
    for (const auto& vertex : vertices) {
        min = glm::min(min, vertex);
        max = glm::max(max, vertex);
    }
    return { min, max };
}

std::pair<float, float> getMassProperties(const std::vector<glm::vec2>& vertices, glm::vec2& com) {
    if (vertices.size() < 3) {
        com = glm::vec2(0.0f);
        return { 0.0f, 0.0f };
    }
    
    // Compute area using shoelace formula (for CCW polygons, area is positive)
    float area = 0.0f;
    float centroidX = 0.0f;
    float centroidY = 0.0f;
    float I_origin = 0.0f;
    
    for (std::size_t i = 0; i < vertices.size(); i++) {
        const glm::vec2& v1 = vertices[i];
        const glm::vec2& v2 = vertices[(i + 1) % vertices.size()];
        
        float cross = v1.x * v2.y - v1.y * v2.x;
        area += cross;
        
        // Compute centroid contributions
        centroidX += (v1.x + v2.x) * cross;
        centroidY += (v1.y + v2.y) * cross;
        
        // Compute moment of inertia about origin
        I_origin += cross * (v1.x * v1.x + v1.x * v2.x + v2.x * v2.x + 
                             v1.y * v1.y + v1.y * v2.y + v2.y * v2.y);
    }
    
    area *= 0.5f;
    if (std::abs(area) < 1e-9f) {
        com = glm::vec2(0.0f);
        return { 0.0f, 0.0f };
    }
    
    // Compute centroid (center of mass)
    float invArea = 1.0f / (6.0f * area);
    float Cx = centroidX * invArea;
    float Cy = centroidY * invArea;
    com = glm::vec2(Cx, Cy);
    
    // Compute moment of inertia about centroid using parallel axis theorem
    // I = I_origin - A * (Cx^2 + Cy^2)
    float I = (I_origin / 12.0f) - area * (Cx * Cx + Cy * Cy);
    
    return { std::abs(area), std::abs(I) };
}

}