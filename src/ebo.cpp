#include "ebo.h"

EBO::EBO(const void* data, unsigned int size): size(size) {
    glGenBuffers(1, &ID);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    unbind();
}

EBO::~EBO() { glDeleteBuffers(1, &ID); }
void EBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }
void EBO::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }