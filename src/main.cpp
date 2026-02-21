#include <basilisk/basilisk.h>
#include <cmath>
#include <algorithm>

using namespace bsk;

int main() {
    Engine* engine = new Engine(1200, 800);

    // Optional: continue with your main scene after splash
    Scene2D* scene = new Scene2D(engine);
    const float spacing = 2.0f;
    const glm::vec2 quadSize(1.2f, 1.2f);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            float r = (i + 1) / 3.0f;
            float g = (j + 1) / 3.0f;
            float b = 0.4f;
            Material* mtl = new Material({r, g, b});
            Node2D* node = new Node2D(scene, nullptr, mtl);
            node->setPosition(glm::vec2((i - 1) * spacing, (j - 1) * spacing));
            node->setScale(quadSize);
            scene->add(node);
        }
    }

    // engine->showSplash();

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }
    delete scene;
    delete engine;
    return 0;
}
