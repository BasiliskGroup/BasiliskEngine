#include <basilisk/basilisk.h>
#include <basilisk/physics/rigid.h>
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace bsk;

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene* scene = new Scene(engine);
    scene->addDefaults(true, true, false);

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    delete engine;
    delete scene;

    return 0;
}