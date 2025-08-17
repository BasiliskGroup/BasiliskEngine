#include "buffer.h"

Buffer::Buffer(const void* data, unsigned int size, unsigned int bufferType, bool dynamic): size(size), bufferType(bufferType) {
    glGenBuffers(1, &ID);
    bind();
    unsigned int drawType = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(bufferType, size, data, drawType);
    unbind();
}

void Buffer::write(const void* data, unsigned int size, unsigned int offset) {
    if (size + offset > this->size) {
        throw std::runtime_error("Buffer: Data out of buffer bounds");
    }

    bind();
    glBufferSubData(bufferType, offset, size, data);
    unbind();
}

Buffer::~Buffer() { glDeleteBuffers(1, &ID); }