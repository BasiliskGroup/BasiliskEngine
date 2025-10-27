#include "render/tbo.h"

TBO::TBO(const void* data, unsigned int size) {
    // Create one buffer, and update VBO with the buffer ID
    glGenBuffers(1, &ID);
    // Bind the vbo to start working on it
    glBindBuffer(GL_TEXTURE_BUFFER, ID);
    // Now, we can add our vertex data to the VBO
    glBufferData(GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW);
    // Unbind the buffer for safety
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    // Create a texture to sample data
    glGenTextures(1, &textureID);
    // Bind to the TBO
    glBindTexture(GL_TEXTURE_BUFFER, textureID);
    // Specify the format for texel samples
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, ID);
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
    // Bind the vbo to start working on it
    glBindBuffer(GL_TEXTURE_BUFFER, ID);
    // Write the data 
    glBufferSubData(GL_TEXTURE_BUFFER, offset, size, data);
    // Unbind for safety
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}