#include <basilisk/scene/sceneRoute.h>
#include <basilisk/util/resolvePath.h>

namespace bsk::internal {

/**
 * @brief Construct a new Scene object. Exclusivly for 3D scenes. 
 * 
 * @param engine Pointer to the parent object
 */
Scene::Scene(Engine* engine) : VirtualScene(engine) {
    camera = new Camera(engine);
    internalCamera = camera;
    shader = new Shader(internalPath("shaders/instance.vert").c_str(), internalPath("shaders/instance.frag").c_str());
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
}

Scene::Scene(Engine* engine, Shader* shader) : VirtualScene(engine) {
    camera = new Camera(engine);
    internalCamera = camera;
    this->shader = shader;
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
}

/**
 * @brief Destroy the Scene object. Deletes scene camera and shader.
 * 
 */
Scene::~Scene() {
    delete internalCamera; internalCamera = nullptr;
    delete shader;
}

/**
 * @brief Update the scene (camera updates)
 * 
 */
void Scene::update() {
    camera->update();
    camera->use(shader);
}

/**
 * @brief Render all the 3D nodes in the scene
 * 
 */
void Scene::render() {
    shader->use();
    for (auto it = ++root->begin(); it != root->end(); ++it) {
        Node* node = *it;
        node->render();
    }
}

}