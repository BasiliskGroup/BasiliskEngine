#include <basilisk/scene/sceneRoute.h>
#include <basilisk/util/resolvePath.h>

namespace bsk::internal {

/**
 * @brief Construct a new Scene object. Exclusivly for 3D scenes. 
 * 
 * @param engine Pointer to the parent object
 */
Scene::Scene(Engine* engine, bool addSkybox, bool addLight, bool addCube) : VirtualScene(engine) {
    camera = new Camera(engine);
    internalCamera = camera;
    shader = new Shader(internalPath("shaders/instance.vert").c_str(), internalPath("shaders/instance.frag").c_str());
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
    lightServer = new LightServer();
    lightServer->setTiles(shader, camera, (unsigned int)engine->getWindow()->getWidth(), (unsigned int)engine->getWindow()->getHeight());
    addDefaults(addSkybox, addLight, addCube);
}

Scene::Scene(Engine* engine, Shader* shader, bool addSkybox, bool addLight, bool addCube) : VirtualScene(engine) {
    camera = new Camera(engine);
    internalCamera = camera;
    this->shader = shader;
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
    lightServer = new LightServer();
    lightServer->setTiles(shader, camera, (unsigned int)engine->getWindow()->getWidth(), (unsigned int)engine->getWindow()->getHeight());
    addDefaults(addSkybox, addLight, addCube);
}

/**
 * @brief Add default resources to the scene
 * 
 */
void Scene::addDefaults(bool addSkybox, bool addLight, bool addCube) {

    // Set camera back
    internalCamera->setPosition(glm::vec3(0, 0, -5));
    internalCamera->lookAt(glm::vec3(0, 0, 0));

    // Default Skybox
    if (addSkybox) {
        skybox = engine->getResourceServer()->defaultSkybox;
    }

    // Default Light
    if (addLight) {
        DirectionalLight* directionalLight = new DirectionalLight(glm::vec3(1, 1, 1), 0.8f, glm::vec3(2, -3, 1));
        add(directionalLight);
        AmbientLight* ambientLight = new AmbientLight(glm::vec3(1, 1, 1), 0.2f);
        add(ambientLight);
    }

    if (addCube) {
        new Node(this, engine->getResourceServer()->defaultCube, engine->getResourceServer()->defaultMaterial, glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 1), glm::vec3(1, 1, 1));
    }
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

void Scene::add(Node* node) {
    root->add(node);
}

void Scene::add(std::shared_ptr<Node> node) {
    childrenPythonMap.emplace(node.get(), node);
    root->add(node.get());
}

void Scene::remove(Node* node) {
    root->remove(node);
}

void Scene::remove(std::shared_ptr<Node> node) {
    childrenPythonMap.erase(node.get());
    root->remove(node.get());
}

}