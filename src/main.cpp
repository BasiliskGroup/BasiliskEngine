#include "includes.h"

#include "engine.h"
#include "scene.h"
#include "shader.h"
#include "node.h"
#include "mesh.h"


int main() {
    Engine  engine(1200, 900, "Basilisk");
    Scene   scene(&engine);

    Shader  shader("shaders/shader.vert", "shaders/shader.frag");
    Mesh    sphereMesh("models/john.obj");

    Texture containerTexture("textures/container.jpg");
    Texture floorTexture("textures/floor_albedo.png");

    int n = 10;
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            for (int z = 0; z < n; z++) {
                Node* node = new Node(&shader, &sphereMesh, &floorTexture);
                scene.add(node);
                node->setPosition({x * 5, y * 5, z * 5}); 
            }
        }
    }

    // Main loop
    while (engine.isRunning()) {
        engine.update();

        scene.update();
        scene.render();

        engine.render();
    }
}