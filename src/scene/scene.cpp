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
    lightServer = new LightServer();
    lightServer->setTiles(shader, camera, (unsigned int)engine->getWindow()->getWidth(), (unsigned int)engine->getWindow()->getHeight());
}

Scene::Scene(Engine* engine, Shader* shader) : VirtualScene(engine) {
    camera = new Camera(engine);
    internalCamera = camera;
    this->shader = shader;
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
    lightServer = new LightServer();
    lightServer->setTiles(shader, camera, (unsigned int)engine->getWindow()->getWidth(), (unsigned int)engine->getWindow()->getHeight());
}

/**
 * @brief Destroy the Scene object. Deletes scene camera and shader.
 * 
 */
Scene::~Scene() {
    delete internalCamera; internalCamera = nullptr;
    delete shader;
    delete lightServer;
}

/**
 * @brief Update the scene (camera updates)
 * 
 */
void Scene::update() {
    camera->update();
    camera->use(shader);
    lightServer->update(shader, camera);
}

/**
 * @brief Render all the 3D nodes in the scene
 * 
 */
void Scene::render() {
    if (skybox) {
        skybox->render(camera);
    }

    shader->use();
    for (auto it = ++root->begin(); it != root->end(); ++it) {
        Node* node = *it;
        node->render();
    }
}

/**
 * @brief Add a light to the scene
 * 
 */
void Scene::add(Light* light) {
    if (auto* directional = dynamic_cast<DirectionalLight*>(light)) {
        lightServer->add(directional);
    } else if (auto* point = dynamic_cast<PointLight*>(light)) {
        lightServer->add(point);
    } else if (auto* ambient = dynamic_cast<AmbientLight*>(light)) {
        lightServer->add(ambient);
    }
}

}