#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <basilisk/render/textureArray.h>

namespace bsk::internal {

/**
 * @brief Construct a new Texture Array object
 * 
 * @param width The desired width of the images in the texture array. Will resize images if needed.
 * @param height The desired height of the images in the texture array. Will resize images if needed.
 * @param images A vector of image pointers
 * @param capacity The inital capacity of the array for preallocation. Will use size of images if not given.
 */
TextureArray::TextureArray(unsigned int width, unsigned int height, std::vector<Image*> images, unsigned int capacity): width(width), height(height), images(images) {
    this->capacity = glm::max((unsigned int)images.size(), capacity);
    glGenTextures(1, &id);
    bind();
    generate();
    unbind();
}

/**
 * @brief Destroy the Texture Array object. Releases gl texture array. Leaves images intact.
 * 
 */
TextureArray::~TextureArray() {
    glDeleteTextures(1, &id);
}

/**
 * @brief Generates the array and uploads all the image data from the image vector to this texture array
 * WARNING: This recreates the texture array. Must only be called when texture is not in use by GPU.
 * 
 */
void TextureArray::generate() {
    // CRITICAL: glTexImage3D with new size invalidates the texture
    // This will break any active rendering using this texture array
    // Only safe to call between render frames when texture is not bound to shaders
    
    // Set the filter to linear by default
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Set up the array with new capacity
    // WARNING: This invalidates the texture - any active shader using it will crash
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, capacity, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Upload the data for each image
    for (unsigned int i = 0; i < images.size(); i++) {
        uploadImage(images.at(i), i); // This now assumes the texture is already bound
    }
}

/**
 * @brief Uploads an image to the texture array. Assumes the given position slot is allocated
 * 
 */
void TextureArray::uploadImage(Image* image, unsigned int position) {

    // Resize the image
    unsigned char* data = new unsigned char[width * height * 4];
    stbir_resize_uint8_linear(image->getData(), image->getWidth(), image->getHeight(), 0, data, width, height, 0, STBIR_RGBA);

    // Add the resized image data to the texture array
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, position, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Free the resized image data
    delete[] data;
}

/**
 * @brief Adds a new image to the texture array.
 * WARNING: If array needs to resize, this will recreate the texture which breaks active rendering.
 * Must only be called between render frames when texture is not in use.
 * 
 * @param image Pointer to the image to add.
 * 
 * @return unsigned int of the location of the image in the array. 
 */
unsigned int TextureArray::add(Image* image) {
    if (!image) return 0;
    
    bind();
    
    images.push_back(image);
    
    if (images.size() > capacity) {
        // CRITICAL: Resizing the texture array recreates it completely
        // This will break any active rendering using this texture array
        // This should only happen between render frames
        capacity *= 2;
        generate();  // Recreates entire texture - breaks active rendering!
    }
    else {
        // Safe: Just upload new image to existing slot
        uploadImage(image, images.size() - 1);
    }

    unbind();

    return images.size() - 1;
}

/**
 * @brief Binds this texture array for use.
 * 
 */
void TextureArray::bind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

/**
 * @brief Unbinds this texture array (binds 0)
 * 
 */
void TextureArray::unbind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

/**
 * @brief Sets the filter of all the images in the texture array
 * 
 * @param magFilter GL_LINEAR
 * @param minFilter GL_LINEAR
 */
void TextureArray::setFilter(unsigned int magFilter, unsigned int minFilter) {
    // Set filter
    bind();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
    unbind();
}

/**
 * @brief Sets the wrap method for all images in the texture array
 * 
 * @param wrap GL_REPEAT 
 */
void TextureArray::setWrap(unsigned int wrap) {
    // Set wrap
    bind();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);	
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
    unbind();
}

}