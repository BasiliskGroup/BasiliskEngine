#include "scene/sceneRoute.h"


Scene2D::Scene2D(Engine* engine): engine(engine) {
    camera = new Camera2D(engine);
    shader = new Shader("shaders/entity_2d.vert", "shaders/entity_2d.frag");
}

Scene2D::~Scene2D() {
    delete camera;
    delete shader;
}

void Scene2D::update() {
    camera->update();
    camera->use(shader);
}

void Scene2D::render() {
    shader->use();
    for (auto it = ++root->begin(); it != root->end(); ++it) {
        
        Node2D* node = *it;
        node->render();
    }
}