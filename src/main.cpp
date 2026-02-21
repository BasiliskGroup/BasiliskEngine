#include <basilisk/basilisk.h>
#include <cmath>
#include <algorithm>

using namespace bsk;

int main() {
    Engine* engine = new Engine(1200, 800);

    // Optional: continue with your main scene after splash
    Scene2D* scene = new Scene2D(engine);
    

    Node2D* node = new Node2D(scene, nullptr, nullptr);
    node->setJacobianMask(glm::vec3(1.0f, 1.0f, 0.0f));
    
    Frame* frame = new Frame(engine, 1200, 800);

    while (engine->isRunning()) {
        engine->update();
        scene->update();

        std::cout << frame->getFBO() << std::endl;

        scene->render();
        engine->render();
    }
    delete scene;
    delete engine;
    return 0;
}
