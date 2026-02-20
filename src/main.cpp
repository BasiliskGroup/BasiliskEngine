#include <basilisk/basilisk.h>
#include <basilisk/physics/rigid.h>
#include <cassert>
#include <iostream>
#include <stdexcept>


using namespace bsk;

int main() {
    Engine* engine = new Engine();
    Scene* scene = new Scene(engine);
    Node* node = new Node(scene);

    Shader* shader = new Shader("shaders/frame.vert", "shaders/post.frag");
    Frame* frame = new Frame(engine, shader, 160, 160);
    frame->setFilterNearest();

    while (engine->isRunning()) {
        engine->update();
        scene->update();       

        frame->clear();
        frame->use();
        scene->render();

        engine->getFrame()->use();
        frame->render(0, 0, engine->getWindow()->getHeight(), engine->getWindow()->getHeight());

        engine->render();
    }

    delete engine;
    delete scene;

    return 0;
}