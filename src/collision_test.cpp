#include <basilisk/physics/collision/gjk.h>
#include <basilisk/basilisk.h>

#include <earcut.hpp>
#include <span>
#include <random>
#include <algorithm>
#include <cmath>

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

// Node2D::updateModel translates by (position.x, -position.y). World XY from getModel()*vertex uses
// that convention; stored position must undo the Y sign for a given world point.
glm::vec2 worldXYToNode2DStore(glm::vec2 worldXY) {
    return glm::vec2(worldXY.x, -worldXY.y);
}

// For the engine default quad (local x, y in [-0.5, 0.5]): rotation and scale so the quad is a thin
// bar along the segment from world `a` to world `b` (same XY space as getModel() * vertex).
// `positionWorldMid` is the segment midpoint in that world space — pass through worldXYToNode2DStore
// before Node2D::setPosition.
struct SegmentSquareTransform {
    glm::vec2 positionWorldMid{};
    float rotation = 0.0f;
    glm::vec2 scale{1.0f, 1.0f};
};

// Arithmetic mean of vertices in world space (matches mesh geometric center for this convex test mesh).
glm::vec2 geomCentroidWorld(const std::vector<glm::vec2>& worldVerts) {
    glm::vec2 c(0.0f);
    for (const glm::vec2& p : worldVerts) {
        c += p;
    }
    const float n = static_cast<float>(worldVerts.size());
    return n > 0.0f ? c / n : c;
}

SegmentSquareTransform segmentSquareTransformFromAToB(glm::vec2 a, glm::vec2 b, float thickness) {
    glm::vec2 d = b - a;
    float len = glm::length(d);
    SegmentSquareTransform xf;
    if (len < 1e-6f) {
        xf.positionWorldMid = a;
        xf.rotation = 0.0f;
        xf.scale = glm::vec2(0.0f, thickness);
        return xf;
    }
    d /= len;
    xf.positionWorldMid = 0.5f * (a + b);
    xf.rotation = -std::atan2(d.y, d.x);
    xf.scale = glm::vec2(len, thickness);
    return xf;
}

} // namespace

int main() {
    bsk::Engine* engine = new bsk::Engine(800, 800, "Collision Test");
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    scene->setCamera(new bsk::StaticCamera2D(engine, glm::vec2(0.0f, 0.0f), 10.0f));

    bsk::Material* red = new bsk::Material(glm::vec3(1.0f, 0.0f, 0.0f));
    bsk::Material* blue = new bsk::Material(glm::vec3(0.0f, 0.0f, 1.0f));
    bsk::Material* green = new bsk::Material(glm::vec3(0.0f, 0.85f, 0.25f));
    bsk::Material* yellow = new bsk::Material(glm::vec3(1.0f, 0.92f, 0.2f));
    bsk::Material* cyan = new bsk::Material(glm::vec3(0.2f, 0.95f, 1.0f));
    bsk::Material* magenta = new bsk::Material(glm::vec3(1.0f, 0.2f, 0.85f));
    bsk::Material* white = new bsk::Material(glm::vec3(0.95f, 0.95f, 0.95f));

    constexpr float kNormalDrawLength = 3.0f;
    constexpr float kNormalThickness = 0.09f;
    constexpr float kNormalTipBlockSize = 0.13f;
    constexpr float kContactMarkerScale = 0.09f;

    // OBJECT A
    std::vector<glm::vec2> vA = GenerateRandomConvexPolygon(10);
    bsk::Mesh* meshA = meshFromCCWPolygon(vA);
    bsk::Node2D* nodeA = new bsk::Node2D(scene, meshA, blue, glm::vec2(0.0f, 0.0f), 0.0f, glm::vec2(1.5f, 2.5f));

    // OBJECT B
    std::vector<glm::vec2> vB = GenerateRandomConvexPolygon(5);
    bsk::Mesh* meshB = meshFromCCWPolygon(vB);
    bsk::Node2D* nodeB = new bsk::Node2D(scene, meshB, red, glm::vec2(1.0f, 1.0f), 5.0f, glm::vec2(1.5f, 2.5f));

    bsk::Node2D* normalNode = new bsk::Node2D(scene, nullptr, green, glm::vec2(0.0f), 0.0f, glm::vec2(0.0f));
    bsk::Node2D* normalTipNode = new bsk::Node2D(scene, nullptr, white, glm::vec2(0.0f), 0.0f, glm::vec2(0.0f));

    bsk::Node2D* contactA1 = new bsk::Node2D(scene, nullptr, yellow, glm::vec2(0.0f), 0.0f, glm::vec2(0.0f));
    bsk::Node2D* contactA2 = new bsk::Node2D(scene, nullptr, cyan, glm::vec2(0.0f), 0.0f, glm::vec2(0.0f));
    bsk::Node2D* contactB1 = new bsk::Node2D(scene, nullptr, magenta, glm::vec2(0.0f), 0.0f, glm::vec2(0.0f));
    bsk::Node2D* contactB2 = new bsk::Node2D(scene, nullptr, white, glm::vec2(0.0f), 0.0f, glm::vec2(0.0f));

    normalNode->setLayer(0.85);
    normalTipNode->setLayer(0.86);
    contactA1->setLayer(0.9);
    contactA2->setLayer(0.9);
    contactB1->setLayer(0.9);
    contactB2->setLayer(0.9);

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
            glm::vec4 w = modelA * glm::vec4(vertex, 0.0f, 1.0f);
            wA.emplace_back(w.x, w.y);
        }
        std::vector<glm::vec2> wB;
        glm::mat4 modelB = nodeB->getModel();
        for (const auto& vertex : vB) {
            glm::vec4 w = modelB * glm::vec4(vertex, 0.0f, 1.0f);
            wB.emplace_back(w.x, w.y);
        }

        const glm::vec2 geoCenterA = geomCentroidWorld(wA);
        const glm::vec2 geoCenterB = geomCentroidWorld(wB);

        // compute collision
        bsk::internal::CollisionPair pair;
        bsk::internal::ConvexShape shapeA(wA, geoCenterA, 0.0f, glm::vec2(1.0f, 1.0f));
        bsk::internal::ConvexShape shapeB(wB, geoCenterB, 0.0f, glm::vec2(1.0f, 1.0f));
        bool collided = bsk::internal::gjk(shapeA, shapeB, pair);
        bool epaOk = false;
        bool satOk = false;
        if (collided) {
            bsk::internal::sat(shapeA.vertices, shapeB.vertices, pair);
            epaOk = true;
            pair.normal = -pair.normal;
            // if (epaOk && glm::length2(pair.normal) > 1e-12f) {
            //     satOk = bsk::internal::sat(shapeA, shapeB, pair);
            // }
            bsk::internal::findContactPoints(shapeA.vertices, shapeB.vertices, pair);
            satOk = true;
            std::cout << pair.numA << " " << pair.numB << std::endl;
            nodeA->setMaterial(red);
            nodeB->setMaterial(red);
        } else {
            nodeA->setMaterial(blue);
            nodeB->setMaterial(blue);
        }

        if (collided && epaOk && glm::length2(pair.normal) > 1e-12f && !wA.empty()) {
            glm::vec2 n = glm::normalize(pair.normal);
            glm::vec2 half = n * (0.5f * kNormalDrawLength);
            glm::vec2 tip = geoCenterA + half;
            SegmentSquareTransform xf = segmentSquareTransformFromAToB(geoCenterA - half, tip, kNormalThickness);
            normalNode->setPosition(worldXYToNode2DStore(xf.positionWorldMid));
            normalNode->setRotation(xf.rotation);
            normalNode->setScale(xf.scale);

            normalTipNode->setPosition(worldXYToNode2DStore(tip + n * (0.5f * kNormalTipBlockSize)));
            normalTipNode->setRotation(xf.rotation);
            normalTipNode->setScale(glm::vec2(kNormalTipBlockSize));
        } else {
            normalNode->setScale(glm::vec2(0.0f));
            normalTipNode->setScale(glm::vec2(0.0f));
        }

        auto placeContact = [&](bsk::Node2D* node, const glm::vec2& worldXY, bool visible) {
            if (visible) {
                node->setPosition(worldXYToNode2DStore(worldXY));
                node->setRotation(0.0f);
                node->setScale(glm::vec2(kContactMarkerScale));
            } else {
                node->setScale(glm::vec2(0.0f));
            }
        };

        if (collided && satOk) {
            placeContact(contactA1, pair.a1, pair.numA >= 1);
            placeContact(contactA2, pair.a2, pair.numA >= 2);
            placeContact(contactB1, pair.b1, pair.numB >= 1);
            placeContact(contactB2, pair.b2, pair.numB >= 2);
        } else {
            contactA1->setScale(glm::vec2(0.0f));
            contactA2->setScale(glm::vec2(0.0f));
            contactB1->setScale(glm::vec2(0.0f));
            contactB2->setScale(glm::vec2(0.0f));
        }

        scene->render();
        engine->render();
    }

    return 0;
}
