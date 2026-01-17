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

// int main() {
//     // Make a Basilisk Engine instance 
//     bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

//     // Create a blank 2D scene
//     bsk::Scene* scene = new bsk::Scene(engine);

//     // Load assets from file
//     bsk::Mesh* cube = new bsk::Mesh("models/john.obj");
//     bsk::Image* image = new bsk::Image("textures/container.jpg");
//     bsk::Material* material = new bsk::Material({1, 1, 1}, image);

//     bsk::Node* node = new bsk::Node(scene, cube, material, {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1});
    
//     // Main loop continues as long as the window is open
//     while (engine->isRunning()) {
//         engine->update();
//         scene->update();
//         scene->render();
//         engine->render();
//     }

//     // Free memory allocations
//     delete image;
//     delete material;
//     delete cube;
//     delete scene;
//     delete engine;
// }

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");

    bsk::VBO* vbo = new bsk::VBO(quad->getVertices());
    bsk::EBO* ebo = new bsk::EBO(quad->getIndices());
    bsk::Shader* shader = new bsk::Shader("shaders/test.vert", "shaders/test.frag");

    bsk::VAO* vao = new bsk::VAO(shader, vbo, ebo);

    std::vector<float> colors {1.0f, 0.2f, 0.2f, 1.0f, 0.2f, 1.0f, 0.2f, 1.0f};

    bsk::UBO* ubo = new bsk::UBO(colors);
    shader->bind("testBlock", ubo, 2);

    // shader->use();
    // unsigned int ubo;
    // glGenBuffers(1, &ubo);
    // glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    // glBufferData(GL_UNIFORM_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    // glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // unsigned int colors_index = glGetUniformBlockIndex(shader->getID(), "testBlock");
    // glUniformBlockBinding(shader->getID(), colors_index, 2);
    // glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo);
    
    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();
        
        shader->use();
        vao->render();

        engine->render();
    }

    // Free memory allocations
    delete vbo;
    delete ebo;
    delete ubo;
    delete quad;
    delete shader;
    delete vao;
    delete engine;
}