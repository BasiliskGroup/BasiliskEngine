#define STB_IMAGE_IMPLEMENTATION
#include <basilisk/render/image.h>
#include <basilisk/util/resolvePath.h>

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
 * 
 @brief Destroy the Image object and free image data
 * 
 */
Image::~Image() {
    stbi_image_free(data);
}

}