#include <basilisk/basilisk.h>

#include "solver/physics.h"
#include "util/random.h"
#include "util/time.h"

int main() {
    std::vector<float> quadData {
        // Triangle 1
        -0.8f, -0.8f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,

        // Triangle 2
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
    };

    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene2D* scene2D = new Scene2D(engine);

    // Data for making node
    // Mesh* quad = new Mesh("models/quad.obj");
    Mesh* quad = new Mesh(quadData);
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
        
    // physics
    Collider* squareCollider = new Collider(scene2D->getSolver(), {{-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}});

    Node2D* square = new Node2D(scene2D, { .mesh=quad, .texture=texture, .collider=squareCollider, .density=1 });
    new Node2D(square, { .mesh=quad, .texture=texture, .position={1, 1} });
    Node2D* clone = new Node2D(*square);
    clone->setPosition({3, 4});

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene2D->update(1.0 / 60.0);
        scene2D->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete quad;
    delete scene2D;
    delete engine;
}