#include "buffer.h"

Buffer::Buffer(const void* data, unsigned int size, unsigned int bufferType, bool dynamic): size(size), bufferType(bufferType) {
    glGenBuffers(1, &ID);
    bind();
    unsigned int drawType = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(bufferType, size, data, drawType);
    unbind();
}

void Buffer::resize() {
    unsigned int newSize = size * 2;
    unsigned int newID;

    glGenBuffers(1, &newID);
    glBindBuffer(bufferType, newID);
    glBufferData(bufferType, newSize, NULL, GL_DYNAMIC_DRAW);

    // Copy data from old VBO to new VBO
    glBindBuffer(GL_COPY_READ_BUFFER, ID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, newID);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

    // Clean up old VBO
    glDeleteBuffers(1, &ID);

    ID = newID;
    size = newSize;

    if (bufferType == GL_TEXTURE_BUFFER) {
        bind();
        glBindTexture(GL_TEXTURE_BUFFER, textureID);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, ID);
        unbind();
    }
}

Buffer::~Buffer() { glDeleteBuffers(1, &ID); }

void Buffer::write(const void* data, unsigned int size, unsigned int offset) {
    while (size + offset > this->size) { resize(); }

    bind();
    glBufferSubData(bufferType, offset, size, data);
    unbind();
}


TBO::TBO(const void* data, unsigned int size, bool dynamic): Buffer(data, size, GL_TEXTURE_BUFFER, dynamic)  {
    createTexture();
}

void TBO::createTexture() {
    bind();
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_BUFFER, textureID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, ID);
    unbind();
}