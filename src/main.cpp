/**
 * C++ equivalent of build/test.py: single quad with textures/main.png.
 * Run the executable from the project root so "textures/main.png" resolves.
 */
 #include <basilisk/basilisk.h>
 #include <basilisk/physics/forces/joint.h>
 
 int main() {
     bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk", false);
 
     bsk::Scene2D* scene = new bsk::Scene2D(engine);
 
     bsk::Collider* collider = new bsk::Collider({glm::vec2(0.5f, 0.5f), glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f), glm::vec2(0.5f, -0.5f)});
 
     // box stack
    //  for (int i = 0; i < 5; i++) {
    //      new bsk::Node2D(scene, nullptr, nullptr, glm::vec2(0.1f * i, 1.0f + i * 1.1f), 0.0f, glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), collider);
    //  }
 
     // add floor
     new bsk::Node2D(scene, nullptr, nullptr, glm::vec2(0.0f, -4.0f), 0.0f, glm::vec2(20.0f, 1.0f), glm::vec3(0.0f), collider, -1.0f);
 
     // Drag joint for mouse interaction
     bsk::Joint* drag = nullptr;
 
     while (engine->isRunning()) {
         engine->update();
 
         // Handle mouse input for dragging rigid bodies
         bsk::Mouse* mouse = engine->getMouse();
         bsk::Keyboard* keyboard = engine->getKeyboard();
         bsk::StaticCamera2D* camera = scene->getCamera();
 
         // Get mouse world position
         glm::vec2 mousePos = glm::vec2(
             static_cast<float>(mouse->getWorldX(camera)),
             static_cast<float>(mouse->getWorldY(camera))
         );
 
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

         if (keyboard->getPressed(bsk::Key::K_SPACE)) {

             // add a new node at the mouse position
             new bsk::Node2D(scene, nullptr, nullptr, mousePos, 0.0f, glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), collider);
         }
 
         scene->update();
         scene->render();
         engine->render();
     }
 
     delete collider;
     delete scene;
     delete engine;
     return 0;
 }