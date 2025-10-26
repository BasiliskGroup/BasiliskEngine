/**
 * @file main.cpp
 * @author Jonah Coffelt
 * @brief ...
 * @version 0.1
 * @date 2025-10-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "render/shader.h"
#include "render/vbo.h"
#include "render/ebo.h"
#include "render/vao.h"
#include "render/image.h"
#include "render/texture.h"
#include "camera/camera.h"
#include "camera/camera2d.h"
#include "render/mesh.h"
#include "instance/instancer.h"
#include "nodes/nodeHandler.h"
#include "engine/engine.h"
#include "camera/virtualCamera.h"

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");

    // Create a camera object
    Camera* camera3d = new Camera({-3, 0, 0});
    Camera2D* camera2d = new Camera2D({0, 0});

    // Load shader from file
    Shader* shader3d = new Shader("shaders/entity_3d.vert", "shaders/entity_3d.frag");
    Shader* shader2d = new Shader("shaders/entity_2d.vert", "shaders/entity_2d.frag");
    
    // Data for making node
    Mesh* cube = new Mesh("models/cube.obj");
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
        
    // Create entities
    NodeHandler* nodeHandler = new NodeHandler();

    Node* node3d = new Node(shader3d, cube, texture);
    Node2D* node2d = new Node2D(shader2d, quad, texture, {100, 100});

    nodeHandler->add(node3d);
    nodeHandler->add(node2d);

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        glm::vec2 pos = node2d->getPosition();
        pos.x += engine->getKeyboard()->getPressed(GLFW_KEY_RIGHT) - engine->getKeyboard()->getPressed(GLFW_KEY_LEFT);
        pos.y += engine->getKeyboard()->getPressed(GLFW_KEY_DOWN) - engine->getKeyboard()->getPressed(GLFW_KEY_UP);
        node2d->setPosition(pos);

        // Update the camera for movement
        camera3d->update(engine);
        camera2d->update(engine);
        camera3d->use(shader3d);
        camera2d->use(shader2d);
        
        nodeHandler->render();

        // Show the screen
        engine->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete node3d;
    delete node2d;
    delete shader3d;
    delete shader2d;
    delete nodeHandler;
    delete cube;
    delete quad;
    delete engine;
}