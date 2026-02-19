#include <basilisk/basilisk.h>
#include <basilisk/physics/rigid.h>
#include <cassert>
#include <iostream>
#include <stdexcept>


using namespace bsk;

int main() {
    Engine* engine = new Engine();
    Scene* scene = new Scene(engine);

    // Add simple node tree
    Node* node = new Node(scene);
    Node* node2 = new Node(node, nullptr, nullptr, {3.0, 2.0, 0.0});
    Node* node3 = new Node(node2, nullptr, nullptr, {3.0, 2.0, 0.0});

    while (engine->isRunning()) {
        engine->update();
        scene->update();

        // Move the base node a little each frame
        glm::vec3 position = node->getPosition();
        position = position + glm::vec3(0.01f, 0.01f, 0.0f);
        node->setPosition(position);
       

        scene->render();
        engine->render();
    }

    delete engine;
    delete scene;

    return 0;
}