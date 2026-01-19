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

    // Load assets from file
    bsk::Mesh* cube = new bsk::Mesh("models/john.obj");
    bsk::Image* image = new bsk::Image("textures/container.jpg");
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    bsk::Node* node = new bsk::Node(scene, cube, material, {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1});

    bsk::DirectionalLight* directionalLight = new bsk::DirectionalLight({1, 1, 1}, 0.8);
    bsk::PointLight* pointLight = new bsk::PointLight({1, 0, 0}, 2.0, {2, 3, 2}, 5.0);
    bsk::PointLight* pointLight2 = new bsk::PointLight({0, 0, 1}, 2.0, {2, 3, -2}, 5.0);
    bsk::PointLight* pointLight3 = new bsk::PointLight({0, 1, 0}, 2.0, {5, 0, 0}, 5.0);
    bsk::AmbientLight* ambientLight = new bsk::AmbientLight({1, 1, 1}, 0.3);

    scene->add(directionalLight);
    scene->add(pointLight);   
    scene->add(pointLight2);
    scene->add(pointLight3);
    scene->add(ambientLight);

    bsk::Skybox* skybox = new bsk::Skybox({"textures/skybox/right.jpg", "textures/skybox/left.jpg", "textures/skybox/top.jpg", "textures/skybox/bottom.jpg", "textures/skybox/front.jpg", "textures/skybox/back.jpg"});
    scene->setSkybox(skybox);

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    // Free memory allocations
    delete image;
    delete material;
    delete cube;
    delete skybox;
    delete scene;
    delete engine;
}