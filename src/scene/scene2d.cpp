#include "scene/sceneRoute.h"


Scene2D::Scene2D(Engine* engine): engine(engine) {
    camera = new Camera2D(engine);
    shader = new Shader("shaders/default2D.vert", "shaders/default2D.frag");
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