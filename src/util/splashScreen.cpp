#include <basilisk/util/splashScreen.h>
#include <basilisk/engine/engine.h>
#include <basilisk/scene/scene2d.h>
#include <basilisk/render/material.h>
#include <basilisk/nodes/node2d.h>
#include <algorithm>
#include <cmath>

namespace bsk::internal {

void showSplash(Engine* engine) {
        Scene2D* scene = new Scene2D(engine);
    
        const float fadeInDuration = 0.5f;
        const float holdDuration = 2.0f;
        const float fadeOutDuration = 0.5f;
        const float totalDuration = fadeInDuration + holdDuration + fadeOutDuration;
    
        Material* material = new Material({1.0f, 1.0f, 1.0f}, engine->getResourceServer()->logoImage, nullptr, 1.0f);
        Node2D* quad = new Node2D(scene, nullptr, material);
        quad->setPosition(glm::vec2(0.0f, 0.0f));
        quad->setScale(glm::vec2(3.0f, 3.0f));
        scene->add(quad);
    
        double startTime = engine->getWindow()->getTime();
    
        while (engine->isRunning()) {
            double elapsed = engine->getWindow()->getTime() - startTime;
            if (elapsed >= totalDuration)
                break;
    
            float alpha;
            if (elapsed < fadeInDuration) {
                alpha = static_cast<float>(elapsed / fadeInDuration);
            } else if (elapsed < fadeInDuration + holdDuration) {
                alpha = 1.0f;
            } else {
                float fadeOutElapsed = static_cast<float>(elapsed - (fadeInDuration + holdDuration));
                alpha = 1.0f - std::min(fadeOutElapsed / fadeOutDuration, 1.0f);
            }
            material->setAlpha(alpha);
    
            engine->update();
            scene->update();
            scene->render();
            engine->render();
        }
    
    delete scene;
}

}