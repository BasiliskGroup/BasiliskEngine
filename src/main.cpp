#include <basilisk/basilisk.h>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    scene->getCamera()->setScale(25.0);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* image = new bsk::Image("textures/knight.png");
    bsk::Image* image2 = new bsk::Image("textures/piss.png");
    bsk::Image* image3 = new bsk::Image("textures/man.png");

    // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);
    bsk::Material* material2 = new bsk::Material({1, 1, 1}, image2);
    bsk::Material* material3 = new bsk::Material({1, 1, 1}, image3);

    // Create floor
    bsk::Node2D* floor = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, -8}, .rotation=0, .scale={50, 1.5}, .collision=true, .density=-1 });

    // Create unmovable obstacles around the scene
    // Left wall
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={-12, 0}, .rotation=0, .scale={1.5, 20}, .collision=true, .density=-1 });
    // Right wall
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={12, 0}, .rotation=0, .scale={1.5, 20}, .collision=true, .density=-1 });
    // Top platform
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, 10}, .rotation=0, .scale={15, 1}, .collision=true, .density=-1 });
    // Middle left obstacle
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={-8, -2}, .rotation=0, .scale={2, 3}, .collision=true, .density=-1 });
    // Middle right obstacle
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={8, -2}, .rotation=0, .scale={2, 3}, .collision=true, .density=-1 });
    // Platform on the right
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={10, 3}, .rotation=0, .scale={3, 1}, .collision=true, .density=-1 });
    // Platform on the left
    new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={-10, 3}, .rotation=0, .scale={3, 1}, .collision=true, .density=-1 });

    // Demo: Create a chain of long bars connected with Joints
    std::vector<bsk::Node2D*> chainNodes;
    bsk::Node2D* prevNode = nullptr;
    float barLength = 2.0f;
    float barWidth = 0.4f;
    for (int i = 0; i < 6; i++) {
        bsk::Node2D* node = new bsk::Node2D(scene, { 
            .mesh=quad, 
            .material=material, 
            .position={-6.0f + i * barLength, 8.0f}, 
            .rotation=0, 
            .scale={barLength, barWidth},  // Long horizontal bars
            .collision=true 
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

    // Demo: Create boxes connected with Springs (scaled up)
    std::vector<bsk::Node2D*> springNodes;
    for (int i = 0; i < 5; i++) {
        bsk::Node2D* node = new bsk::Node2D(scene, { 
            .mesh=quad, 
            .material=material2, 
            .position={6.0f + i * 2.5f, 10.0f}, 
            .rotation=0, 
            .scale={1.2f, 1.2f},  // Scaled up
            .collision=true 
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

    // Add some free-floating boxes that will bounce around
    for (int i = 0; i < 3; i++) {
        new bsk::Node2D(scene, { 
            .mesh=quad, 
            .material=material3, 
            .position={-4.0f + i * 4.0f, 6.0f}, 
            .rotation=0.3f * i, 
            .scale={1.3f, 1.3f}, 
            .collision=true 
        });
    }

    std::vector<bsk::Node2D*> contactNodes;

    // Main loop continues as long as the window is open
    int i = 0;
    while (engine->isRunning()) {

        // std::cout << square->getChildren().size() << std::endl;

        // Iterate through forces and visualize contact points
        for (bsk::Force* force = scene->getSolver()->forces; force != nullptr; force = force->next) {
            bsk::Manifold* manifold = dynamic_cast<bsk::Manifold*>(force);
            if (manifold == nullptr) continue;

            // Iterate through contacts in the manifold
            for (int i = 0; i < manifold->numContacts; i++) {
                // Transform local contact positions to world coordinates
                glm::vec2 rAW = bsk::internal::transform(manifold->bodyA->position, manifold->contacts[i].rA);
                glm::vec2 rBW = bsk::internal::transform(manifold->bodyB->position, manifold->contacts[i].rB);

                bsk::Node2D* nodeA = new bsk::Node2D(scene, { .mesh=quad, .material=material2, .position=rAW, .scale={0.125, 0.125}, .collision=false });
                nodeA->setLayer(0.9);
                contactNodes.push_back(nodeA);

                bsk::Node2D* nodeB = new bsk::Node2D(scene, { .mesh=quad, .material=material3, .position=rBW, .scale={0.125, 0.125}, .collision=false });
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

        // if (square->getPosition().y < -2.1) break;

        i++;
    }

    // Free memory allocations
    delete image;
    delete image2;
    delete image3;
    delete material;
    delete material2;
    delete material3;
    delete quad;
    delete scene;
    delete voidScene;
    delete engine;
}