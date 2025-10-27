#include "entity/entityHandler.h"

EntityHandler::EntityHandler() {

}

EntityHandler::~EntityHandler() {

}

void EntityHandler::add(Entity* entity) {
    entities3d.push_back(entity);
}

void EntityHandler::add(Entity2D* entity) {
    entities2d.push_back(entity);
}

void EntityHandler::render() {
    for (Entity* entity : entities3d) {
        entity->render();
    }
    for (Entity2D* entity : entities2d) {
        entity->render();
    }
}