#include "resource/textureServer.h"

/**
 * @brief Construct a new Texture Server object to maintin multiple texture arrays
 * 
 * @param sizeBuckets Vector of sizes for the texture arrays. All images will be clamped to these values. 
 */
TextureServer::TextureServer(std::vector<unsigned int> sizeBuckets): sizeBuckets(sizeBuckets) {
    for (unsigned int size : sizeBuckets) {
        TextureArray* array = new TextureArray(size, size);
        textureArrays.push_back(array);
    }
}

/**
 * @brief Clamps a given size x to the nearest bucket size
 * 
 * @param x 
 * @return unsigned int 
 */
unsigned int TextureServer::getClosestSize(unsigned int x) {
    return *std::min_element(sizeBuckets.begin(), sizeBuckets.end(),
        [x](unsigned int a, unsigned int b) { 
            return std::abs((int)a - (int)x) < std::abs((int)b - (int)x);
        }
    );
}

/**
 * @brief Adds an image to the array and returns it mapping as a pair <index of the array, index in the array>
 * 
 * @param image 
 * @return std::pair<unsigned int, unsigned int> 
 */
std::pair<unsigned int, unsigned int> TextureServer::add(Image* image) {

    if (imageMapping.count(image)) {
        return get(image);
    }

    unsigned int size = getClosestSize(image->getWidth());

    // Get the index of the closest bucket size
    auto it = std::find(sizeBuckets.begin(), sizeBuckets.end(), size);
    int arrayIndex = (it != sizeBuckets.end()) ? std::distance(sizeBuckets.begin(), it) : -1;

    // Add the image to the closest array
    TextureArray* array = textureArrays.at(arrayIndex);
    unsigned int imageIndex = array->add(image);

    // Update the pointer mapping
    std::pair<unsigned int, unsigned int> location(arrayIndex, imageIndex);
    imageMapping[image] = location;

    return location;
}

void TextureServer::write(Shader* shader, std::string name, unsigned int startSlot) {
    unsigned int slot = 0;
    for (TextureArray* array : textureArrays) {
        shader->bind((name + "[" + std::to_string(slot) + "]").c_str(), array, startSlot + slot);
        slot++;
    }
}