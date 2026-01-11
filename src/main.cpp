#include <basilisk/basilisk.h>
#include <iostream>


int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene* scene = new bsk::Scene(engine);

    // Load assets from file
    bsk::Mesh* cube = new bsk::Mesh("models/cube.obj");
    bsk::Image* image = new bsk::Image("textures/container.jpg");

    // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    bsk::Node* node = new bsk::Node(scene, {.mesh=cube, .material=material});

    bool i = true;
    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    // Free memory allocations
    delete image;
    delete material;
    delete cube;
    delete scene;
    delete engine;
}