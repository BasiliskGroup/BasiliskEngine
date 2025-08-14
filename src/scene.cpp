#include "scene.h"

Scene::Scene(Engine* engine): engine(engine), camera(Camera(engine)) {}

void Scene::update() {
    nodeHanlder.update(engine->getDeltaTime());
    camera.update();
}

void Scene::render() {
    nodeHanlder.render(&camera);
}