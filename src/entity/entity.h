#ifndef ENTITY_H
#define ENTITY_H

#include "util/includes.h"
#include "entity/virtualEntity.h"

class Entity : public VirtualEntity<glm::vec3, glm::vec3, glm::vec3>{
    protected:
        void updateModel() override;
    public:
        Entity(Shader* shader, Mesh* mesh, Texture* texture, glm::vec3 position={0, 0, 0}, glm::vec3 rotation={0, 0, 0}, glm::vec3 scale={1, 1, 1})
            : VirtualEntity(shader, mesh, texture, position, rotation, scale) {
                updateModel();
        }
};

#endif