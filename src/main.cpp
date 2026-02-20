#include <basilisk/basilisk.h>
#include <basilisk/physics/rigid.h>
#include <cassert>
#include <iostream>
#include <stdexcept>


using namespace bsk;

int main() {
    Engine* engine = new Engine(1200, 800);
    Scene* scene = new Scene(engine);
    Node* node = new Node(scene);


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