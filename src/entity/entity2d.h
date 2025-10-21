#ifndef ENTITY2D_H
#define ENTITY2D_H

#include "util/includes.h"
#include "entity/virtualEntity.h"

class Entity2D : public VirtualEntity<glm::vec2, float, glm::vec2>{
    protected:
        float layer=0.0;
        void updateModel() override;

    public:
        Entity2D(Shader* shader, Mesh* mesh, Texture* texture, glm::vec2 position={0, 0}, float rotation=0, glm::vec2 scale={100, 100})
            : VirtualEntity(shader, mesh, texture, position, rotation, scale) {
                updateModel();
            }
};

#endif