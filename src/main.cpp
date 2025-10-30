#include <basilisk/basilisk.h>

#include "solver/physics.h"
#include "util/random.h"
#include "util/time.h"

int main() {
    // Solver* solver = new Solver();
    

    // float dx = 5;
    // float dr = 2 * 3.14;

    // // create a list of rigids
    // std::vector<Rigid*> objects;
    // for (int i = 0; i < 100; i++) {
    //     objects.push_back(new Rigid(solver, {uniform(-dx, dx), uniform(-dx, dx), uniform(0, dr)}, {1, 1}, 1, 0.4, {0, 0, 0}, cubeCollider));
    // }

    // for (int i = 0; i < 10; i++) {
    //     solver->step(1.0 / 60.0);

    //     // testing mid simulation body deletion
    //     int deleteIndex = randrange(0, objects.size());
    //     delete objects[deleteIndex];
    //     objects.erase(objects.begin() + deleteIndex);
    // }

    // // delete collider
    // delete cubeCollider;

    // // deleting solver should always be last
    // delete solver;
    // return 0;
    
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

    Node2D* square = new Node2D(scene2D, { .mesh=quad, .texture=texture, .collider=squareCollider });
    new Node2D(square, { .mesh=quad, .texture=texture, .position={1, 1} });
    Node2D* clone = new Node2D(*square);
    clone->setPosition({3, 4});

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene2D->update();
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