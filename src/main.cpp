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
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    // Load assets from file
    bsk::Mesh* cube = new bsk::Mesh("models/cube.obj");
    bsk::Image* image = new bsk::Image("textures/container.jpg");
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);

    new bsk::Node(scene, cube, material, {0, -5, 0}, {1, 0, 0, 0}, {30, .1, 30});
    
    std::vector<bsk::PointLight*> lights;
    for (int i = 0; i < 50; i++) {
        glm::vec3 color(bsk::internal::uniform(0.0f, 1.0f), bsk::internal::uniform(0.0f, 1.0f), bsk::internal::uniform(0.0f, 1.0f));
        float d = 30.0f;
        glm::vec3 position(bsk::internal::uniform(-d, d), bsk::internal::uniform(-5.0f, 5.0f), bsk::internal::uniform(-d, d));
        lights.push_back(new bsk::PointLight(color, 1.0f, position, 5.0f));
        scene->add(lights.back());
    }

    // bsk::Node* node = new bsk::Node(scene, cube, material, {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1});

    // bsk::DirectionalLight* directionalLight = new bsk::DirectionalLight({1, 0, 1}, 0.8);
    // bsk::PointLight* pointLight = new bsk::PointLight({1, 0, 0}, 8.0, {2, 3, 2}, 5.0);
    // bsk::PointLight* pointLight2 = new bsk::PointLight({0, 0, 1}, 8.0, {2, 3, -2}, 5.0);
    // bsk::PointLight* pointLight3 = new bsk::PointLight({0, 1, 0}, 8.0, {5, 0, 0}, 5.0);
    // bsk::AmbientLight* ambientLight = new bsk::AmbientLight({1, 1, 1}, 0.3);

    // scene->add(directionalLight);
    // scene->add(pointLight);   
    // scene->add(pointLight2);
    // scene->add(pointLight3);
    // scene->add(ambientLight);

    bsk::Skybox* skybox = new bsk::Skybox({"textures/skybox/right.jpg", "textures/skybox/left.jpg", "textures/skybox/top.jpg", "textures/skybox/bottom.jpg", "textures/skybox/front.jpg", "textures/skybox/back.jpg"});
    scene->setSkybox(skybox);

    bsk::Keyboard* keys = engine->getKeyboard();


    float t = 0.0;

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();
        
        // Handle mouse input for dragging rigid bodies
        bsk::Mouse* mouse = engine->getMouse();
        bsk::StaticCamera2D* camera = scene->getCamera();
        
        // Get mouse world position
        glm::vec2 mousePos = glm::vec2(
            static_cast<float>(mouse->getWorldX(camera)),
            static_cast<float>(mouse->getWorldY(camera))
        );
        
        // Update mouse cursor position to follow the mouse
        mouseCursor->setPosition(mousePos + glm::vec2{mouseScale * 0.5f, -mouseScale * 0.5f});
        
        // Handle drag joint
        if (mouse->getLeftDown()) {
            if (!drag) {
                // Try to pick a body at the mouse position
                glm::vec2 local;
                bsk::Rigid* pickedBody = scene->getSolver()->pick(mousePos, local);
                if (pickedBody) {
                    // Create a joint between the mouse (world position) and the picked body
                    drag = new bsk::Joint(
                        scene->getSolver(),
                        nullptr,           // bodyA = nullptr means world anchor
                        pickedBody,        // bodyB = the picked rigid body
                        mousePos,          // rA = world position (attachment point in world)
                        local,             // rB = local position on the body
                        glm::vec3(1000.0f, 1000.0f, 0.0f)  // stiffness
                    );
                }
            } else {
                // Update the world anchor position to follow the mouse
                drag->setRA(mousePos);
            }
        } else if (mouse->getLeftReleased() && drag) {
            // Delete the drag joint when mouse is released
            delete drag;
            drag = nullptr;
        }
        
        scene->update();
        scene->render();
        engine->render();

        t += engine->getDeltaTime();
        if (t > 1.0) {
            std::cout << "FPS: " << engine->getWindow()->getFPS() << std::endl;
            t = 0.0;
        }

        if (keys->getPressed(bsk::Key::K_1)) {
            engine->getWindow()->enableVSync();
        }
        if (keys->getPressed(bsk::Key::K_2)) {
            engine->getWindow()->disableVSync();
        }
    }

    // Free memory allocations
    delete image;
    delete material;
    delete cube;
    delete skybox;
    delete scene;
    delete voidScene;
    delete engine;
}