#include <basilisk/basilisk.h>

#include <basilisk/solver/physics.h>
#include <basilisk/util/random.h>
#include <basilisk/util/time.h>
#include <basilisk/forces/force.h>
#include <basilisk/tables/forceTable.h>
#include <basilisk/tables/manifoldTable.h>
#include <basilisk/shapes/rigid.h>

using namespace bsk;

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene2D* scene2D = new Scene2D(engine);

    // Data for making node
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/man.png");
    Image* containerImage = new Image("textures/container.jpg");
    Material* material1 = new Material({1.0, 1.0, 0.0}, image);
    Material* material2 = new Material({1.0, 1.0, 0.0}, containerImage);

    // Use Static Cam
    StaticCamera2D* camera = new StaticCamera2D(engine);
    scene2D->setCamera(camera);

    // create "paper"
    std::vector<glm::vec2> vertices = {
        {100, 100}, {400, 100}, {400, 300}, {100, 300},
        {200, 200}, {300, 200}, {300, 250}, {200, 250},
        {200, 125}, {300, 125}, {275, 150}
    };

    std::vector<int> rings = { 4, 8, 11 };

    for (glm::vec2& vertex : vertices) {
        vertex /= 50;
        new Node2D(scene2D, { .mesh=quad, .material=material1, .position=vertex, .scale={ 0.1, 0.1 } });
    }
        
    // normal nodes
    Node2D* square = new Node2D(scene2D, { .mesh=quad, .material=material2 });

    // TODO: Figure out why needs to be rewritten 
    engine->getResourceServer()->write(scene2D->getShader(), "textureArrays", "materials");

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene2D->update(1 / 60);
        scene2D->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete containerImage;
    delete camera;
    delete material1;
    delete material2;
    delete quad;
    delete scene2D;
    delete engine;
}