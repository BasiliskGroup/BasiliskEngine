#include <basilisk/basilisk.h>
 #include <basilisk/physics/forces/joint.h>
 #include <string>
 
 int main() {
     bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk", false);
     bsk::Scene2D* scene  = new bsk::Scene2D(engine);
 
     bsk::Collider* collider = new bsk::Collider({
         glm::vec2( 0.5f,  0.5f),
         glm::vec2(-0.5f,  0.5f),
         glm::vec2(-0.5f, -0.5f),
         glm::vec2( 0.5f, -0.5f)
     });
 
     // floor
     new bsk::Node2D(scene, nullptr, nullptr,
         glm::vec2(0.0f, -4.0f), 0.0f, glm::vec2(200.0f, 1.0f),
         glm::vec3(0.0f), collider, -1.0f);
 
     bsk::Joint* drag = nullptr;
 
     while (engine->isRunning()) {
         engine->update();

         double fps = 1.0 / engine->getDeltaTime();
         int numRigids = scene->getSolver()->getNumRigids();
         std::string title = "Basilisk | Rigids: " + std::to_string(numRigids) + " | FPS: " + std::to_string(static_cast<int>(fps));
         engine->getWindow()->setTitle(title.c_str());
 
         bsk::Mouse*         mouse    = engine->getMouse();
         bsk::Keyboard*      keyboard = engine->getKeyboard();
         bsk::StaticCamera2D* camera  = scene->getCamera();
 
         glm::vec2 mousePos = {
             static_cast<float>(mouse->getWorldX(camera)),
             static_cast<float>(mouse->getWorldY(camera))
         };
 
         // --- Drag ---
         if (mouse->getLeftDown()) {
             if (!drag) {
                 glm::vec2 local;
                 bsk::Rigid* picked = scene->getSolver()->pick(mousePos, local);
                 if (picked) {
                     drag = new bsk::Joint(
                         scene->getSolver(),
                         nullptr, picked,
                         mousePos, local,
                         glm::vec3(1000.0f, 1000.0f, 0.0f)
                     );
                 }
             } else {
                 drag->setRA(mousePos);
             }
         } else if (mouse->getLeftReleased() && drag) {
             delete drag;
             drag = nullptr;
         }
 
         // --- Spawn / Delete ---
         if (keyboard->getPressed(bsk::Key::K_SPACE)) {
             bsk::Node2D* hovered = scene->pick(mousePos);
             if (hovered) {
                 if (drag && drag->getBodyB() == hovered->getRigid()) {
                     delete drag;
                     drag = nullptr;
                 }
                 delete hovered;
             } else {
                 new bsk::Node2D(scene, nullptr, nullptr,
                     mousePos, 0.0f, glm::vec2(1.0f, 1.0f),
                     glm::vec3(0.0f), collider);
             }
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