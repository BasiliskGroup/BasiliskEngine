/**
 * @file 13_instance.cpp
 * @author Jonah Coffelt
 * @brief Shows how to use the VAO class to do instanced rendering
 * @version 0.1
 * @date 2025-10-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "window.h"
#include "shader.h"
#include "vbo.h"
#include "ebo.h"
#include "vao.h"
#include "image.h"
#include "texture.h"
#include "mat.h"
#include "camera.h"
#include "mouse.h"
#include "keyboard.h"
#include "mesh.h"
#include "instancer.h"

struct point {
    float x;
    float y;
    float z;
};

int main() {
    // Create a GLFW window
    Window* window = new Window(800, 800, "Example 13: Instance");
    glEnable(GL_CULL_FACE);  // For preformance
    
    // Create a key object for keyboard inputs
    Keyboard* keys = new Keyboard(window);
    // Create a mouse object for mouse input
    Mouse* mouse = new Mouse(window);
    mouse->setGrab();

    // Create a camera object
    Camera camera({-3, 0, 0});

    // Vertex data for a cube
    Mesh* mesh = new Mesh("models/cube.obj");
    // Load shader from file
    Shader* shader = new Shader("shaders/13_instance.vert", "shaders/13_instance.frag");
    // Create a texture from image
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
    texture->setFilter(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

    Instancer<point>* instancer = new Instancer<point>(shader, mesh, {"in_position", "in_uv", "in_normal"}, {"instance_position"});

    // Create buffer of instance data (just a grid of positions in this case)
    std::vector<float> translations {};
    int n = 10;
    float offset = 5.0f;
    for (int x = -n; x < n; x++) {
        for (int y = -n; y < n; y++) {
            for (int z = -n; z < n; z++) {
                instancer->add({x * offset, y * offset, z * offset});
            }
        }
    }

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

        // Render the vao with specified number of instances
        instancer->render();

        // Show the screen
        window->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete shader;
    delete mesh;
    delete window;
}