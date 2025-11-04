#ifndef BSK_TEXTURE_SERVER_H
#define BSK_TEXTURE_SERVER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/image.h>
#include <basilisk/render/textureArray.h>
#include <basilisk/render/shader.h>

namespace bsk::internal {

class TextureServer {
    private:
        std::vector<unsigned int> sizeBuckets;
        std::unordered_map<Image*, std::pair<unsigned int, unsigned int>> imageMapping;
        std::vector<TextureArray*> textureArrays;

        unsigned int getClosestSize(unsigned int x);

    public:
        TextureServer(std::vector<unsigned int> sizeBuckets = {256, 512, 1024, 2048});
        ~TextureServer();

        std::pair<unsigned int, unsigned int> add(Image* image);
        std::pair<unsigned int, unsigned int> get(Image* image);

        std::vector<TextureArray*>& getArrays() { return textureArrays; }

        void write(Shader* shader, std::string name, unsigned int startSlot = 0);
};

}

#endif