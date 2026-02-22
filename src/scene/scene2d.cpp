#include <basilisk/scene/sceneRoute.h>
#include <basilisk/util/resolvePath.h>

namespace bsk::internal {

/**
 * @brief Construct a new Scene2D object. Exclusivly for 2D scenes. 
 * 
 * @param engine 
 */
Scene2D::Scene2D(Engine* engine) : VirtualScene(engine) {
    camera = new Camera2D(engine);
    internalCamera = camera;
    shader = new Shader(internalPath("shaders/instance2D.vert").c_str(), internalPath("shaders/instance2D.frag").c_str());
    solver = new Solver();
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
    customShader = false;
}

Scene2D::Scene2D(Engine* engine, Shader* shader) : VirtualScene(engine) {
    camera = new Camera2D(engine);
    internalCamera = camera;
    this->shader = shader;
    solver = new Solver();
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
    customShader = true;
}

/**
 * @brief Destroy the Scene2D object. Deletes scene camera and shader.
 * 
 */
Scene2D::~Scene2D() {
    VirtualScene::clear();

    delete internalCamera; internalCamera = nullptr;
    if (!customShader) {
        delete shader; shader = nullptr;
    }
    delete solver; solver = nullptr;
}

/**
 * @brief Update the scene (camera updates)
 * 
 */
void Scene2D::update() {
    // physics
    solver->step(engine->getDeltaTime());

    // camera
    camera->update();
    camera->use(shader);
}

/**
 * @brief Render all the 2D nodes in the scene
 * 
 */
void Scene2D::render() {
    engine->disableCullFace();
    shader->use();
    for (auto it = ++root->begin(); it != root->end(); ++it) {
        
        Node2D* node = *it;
        node->render();
    }
    engine->enableCullFace();
}

void Scene2D::add(Node2D* node) {
    root->add(node);
}

void Scene2D::add(std::shared_ptr<Node2D> node) {
    childrenPythonMap.emplace(node.get(), node);
    if (node->getParent() == nullptr) {
        root->add(node.get());
    }
}

void Scene2D::remove(Node2D* node) {
    if (node == nullptr) return;
    Node2D* parent = node->getParent();
    if (parent != nullptr) {
        parent->remove(node);
    } else {
        root->remove(node);
    }
}

void Scene2D::remove(std::shared_ptr<Node2D> node) {
    childrenPythonMap.erase(node.get());
    remove(node.get());
}
    
// raycasting
RayCastResult2D Scene2D::raycast(glm::vec2 origin, glm::vec2 direction) {
    RayCastResult2D best;

    for (auto it = ++root->begin(); it != root->end(); ++it) {
        Node2D* node = *it;
        RayCastResult2D hit = node->raycast(origin, direction);

        // Match 3D behavior: choose nearest positive-distance hit in world space.
        if (hit.node && hit.distance < best.distance && hit.distance > 0.0f) {
            best = hit;
        }
    }
    return best;
}

// return the node at the position
Node2D* Scene2D::pick(glm::vec2 position) {
    float bestLayer = -FLT_MAX;
    Node2D* bestNode = nullptr;

    for (auto it = ++root->begin(); it != root->end(); ++it) {
        Node2D* node = *it;
        if (node->pointIsInside(position) && node->getLayer() > bestLayer) {
            bestLayer = node->getLayer();
            bestNode = node;
        }
    }
    return bestNode;
}

}