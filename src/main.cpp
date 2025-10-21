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

#include "IO/window.h"
#include "render/shader.h"
#include "render/vbo.h"
#include "render/ebo.h"
#include "render/vao.h"
#include "render/image.h"
#include "render/texture.h"
#include "scene/camera.h"
#include "IO/mouse.h"
#include "IO/keyboard.h"
#include "render/mesh.h"
#include "instance/instancer.h"
#include "scene/camera2d.h"
#include "entity/entityHandler.h"

int main() {
    // Create a GLFW window
    Window* window = new Window(800, 800, "Example 13: Instance");
    
    // Create a key object for keyboard inputs
    Keyboard* keys = new Keyboard(window);
    // Create a mouse object for mouse input
    Mouse* mouse = new Mouse(window);
    mouse->setGrab();

    // Create a camera object
    Camera camera3d({-3, 0, 0});
    Camera2D camera2d({0, 0});

    // Load shader from file
    Shader* shader3d = new Shader("shaders/entity_3d.vert", "shaders/entity_3d.frag");
    Shader* shader2d = new Shader("shaders/entity_2d.vert", "shaders/entity_2d.frag");
    
    // Data for making entity
    Mesh* cube = new Mesh("models/cube.obj");
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
    texture->setFilter(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
        
    // Create entities
    EntityHandler* entityHandler = new EntityHandler();

    Entity* entity3d = new Entity(shader3d, cube, texture);
    Entity2D* entity2d = new Entity2D(shader2d, quad, texture, {100, 100});

    entityHandler->add(entity3d);
    entityHandler->add(entity2d);

    // Main loop continues as long as the window is open
    while (window->isRunning()) {
        // Fill the screen with a low blue color
        window->clear(0.2, 0.3, 0.3, 1.0);
        // Update Mouse
        mouse->update();
        if (keys->getPressed(GLFW_KEY_ESCAPE)) {
            mouse->setVisible();
        }
        if (mouse->getClicked()) {
            mouse->setGrab();
        }

        glm::vec2 pos = entity2d->getPosition();
        pos.x += keys->getPressed(GLFW_KEY_RIGHT) - keys->getPressed(GLFW_KEY_LEFT);
        pos.y += keys->getPressed(GLFW_KEY_DOWN) - keys->getPressed(GLFW_KEY_UP);
        entity2d->setPosition(pos);

        // Update the camera for movement
        camera2d.setPosition({camera3d.getX() * 10, camera3d.getY() * 10});
        camera3d.update(mouse, keys);
        camera2d.update(mouse, keys);
        camera3d.use(shader3d);
        camera2d.use(shader2d);
        
        entityHandler->render();

        // Show the screen
        window->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete entity3d;
    delete entity2d;
    delete shader3d;
    delete shader2d;
    delete entityHandler;
    delete cube;
    delete quad;
    delete window;
    delete keys;
    delete mouse;
}