#include <basilisk/basilisk.h>
#include <cmath>

using namespace bsk;

int main() {
    Engine* engine = new Engine(1200, 800);
    Scene2D* scene = new Scene2D(engine);

    Material* material = new Material({1.0f, 0.0f, 1.0f}, engine->getResourceServer()->logoImage);
    Node2D* node = new Node2D(scene, nullptr, material);

    float t = 0.0f;

    while (engine->isRunning()) {
        engine->update();
        scene->update();


        t += engine->getDeltaTime();
        material->setColor({sin(t), cos(t), 0.5f});


        scene->render();
        engine->render();
    }

    delete scene;
    delete engine;

    return 0;
}
