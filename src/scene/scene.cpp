#include <basilisk/scene/sceneRoute.h>
#include <basilisk/util/print.h>

/**
 * @brief Construct a new Scene object. Exclusivly for 3D scenes. 
 * 
 * @param engine Pointer to the parent object
 */
Scene::Scene(Engine* engine) : VirtualScene<Node, vec3, quat, vec3>(), engine(engine) {
    camera = new Camera(engine);
    shader = new Shader("shaders/default.vert", "shaders/default.frag");
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
}

/**
 * @brief Destroy the Scene object. Deletes scene camera and shader.
 * 
 */
Scene::~Scene() {
    delete camera;
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