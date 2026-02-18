#define STB_IMAGE_IMPLEMENTATION
#include <basilisk/render/image.h>
#include <basilisk/util/resolvePath.h>
#include <cstdlib>

namespace bsk::internal {

/**
 * @brief Construct a new Image object from image file
 * 
 * @param file Path to the image to load
 */
Image::Image(std::string file, bool flip_vertically) {
    stbi_set_flip_vertically_on_load(flip_vertically);  
    std::string resolvedPath = externalPath(file);
    data = stbi_load(resolvedPath.c_str(), &width, &height, &nChannels, 4);
    if (!data) {
        std::cout << "Failed to load texture from path: " << resolvedPath << std::endl;
    }
}

/**
 * @brief Construct a new Image object from float vector data
 *        Converts float values (0.0-1.0) to unsigned char (0-255)
 * 
 * @param data Vector of float values in range [0.0, 1.0]
 * @param width Image width
 * @param height Image height
 * @param nChannels Number of channels (default 4 for RGBA)
 */
Image::Image(const std::vector<float>& data, int width, int height, int nChannels) {
    this->width = width;
    this->height = height;
    this->nChannels = nChannels;
    
    // Allocate memory (compatible with stbi_image_free)
    size_t dataSize = width * height * nChannels;
    this->data = (unsigned char*)std::malloc(dataSize);
    if (!this->data) {
        std::cout << "Failed to allocate memory for image data" << std::endl;
        return;
    }
    
    // Convert floats (0.0-1.0) to unsigned char (0-255)
    for (size_t i = 0; i < std::min(data.size(), dataSize); ++i) {
        float value = std::max(0.0f, std::min(1.0f, data[i]));
        this->data[i] = static_cast<unsigned char>(value * 255.0f);
    }
}

/**
 * 
 @brief Destroy the Image object and free image data
 * 
 */
Image::~Image() {
    stbi_image_free(data);
}

}