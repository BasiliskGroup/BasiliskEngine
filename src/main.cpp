#include <basilisk/basilisk.h>
#include <iostream>


int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    scene->getCamera()->setScale(8.0);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* image = new bsk::Image("textures/knight.png");

    // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    bsk::Node2D* node = new bsk::Node2D(scene, {.mesh=quad, .material=material});

    bsk::Frame* frame = new bsk::Frame(engine, 800, 800);
    bsk::Frame* frame2 = new bsk::Frame(engine, 800, 800);

    bool i = true;
    while (engine->isRunning()) {
        engine->update();

        glm::vec2 pos = node->getPosition();
        node->setPosition({pos.x + 0.01, 0.0});
        
        frame->use();
        frame->clear(1.0, 0.0, 0.0, 1.0);   
        scene->update();
        scene->render();
        
        engine->getFrame()->use();
        frame->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete material;
    delete quad;
    delete scene;
    delete engine;
}