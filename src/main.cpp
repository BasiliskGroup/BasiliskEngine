#include <basilisk/basilisk.h>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene* scene = new bsk::Scene(engine);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/cube.obj");
    bsk::Image* image = new bsk::Image("textures/container.jpg");

    // // Create a material from image
    // bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    // normal nodes
    bsk::Node* square = new bsk::Node(scene, { .mesh=quad, .material=nullptr });

    // Main loop continues as long as the window is open
    int i = 0;
    while (engine->isRunning()) {
        engine->update();

        scene->update();
        scene->render();

        engine->render();

        i++;
    }

    // Free memory allocations
    // delete image;
    // delete material;
    // delete quad;
    // delete scene;
    // delete engine;
}