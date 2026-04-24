#include <basilisk/basilisk.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/util/maths.h>

#include <earcut.hpp>
#include <span>
#include <random>
#include <algorithm>
#include <cmath>
#include <vector>

// ---- Convex polygon helpers (adapted from collision_test.cpp) ----

static std::vector<glm::vec2> GenerateRandomConvexPolygon(int n) {
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

    float minX = xs.front(), maxX = xs.back();
    float minY = ys.front(), maxY = ys.back();

    std::vector<float> xVec, yVec;

    float lastTop = minX, lastBot = minX;
    for (int i = 1; i < n - 1; ++i) {
        float x = xs[i];
        if (dist(gen) < 0.5f) { xVec.push_back(x - lastTop); lastTop = x; }
        else                   { xVec.push_back(lastBot - x); lastBot = x; }
    }
    xVec.push_back(maxX - lastTop);
    xVec.push_back(lastBot - maxX);

    float lastLeft = minY, lastRight = minY;
    for (int i = 1; i < n - 1; ++i) {
        float y = ys[i];
        if (dist(gen) < 0.5f) { yVec.push_back(y - lastLeft);  lastLeft  = y; }
        else                   { yVec.push_back(lastRight - y); lastRight = y; }
    }
    yVec.push_back(maxY - lastLeft);
    yVec.push_back(lastRight - maxY);

    std::shuffle(yVec.begin(), yVec.end(), gen);

    std::vector<glm::vec2> vecs;
    vecs.reserve(n);
    for (int i = 0; i < n; ++i)
        vecs.emplace_back(xVec[i], yVec[i]);

    std::sort(vecs.begin(), vecs.end(), [](const glm::vec2& a, const glm::vec2& b) {
        return std::atan2(a.y, a.x) < std::atan2(b.y, b.x);
    });

    std::vector<glm::vec2> points;
    points.reserve(n);
    glm::vec2 current(0.0f);
    for (const auto& v : vecs) {
        points.push_back(current);
        current += v;
    }

    // Centre on origin so the collider and mesh share the same local origin.
    glm::vec2 centroid(0.0f);
    for (const auto& p : points) centroid += p;
    centroid /= static_cast<float>(points.size());
    for (auto& p : points) p -= centroid;

    return points;
}

static bsk::Mesh* meshFromCCWPolygon(std::span<const glm::vec2> ccwVerts) {
    std::vector<float> vertices;
    vertices.reserve(ccwVerts.size() * 5);
    for (const glm::vec2& v : ccwVerts) {
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
    }

    std::vector<std::vector<std::array<double, 2>>> polygon(1);
    polygon[0].reserve(ccwVerts.size());
    for (const glm::vec2& p : ccwVerts)
        polygon[0].push_back({ static_cast<double>(p.x), static_cast<double>(p.y) });

    std::vector<uint32_t> indices =
        ccwVerts.size() < 3 ? std::vector<uint32_t>{} : mapbox::earcut<uint32_t>(polygon);

    return new bsk::Mesh(vertices, indices);
}

static bsk::Mesh* meshFromCCWPolygon(const std::vector<glm::vec2>& v) {
    return meshFromCCWPolygon(std::span<const glm::vec2>(v.data(), v.size()));
}

// ---- Spawn helper ----

struct PolyNode {
    bsk::Node2D*    node     = nullptr;
    bsk::Collider*  collider = nullptr;
    bsk::Mesh*      mesh     = nullptr;
};

static PolyNode spawnPolyNode(bsk::Scene2D* scene, glm::vec2 pos) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> sideDist(4, 10);

    std::vector<glm::vec2> verts = GenerateRandomConvexPolygon(sideDist(rng));

    bsk::Collider* collider = new bsk::Collider(verts);
    for (auto& v : verts) {
        v.y *= -1.0f;
    }
    bsk::Mesh*     mesh     = meshFromCCWPolygon(verts);

    bsk::Node2D* node = new bsk::Node2D(
        scene, mesh, nullptr,
        pos, 0.0f, glm::vec2(1.0f, 1.0f),
        glm::vec3(0.0f), collider);

    return { node, collider, mesh };
}

int main() {
    bsk::Engine*  engine = new bsk::Engine(800, 800, "Basilisk", false);
    bsk::Scene2D* scene  = new bsk::Scene2D(engine);

    bsk::Material* material = new bsk::Material(glm::vec3(1.0f, 0.0f, 0.0f));

    // Floor — plain box, no mesh needed.
    bsk::Collider* floorCollider = new bsk::Collider({
        glm::vec2( 0.5f,  0.5f),
        glm::vec2(-0.5f,  0.5f),
        glm::vec2(-0.5f, -0.5f),
        glm::vec2( 0.5f, -0.5f)
    });
    new bsk::Node2D(scene, nullptr, nullptr,
        glm::vec2(0.0f, -4.0f), 0.0f, glm::vec2(20.0f, 1.0f),
        glm::vec3(0.0f), floorCollider, -1.0f);

    bsk::Joint*    drag               = nullptr;
    bsk::Manifold* rightClickManifold = nullptr;

    std::vector<PolyNode>    spawnedNodes;
    std::vector<bsk::Node2D*> contactMarkers;

    auto manifoldIsAlive = [&](bsk::Manifold* manifold) -> bool {
        if (!manifold) return false;
        for (bsk::Force* f = scene->getSolver()->getForces(); f; f = f->getNext())
            if (f == manifold) return true;
        return false;
    };

    while (engine->isRunning()) {
        engine->update();

        bsk::Mouse*          mouse    = engine->getMouse();
        bsk::Keyboard*       keyboard = engine->getKeyboard();
        bsk::StaticCamera2D* camera   = scene->getCamera();

        glm::vec2 mousePos = {
            static_cast<float>(mouse->getWorldX(camera)),
            static_cast<float>(mouse->getWorldY(camera))
        };

        // --- Drag ---
        if (mouse->getLeftDown()) {
            if (!drag) {
                glm::vec2 local;
                bsk::Rigid* picked = scene->getSolver()->pick(mousePos, local);
                if (picked) {
                    drag = new bsk::Joint(
                        scene->getSolver(),
                        nullptr, picked,
                        mousePos, local,
                        glm::vec3(1000.0f, 1000.0f, 0.0f)
                    );
                }
            } else {
                drag->setRA(mousePos);
            }
        } else if (mouse->getLeftReleased() && drag) {
            delete drag;
            drag = nullptr;
        }

        // --- Spawn / Delete ---
        if (keyboard->getPressed(bsk::Key::K_SPACE)) {
            bsk::Node2D* hovered = scene->pick(mousePos);
            if (hovered) {
                // Delete the hovered node and its resources.
                if (drag && drag->getBodyB() == hovered->getRigid()) {
                    delete drag;
                    drag = nullptr;
                }
                auto it = std::find_if(spawnedNodes.begin(), spawnedNodes.end(),
                    [&](const PolyNode& pn) { return pn.node == hovered; });
                if (it != spawnedNodes.end()) {
                    delete it->collider;
                    delete it->mesh;
                    spawnedNodes.erase(it);
                }
                delete hovered;
            } else {
                spawnedNodes.push_back(spawnPolyNode(scene, mousePos));
            }
        }

        // --- Static test manifold (right-click) ---
        if (mouse->getRightClicked()) {
            if (manifoldIsAlive(rightClickManifold))
                delete rightClickManifold;
            rightClickManifold = nullptr;

            glm::vec2 local;
            bsk::Rigid* picked = scene->getSolver()->pick(mousePos, local);
            if (picked) {
                std::vector<glm::vec2> worldBox = {
                    mousePos + glm::vec2( 0.5f,  0.5f),
                    mousePos + glm::vec2(-0.5f,  0.5f),
                    mousePos + glm::vec2(-0.5f, -0.5f),
                    mousePos + glm::vec2( 0.5f, -0.5f)
                };
                rightClickManifold = new bsk::Manifold(scene->getSolver(), picked, worldBox);
            }
        }
        if (mouse->getRightReleased() && manifoldIsAlive(rightClickManifold)) {
            delete rightClickManifold;
            rightClickManifold = nullptr;
        }

        scene->update();
        if (!manifoldIsAlive(rightClickManifold))
            rightClickManifold = nullptr;

        // Rebuild contact markers every frame.
        for (bsk::Node2D* marker : contactMarkers) delete marker;
        contactMarkers.clear();

        for (bsk::Force* force = scene->getSolver()->getForces(); force; force = force->getNext()) {
            bsk::Manifold* manifold = dynamic_cast<bsk::Manifold*>(force);
            if (!manifold) continue;

            bsk::Rigid* bodyA = manifold->getBodyA();
            bsk::Rigid* bodyB = manifold->getBodyB();
            if (!bodyA) continue;

            const int numContacts = manifold->getNumContacts();
            for (int i = 0; i < numContacts; ++i) {
                const auto& c = manifold->getContact(i);
                glm::vec2 worldA = bsk::internal::transform(bodyA->getPosition(), c.rA);
                glm::vec2 worldB = bodyB
                    ? bsk::internal::transform(bodyB->getPosition(), c.rB)
                    : c.rB;

                auto makeMarker = [&](glm::vec2 pos) {
                    bsk::Node2D* m = new bsk::Node2D(
                        scene, nullptr, material,
                        pos, 0.0f, glm::vec2(0.08f, 0.08f),
                        glm::vec3(0.0f), nullptr);
                    m->setLayer(0.9f);
                    contactMarkers.push_back(m);
                };
                makeMarker(worldA);
                makeMarker(worldB);
            }
        }

        scene->render();
        engine->render();
    }

    if (manifoldIsAlive(rightClickManifold)) delete rightClickManifold;
    for (bsk::Node2D* marker : contactMarkers) delete marker;
    for (PolyNode& pn : spawnedNodes) {
        delete pn.collider;
        delete pn.mesh;
    }
    delete floorCollider;
    delete material;
    delete scene;
    delete engine;
    return 0;
}