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

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    // Load assets
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* images[] = {
        new bsk::Image("textures/metal.png"),
        new bsk::Image("textures/rope.png"),
        new bsk::Image("textures/bricks.jpg"),
        new bsk::Image("textures/container.jpg")
    };
    bsk::Material* metalMaterial = new bsk::Material({1, 1, 1}, images[0]);
    bsk::Material* ropeMaterial = new bsk::Material({1, 1, 1}, images[1]);
    bsk::Material* bricksMaterial = new bsk::Material({1, 1, 1}, images[2]);
    bsk::Material* containerMaterial = new bsk::Material({1, 1, 1}, images[3]);

    // Create a box collider (unit box vertices) - can be shared by all box-shaped objects
    bsk::Collider* boxCollider = new bsk::Collider(scene->getSolver(), {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}});

    // Create a 50x50 grid of quads with some missing
    const int gridSize = 20;
    const float spacing = 2.0f;  // Space between quads
    const float quadSize = 0.8f;  // Size of each quad
    const float missingProbability = 0.0f;

    scene->getCamera()->setScale(gridSize * 3.0f);  // Zoom out to see the full grid
    
    // Increase camera speed by 5x (default is 3.0, so set to 15.0)
    if (auto* camera2D = dynamic_cast<bsk::Camera2D*>(scene->getCamera())) {
        camera2D->setSpeed(15.0f);
    }
    
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
                .scale={quadSize * bsk::internal::uniform(1.0f, 2.0f), quadSize * bsk::internal::uniform(1.0f, 2.0f)},
                .collider=boxCollider,
                .density=1.0e1f,
                .velocity=10.0f * glm::vec3{bsk::internal::uniform(-1.0f, 1.0f), bsk::internal::uniform(-1.0f, 1.0f), 0.0f}
            });
            gridNodes.push_back(node);
        }
    }
    
    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    // Cleanup
    for (auto* img : images) delete img;
    delete metalMaterial;
    delete ropeMaterial;
    delete bricksMaterial;
    delete containerMaterial;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}