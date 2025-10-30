#include "scene/sceneRoute.h"


/**
 * @brief Construct a new Scene2D object. Exclusivly for 2D scenes. 
 * 
 * @param engine 
 */
Scene2D::Scene2D(Engine* engine): engine(engine) {
    camera = new Camera2D(engine);
    shader = new Shader("shaders/instance2D.vert", "shaders/instance2D.frag");
    solver = new Solver();
    engine->getResourceServer()->write(shader, "textureArrays", "materials");
}

/**
 * @brief Destroy the Scene2D object. Deletes scene camera and shader.
 * 
 */
Scene2D::~Scene2D() {
    delete camera;
    delete shader;
    delete solver;
}

/**
 * @brief Update the scene (camera updates)
 * 
 */
void Scene2D::update() {
    camera->update();
    camera->use(shader);
}

/**
 * @brief Render all the 2D nodes in the scene
 * 
 */
void Scene2D::render() {
    shader->use();
    for (auto it = ++root->begin(); it != root->end(); ++it) {
        
        Node2D* node = *it;
        node->render();
    }
}