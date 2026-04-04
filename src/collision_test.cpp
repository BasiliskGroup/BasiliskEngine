#include <basilisk/physics/collision/gjk.h>
#include <basilisk/basilisk.h>

#include <earcut.hpp>
#include <span>
#include <random>
#include <algorithm>

namespace {

    std::vector<glm::vec2> GenerateRandomConvexPolygon(int n) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
        std::vector<float> xs(n), ys(n);
        for (int i = 0; i < n; ++i) {
            xs[i] = dist(gen);
            ys[i] = dist(gen);
        }
    
        std::sort(xs.begin(), xs.end());
        std::sort(ys.begin(), ys.end());
    
        std::vector<float> xVec, yVec;
    
        float minX = xs.front(), maxX = xs.back();
        float minY = ys.front(), maxY = ys.back();
    
        float lastTop = minX, lastBot = minX;
        for (int i = 1; i < n - 1; ++i) {
            float x = xs[i];
            if (dist(gen) < 0.5f) {
                xVec.push_back(x - lastTop);
                lastTop = x;
            } else {
                xVec.push_back(lastBot - x);
                lastBot = x;
            }
        }
        xVec.push_back(maxX - lastTop);
        xVec.push_back(lastBot - maxX);
    
        float lastLeft = minY, lastRight = minY;
        for (int i = 1; i < n - 1; ++i) {
            float y = ys[i];
            if (dist(gen) < 0.5f) {
                yVec.push_back(y - lastLeft);
                lastLeft = y;
            } else {
                yVec.push_back(lastRight - y);
                lastRight = y;
            }
        }
        yVec.push_back(maxY - lastLeft);
        yVec.push_back(lastRight - maxY);
    
        std::shuffle(yVec.begin(), yVec.end(), gen);
    
        std::vector<glm::vec2> vecs;
        for (int i = 0; i < n; ++i) {
            vecs.emplace_back(xVec[i], yVec[i]);
        }
    
        // Sort by angle
        std::sort(vecs.begin(), vecs.end(), [](const glm::vec2& a, const glm::vec2& b) {
            return atan2(a.y, a.x) < atan2(b.y, b.x);
        });
    
        // Build polygon
        std::vector<glm::vec2> points;
        glm::vec2 current(0.0f);
        for (const auto& v : vecs) {
            points.push_back(current);
            current += v;
        }
    
        return points;
    }

// `ccwVerts` is a simple polygon (no holes), wound counter-clockwise.
bsk::Mesh* meshFromCCWPolygon(std::span<const glm::vec2> ccwVerts) {
    std::vector<float> vertices;
    vertices.reserve(ccwVerts.size() * 5);
    for (const glm::vec2& vertex : ccwVerts) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
    }

    std::vector<std::vector<std::array<double, 2>>> polygon(1);
    polygon[0].reserve(ccwVerts.size());
    for (const glm::vec2& p : ccwVerts) {
        polygon[0].push_back({static_cast<double>(p.x), static_cast<double>(p.y)});
    }

    std::vector<uint32_t> indices =
        ccwVerts.size() < 3 ? std::vector<uint32_t>{} : mapbox::earcut<uint32_t>(polygon);

    return new bsk::Mesh(vertices, indices);
}

bsk::Mesh* meshFromCCWPolygon(const std::vector<glm::vec2>& ccwVerts) {
    return meshFromCCWPolygon(std::span<const glm::vec2>(ccwVerts.data(), ccwVerts.size()));
}

} // namespace

int main() {
    bsk::Engine* engine = new bsk::Engine(800, 800, "Collision Test");
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    scene->setCamera(new bsk::StaticCamera2D(engine, glm::vec2(0.0f, 0.0f), 15.0f));

    bsk::Material* red = new bsk::Material(glm::vec3(1.0f, 0.0f, 0.0f));
    bsk::Material* blue = new bsk::Material(glm::vec3(0.0f, 0.0f, 1.0f));

    // OBJECT A
    std::vector<glm::vec2> vA = GenerateRandomConvexPolygon(10);
    bsk::Mesh* meshA = meshFromCCWPolygon(vA);
    bsk::Node2D* nodeA = new bsk::Node2D(scene, meshA, blue, glm::vec2(0.0f, 0.0f), 0.0f, glm::vec2(1.5f, 2.5f));

    // OBJECT B
    std::vector<glm::vec2> vB = GenerateRandomConvexPolygon(5);
    bsk::Mesh* meshB = meshFromCCWPolygon(vB);
    bsk::Node2D* nodeB = new bsk::Node2D(scene, meshB, red, glm::vec2(1.0f, 1.0f), 5.0f, glm::vec2(1.5f, 2.5f));

    bsk::Keyboard* keys = engine->getKeyboard();
    const float moveSpeed = 2.0f;

    while (engine->isRunning()) {
        engine->update();
        scene->update();

        // controls
        const float step = moveSpeed * static_cast<float>(engine->getDeltaTime());
        glm::vec2 posA = nodeA->getPosition();
        posA.x += (keys->getDown(bsk::Key::K_D) - keys->getDown(bsk::Key::K_A)) * step;
        posA.y += (keys->getDown(bsk::Key::K_W) - keys->getDown(bsk::Key::K_S)) * step;
        nodeA->setPosition(posA);

        glm::vec2 posB = nodeB->getPosition();
        posB.x += (keys->getDown(static_cast<bsk::Key>(GLFW_KEY_RIGHT)) - keys->getDown(static_cast<bsk::Key>(GLFW_KEY_LEFT))) * step;
        posB.y += (keys->getDown(bsk::Key::K_UP) - keys->getDown(bsk::Key::K_DOWN)) * step;
        nodeB->setPosition(posB);

        // compute world space positions of each object's mesh
        std::vector<glm::vec2> wA;
        glm::mat4 modelA = nodeA->getModel();
        for (const auto& vertex : vA) {
            wA.push_back(modelA * glm::vec4(vertex, 0.0f, 1.0f));
        }
        std::vector<glm::vec2> wB;
        glm::mat4 modelB = nodeB->getModel();
        for (const auto& vertex : vB) {
            wB.push_back(modelB * glm::vec4(vertex, 0.0f, 1.0f));
        }

        // compute collision
        bsk::internal::CollisionPair pair;
        bsk::internal::ConvexShape shapeA(wA, glm::vec2(0.0f, 0.0f), 0.0f, glm::vec2(1.0f, 1.0f));
        bsk::internal::ConvexShape shapeB(wB, glm::vec2(1.0f, 1.0f), 0.0f, glm::vec2(1.0f, 1.0f));
        bool collided = bsk::internal::gjk(shapeA, shapeB, pair);
        if (collided) {
            nodeA->setMaterial(red);
            nodeB->setMaterial(red);
        } else {
            nodeA->setMaterial(blue);
            nodeB->setMaterial(blue);
        }

        scene->render();
        engine->render();
    }

    return 0;
}
