#include "vbo.h"

VBO::VBO(const void* data, unsigned int size): size(size) {
    glGenBuffers(1, &ID);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    unbind();
}

VBO::~VBO() { glDeleteBuffers(1, &ID); }
void VBO::bind() { glBindBuffer(GL_ARRAY_BUFFER, ID); }
void VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }