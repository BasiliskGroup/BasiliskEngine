#include <basilisk/basilisk.h>

int main() {
    // Make a Basilisk Engine instance 
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk");

    // Create a blank 2D scene
    bsk::Scene2D* scene = new bsk::Scene2D(engine);
    bsk::Scene2D* voidScene = new bsk::Scene2D(engine);

    scene->getCamera()->setScale(8.0);

    // Load assets from file
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* image = new bsk::Image("textures/knight.png");
    bsk::Image* image2 = new bsk::Image("textures/piss.png");
    bsk::Image* image3 = new bsk::Image("textures/man.png");

    // Create a material from image
    bsk::Material* material = new bsk::Material({1, 1, 1}, image);
    bsk::Material* material2 = new bsk::Material({1, 1, 1}, image2);
    bsk::Material* material3 = new bsk::Material({1, 1, 1}, image3);

    // normal nodes
    bsk::Node2D* floor = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, -3}, .rotation=0, .scale={9, 1}, .collision=true, .density=-1 });

    for (int i = 0; i < 10; i++) {
        float r = 0.1 * i;
        bsk::Node2D* square = new bsk::Node2D(scene, { .mesh=quad, .material=material, .position={0, i * 2}, .rotation=r, .scale={1, 1}, .collision=true });
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