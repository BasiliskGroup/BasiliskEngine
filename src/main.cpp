/**
 * C++ equivalent of build/test.py: single quad with textures/main.png.
 * Run the executable from the project root so "textures/main.png" resolves.
 */
#include <basilisk/basilisk.h>

int main() {
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    bsk::Image* image = new bsk::Image("textures/main.png");
    bsk::Material* material = new bsk::Material(glm::vec3(1.0f, 1.0f, 1.0f), image);
    bsk::Node2D* node = new bsk::Node2D(scene, nullptr, material, glm::vec2(0.0f, 0.0f));
    scene->add(node);

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    delete node;
    delete material;
    delete image;
    delete scene;
    delete engine;
    return 0;
}
