#include <basilisk/basilisk.h>

#include "solver/physics.h"
#include "util/random.h"
#include "util/time.h"
#include "forces/force.h"
#include "tables/forceTable.h"
#include "tables/manifoldTable.h"
#include "shapes/rigid.h"

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

    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene2D* scene = new Scene2D(engine);

    // Data for making node
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/container.jpg");
    Image* image2 = new Image("textures/floor_albedo.png");
    Material* material = new Material({1.0, 1.0, 0.0}, image);
    Material* material2 = new Material({1.0, 1.0, 0.0}, image2);

    Node2D* square = new Node2D(scene, { .mesh=quad, .material=material});
    Node2D* square2 = new Node2D(scene, { .mesh=quad, .material=material2, .position={3, 4}});

    // TODO: Move these things to be automatic
    // This needs to be moved into node render call
    scene->getShader()->setUniform("uMaterialID", (int)scene->getEngine()->getResourceServer()->getMaterialServer()->get(material2));

    // TODO: Figure out why needs to be rewritten 
    engine->getResourceServer()->write(scene->getShader(), "textureArrays", "materials");
    
    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();
        
        scene->update(1.0 / 300);
        scene->render();

        engine->render();
    }

    delete material;
    delete material2;
    delete image;
    delete image2;
    delete square;
    delete square2;
    delete quad;
    delete scene;
    delete engine;
}