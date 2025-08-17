#include "node.h"

unsigned int Node::maxID = 0;
std::vector<unsigned int> Node::freeIDs = {};

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

    // Each node needs a unique ID for batching
    if (freeIDs.empty()) {
        id = maxID++;
    }
    else {
        id = freeIDs.back();
        freeIDs.pop_back();
    }
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