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

    // Load assets
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* images[] = {
        new bsk::Image("textures/metal.png"),
        new bsk::Image("textures/rope.png"),
        new bsk::Image("textures/bricks.jpg"),
        new bsk::Image("textures/container.jpg"),
        new bsk::Image("textures/mouse.png")
    };
    bsk::Material* metalMaterial = new bsk::Material({1, 1, 1}, images[0]);
    bsk::Material* ropeMaterial = new bsk::Material({1, 1, 1}, images[1]);
    bsk::Material* bricksMaterial = new bsk::Material({1, 1, 1}, images[2]);
    bsk::Material* containerMaterial = new bsk::Material({1, 1, 1}, images[3]);
    bsk::Material* mouseMaterial = new bsk::Material({1, 1, 1}, images[4]);

    // Create a box collider (unit box vertices) - can be shared by all box-shaped objects
    bsk::Collider* boxCollider = new bsk::Collider(scene->getSolver(), {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}});

    // Create a 50x50 grid of quads with some missing
    const int gridSize = 20;
    const float spacing = 2.0f;  // Space between quads
    const float quadSize = 0.8f;  // Size of each quad
    const float missingProbability = 0.0f;

    scene->getCamera()->setScale(gridSize * 1.5f);  // Zoom out to see the full grid
    
    // Increase camera speed by 5x (default is 3.0, so set to 15.0)
    if (auto* camera2D = dynamic_cast<bsk::Camera2D*>(scene->getCamera())) {
        camera2D->setSpeed(15.0f);
    }
    
    // Calculate grid offset to center it
    float gridOffset = -(gridSize - 1) * spacing * 0.5f;
    
    std::vector<bsk::Node2D*> gridNodes;
    gridNodes.reserve(gridSize * gridSize);
    
    for (int row = 0; row < gridSize; row++) {
        for (int col = 0; col < gridSize; col++) {
            // Randomly skip some quads
            if (bsk::internal::uniform() < missingProbability) {
                continue;
            }
            
            float x = gridOffset + col * spacing;
            float y = gridOffset + row * spacing;
            
            bsk::Node2D* node = new bsk::Node2D(scene, quad, bricksMaterial, {x, y}, 0.0f, {quadSize * bsk::internal::uniform(1.0f, 2.0f), quadSize * bsk::internal::uniform(1.0f, 2.0f)}, {0, 0, 0}, boxCollider, 1.0e1f, 0.5f);
            gridNodes.push_back(node);
        }
    }

    float mouseScale = 1.5f;
    
    // Create a Node2D that follows the mouse (no physics)
    bsk::Node2D* mouseCursor = new bsk::Node2D(scene, quad, mouseMaterial, {0.0f, 0.0f}, 0.0f, {mouseScale, mouseScale}, {0, 0, 0}, nullptr, 0.5f, 0.5f);
    mouseCursor->setLayer(1.0f);
    
    // Drag joint for mouse interaction
    bsk::Joint* drag = nullptr;
    
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
    }

    // Cleanup
    for (auto* img : images) delete img;
    delete metalMaterial;
    delete ropeMaterial;
    delete bricksMaterial;
    delete containerMaterial;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}