#include "entity/virtualEntity.h"

/**
 * @brief Construct a new Entity object
 * 
 * @param shader 
 * @param mesh 
 * @param texture 
 * @param position 
 * @param rotation
 * @param scale 
 */
template<typename position_type, typename rotation_type, typename scale_type>
VirtualEntity<position_type, rotation_type, scale_type>::VirtualEntity(Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale):
    shader(shader), mesh(mesh), texture(texture), position(position), rotation(rotation), scale(scale){

    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);
}

/**
 * @brief Destroy the Entity object. Release the vbo, ebo, and vao
 * 
 */
template<typename position_type, typename rotation_type, typename scale_type>
VirtualEntity<position_type, rotation_type, scale_type>::~VirtualEntity() {
    delete vao;
    delete vbo;
    delete ebo;
}

/**
 * @brief Render the vao on this entity
 * 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualEntity<position_type, rotation_type, scale_type>::render() {
    shader->bind("uTexture", texture, 0);
    shader->setUniform("uModel", model);
    vao->render();
}

/**
 * @brief Update the position of the entity
 * 
 * @param position 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualEntity<position_type, rotation_type, scale_type>::setPosition(position_type position) {
    this->position = position;
    updateModel();
}

/**
 * @brief Update the rotation of the entity
 * 
 * @param rotation 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualEntity<position_type, rotation_type, scale_type>::setRotation(rotation_type rotation) {
    rotation = rotation;
    updateModel();
}

/**
 * @brief Update the scale of the entity
 * 
 * @param scale 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualEntity<position_type, rotation_type, scale_type>::setScale(scale_type scale) {
    scale = scale;
    updateModel();
}
