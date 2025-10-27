#include "scene/sceneRoute.h"
#include "util/print.h"


Scene::Scene(Engine* engine) : VirtualScene<Node, vec3, quat, vec3>(), engine(engine) {
    camera = new Camera(engine);
    shader = new Shader("shaders/default.vert", "shaders/default.frag");
}

Scene::~Scene() {
    delete camera;
    delete shader;
}

void Scene::update() {
    camera->update();
    camera->use(shader);
}

void Scene::render() {
    shader->use();
    for (auto it = ++root->begin(); it != root->end(); ++it) {
        Node* node = *it;
        node->render();
    }
}