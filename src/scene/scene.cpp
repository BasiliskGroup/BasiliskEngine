#include "scene.h"

Scene::Scene(Engine* engine): engine(engine) {
    camera = new Camera(engine);
    nodeHandler = new NodeHandler();
    shader = new Shader("shaders/entity_3d.vert", "shaders/entity_3d.frag");
}

Scene::~Scene() {
    delete camera;
    delete nodeHandler;
    delete shader;
}

void Scene::add(Node* node) {
    nodeHandler->add(node);
}

void Scene::update() {
    camera->update();
    camera->use(shader);
}

void Scene::render() {
    shader->use();
    nodeHandler->render();
}
