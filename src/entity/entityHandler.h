#ifndef ENTITY_HANDLER_H
#define ENTITY_HANDLER_H

#include "util/includes.h"
#include "entity/entity.h"
#include "entity/entity2d.h"


class EntityHandler {
    private:
        std::vector<Entity*> entities3d;
        std::vector<Entity2D*> entities2d;

    public:
        EntityHandler();
        ~EntityHandler();

        void render();

        void add(Entity* entity);
        void add(Entity2D* entity);
};

#endif