#include "node.h"


Node::Node(Shader* shader, Mesh* mesh, Texture* texture) : shader(shader), mesh(mesh), texture(texture) {
    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);

    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    scale    = glm::vec3(1.0f, 1.0f, 1.0f);
}


Node::~Node() {
    delete vbo;
    delete ebo;
}


void Node::update() {
    modelMatrix.makeIdentity();
    modelMatrix.translate(position);
    modelMatrix.rotate(glm::vec3(1.0, 0.0, 0.0), rotation.x);
    modelMatrix.rotate(glm::vec3(0.0, 1.0, 0.0), rotation.y);
    modelMatrix.rotate(glm::vec3(0.0, 0.0, 1.0), rotation.z);
    modelMatrix.scale(scale);
}


void Node::render() {
    shader->bind("uTexture", texture, 0);
    shader->setUniform("model", modelMatrix.getMatrix());
    vao->render();
}