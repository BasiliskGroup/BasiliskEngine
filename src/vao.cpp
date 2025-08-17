#include "vao.h"


void VAO::setupVAO(const std::vector<Attribute>& attributes) {
    glGenVertexArrays(1, &ID);
    bind();
    vbo->bind();

    if (ebo) { ebo->bind(); }
    
    bindAttributes(attributes);
    unbind();
}

VAO::VAO(Shader* shader, VBO* vertices): shader(shader), vbo(vertices), ebo(nullptr) {
    std::vector<Attribute> attributes = getAttributes(shader);
    setupVAO(attributes);
    vertexCount = vbo->getSize() / getStride(attributes);
}

VAO::VAO(Shader* shader, VBO* vertices, EBO* indices): shader(shader), vbo(vertices), ebo(indices) {
    std::vector<Attribute> attributes = getAttributes(shader);
    setupVAO(attributes);
    vertexCount = ebo->getSize() / sizeof(unsigned int);
}

VAO::~VAO() { glDeleteVertexArrays(1, &ID); }


void VAO::bindAttribute(GLint location, GLint count, unsigned int stride, unsigned int offset, unsigned int dataType, unsigned int divisor) {    
    glVertexAttribPointer(location, count, dataType, GL_FALSE, stride, (const void*)(GLintptr)offset);
    glEnableVertexAttribArray(location);
    if (divisor) { glVertexAttribDivisor(location, divisor); }

}

void VAO::bindAttributes(const std::vector<Attribute>& attributes) {
    unsigned int stride = getStride(attributes);
    unsigned int offset = 0;
    for (const Attribute& attribute : attributes) {    
        bindAttribute(attribute.location, attribute.count, stride, offset);
        offset += attribute.size;
    }
}


void VAO::render(GLenum drawMode) {
    shader->use();
    bind();
    ebo ? glDrawElements(drawMode, vertexCount, GL_UNSIGNED_INT, 0) : glDrawArrays(drawMode, 0, vertexCount);
    unbind();
}