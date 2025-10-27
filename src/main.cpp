#include "basilisk.h"

#include "resource/textureServer.h"

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene* scene = new Scene(engine);

    // Data for making node
    Mesh* cube = new Mesh("models/cube.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
        
    Node* parent = new Node(scene, scene->getShader(), cube, texture);
    Node* node = new Node(parent, scene->getShader(), cube, texture);
    new Node(node, scene->getShader(), cube, texture);

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