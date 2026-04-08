#include <basilisk/basilisk.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/util/maths.h>

#include <vector>
 
 int main() {
     bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk", false);
     bsk::Scene2D* scene  = new bsk::Scene2D(engine);
 
     bsk::Collider* collider = new bsk::Collider({
         glm::vec2( 0.5f,  0.5f),
         glm::vec2(-0.5f,  0.5f),
         glm::vec2(-0.5f, -0.5f),
         glm::vec2( 0.5f, -0.5f)
     });

     bsk::Material* material = new bsk::Material(glm::vec3(1.0f, 0.0f, 0.0f));
 
     // floor
     new bsk::Node2D(scene, nullptr, nullptr,
         glm::vec2(0.0f, -4.0f), 0.0f, glm::vec2(20.0f, 1.0f),
         glm::vec3(0.0f), collider, -1.0f);
 
     bsk::Joint* drag = nullptr;
    std::vector<bsk::Node2D*> contactMarkers;
 
     while (engine->isRunning()) {
         engine->update();
 
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
                 bsk::Node2D* node = new bsk::Node2D(nullptr, nullptr,
                     mousePos, 0.0f, glm::vec2(1.0f, 1.0f),
                     glm::vec3(0.0f), collider);
                scene->add(node);
                //  if (rand() % 2 == 0) {
                //     node->setHasGravity(false);
                //  }
             }
         }
 
        scene->update();

        // Rebuild contact markers every frame.
        for (bsk::Node2D* marker : contactMarkers) delete marker;
        contactMarkers.clear();

        for (bsk::Force* force = scene->getSolver()->getForces(); force; force = force->getNext()) {
            bsk::Manifold* manifold = dynamic_cast<bsk::Manifold*>(force);
            if (!manifold) continue;

            bsk::Rigid* bodyA = manifold->getBodyA();
            bsk::Rigid* bodyB = manifold->getBodyB();
            if (!bodyA || !bodyB) continue;

            const int numContacts = manifold->getNumContacts();
            for (int i = 0; i < numContacts; ++i) {
                const auto& c = manifold->getContact(i);
                glm::vec2 worldA = bsk::internal::transform(bodyA->getPosition(), c.rA);
                glm::vec2 worldB = bsk::internal::transform(bodyB->getPosition(), c.rB);

                bsk::Node2D* markerA = new bsk::Node2D(
                    scene, nullptr, material,
                    worldA, 0.0f, glm::vec2(0.08f, 0.08f),
                    glm::vec3(0.0f), nullptr);
                markerA->setLayer(0.9f);
                contactMarkers.push_back(markerA);

                bsk::Node2D* markerB = new bsk::Node2D(
                    scene, nullptr, material,
                    worldB, 0.0f, glm::vec2(0.08f, 0.08f),
                    glm::vec3(0.0f), nullptr);
                markerB->setLayer(0.9f);
                contactMarkers.push_back(markerB);
            }
        }

         scene->render();
         engine->render();
     }
 
    for (bsk::Node2D* marker : contactMarkers) delete marker;
     delete collider;
     delete scene;
     delete engine;
     return 0;
 }