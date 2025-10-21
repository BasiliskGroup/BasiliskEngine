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
#include "entity/entity.h"
#include "entity/entity2d.h"
#include "scene/camera2d.h"


int main() {
    // Create a GLFW window
    Window* window = new Window(800, 800, "Example 13: Instance");
    
    // Create a key object for keyboard inputs
    Keyboard* keys = new Keyboard(window);
    // Create a mouse object for mouse input
    Mouse* mouse = new Mouse(window);
    mouse->setGrab();

    // Create a camera object
    Camera camera({-3, 0, 0});

    // Load shader from file
    Shader* shader = new Shader("shaders/entity_3d.vert", "shaders/entity_3d.frag");
    
    // Data for making entity
    Mesh* mesh = new Mesh("models/cube.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
    texture->setFilter(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
        
    // Create an entity
    Entity* entity = new Entity(shader, mesh, texture);
    Entity* entity2 = new Entity(shader, mesh, texture, {0, 4, 0});

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

        // Update the camera for movement
        camera.update(mouse, keys);
        // Use the camera on the shader
        camera.use(shader);
        entity->render();
        entity2->render();

        // Show the screen
        window->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    // delete entity;
    delete shader;
    // delete shader2D;
    // delete quad;
    delete window;
    delete keys;
    delete mouse;
}