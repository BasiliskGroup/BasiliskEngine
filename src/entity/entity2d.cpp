#include "entity/entity2d.h"

/**
 * @brief Helper to update the model matrix when entity is updated. 
 * 
 */
void Entity2D::updateModel() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, layer));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
}