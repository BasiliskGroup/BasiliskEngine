#include <basilisk/basilisk.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/util/random.h>
#include <basilisk/util/maths.h>
#include <basilisk/physics/collision/bvh.h>
#include <cmath>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* metalImage = new bsk::Image("textures/metal.png");
    bsk::Image* ropeImage = new bsk::Image("textures/rope.png");
    bsk::Image* bricksImage = new bsk::Image("textures/bricks.jpg");
    bsk::Image* containerImage = new bsk::Image("textures/container.jpg");

    // Create materials from images
    bsk::Material* metalMaterial = new bsk::Material({1, 1, 1}, metalImage);
    bsk::Material* ropeMaterial = new bsk::Material({1, 1, 1}, ropeImage);
    bsk::Material* bricksMaterial = new bsk::Material({1, 1, 1}, bricksImage);
    bsk::Material* containerMaterial = new bsk::Material({1, 1, 1}, containerImage);

    // Create a box collider (unit box vertices) - can be shared by all box-shaped objects
    bsk::Collider* boxCollider = new bsk::Collider(scene->getSolver(), {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}});

    // Create a 50x50 grid of quads with some missing
    const int gridSize = 25;
    const float spacing = 2.0f;  // Space between quads
    const float quadSize = 0.8f;  // Size of each quad
    const float missingProbability = 0.50f;

    scene->getCamera()->setScale(gridSize * 2.0f);  // Zoom out to see the full grid
    
    // Calculate grid offset to center it
    float gridOffset = -(gridSize - 1) * spacing * 0.5f;
    
    std::vector<bsk::Node2D*> gridNodes;
    gridNodes.reserve(gridSize * gridSize);
    
    for (int row = 0; row < gridSize; row++) {
        for (int col = 0; col < gridSize; col++) {
            // Randomly skip some quads
            if (bsk::internal::uniform() < missingProbability) {
                continue;
            }
            
            float x = gridOffset + col * spacing;
            float y = gridOffset + row * spacing;
            
            bsk::Node2D* node = new bsk::Node2D(scene, {
                .mesh=quad,
                .material=bricksMaterial,
                .position={x, y},
                .rotation=0.0f,
                .scale={quadSize, quadSize},
                .collider=boxCollider,
                .density=1.0e3f
            });
            gridNodes.push_back(node);
        }
    }

    // Create a simple material for AABB visualization (wireframe-like)
    bsk::Material* aabbMaterial = metalMaterial;  // Green color, no texture
    const float aabbLineWidth = 0.05f;  // Width of AABB lines
    
    // Boolean flag to toggle BVH visualization
    bool showBVH = true;
    
    // Main loop continues as long as the window is open
    std::vector<bsk::Node2D*> aabbNodes;
    while (engine->isRunning()) {
        // Delete and clear all previous AABB visualizations before rebuilding
        for (bsk::Node2D* node : aabbNodes) {
            if (node != nullptr) {
                delete node;
            }
        }
        aabbNodes.clear();
        
        // Get all primatives from BVH and visualize their AABBs
        if (showBVH) {
            std::vector<bsk::internal::PrimativeInfo> primatives = scene->getSolver()->getBodyTable()->getBVH()->getAllPrimatives();
            for (const auto& info : primatives) {
            // Draw 4 edges of the AABB using connectSquare
            // Bottom edge: from (bl.x, bl.y) to (tr.x, bl.y)
            auto [bottomPos, bottomScale] = bsk::internal::connectSquare(
                glm::vec2(info.bl.x, info.bl.y),
                glm::vec2(info.tr.x, info.bl.y),
                aabbLineWidth
            );
            bsk::Node2D* bottomNode = new bsk::Node2D(scene, {
                .mesh=quad,
                .material=aabbMaterial,
                .position={bottomPos.x, bottomPos.y},
                .rotation=bottomPos.z,
                .scale=bottomScale
            });
            bottomNode->setLayer(0.95f);
            aabbNodes.push_back(bottomNode);
            
            // Right edge: from (tr.x, bl.y) to (tr.x, tr.y)
            auto [rightPos, rightScale] = bsk::internal::connectSquare(
                glm::vec2(info.tr.x, info.bl.y),
                glm::vec2(info.tr.x, info.tr.y),
                aabbLineWidth
            );
            bsk::Node2D* rightNode = new bsk::Node2D(scene, {
                .mesh=quad,
                .material=aabbMaterial,
                .position={rightPos.x, rightPos.y},
                .rotation=rightPos.z,
                .scale=rightScale
            });
            rightNode->setLayer(0.95f);
            aabbNodes.push_back(rightNode);
            
            // Top edge: from (tr.x, tr.y) to (bl.x, tr.y)
            auto [topPos, topScale] = bsk::internal::connectSquare(
                glm::vec2(info.tr.x, info.tr.y),
                glm::vec2(info.bl.x, info.tr.y),
                aabbLineWidth
            );
            bsk::Node2D* topNode = new bsk::Node2D(scene, {
                .mesh=quad,
                .material=aabbMaterial,
                .position={topPos.x, topPos.y},
                .rotation=topPos.z,
                .scale=topScale
            });
            topNode->setLayer(0.95f);
            aabbNodes.push_back(topNode);
            
            // Left edge: from (bl.x, tr.y) to (bl.x, bl.y)
            auto [leftPos, leftScale] = bsk::internal::connectSquare(
                glm::vec2(info.bl.x, info.tr.y),
                glm::vec2(info.bl.x, info.bl.y),
                aabbLineWidth
            );
            bsk::Node2D* leftNode = new bsk::Node2D(scene, {
                .mesh=quad,
                .material=aabbMaterial,
                .position={leftPos.x, leftPos.y},
                .rotation=leftPos.z,
                .scale=leftScale
            });
            leftNode->setLayer(0.95f);
            aabbNodes.push_back(leftNode);
            }
        }
        
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    // Free memory allocations
    delete metalImage;
    delete ropeImage;
    delete bricksImage;
    delete containerImage;
    delete metalMaterial;
    delete ropeMaterial;
    delete bricksMaterial;
    delete containerMaterial;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}