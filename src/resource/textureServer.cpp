#include "resource/textureServer.h"


TextureServer::TextureServer(std::vector<unsigned int> sizeBuckets): sizeBuckets(sizeBuckets) {
    for (unsigned int size : sizeBuckets) {
        TextureArray* array = new TextureArray(size, size);
        textureArrays.push_back(array);
    }
}


unsigned int TextureServer::getClosestSize(unsigned int x) {
    return *std::min_element(sizeBuckets.begin(), sizeBuckets.end(),
        [x](unsigned int a, unsigned int b) { 
            return std::abs((int)a - (int)x) < std::abs((int)b - (int)x);
        }
    );
}


std::pair<unsigned int, unsigned int> TextureServer::add(Image* image) {
    unsigned int size = getClosestSize(image->getWidth());

    // Get the index of the closest bucket size
    auto it = std::find(sizeBuckets.begin(), sizeBuckets.end(), size);
    int arrayIndex = (it != sizeBuckets.end()) ? std::distance(sizeBuckets.begin(), it) : -1;

    // Add the image to the closest array
    TextureArray* array = textureArrays.at(arrayIndex);
    unsigned int imageIndex = array->add(image);

    // Update the pointer mapping
    std::pair<unsigned int, unsigned int> location(arrayIndex, imageIndex);
    imageMapping.at(image) = location;

    return location;
}