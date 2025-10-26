#ifndef TEXTURE_SERVER_H
#define TEXTURE_SERVER_H

#include "util/includes.h"
#include "render/image.h"
#include "render/textureArray.h"

class TextureServer {
    private:
        std::vector<unsigned int> sizeBuckets;
        std::unordered_map<Image*, std::pair<unsigned int, unsigned int>> imageMapping;
        std::vector<TextureArray*> textureArrays;

        unsigned int getClosestSize(unsigned int x);

    public:
        TextureServer(std::vector<unsigned int> sizeBuckets = {256, 512, 1024, 2048});

        std::pair<unsigned int, unsigned int> add(Image* image);
        std::pair<unsigned int, unsigned int> get(Image* image);
};

#endif