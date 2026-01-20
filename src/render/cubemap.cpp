#include <basilisk/render/cubemap.h>

namespace bsk::internal {

/**
 * @brief Construct a new Cubemap object
 * 
 * @param faces A vector of Image pointers representing the faces of the cubemap
 */
Cubemap::Cubemap(const std::vector<Image*>& faces) {
    // Generate a new texture
    glGenTextures(1, &ID);
    // Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    // Apply the images to the texture
    applyImages(faces);
    // Unbind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

/**
 * @brief Construct a new Cubemap object from a vector of file paths
 * 
 * @param faces A vector of file paths representing the faces of the cubemap
 */
Cubemap::Cubemap(const std::vector<std::string>& faces) {
    // Load the images from the file paths
    std::vector<Image*> images;
    for (const auto& face : faces) {
        images.push_back(new Image(face, false));
    }
    
    // Generate a new texture
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    applyImages(images);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    // Delete the temporary images
    for (const auto& image : images) {
        delete image;
    }
}

/**
 * @brief Applies the images to the cubemap
 * 
 * @param faces A vector of Image pointers representing the faces of the cubemap
 */
void Cubemap::applyImages(const std::vector<Image*>& faces) {
    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // Upload the faces to the texture
    for (unsigned int i = 0; i < faces.size(); i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, faces[i]->getWidth(), faces[i]->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, faces[i]->getData());
    }
}

/**
 * @brief Destroy the Cubemap object, release GL data
 * 
 */
Cubemap::~Cubemap() {
    glDeleteTextures(1, &ID);
}

/**
 * @brief Binds the cubemap for use
 * 
 */
void Cubemap::bind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

/**
 * @brief Unbinds the cubemap
 * 
 */
void Cubemap::unbind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

}