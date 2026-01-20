#include <basilisk/render/ubo.h>

namespace bsk::internal {

/**
 * @brief Construct a new UBO from the data provided at the given pointer
 * 
 * @param data A pointer to the data to populate the UBO.
 * @param size The size of the data provided in bytes.
 * @param drawType Specifies how OpenGL should store/write to buffer data.
 *                 GL_STATIC_DRAW by defult. 
 *                 May also be GL_DYNAMIC_DRAW or GL_STREAM_DRAW.
 */
UBO::UBO(const void* data, unsigned int size, unsigned int drawType): size(size) {
    // Create one buffer, and update ubo with the buffer ID
    glGenBuffers(1, &ID);
    // Bind the ubo to start working on it
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    // Now, we can add our vertex data to the UBO
    glBufferData(GL_UNIFORM_BUFFER, size, data, drawType);
    // Unbind the buffer for safety
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/**
 * @brief Destroy the UBO object
 * 
 */
UBO::~UBO() {
    glDeleteBuffers(1, &ID);
}

/**
 * @brief Binds this UBO for use (does not bind to shdader, use shader->bind(UBO) for this)
 * 
 */
void UBO::bind() {
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
}

/**
 * @brief Unbinds this UBO
 * 
 */
void UBO::unbind() {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/**
 * @brief Writes an array of data to an existing allocation of the UBO.
 * 
 * @param data A pointer to the array of data to be writen
 * @param size The size of the data in bytes
 * @param offset The location in bytes to start writing
 */
void UBO::write(const void* data, unsigned int size, unsigned int offset) {
    // Bind the vbo to start working on it
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    // Write the data 
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    // Unbind for safety
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/**
 * @brief Writes a vector of data to an existing allocation of the UBO.
 * 
 * @param data A reference to the vecotre of data to be writen
 * @param offset The location in bytes to start writing
 */
template<typename T>
void UBO::write(const std::vector<T>& data, unsigned int offset) {
    write(data.data(), data.size() * sizeof(T), offset);
}

// Explicit instantiations for common types used in uniform buffers
template void UBO::write<glm::vec4>(const std::vector<glm::vec4>& data, unsigned int offset);

}