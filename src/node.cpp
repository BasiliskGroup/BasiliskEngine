#include "node.h"


Node::Node(
    Shader* shader, 
    Mesh* mesh, 
    Texture* texture,
    vec3 position, 
    quat rotation, 
    vec3 scale
): 
    vao        (VAO(shader, mesh->getVBO(), mesh->getEBO())),
    mesh       (mesh),
    texture    (texture),
    position   (position),
    rotation   (rotation),
    scale      (scale) {
}

void Node::render() {
    vao.getShader()->bind("texture1", texture, 0);
    vao.getShader()->setUniform("model", model);
    vao.render();
}

void Node::update(float deltaTime) {
    mat4x4 modelTranslation = glm::translate(mat4x4(1), position);
    mat4x4 modelRotation =  mat4x4(rotation);
    mat4x4 modelScale = glm::scale(mat4x4(1), scale);

    model = modelTranslation * modelRotation * modelScale;
}