#include <basilisk/resource/textureServer.h>

namespace bsk::internal {

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

TextureServer::~TextureServer() {
    for (TextureArray* array : textureArrays) {
        delete array;
    }
}

/**
 * @brief Clamps a given size x to the nearest bucket size
 * 
 * @param x Value to clamp
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
 * @param image Pointer to the image to add to the server
 * @return std::pair<unsigned int, unsigned int> 
 */
std::pair<unsigned int, unsigned int> TextureServer::add(Image* image) {

    // Return default image if nullptr is passed
    if (image == nullptr) {
        return {0, 0};
    }

    // Do not double add image, simply return existing mapping
    if (imageMapping.count(image)) {
        return get(image);
    }

    // Get the buckest size to add this image to. Will likely resize the image. 
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

/**
 * @brief Get the mapping of the image as a pair <index of the array, index in the array>
 * 
 * @param image The image to get
 * @return std::pair<unsigned int, unsigned int> 
 */
std::pair<unsigned int, unsigned int> TextureServer::get(Image* image) { 
    // Return default image if nullptr is passed or the image has not been added.
    if (image == nullptr || imageMapping.count(image) <= 0) {
        return {0, 0};
    }
    // Return mapping
    return imageMapping.at(image); 
}

/**
 * @brief Write all of the texture arrays in this server to a given shader
 * 
 * @param shader The shader to write to
 * @param name The name of the uniform array. For example, if you have `textureArrays[NUM_ARRAYS]` in your shader, use "textureArrays" for the name.
 * @param startSlot The first GPU slot to use. Each bucket will use a slot. Slots used contiguously. 
 */
void TextureServer::write(Shader* shader, std::string name, unsigned int startSlot) {
    unsigned int slot = 0;
    for (TextureArray* array : textureArrays) {
        shader->bind((name + "[" + std::to_string(slot) + "].array").c_str(), array, startSlot + slot);
        slot++;
    }
}

}