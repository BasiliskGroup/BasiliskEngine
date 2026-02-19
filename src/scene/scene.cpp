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
    // Only add to root if the node doesn't already have a parent
    // (nodes created with a parent are already in the tree)
    if (node->getParent() == nullptr) {
        root->add(node.get());
    }
}

void Scene::remove(Node* node) {
    root->remove(node);
}

void Scene::remove(std::shared_ptr<Node> node) {
    childrenPythonMap.erase(node.get());
    root->remove(node.get());
}

// raycasting - origin and direction in world space
RayCastResult Scene::raycast(glm::vec3 origin, glm::vec3 direction) {
    RayCastResult result;

    for (auto it = ++root->begin(); it != root->end(); ++it) {
        Node* node = *it;
        RayCastResult hit = node->raycast(origin, direction);

        if (hit.node && hit.distance < result.distance && hit.distance > 0.0f) {
            result = hit;
        }
    }

    return result;
}

RayCastResult Scene::pick(glm::vec2 mousePosition) {
    // Convert mouse position (screen pixels) to normalized direction vector in world space
    int width = static_cast<int>(engine->getWindow()->getWidth() * engine->getWindow()->getWindowScaleX());
    int height = static_cast<int>(engine->getWindow()->getHeight() * engine->getWindow()->getWindowScaleY());

    // Screen to NDC: [0,width] x [0,height] -> [-1,1] x [-1,1], Y flipped (screen Y down, NDC Y up)
    float ndcX = (2.0f * mousePosition.x) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * mousePosition.y) / height;

    // Point on near plane in clip space
    glm::vec4 clipNear(ndcX, ndcY, -1.0f, 1.0f);
    // Point on far plane in clip space (for direction)
    glm::vec4 clipFar(ndcX, ndcY, 1.0f, 1.0f);

    glm::mat4 view = camera->getView();
    glm::mat4 projection = camera->getProjection();
    glm::mat4 invViewProj = glm::inverse(projection * view);

    glm::vec4 worldNear = invViewProj * clipNear;
    glm::vec4 worldFar = invViewProj * clipFar;
    worldNear /= worldNear.w;
    worldFar /= worldFar.w;

    glm::vec3 origin = camera->getPosition();
    glm::vec3 direction = glm::normalize(glm::vec3(worldFar) - glm::vec3(worldNear));

    return raycast(origin, direction);
}

}  // namespace bsk::internal