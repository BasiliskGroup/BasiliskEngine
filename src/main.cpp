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

    // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    // collider
    bsk::Collider* quadCollider = new bsk::Collider(scene->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}});

    // normal nodes
    bsk::Node2D* square = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, 1}, .collider=quadCollider });
    bsk::Node2D* floor = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, -3}, .scale={3, 1},  .collider=quadCollider, .density=-1 });

    // bsk::Node2D* goofyGuy = new bsk::Node2D(voidScene, { .mesh=quad, .material=material, .position={3, 3} });
    // // goofyGuy->setScene(scene);
    // square->add(goofyGuy);

    // Main loop continues as long as the window is open
    int i = 0;
    while (engine->isRunning()) {

        // std::cout << square->getChildren().size() << std::endl;

        engine->update();

        scene->update();
        scene->render();

        engine->render();

        // if (square->getPosition().y < -2.1) break;

        i++;
    }

    // Free memory allocations
    delete image;
    delete material;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}