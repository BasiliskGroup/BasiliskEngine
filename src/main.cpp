#include <basilisk/basilisk.h>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* image = new bsk::Image("textures/container.jpg");

    // // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    // normal nodes
    bsk::Node2D* square = new bsk::Node2D(scene, { .mesh=quad, .material=material });

    // Main loop continues as long as the window is open
    int i = 0;
    while (engine->isRunning()) {
        engine->update();

        scene->update();
        scene->render();

        std::cout << engine->getMouse()->getWorldX(scene->getCamera()) << ", " << engine->getMouse()->getWorldY(scene->getCamera()) << std::endl;

        engine->render();

        i++;
    }

    // Free memory allocations
    delete image;
    delete material;
    delete quad;
    delete scene;
    delete engine;
}