#ifndef BSK_TEXTURE_H
#define BSK_TEXTURE_H

#include <basilisk/render/image.h>

namespace bsk::internal {

class Texture {
    private:
        unsigned int ID;
        unsigned int width;
        unsigned int height;

        Image* image;

    public:
        Texture(const void* data, unsigned int width, unsigned int height, unsigned int format=GL_RGBA, unsigned int type=GL_UNSIGNED_BYTE);
        template<typename T>
        Texture(const std::vector<T>& data, unsigned int width, unsigned int height, unsigned int format=GL_RGBA, unsigned int type=GL_UNSIGNED_BYTE): Texture(data.data(), width, height, format, type) {}
        Texture(Image* image): Texture(image->getData(), image->getWidth(), image->getHeight()) {};
        ~Texture();

        void bind();        
        void setFilter(unsigned int magFilter, unsigned int minFilter);
        void setWrap(unsigned int wrap);

        void write(const void* data, unsigned int width=0, unsigned int height=0, unsigned int xOffset=0, unsigned int yOffset=0);
        template<typename T>
        void write(const std::vector<T>& data, unsigned int width=0, unsigned int height=0, unsigned int xOffset=0, unsigned int yOffset=0) {
            write(data.data(), width, height, xOffset, yOffset);
        }

        inline Image* getImage() { return image; }
        inline unsigned int getID() { return ID; }
        inline unsigned int getWidth() { return width; }
        inline unsigned int getHeight() { return height; }
};

}

#endif