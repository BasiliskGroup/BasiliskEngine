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
}

/**
 * @brief Destroy the Scene2D object. Deletes scene camera and shader.
 * 
 */
Scene2D::~Scene2D() {
    VirtualScene::clear();

    delete internalCamera; internalCamera = nullptr;
    delete shader; shader = nullptr;
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
    root->add(node.get());  
}

void Scene2D::remove(Node2D* node) {
    root->remove(node);
}

void Scene2D::remove(std::shared_ptr<Node2D> node) {
    childrenPythonMap.erase(node.get());
    root->remove(node.get());
}

}