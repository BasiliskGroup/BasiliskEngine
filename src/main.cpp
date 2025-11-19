#include <basilisk/basilisk.h>
#include <iostream>


int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* image = new bsk::Image("textures/container.jpg");
    bsk::Image* image2 = new bsk::Image("textures/piss.png");
    bsk::Image* image3 = new bsk::Image("textures/man.png");

    // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);
    bsk::Material* material2 = new bsk::Material({1, 1, 1}, image2);
    bsk::Material* material3 = new bsk::Material({1, 1, 1}, image3);

    // collider
    bsk::Collider* quadCollider = new bsk::Collider(scene->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}});

    // normal nodes
    
    bsk::Node2D* floor = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, -3}, .rotation=0, .scale={9, 1},  .collider=quadCollider, .density=-1 });

    for (int i = 0; i < 10; i++)
        bsk::Node2D* square = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, i * 2}, .scale={1, 1}, .rotation=1.2, .collider=quadCollider });

    std::vector<bsk::Node2D*> contactNodes;

    // Main loop continues as long as the window is open
    int i = 0;
    while (engine->isRunning()) {

        // std::cout << square->getChildren().size() << std::endl;

        for (bsk::Force* force = scene->getSolver()->getForces(); force != nullptr; force = force->getNext()) {
            bsk::Manifold* manifold = (bsk::Manifold*) force;

            for (int i = 0; i < 2; i++) {
                glm::vec2 rAW = manifold->getBodyA()->getRMat() * manifold->getRA()[i];
                glm::vec2 rBW = manifold->getBodyB()->getRMat() * manifold->getRB()[i];

                // glm::vec2 rAW = manifold->getRAW()[i];
                // glm::vec2 rBW = manifold->getRBW()[i];

                rAW += glm::vec2(manifold->getBodyA()->getPos());
                rBW += glm::vec2(manifold->getBodyB()->getPos());

                bsk::Node2D* nodeA = new bsk::Node2D(scene, { .mesh=quad, .material=material2, .position=rAW, .scale={0.125, 0.125} });
                nodeA->setLayer(0.9);
                contactNodes.push_back(nodeA);

                bsk::Node2D* nodeB = new bsk::Node2D(scene, { .mesh=quad, .material=material3, .position=rBW, .scale={0.125, 0.125} });
                nodeB->setLayer(0.9);
                contactNodes.push_back(nodeB);
            }
        }

        engine->update();

        scene->update();
        scene->render();

        engine->render();

        for (bsk::Node2D* node : contactNodes) {
            delete node;
        }
        contactNodes.clear();

        // if (square->getPosition().y < -2.1) break;

        i++;
    }

    // Free memory allocations
    delete image;
    delete image2;
    delete image3;
    delete material;
    delete material2;
    delete material3;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}