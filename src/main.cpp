/**
 * @file main.cpp
 * @author Jonah Coffelt
 * @brief ...
 * @version 0.1
 * @date 2025-10-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include "basilisk.h"


int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene* scene = new Scene(engine);

    // Data for making node
    Mesh* cube = new Mesh("models/cube.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
        
    Node* node = new Node(scene->getShader(), cube, texture);
    scene->add(node);

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene->update();
        scene->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete node;
    delete cube;
    delete scene;
    delete engine;
}