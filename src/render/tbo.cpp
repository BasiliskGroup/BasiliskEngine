#include <basilisk/render/tbo.h>

namespace bsk::internal {

/**
 * @brief Construct a new TBO object. Can be used to store arbitray data on the GPU
 * 
 * @param data Inital data to store
 * @param size Size of the data in bytes
 */
TBO::TBO(const void* data, unsigned int size, unsigned int reserve): size(size) { 
    capacity = glm::max(size, reserve);

    // Create one buffer, and update VBO with the buffer ID
    glGenBuffers(1, &ID);
    // Bind the vbo to start working on it
    glBindBuffer(GL_TEXTURE_BUFFER, ID);
    // Allocate based on capacity
    glBufferData(GL_TEXTURE_BUFFER, capacity, nullptr, GL_DYNAMIC_DRAW);
    // Upload the given data
    if (data && size) {
        glBufferSubData(GL_TEXTURE_BUFFER, 0, size, data);
    }
    // Unbind the buffer for safety
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    // Create a texture to sample data
    glGenTextures(1, &textureID);
    // Bind to the TBO
    glBindTexture(GL_TEXTURE_BUFFER, textureID);
    // Specify the format for texel samples
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, ID);
    // Unbind texture for safety
    glBindTexture(GL_TEXTURE_BUFFER, 0);
}

/**
 * @brief Destroy the TBO object. Releases buffer and texture. 
 * 
 */
TBO::~TBO() {
    glDeleteTextures(1, &textureID);
    glDeleteBuffers(1, &ID);
}

/**
 * @brief Binds this TBO for use
 * 
 */
void TBO::bind() {
    glBindBuffer(GL_TEXTURE_BUFFER, ID);
}

/**
 * @brief Unbinds this TBO
 * 
 */
void TBO::unbind() {
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

/**
 * @brief Writes an array of data to an existing allocation of the TBO.
 * 
 * @param data A pointer to the array of data to be writen
 * @param size The size of the data in bytes
 * @param offset The location in bytes to start writing
 */
void TBO::write(const void* data, unsigned int size, unsigned int offset) {
    // Update the capacity if needed
    if (size + offset > capacity) {
        resize();
    }
    // Update size
    this->size = glm::max(this->size, size + offset);
    // Bind the vbo to start working on it
    glBindBuffer(GL_TEXTURE_BUFFER, ID);
    // Write the data 
    glBufferSubData(GL_TEXTURE_BUFFER, offset, size, data);
    // Unbind for safety
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void TBO::resize() {
    // Create a new larger buffer
    unsigned int newCapacity = glm::max(capacity * 2, 1u);
    GLuint newBuffer;
    glGenBuffers(1, &newBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, newBuffer);
    glBufferData(GL_TEXTURE_BUFFER, newCapacity, nullptr, GL_DYNAMIC_DRAW);

    // Copy existing data to the new buffer
    glBindBuffer(GL_COPY_READ_BUFFER, ID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, newBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

    // Delete the old buffer
    glDeleteBuffers(1, &ID);

    // Replace IDs
    ID = newBuffer;
    capacity = newCapacity;

    // Rebind texture to use the new buffer
    glBindTexture(GL_TEXTURE_BUFFER, textureID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, ID);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    // Unbind for safety
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

}