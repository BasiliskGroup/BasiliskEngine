#include <basilisk/basilisk.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/util/random.h>
#include <basilisk/util/maths.h>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene* scene = new bsk::Scene(engine);

    // Sample Image data for a smiley face (16x16 pixels, RGBA)
    // Yellow background with black eyes and smile
    const int smileySize = 16;
    std::vector<float> smileyFaceData;
    smileyFaceData.reserve(smileySize * smileySize * 4);
    
    for (int y = 0; y < smileySize; y++) {
        for (int x = 0; x < smileySize; x++) {
            float r = 1.0f, g = 1.0f, b = 0.0f, a = 1.0f; // Yellow background
            
            // Left eye (2x2 pixels at (4, 5))
            if (x >= 4 && x <= 5 && y >= 5 && y <= 6) {
                r = g = b = 0.0f; // Black
            }
            // Right eye (2x2 pixels at (10, 5))
            else if (x >= 10 && x <= 11 && y >= 5 && y <= 6) {
                r = g = b = 0.0f; // Black
            }
            // Smile (curved line from x=3 to x=12, centered around y=11)
            else if (x >= 3 && x <= 12 && y >= 10 && y <= 12) {
                int centerX = smileySize / 2;
                int dx = x - centerX;
                // Parabolic curve: lower in the middle, higher at edges
                int smileY = 11 + (dx * dx) / 6;
                if (y == smileY || (y == smileY + 1 && std::abs(dx) < 4)) {
                    r = g = b = 0.0f; // Black
                }
            }
            
            smileyFaceData.push_back(r);
            smileyFaceData.push_back(g);
            smileyFaceData.push_back(b);
            smileyFaceData.push_back(a);
        }
    }

    bsk::Image* image = new bsk::Image(smileyFaceData, smileySize, smileySize);

    // Load assets
    bsk::Mesh* cube = new bsk::Mesh("models/cube.obj");
    bsk::Material* material = new bsk::Material({1.0, 1.0, 1.0}, image);

    bsk::Node* node = new bsk::Node(scene, cube, material, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
    scene->add(node);

    bsk::AmbientLight* ambientLight = new bsk::AmbientLight({1.0, 1.0, 1.0}, 0.2);
    scene->add(ambientLight);

    // Add skybox
    bsk::Cubemap* cubemap = new bsk::Cubemap({
        "textures/skybox/right.jpg",
        "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",
        "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg",
        "textures/skybox/back.jpg"
    });
    bsk::Skybox* skybox = new bsk::Skybox(cubemap);
    scene->setSkybox(skybox);

    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    delete engine;
    delete scene;
    delete cube;
    delete image;
    delete material;
    delete node;
    delete ambientLight;
    delete cubemap;
    delete skybox;
}