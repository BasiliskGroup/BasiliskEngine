#include <basilisk/basilisk.h>

int main() {
    bsk::Engine* engine = new bsk::Engine(800, 800, "Mesh Test");
    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    bsk::Mesh* mesh = new bsk::Mesh(
        std::vector<float>{
            2, 2, 0, 0, 0,
            -1, 1, 0, 0, 0,
            -1, -1, 0, 0, 0,
            1, -1, 0, 0, 0,
        },
        std::vector<unsigned int>{
            0, 1, 2,
            2, 3, 0
        }
    );
    bsk::Node2D* node = new bsk::Node2D(mesh, nullptr);
    scene->add(node);

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    return 0;
}