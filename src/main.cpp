#include <basilisk/basilisk.h>

#include "solver/physics.h"
#include "util/random.h"
#include "util/time.h"
#include "forces/force.h"
#include "tables/forceTable.h"
#include "tables/manifoldTable.h"
#include "shapes/rigid.h"

int main() {
    std::vector<float> quadData {
        // Triangle 1
        -0.8f, -0.8f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,

        // Triangle 2
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
    };

    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene2D* scene2D = new Scene2D(engine);
    scene2D->getSolver()->setGravity(0);

    // Data for making node
    // Mesh* quad = new Mesh("models/quad.obj");
    Mesh* quad = new Mesh("models/quad.obj");
    Image* manImage = new Image("textures/man.png");
    Image* pissImage = new Image("textures/piss.png");
    Texture* manTexture = new Texture(manImage);
    Texture* pissTexture = new Texture(pissImage);
        
    // physics
    Collider* squareCollider = new Collider(scene2D->getSolver(), {{-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}});

    Node2D* square = new Node2D(scene2D, { .mesh=quad, .texture=manTexture, .collider=squareCollider, .density=1, .velocity={1, -4, 3} });

    // floor
    Node2D* floor = new Node2D(scene2D, { .position={0, -3}, .scale={1, 1}, .mesh=quad, .texture=manTexture, .collider=squareCollider, .density=-1 });

    // new Node2D(scene2D, { .mesh=quad, .scale={0.1, 0.1}, .texture=pissTexture});

    std::vector<Node2D*> contacts;

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        // create new contact points
        ForceTable* forceTable = scene2D->getSolver()->getForceTable();
        ManifoldTable* manifoldTable = forceTable->getManifoldTable();

        for (uint i = 0; i < forceTable->getSize(); i++) {
            Force* force = forceTable->getForces()[i];
            uint specialIndex = forceTable->getSpecial()[i];
            Rigid* body = force->getBodyA();
            bool isA = forceTable->getIsA()[i];

            if (force->getType() != MANIFOLD) {
                continue;
            }

            Vec2Pair RAW, RA;
            if (isA) {
                RAW = manifoldTable->getRAW()[specialIndex];
                RA = manifoldTable->getRA()[specialIndex];
            } else {
                RAW = manifoldTable->getRBW()[specialIndex];
                RA = manifoldTable->getRB()[specialIndex];
            }

            for (uint j = 0; j < 2; j++) {
                contacts.push_back(new Node2D(scene2D, { .mesh=quad, .scale={0.1, 0.1}, .texture=pissTexture, .position = RAW[j] + vec2(body->getPos())}));
                contacts.push_back(new Node2D(scene2D, { .mesh=quad, .scale={0.1, 0.1}, .texture=manTexture, .position = RA[j] + vec2(body->getPos())}));
            }
        }

        print(square->isTouching(floor));

        scene2D->update(1.0 / 600.0);
        scene2D->render();

        // clear previous contact points
        for (uint i = 0; i < contacts.size(); i++) {
            delete contacts[i];
        }
        contacts.clear();

        engine->render();
    }

    // Free memory allocations
    delete manImage;
    delete pissImage;
    delete manTexture;
    delete pissTexture;
    delete quad;
    delete scene2D;
    delete engine;
}