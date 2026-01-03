#include <basilisk/basilisk.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    scene->getCamera()->setScale(25.0);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* metalImage = new bsk::Image("textures/metal.png");
    bsk::Image* ropeImage = new bsk::Image("textures/rope.png");
    bsk::Image* bricksImage = new bsk::Image("textures/bricks.jpg");
    bsk::Image* containerImage = new bsk::Image("textures/container.jpg");

    // Create materials from images
    bsk::Material* metalMaterial = new bsk::Material({1, 1, 1}, metalImage);
    bsk::Material* ropeMaterial = new bsk::Material({1, 1, 1}, ropeImage);
    bsk::Material* bricksMaterial = new bsk::Material({1, 1, 1}, bricksImage);
    bsk::Material* containerMaterial = new bsk::Material({1, 1, 1}, containerImage);

    // Create a box collider (unit box vertices) - can be shared by all box-shaped objects
    bsk::Collider* boxCollider = new bsk::Collider(scene->getSolver(), {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}});

    // Create a rotating circle made of rectangles with bumps (like a drying machine)
    const int numSegments = 16;  // Number of rectangles in the circle
    const float circleRadius = 12.5f;  // Radius of the circle
    const float segmentWidth = 0.5f;  // Width of each rectangle
    const float segmentLength = 6.0f;  // Length of each rectangle
    const float rotationSpeed = 0.1f;  // Radians per second
    const float bumpSize = 0.5f;  // Size of the bumps
    
    std::vector<bsk::Node2D*> circleSegments;
    std::vector<bsk::Node2D*> circleBumps;  // Store bumps separately for rotation updates
    circleSegments.reserve(numSegments);
    circleBumps.reserve(numSegments);
    
    // Create rectangles arranged in a circle with bumps
    for (int i = 0; i < numSegments; i++) {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / numSegments;
        float x = circleRadius * cosf(angle);
        float y = circleRadius * sinf(angle);
        
        // Main segment
        bsk::Node2D* segment = new bsk::Node2D(scene, {
            .mesh=quad,
            .material=bricksMaterial,
            .position={x, y},
            .rotation=angle + static_cast<float>(M_PI) / 2.0f,  // Perpendicular to radius
            .scale={segmentLength, segmentWidth},
            .collider=boxCollider,
            .density=-1  // Unmovable
        });
        circleSegments.push_back(segment);
        
        bsk::Node2D* bump = new bsk::Node2D(scene, {
            .mesh=quad,
            .material=bricksMaterial,
            .position={x, y},
            .rotation=angle + static_cast<float>(M_PI) / 2.0f,  // Same rotation as segment
            .scale={bumpSize, 10 * bumpSize},
            .collider=boxCollider,
            .density=-1  // Unmovable
        });
        circleBumps.push_back(bump);
    }
    
    float circleRotation = 0.0f;  // Current rotation angle of the circle

    // Demo: Create a chain of long bars connected with Joints (rope texture)
    std::vector<bsk::Node2D*> chainNodes;
    bsk::Node2D* prevNode = nullptr;
    float barLength = 2.0f;
    float barWidth = 0.4f;
    for (int i = 0; i < 6; i++) {
        bsk::Node2D* node = new bsk::Node2D(scene, { 
            .mesh=quad, 
            .material=ropeMaterial, 
            .position={-6.0f + i * barLength, 8.0f}, 
            .rotation=0, 
            .scale={barLength, barWidth},  // Long horizontal bars
            .collider=boxCollider 
        });
        chainNodes.push_back(node);

        // Connect with Joint if not the first node - joints connect the ends of the bars
        if (prevNode != nullptr && prevNode->getRigid() != nullptr && node->getRigid() != nullptr) {
            bsk::Solver* solver = scene->getSolver();
            // Joint connects at the ends of the bars (local coordinates)
            // rA: right edge of previous bar, rB: left edge of current bar
            // Set angle stiffness to 0 to allow free rotation (rope-like behavior)
            // Position constraints (x, y) remain INFINITY to keep them connected
            new bsk::Joint(solver, prevNode->getRigid(), node->getRigid(), 
                {barLength * 0.5f, 0.0f},  // rA: right end of previous bar
                {-barLength * 0.5f, 0.0f}, // rB: left end of current bar
                {INFINITY, INFINITY, 0.0f} // stiffness: position locked, angle free
            );
        }
        prevNode = node;
    }

    // Demo: Create boxes connected with Springs (metal texture) - closer to origin
    std::vector<bsk::Node2D*> springNodes;
    for (int i = 0; i < 5; i++) {
        bsk::Node2D* node = new bsk::Node2D(scene, { 
            .mesh=quad, 
            .material=metalMaterial, 
            .position={-2.0f + i * 1.5f, 2.0f},  // Closer to origin
            .rotation=0, 
            .scale={1.2f, 1.2f},  // Scaled up
            .collider=boxCollider 
        });
        springNodes.push_back(node);
    }

    // Connect spring nodes with Springs (looser springs)
    for (size_t i = 0; i < springNodes.size() - 1; i++) {
        if (springNodes[i]->getRigid() != nullptr && springNodes[i+1]->getRigid() != nullptr) {
            bsk::Solver* solver = scene->getSolver();
            // Spring connects centers of boxes with lower stiffness (looser)
            new bsk::Spring(solver, springNodes[i]->getRigid(), springNodes[i+1]->getRigid(),
                {0.0f, 0.0f},  // rA: center of first box
                {0.0f, 0.0f},  // rB: center of second box
                300.0f,        // stiffness (reduced from 1500 to 300 for looser springs)
                2.5f           // rest length (scaled for larger boxes)
            );
        }
    }

    // Add some free-floating boxes that will bounce around (container texture)
    for (int i = 0; i < 7; i++) {
        new bsk::Node2D(scene, { 
            .mesh=quad, 
            .material=containerMaterial, 
            .position={-4.0f + i * 4.0f, 6.0f}, 
            .rotation=0.3f * i, 
            .scale={1.3f, 1.3f}, 
            .collider=boxCollider 
        });
    }

    std::vector<bsk::Node2D*> contactNodes;

    // Main loop continues as long as the window is open
    int i = 0;
    while (engine->isRunning()) {
        
        // Update rotating circle manually
        double dt = engine->getDeltaTime();
        circleRotation += rotationSpeed * static_cast<float>(dt);
        
        // Update each segment's position and rotation
        for (int j = 0; j < numSegments; j++) {
            float baseAngle = (2.0f * static_cast<float>(M_PI) * j) / numSegments;
            float currentAngle = baseAngle + circleRotation;
            float x = circleRadius * cosf(currentAngle);
            float y = circleRadius * sinf(currentAngle);
            
            // Update main segment
            circleSegments[j]->setPosition({x, y});
            circleSegments[j]->setRotation(currentAngle + static_cast<float>(M_PI) / 2.0f);
            
            // Update physics body position and rotation if it exists
            if (circleSegments[j]->getRigid() != nullptr) {
                circleSegments[j]->getRigid()->setPosition({x, y, currentAngle + static_cast<float>(M_PI) / 2.0f});
            }
            
            circleBumps[j]->setPosition({x, y});
            circleBumps[j]->setRotation(currentAngle + static_cast<float>(M_PI) / 2.0f);
            
            if (circleBumps[j]->getRigid() != nullptr) {
                circleBumps[j]->getRigid()->setPosition({x, y, currentAngle + static_cast<float>(M_PI) / 2.0f});
            }
        }

        // Iterate through forces and visualize contact points
        for (bsk::Force* force = scene->getSolver()->getForces(); force != nullptr; force = force->getNext()) {
            bsk::Manifold* manifold = dynamic_cast<bsk::Manifold*>(force);
            if (manifold == nullptr) continue;

            // Iterate through contacts in the manifold
            for (int i = 0; i < manifold->getNumContacts(); i++) {
                // Transform local contact positions to world coordinates
                bsk::Manifold::Contact contact = manifold->getContact(i);
                glm::vec2 rAW = bsk::internal::transform(manifold->getBodyA()->getPosition(), contact.rA);
                glm::vec2 rBW = bsk::internal::transform(manifold->getBodyB()->getPosition(), contact.rB);

                bsk::Node2D* nodeA = new bsk::Node2D(scene, { .mesh=quad, .material=metalMaterial, .position=rAW, .scale={0.125, 0.125} });
                nodeA->setLayer(0.9);
                contactNodes.push_back(nodeA);

                bsk::Node2D* nodeB = new bsk::Node2D(scene, { .mesh=quad, .material=containerMaterial, .position=rBW, .scale={0.125, 0.125} });
                nodeB->setLayer(0.9);
                contactNodes.push_back(nodeB);
            }
        }

        engine->update();
        scene->update();
        scene->render();
        engine->render();

        for (bsk::Node2D* node : contactNodes) {
            delete node;
        }
        contactNodes.clear();

        i++;
    }

    // Free memory allocations
    delete metalImage;
    delete ropeImage;
    delete bricksImage;
    delete containerImage;
    delete metalMaterial;
    delete ropeMaterial;
    delete bricksMaterial;
    delete containerMaterial;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}